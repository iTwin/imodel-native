/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspsurf.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|  Include Files                                                        |
|                                                                       |
+----------------------------------------------------------------------*/
#define __NORECTANGLE__     /* need this when we include windows.h */


/* tools subsystem */

/*----------------------------------------------------------------------+
|                                                                       |
| Local defines                                                         |
|                                                                       |
+----------------------------------------------------------------------*/
#define MAX_BATCH               10
#define BOUNDARY_JUMP_TOLERANCE 0.90

/*----------------------------------------------------------------------+
|                                                                       |
|   Function declarations                                               |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|    Surface Utility Routines                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_freeSurface                                     |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_freeSurface
(
MSBsplineSurface *surface
)
    {
    if (surface->poles)
        {
        BSIBaseGeom::Free (surface->poles);
        surface->poles = NULL;
        }

    if (surface->uKnots)
        {
        BSIBaseGeom::Free (surface->uKnots);
        surface->uKnots = NULL;
        }

    if (surface->vKnots)
        {
        BSIBaseGeom::Free (surface->vKnots);
        surface->vKnots = NULL;
        }

    if (surface->rational && surface->weights)
        {
        BSIBaseGeom::Free (surface->weights);
        surface->weights = NULL;
        }

    bspsurf_freeBoundaries (surface);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_allocateSurface                                 |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_allocateSurface
(
MSBsplineSurface    *surface
)
    {
    int         totalPoles, allocSize;

    /* Initiailize all pointers to zero */
    surface->poles = NULL;
    surface->weights = surface->uKnots = surface->vKnots = NULL;
    surface->boundaries = NULL;

    totalPoles = surface->uParams.numPoles * surface->vParams.numPoles;
    if (totalPoles <= 0)
        return ERROR;

    /* Allocate poles memory */
    allocSize = totalPoles * sizeof (DPoint3d);
    if (NULL == (surface->poles = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
        return ERROR;

    /* Allocate U Knot Vector */
    allocSize = bspknot_numberKnots (surface->uParams.numPoles, surface->uParams.order,
                                     surface->uParams.closed) * sizeof(double);

    if (NULL == (surface->uKnots = (double*)BSIBaseGeom::Malloc (allocSize)))
        {
        bspsurf_freeSurface (surface);
        return ERROR;
        }

    /* Allocate V Knot Vector */
    allocSize = bspknot_numberKnots (surface->vParams.numPoles, surface->vParams.order,
                                     surface->vParams.closed) * sizeof(double);

    if (NULL == (surface->vKnots = (double*)BSIBaseGeom::Malloc (allocSize)))
        {
        bspsurf_freeSurface (surface);
        return ERROR;
        }

    if (surface->rational)
        {
        allocSize = totalPoles * sizeof (double);
        if (NULL == (surface->weights = (double*)BSIBaseGeom::Malloc (allocSize)))
            {
            bspsurf_freeSurface (surface);
            return ERROR;
            }
        }

    if (surface->numBounds)
        {
        if (NULL == (surface->boundaries = (BsurfBoundary*)BSIBaseGeom::Calloc (surface->numBounds, sizeof (BsurfBoundary))))
            {
            bspsurf_freeSurface (surface);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_copyBoundaries                                  |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_copyBoundaries
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    bspsurf_freeBoundaries (out);
    out->numBounds = 0;    
    out->holeOrigin = in->holeOrigin;
    return bspsurf_appendCopyBoundaries (out, in, NULL);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_copySurface                                     |
|                                                                       |
| author        BSI                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_copySurface
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input
)
    {
    int             status, numPoles, numKnots;

    if (output == input)
        return SUCCESS;
    *output = *input;

    if (SUCCESS != (status = bspsurf_allocateSurface (output)))
        return status;

    numPoles = output->uParams.numPoles * output->vParams.numPoles;

    memcpy (output->poles, input->poles, numPoles * sizeof(DPoint3d));

    if (output->rational)
        memcpy (output->weights, input->weights, numPoles * sizeof(double));
    else
        output->weights = NULL;

    numKnots = bspknot_numberKnots (output->uParams.numPoles, output->uParams.order,
                                    output->uParams.closed);

    if (input->uKnots)
        memcpy (output->uKnots, input->uKnots, numKnots * sizeof (double));
    else
        bspknot_computeKnotVector (output->uKnots, &output->uParams, NULL);


    numKnots = bspknot_numberKnots (output->vParams.numPoles, output->vParams.order,
                                    output->vParams.closed);
    if (input->vKnots)
        memcpy (output->vKnots, input->vKnots, numKnots * sizeof (double));
    else
        bspknot_computeKnotVector (output->vKnots, &output->vParams, NULL);


    if (output->numBounds)
        bspsurf_copyBoundaries (output, input);

    return SUCCESS;
    }

#ifdef CompileComputeBoundarySpans
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_computeBoundarySpans                            |
|                                                                       |
| author        BSI                                     9/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_computeBoundarySpans
(
double                     **spans,
double                     value,
const MSBsplineSurface    *bspline,
int                        horizontal
)
    {
    EmbeddedDoubleArray *pFractionArray = jmdlEmbeddedDoubleArray_grab ();
    bspsurf_intersectBoundariesWithUVLine (pFractionArray, value, bspline, horizontal);
    size_t numIntersect = pFractionArray->size();
    size_t numBytes     = numIntersect * sizeof (double);
    if (NULL == (*spans = (double*)BSIBaseGeom::Malloc (numBytes)))
        numIntersect = 0;
    if (numIntersect > 0)
        memcpy (*spans, jmdlEmbeddedDoubleArray_getPtr (pFractionArray, 0), numBytes);
    jmdlEmbeddedDoubleArray_drop (pFractionArray);
    return (int)numIntersect;
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Surface Compatiblity Routines                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_matchCurveParams                                |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_matchCurveParams
(
MSBsplineSurface    *surface,
MSBsplineCurveCP    curve,
int                 direction
)
    {
    StatusInt status = SUCCESS;
    if ((curve->params.order >
        (direction == BSSURF_U ? surface->uParams.order : surface->vParams.order)))
        {
        if (SUCCESS != (status = bsprsurf_elevateDegreeSurface (surface, surface,
                                                             curve->params.order - 1,
                                                             direction)))
            return status;
        }
    double knotTol = bspknot_knotTolerance (curve);
    KnotData knotData;
    knotData.LoadCurveKnots (*curve);
    double value;
    size_t multiplicity;
    for (size_t activeIndex = 0;
         knotData.GetKnotByActiveIndex (activeIndex, value, multiplicity);
         activeIndex++)
        {
        if (SUCCESS != (status = bspknot_addKnotSurface (surface, value, knotTol, (int)multiplicity, false,
                                             direction)))
            return status;
        }

    if (curve->rational)
        status = bspsurf_makeRationalSurface (surface, surface);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_make2SurfacesCompatible                         |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_make2SurfacesCompatible
(
MSBsplineSurface    *surface0,
MSBsplineSurface    *surface1,
int                 direction
)
    {
    int                 status;
    MSBsplineCurve      crv0, crv1;
    MSBsplineSurface    srf0, srf1;

    memset (&crv0, 0, sizeof(crv0));    memset (&crv1, 0, sizeof(crv1));
    memset (&srf0, 0, sizeof(srf0));    memset (&srf1, 0, sizeof(srf1));

    if (bspsurf_copySurface (&srf0, surface0) ||
        bspsurf_copySurface (&srf1, surface1) ||
        bspconv_getCurveFromSurface (&crv0, &srf0, direction, -1) ||
        bspconv_getCurveFromSurface (&crv1, &srf1, direction, 0))
        {
        status = ERROR;
        goto wrapup;
        }

    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&crv0, &crv1)))
        goto wrapup;

    if ((SUCCESS != (status = bspsurf_matchCurveParams (&srf0, &crv0, direction))) ||
        (SUCCESS != (status = bspsurf_matchCurveParams (&srf1, &crv1, direction))))
        goto wrapup;

    bspsurf_freeSurface (surface0);
    bspsurf_freeSurface (surface1);
    *surface0 = srf0;
    *surface1 = srf1;

wrapup:
    if (status)
        {
        bspsurf_freeSurface (&srf0);
        bspsurf_freeSurface (&srf1);
        }
    bspcurv_freeCurve (&crv0);
    bspcurv_freeCurve (&crv1);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Surface Routines - acting on single surface                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_isDegenrateEdge                                 |
|                                                                       |
| author        BrianPeters                             6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bspsurf_isDegenerateEdge
(
int                 edgeCode,
MSBsplineSurfaceCP   surf,
double              tolerance
)
    {
    bool            code;
    DPoint3d        *pP, *endP;
    MSBsplineCurve  curve;

    bspconv_getEdgeFromSurface (&curve, surf, edgeCode);
    if (curve.rational)
        bsputil_unWeightPoles (curve.poles, curve.poles, curve.weights,
                               curve.params.numPoles);

    for (pP = endP = curve.poles, endP += curve.params.numPoles, pP += 1, code = true;
         code && pP < endP; pP++)
        {
        if (pP->Distance (*(curve.poles)) > tolerance)
            code = false;
        }

    bspcurv_freeCurve (&curve);
    return code;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_isSolid                                         |
|                                                                       |
| author        BrianPeters                             6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool      bspsurf_isSolid
(
MSBsplineSurface    *surf,
double              tolerance
)
    {
    if (surf->numBounds == 0)
        {
        if (surf->uParams.closed)
            {
            if (surf->vParams.closed)
                return true;
            else
                return (bspsurf_isDegenerateEdge (V0_EDGE, surf, tolerance) &&
                        bspsurf_isDegenerateEdge (V1_EDGE, surf, tolerance));
            }
        else
            {
            if (surf->vParams.closed)
                return (bspsurf_isDegenerateEdge (U0_EDGE, surf, tolerance) &&
                        bspsurf_isDegenerateEdge (U1_EDGE, surf, tolerance));
            else
                return false;
            }
        }
    else
        {
        return false;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_isPhysicallyClosed                              |
|                                                                       |
| author        BrianPeters                             11/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_isPhysicallyClosed
(
bool                *uClosed,
bool                *vClosed,
MSBsplineSurface    *surf
)
    {
    int         totalPoles, uPoles;
    DPoint3d    *pt0, *pt1, *endP;

    uPoles = surf->uParams.numPoles;
    totalPoles = surf->uParams.numPoles * surf->vParams.numPoles;
    double tolerance = bsputil_surfaceTolerance (surf);
    if (surf->rational)
        bsputil_unWeightPoles (surf->poles, surf->poles, surf->weights, totalPoles);

    if (surf->vParams.closed)
        *vClosed = true;
    else
        {
        for (*vClosed = true,
             pt0 = endP = surf->poles,
             pt1 = surf->poles + uPoles * (surf->vParams.numPoles - 1),
             endP += uPoles;
             *vClosed && (pt0 < endP);
             pt0++, pt1++)
            *vClosed = bsputil_isSamePointTolerance (pt0, pt1, tolerance);
        }

    if (surf->uParams.closed)
        *uClosed = true;
    else
        {
        for (*uClosed = true,
             pt0 = endP = surf->poles,
             pt1 = surf->poles + uPoles - 1,
             endP += totalPoles;
             *uClosed && (pt0 < endP);
             pt0 += uPoles, pt1 += uPoles)
            *uClosed = bsputil_isSamePointTolerance (pt0, pt1, tolerance);
        }

    if (surf->rational)
        bsputil_weightPoles (surf->poles, surf->poles, surf->weights, totalPoles);
    }
// What is this used for.
// What it does  ..
// along one edge (u=0, u=1, v=0, or v=1) compute each quad.  Compute vectors "along the edge itself" and "along the first inboard polygon line.
// Sum the cross products of the vectors, divide by number of quads.
//  This is zero if (a) the edge is degenerate, (b) the edge and inbound polygon line are always parallel.
// So it is a curviness measure for the boundary quads.
// called by _computePartials if the normal vanishes.
// this is exported only for testing ...
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_computeZeroSurfNorm                             |
|                                                                       |
| author        LuHan                                   2/93            |
|                                                                       |
+----------------------------------------------------------------------*/
GEOMDLLIMPEXP void    bspsurf_computeZeroSurfNorm
(
DPoint3d            *normP,
const MSBsplineSurface    *surfP,
double              param,
int                 direction
)
    {
    int         numU, numV;
    DPoint3d    pole11, pole12, pole21, pole22, diff1, diff2, sum, cross;
    sum.x = sum.y = sum.z = 0.0;
    numU = surfP->uParams.numPoles;
    numV = surfP->vParams.numPoles;
    if (direction == BSSURF_U)
        {
        size_t j0 = 0;
        size_t j1 = 1;
        double scale = 1.0 / numU;
        if (param >= fc_epsilon)
            {
            j0 = numV - 1;
            j1 = numV - 2;
            scale = -scale;
            }
        for (int i=0; i + 1 < numU; i++)
            {
            pole11 = surfP->GetUnWeightedPole (i,j0);
            pole12 = surfP->GetUnWeightedPole (i, j1);
            pole21 = surfP->GetUnWeightedPole (i+1, j0);
            pole22 = surfP->GetUnWeightedPole (i+1, j1);

            diff1.DifferenceOf (pole12, pole11);
            diff2.DifferenceOf (pole22, pole21);

            cross.CrossProduct (diff2, diff1);
            sum.SumOf (sum, cross);
            }
        normP->Scale (sum, scale);
        }
    else
        {
        size_t i0 = 0;
        size_t i1 = 1;
        double sign = -1.0;
        if (param >= fc_epsilon)
            {
            i0 = numU - 1;
            i1 = numU - 2;
            sign = 1.0;
            }
        for (int j=0; j + 1 < numV; j++)
            {
            pole11 = surfP->GetUnWeightedPole (i0, j);
            pole12 = surfP->GetUnWeightedPole (i1, j);
            pole21 = surfP->GetUnWeightedPole (i0, j+1);
            pole22 = surfP->GetUnWeightedPole (i1, j+1);

            diff1.DifferenceOf (pole12, pole11);
            diff2.DifferenceOf (pole22, pole21);

            cross.CrossProduct (diff2, diff1);
            sum.SumOf (sum, cross);
            }
        normP->Scale (sum, sign/numV);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_computePartials                                 |
|                                                                       |
| author        LuHan                                   2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_computePartials
(
DPoint3d            *pointP,
double              *weightP,
DPoint3d            *dPdU,
DPoint3d            *dPdV,
DPoint3d            *dPdUU,
DPoint3d            *dPdVV,
DPoint3d            *dPdUV,
DPoint3d            *normP,
double              u,
double              v,
MSBsplineSurfaceCP  surfP
)
    {
    int         uLeft, uSpan, vLeft, vSpan, index, uNumPoles, vNumPoles;
    double      uBlend[MAX_BSORDER], vBlend[MAX_BSORDER], *weights,
                dUBlend[MAX_BSORDER], dVBlend[MAX_BSORDER], *dUBLoc, *dVBLoc,
                dUUBlend[MAX_BSORDER], dVVBlend[MAX_BSORDER], *dUUBLoc, *dVVBLoc,
                h, *uBPtr, *vBPtr, *dVBPtr, *dUBPtr, dHdU, dHdV,
                *dVVBPtr, *dUUBPtr, dHdUU, dHdVV, dHdUV,
                product, weightFactor, *uBEnd, *vBEnd,
                magU, magV;
    DPoint3d    *pPtr, *poles, point;

    /* Compute The U Span and Blending Functions */
    dUBLoc = dPdU ? dUBlend : NULL;
    dUUBLoc = dPdUU ? dUUBlend : NULL;
    bsputil_blendingsForSecondPars (uBlend, dUBLoc, dUUBLoc, &uLeft, surfP->uKnots,
                                  u, 1.0, surfP->uParams.order, surfP->uParams.closed);
    /* Compute The V Span and Blending Functions */
    dVBLoc = dPdV ? dVBlend : NULL;
    dVVBLoc = dPdVV ? dVVBlend : NULL;
    bsputil_blendingsForSecondPars (vBlend, dVBLoc, dVVBLoc, &vLeft, surfP->vKnots,
                                  v, 1.0, surfP->vParams.order, surfP->vParams.closed);

    point.x = point.y = point.z = h = dHdU = dHdV = dHdUU = dHdVV = dHdUV = 0.0;
    if (dPdU) dPdU->x = dPdU->y = dPdU->z = 0.0;
    if (dPdV) dPdV->x = dPdV->y = dPdV->z = 0.0;
    if (dPdUU) dPdUU->x = dPdUU->y = dPdUU->z = 0.0;
    if (dPdVV) dPdVV->x = dPdVV->y = dPdVV->z = 0.0;
    if (dPdUV) dPdUV->x = dPdUV->y = dPdUV->z = 0.0;

    uSpan = uLeft - surfP->uParams.order;
    if (surfP->uParams.closed && uSpan < 0) uSpan += surfP->uParams.numPoles;
    uBPtr = uBlend;
    dUBPtr = dUBlend;
    dUUBPtr = dUUBlend;
    uBEnd = uBlend + surfP->uParams.order;

    uNumPoles = surfP->uParams.numPoles;
    vNumPoles = surfP->vParams.numPoles;
    poles = surfP->poles;
    if (surfP->rational)
        {
        weights = surfP->weights;
        for (; uBPtr < uBEnd; uSpan++, uBPtr++, dUBPtr++, dUUBPtr++)
            {
            uSpan %= uNumPoles;

            vSpan = vLeft - surfP->vParams.order;
            if (surfP->vParams.closed && vSpan < 0) vSpan += vNumPoles;

            vBPtr = vBlend;
            dVBPtr = dVBlend;
            dVVBPtr = dVVBlend;
            vBEnd = vBlend + surfP->vParams.order;
            for (; vBPtr < vBEnd;  vSpan++, vBPtr++, dVBPtr++, dVVBPtr++)
                {
                vSpan %= vNumPoles;

                index = vSpan * uNumPoles + uSpan;
                pPtr = poles + index;
                weightFactor = *(weights+index);

                product = *uBPtr * *vBPtr;
                h += product * weightFactor;

                point.x += product * pPtr->x;
                point.y += product * pPtr->y;
                point.z += product * pPtr->z;

                if (dPdU)
                    {
                    product = *dUBPtr * *vBPtr;
                    dPdU->x += product * pPtr->x;
                    dPdU->y += product * pPtr->y;
                    dPdU->z += product * pPtr->z;
                    dHdU += weightFactor * product;
                    }
                if (dPdV)
                    {
                    product = *uBPtr * *dVBPtr;
                    dPdV->x += product * pPtr->x;
                    dPdV->y += product * pPtr->y;
                    dPdV->z += product * pPtr->z;
                    dHdV += weightFactor * product;
                    }
                if (dPdUU)
                    {
                    product = *dUUBPtr * *vBPtr;
                    dPdUU->x += product * pPtr->x;
                    dPdUU->y += product * pPtr->y;
                    dPdUU->z += product * pPtr->z;
                    dHdUU += weightFactor * product;
                    }
                if (dPdVV)
                    {
                    product = *uBPtr * *dVVBPtr;
                    dPdVV->x += product * pPtr->x;
                    dPdVV->y += product * pPtr->y;
                    dPdVV->z += product * pPtr->z;
                    dHdVV += weightFactor * product;
                    }
                if (dPdUV)
                    {
                    product = *dUBPtr * *dVBPtr;
                    dPdUV->x += product * pPtr->x;
                    dPdUV->y += product * pPtr->y;
                    dPdUV->z += product * pPtr->z;
                    dHdUV += weightFactor * product;
                    }
                }
            }
        if (weightP)
            *weightP = h;

        point.x /= h;
        point.y /= h;
        point.z /= h;

        if (dPdUU)
            {
            dPdUU->x = (dPdUU->x - point.x * dHdUU +
                       2.0 * dHdU * (point.x * dHdU - dPdU->x)/h)/h;
            dPdUU->y = (dPdUU->y - point.y * dHdUU +
                       2.0 * dHdU * (point.y * dHdU - dPdU->y)/h)/h;
            dPdUU->z = (dPdUU->z - point.z * dHdUU +
                       2.0 * dHdU * (point.z * dHdU - dPdU->z)/h)/h;
            }
        if (dPdVV)
            {
            dPdVV->x = (dPdVV->x - point.x * dHdVV +
                       2.0 * dHdV * (point.x * dHdV - dPdV->x)/h)/h;
            dPdVV->y = (dPdVV->y - point.y * dHdVV +
                       2.0 * dHdV * (point.y * dHdV - dPdV->y)/h)/h;
            dPdVV->z = (dPdVV->z - point.z * dHdVV +
                       2.0 * dHdV * (point.z * dHdV - dPdV->z)/h)/h;
            }
        if (dPdUV)
            {
            dPdUV->x = (dPdUV->x - point.x * dHdUV +
                       (2.0 * point.x * dHdU * dHdV
                        - dPdU->x * dHdV - dPdV->x * dHdU)/h)/h;
            dPdUV->y = (dPdUV->y - point.y * dHdUV +
                       (2.0 * point.y * dHdU * dHdV
                        - dPdU->y * dHdV - dPdV->y * dHdU)/h)/h;
            dPdUV->z = (dPdUV->z - point.z * dHdUV +
                       (2.0 * point.z * dHdU * dHdV
                        - dPdU->z * dHdV - dPdV->z * dHdU)/h)/h;
            }
        if (dPdU)
            {
            dPdU->x = (dPdU->x - point.x * dHdU)/h;
            dPdU->y = (dPdU->y - point.y * dHdU)/h;
            dPdU->z = (dPdU->z - point.z * dHdU)/h;
            }
        if (dPdV)
            {
            dPdV->x = (dPdV->x - point.x * dHdV)/h;
            dPdV->y = (dPdV->y - point.y * dHdV)/h;
            dPdV->z = (dPdV->z - point.z * dHdV)/h;
            }
        }
    else    /* non-rational */
        {
        for (; uBPtr<uBEnd;  uSpan++, uBPtr++, dUBPtr++, dUUBPtr++)
            {
            uSpan %= uNumPoles;

            vSpan = vLeft - surfP->vParams.order;
            if (surfP->vParams.closed && vSpan < 0) vSpan += vNumPoles;
            vBPtr = vBlend;
            dVBPtr = dVBlend;
            dVVBPtr = dVVBlend;
            vBEnd = vBlend + surfP->vParams.order;
            for (; vBPtr<vBEnd;  vSpan++, vBPtr++, dVBPtr++, dVVBPtr++)
                {
                vSpan %= vNumPoles;

                index = vSpan * uNumPoles + uSpan;
                pPtr = poles + index;

                product = *uBPtr * *vBPtr;

                point.x += product * pPtr->x;
                point.y += product * pPtr->y;
                point.z += product * pPtr->z;

                if (dPdU)
                    {
                    product = *dUBPtr * *vBPtr;
                    dPdU->x += product * pPtr->x;
                    dPdU->y += product * pPtr->y;
                    dPdU->z += product * pPtr->z;
                    }
                if (dPdV)
                    {
                    product = *uBPtr * *dVBPtr;
                    dPdV->x += product * pPtr->x;
                    dPdV->y += product * pPtr->y;
                    dPdV->z += product * pPtr->z;
                    }
                if (dPdUU)
                    {
                    product = *dUUBPtr * *vBPtr;
                    dPdUU->x += product * pPtr->x;
                    dPdUU->y += product * pPtr->y;
                    dPdUU->z += product * pPtr->z;
                    }
                if (dPdVV)
                    {
                    product = *uBPtr * *dVVBPtr;
                    dPdVV->x += product * pPtr->x;
                    dPdVV->y += product * pPtr->y;
                    dPdVV->z += product * pPtr->z;
                    }
                if (dPdUV)
                    {
                    product = *dUBPtr * *dVBPtr;
                    dPdUV->x += product * pPtr->x;
                    dPdUV->y += product * pPtr->y;
                    dPdUV->z += product * pPtr->z;
                    }
                }
            }
        if (weightP)
            *weightP = 1.0;
        }
    if (pointP)
        *pointP = point;

    /* Compute surface normal vector */
    if (dPdU && dPdV && normP)
        {
        normP->CrossProduct (*dPdU, *dPdV);

        /* Recalculate surface normal using next row or colunm of poles */
        magU = dPdU->Magnitude ();
        magV = dPdV->Magnitude ();

        if (magU < 0.001)
            bspsurf_computeZeroSurfNorm (normP, surfP, v, BSSURF_U);
        else if (magV < 0.001)
            bspsurf_computeZeroSurfNorm (normP, surfP, u, BSSURF_V);
        }

    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_evaluateSurfacePoint                            |
|                                                                       |
| author        RBB                                     9/89            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_evaluateSurfacePoint
(
DPoint3d                    *pointP,
double                      *weightP,
DPoint3d                    *dPdU,
DPoint3d                    *dPdV,
double                      u,
double                      v,
const MSBsplineSurface      *surfP
)
    {
    int         uLeft, uSpan, vLeft, vSpan, index, uNumPoles, vNumPoles;
    double      uBlend[MAX_BSORDER], vBlend[MAX_BSORDER], *weights,
                dUBlend[MAX_BSORDER], dVBlend[MAX_BSORDER], *dUBLoc, *dVBLoc,
                h, *uBPtr, *vBPtr, *dVBPtr, *dUBPtr, dHdU, dHdV,
                product, weightFactor, *uBEnd, *vBEnd;
    DPoint3d    *pPtr, *poles, point;

    /* Compute The U Span and Blending Functions */
    dUBLoc = dPdU ? dUBlend : NULL;

    bsputil_computeBlendingFunctions (uBlend, dUBLoc, &uLeft, surfP->uKnots, u, surfP->uParams.numPoles, surfP->uParams.order, surfP->uParams.closed);

    /* Compute The V Span and Blending Functions */
    dVBLoc = dPdV ? dVBlend : NULL;
    bsputil_computeBlendingFunctions (vBlend, dVBLoc, &vLeft, surfP->vKnots, v, surfP->vParams.numPoles, surfP->vParams.order, surfP->vParams.closed);

    point.x = point.y = point.z = h = dHdU = dHdV = 0.0;
    if (dPdU) dPdU->x = dPdU->y = dPdU->z = 0.0;
    if (dPdV) dPdV->x = dPdV->y = dPdV->z = 0.0;

    uSpan = uLeft - surfP->uParams.order;
    if (surfP->uParams.closed && uSpan < 0) uSpan += surfP->uParams.numPoles;
    uBPtr = uBlend;
    dUBPtr = dUBlend;
    uBEnd = uBlend + surfP->uParams.order;

    uNumPoles = surfP->uParams.numPoles;
    vNumPoles = surfP->vParams.numPoles;
    poles = surfP->poles;
    if (surfP->rational)
        {
        weights = surfP->weights;
        for (; uBPtr < uBEnd; uSpan++, uBPtr++, dUBPtr++)
            {
            uSpan %= uNumPoles;

            vSpan = vLeft - surfP->vParams.order;
            if (surfP->vParams.closed && vSpan < 0) vSpan += vNumPoles;

            vBPtr = vBlend;
            dVBPtr = dVBlend;
            vBEnd = vBlend + surfP->vParams.order;
            for (; vBPtr < vBEnd;  vSpan++, vBPtr++, dVBPtr++)
                {
                vSpan %= vNumPoles;

                index = vSpan * uNumPoles + uSpan;
                pPtr = poles + index;
                weightFactor = *(weights+index);

                product = *uBPtr * *vBPtr;
                h += product * weightFactor;

                point.x += product * pPtr->x;
                point.y += product * pPtr->y;
                point.z += product * pPtr->z;

                if (dPdU)
                    {
                    product = *dUBPtr * *vBPtr;
                    dPdU->x += product * pPtr->x;
                    dPdU->y += product * pPtr->y;
                    dPdU->z += product * pPtr->z;
                    dHdU += weightFactor * product;
                    }
                if (dPdV)
                    {
                    product = *uBPtr * *dVBPtr;
                    dPdV->x += product * pPtr->x;
                    dPdV->y += product * pPtr->y;
                    dPdV->z += product * pPtr->z;
                    dHdV += weightFactor * product;
                    }
                }
            }
        if (weightP)
            *weightP = h;

        point.x /= h;
        point.y /= h;
        point.z /= h;

        if (dPdU)
            {
            dPdU->x = (dPdU->x - point.x * dHdU)/h;
            dPdU->y = (dPdU->y - point.y * dHdU)/h;
            dPdU->z = (dPdU->z - point.z * dHdU)/h;
            }
        if (dPdV)
            {
            dPdV->x = (dPdV->x - point.x * dHdV)/h;
            dPdV->y = (dPdV->y - point.y * dHdV)/h;
            dPdV->z = (dPdV->z - point.z * dHdV)/h;
            }
        }
    else    /* non-rational */
        {
        for (; uBPtr<uBEnd;  uSpan++, uBPtr++, dUBPtr++)
            {
            uSpan %= uNumPoles;

            vSpan = vLeft - surfP->vParams.order;
            if (surfP->vParams.closed && vSpan < 0) vSpan += vNumPoles;
            vBPtr = vBlend;
            dVBPtr = dVBlend;
            vBEnd = vBlend + surfP->vParams.order;
            for (; vBPtr<vBEnd;  vSpan++, vBPtr++, dVBPtr++)
                {
                vSpan %= vNumPoles;

                index = vSpan * uNumPoles + uSpan;
                pPtr = poles + index;

                product = *uBPtr * *vBPtr;

                point.x += product * pPtr->x;
                point.y += product * pPtr->y;
                point.z += product * pPtr->z;

                if (dPdU)
                    {
                    product = *dUBPtr * *vBPtr;
                    dPdU->x += product * pPtr->x;
                    dPdU->y += product * pPtr->y;
                    dPdU->z += product * pPtr->z;
                    }
                if (dPdV)
                    {
                    product = *uBPtr * *dVBPtr;
                    dPdV->x += product * pPtr->x;
                    dPdV->y += product * pPtr->y;
                    dPdV->z += product * pPtr->z;
                    }
                }
            }
        if (weightP)
            *weightP = 1.0;
        }
    if (pointP)
        *pointP = point;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_makeRationalSurface                             |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_makeRationalSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    int                 status, total;
    double              *wP, *endW;
    MSBsplineSurface    surface;

    if (in->rational)
        return bspsurf_copySurface (out, in);

    if (SUCCESS != (status = bspsurf_copySurface (&surface, in)))
        return status;

    surface.rational = true;
    total = surface.uParams.numPoles * surface.vParams.numPoles;
    if (NULL == (surface.weights = (double*)BSIBaseGeom::Malloc (total * sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }
    else
        {
        for (wP=endW=surface.weights, endW += total; wP < endW; wP++)
            *wP = 1.0;
        }

    if (out == in)
        bspsurf_freeSurface (out);

    *out = surface;

wrapup:
    if (status)
        bspsurf_freeSurface (&surface);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_swapUV                                          |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_swapUV
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    int                 i, j;
    double              *inWP, *tmpWP;
    DPoint3d            *inPP, *tmpPP;
    MSBsplineSurface    tmp;

    tmp = *in;
    tmp.uParams  = in->vParams;
    tmp.vParams  = in->uParams;

    if (bspsurf_allocateSurface (&tmp))
        return ERROR;

    memcpy (tmp.uKnots, in->vKnots,
            bspknot_numberKnots (in->vParams.numPoles, in->vParams.order,
                                 in->vParams.closed) * sizeof(double));
    memcpy (tmp.vKnots, in->uKnots,
            bspknot_numberKnots (in->uParams.numPoles, in->uParams.order,
                                 in->uParams.closed) * sizeof(double));

    for (j=0, inPP = in->poles; j < in->vParams.numPoles; j++)
        for (i=0, tmpPP = tmp.poles + j; i < in->uParams.numPoles;
             i++, inPP++, tmpPP += tmp.uParams.numPoles)
            {
            *tmpPP = *inPP;
            }

    if (tmp.rational)
        {
        for (j=0, inWP = in->weights; j < in->vParams.numPoles; j++)
            for (i=0, tmpWP = tmp.weights + j; i < in->uParams.numPoles;
                i++, inWP++, tmpWP += tmp.uParams.numPoles)
                {
                *tmpWP = *inWP;
                }
        }

    if (tmp.numBounds)
        {
        Transform xySwap;
        bspsurf_copyBoundaries (&tmp, in);
        xySwap.InitFromRowValues (
                    0, 1, 0, 0,
                    1, 0, 0, 0,
                    0, 0, 1, 0
                    );
        bspsurf_transformAllBoundaries (&tmp, &xySwap);
        }

    if (out == in)
        out->ReleaseMem ();
    *out = tmp;

    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_reversePolesAndWeights                          |
|               To reverse U direction :                                |
|                   rows   = vParams.numPoles                           |
|                   cols   = uParams.numPoles                           |
|                   rowInc = 1, colInc = uParams.numPoles               |
|               To reverse V direction :                                |
|                   rows   = uParams.numPoles                           |
|                   cols   = vParams.numPoles                           |
|                   rowInc = uParams.numPoles, colInc = 1               |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static void    bspsurf_reversePolesAndWeights
(
MSBsplineSurface    *out,              /* <= reversed surface, pre-allocated */
MSBsplineSurfaceCP  in,               /* => different than out */
int                 rows,
int                 cols,
int                 rowInc,
int                 colInc
)
    {
    int             i, j;
    double          *inWP, *outWP, *inRowWP, *outRowWP;
    DPoint3d        *inPP, *outPP, *inRowPP, *outRowPP;

    for (j=0,
         inRowPP = in->poles + (cols - 1) * rowInc,
         outRowPP = out->poles;
         j < rows; j++, inRowPP += colInc, outRowPP += colInc)
        for (i=0, inPP = inRowPP, outPP = outRowPP;
             i < cols; i++, inPP -= rowInc, outPP += rowInc)
            {
            *outPP = *inPP;
            }

    if (in->rational)
        {
        for (j=0,
            inRowWP = in->weights + (cols - 1) * rowInc,
            outRowWP = out->weights;
            j < rows; j++, inRowWP += colInc, outRowWP += colInc)
            for (i=0, inWP = inRowWP, outWP = outRowWP;
                i < cols; i++, inWP -= rowInc, outWP += rowInc)
                {
                *outWP = *inWP;
                }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_reverseSurface                                  |
|                                                                       |
| author        BSI                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_reverseSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP  in,
int                 direction
)
    {
    int                 i, numKnots;
    double              *inKP, *tmpKP;
    MSBsplineSurface    tmp;

    Transform transform;
    if (direction == BSSURF_U)
        transform.InitFromRowValues (
                            -1, 0, 0, 1,
                             0, 1, 0, 0,
                             0, 0, 1, 0
                             );
    else
        transform.InitFromRowValues (
                             1,  0, 0, 0,
                             0, -1, 0, 1,
                             0,  0, 1, 0
                             );

    // pole reversal is NOT inplace.  Copy the whole surface....
    if (SUCCESS != bspsurf_copySurface (&tmp, in))
        return ERROR;

    bspsurf_transformAllBoundaries (&tmp, &transform);

    if (direction == BSSURF_U)
        {
        bspsurf_reversePolesAndWeights (&tmp, in, in->vParams.numPoles, in->uParams.numPoles,
                                        1, in->uParams.numPoles);

        memcpy (tmp.vKnots, in->vKnots,
                bspknot_numberKnots (in->vParams.numPoles, in->vParams.order,
                                     in->vParams.closed) * sizeof(double));

        numKnots = bspknot_numberKnots (in->uParams.numPoles,
                                        in->uParams.order, in->uParams.closed);
        for (i=0, inKP = in->uKnots + numKnots - 1, tmpKP = tmp.uKnots;
             i < numKnots; i++, inKP--, tmpKP++)
            *tmpKP = *inKP;
        bspknot_normalizeKnotVector (tmp.uKnots, tmp.uParams.numPoles,
                                     tmp.uParams.order, tmp.uParams.closed);
        }
    else
        {
        bspsurf_reversePolesAndWeights (&tmp, in, in->uParams.numPoles, in->vParams.numPoles,
                                        in->uParams.numPoles, 1);

        memcpy (tmp.uKnots, in->uKnots,
                bspknot_numberKnots (in->uParams.numPoles, in->uParams.order,
                                     in->uParams.closed) * sizeof(double));

        numKnots = bspknot_numberKnots (in->vParams.numPoles,
                                        in->vParams.order, in->vParams.closed);
        for (i=0, inKP = in->vKnots + numKnots - 1, tmpKP = tmp.vKnots;
             i < numKnots; i++, inKP--, tmpKP++)
            *tmpKP = *inKP;
        bspknot_normalizeKnotVector (tmp.vKnots, tmp.vParams.numPoles,
                                     tmp.vParams.order, tmp.vParams.closed);
        }

    if (out == in)
        bspsurf_freeSurface (out);
    *out = tmp;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_evaluateSurface                                 |
|                                                                       |
| author        BSI                                     9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_evaluateSurface
(
DPoint3d                    **pts,
DPoint2d                    *data,
int                         *numPts,
const MSBsplineSurface      *surface
)
    {
    int         i, j, strokes=0, status;
    //double      *iWeights=NULL;
    double      x, y, deltaX, deltaY, tol;
    DPoint2d    *dPtr;
    DPoint3d    *pPtr, out[MAX_BATCH+1];
    //DPoint3d    *iPoles=NULL;

    if (*numPts)
        {
        if (data)                               /* case 1 */
            {
            if (NULL == (*pts = (DPoint3d*)BSIBaseGeom::Malloc (*numPts * sizeof(DPoint3d))))
                return ERROR;
            pPtr = *pts;
            for (i=0, dPtr=data; i < *numPts; i++, dPtr++, pPtr++)
                bspsurf_evaluateSurfacePoint (pPtr, NULL, NULL, NULL,
                                             dPtr->x, dPtr->y, surface);

            status = SUCCESS;
            }
/*      else if (surface->display.rulesByLength)  case 2
            {
            Not currently supported
            }*/
        else                                    /* case 3 */
            {
            if (NULL == (*pts =
                         (DPoint3d*)BSIBaseGeom::Malloc (numPts[0]*numPts[1] * sizeof(DPoint3d))))
                return ERROR;
            pPtr = *pts;
            deltaX = 1.0 / (double) (surface->uParams.closed ? numPts[0] : numPts[0] - 1);
            deltaY = 1.0 / (double) (surface->vParams.closed ?  numPts[1] : numPts[1] - 1);
            for (j=0, y=0.0; j < numPts[1]; j++, y += deltaY)
                for (i=0, x=0.0; i < numPts[0]; i++, pPtr++, x += deltaX)
                    bspsurf_evaluateSurfacePoint (pPtr, NULL, NULL, NULL, x, y, surface);

            status = SUCCESS;
            }
        }
    else if (data)                              /* case 4 */
        {
#ifdef dunnoWhich
        double minTolerance = bsputil_surfaceTolerance (surface);
        tol = data->x;
        if (fabs (tol) < minTolerance)
            return ERROR;
#else
        double surfaceTol = bsputil_surfaceTolerance ((MSBsplineSurfaceCP)surface);
        tol = data->x;
        if (tol <= surfaceTol)
            return ERROR;
#endif
#if defined (NEEDS_WORK)
        bspproc_processSurfaceMesh (surface, tol, bspline_returnStrokesSurface,
                                    &meshSurfaceParams);

#endif
        status = bsputil_flushStrokes (out, strokes, pts, numPts);
        }
    else
        status = ERROR;

    return status;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_segmentSurface                                  |
|                                                                       |
| author        BSI                                     9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_segmentSurface
(
MSBsplineSurface    *segment,
MSBsplineSurfaceCP  surface,
DPoint2dP          initial,
DPoint2dP          final
)
    {
    int               i, status;
    double            *weights, *wP = NULL, *endW;
    DPoint2d          uvInit, uvLFinal;
    DPoint3d          *poles, *pP = NULL, *endP;
    MSBsplineCurve    curve, segCurv;
    MSBsplineSurface  tmpSurf, segSurf;
    int numLinear, numPCurve;

    static int s_showSplits = 0;
    if (s_showSplits)
        {
        GEOMAPI_PRINTF (" (segmentSurface (%g,%g) (%g,%g)\n", initial->x, initial->y, final->x, final->y);
        }
#if defined (needs_work)
/* This could be speeded up substantially by not repeatedly calling
    bspcurv_segmentCurve. Each call has the same intial & final values.
    Keep track of needed info here and avoid all the repetition.
    This will do for now. */
#endif

    uvInit.Zero ();
    uvLFinal.Zero ();
    segSurf.Zero ();
    if (segment != surface)
        segment->Zero ();
    tmpSurf.Zero ();
    curve.Zero ();
    segCurv.Zero ();

    /* Segment in the U direction */
    curve.rational = surface->rational;
    curve.params   = surface->uParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, surface->uKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    poles   = surface->poles;
    weights = surface->weights;

    for (i=0; i < surface->vParams.numPoles;
         i++, poles += surface->uParams.numPoles, pP += tmpSurf.uParams.numPoles)
        {
        memcpy (curve.poles, poles, curve.params.numPoles * sizeof(DPoint3d));
        if (curve.rational)
            memcpy (curve.weights, weights, curve.params.numPoles * sizeof(double));

        if (SUCCESS !=
            (status = bspcurv_segmentCurve (&segCurv, &curve, initial->x, final->x)))
            goto wrapup;

        if (i==0)
            {
            tmpSurf.rational = surface->rational;
            tmpSurf.uParams = segCurv.params;
            tmpSurf.vParams = surface->vParams;
            tmpSurf.numBounds = 0;    /* Boundaries will be rebuilt from boolean below. */
            if (SUCCESS != (status = bspsurf_allocateSurface (&tmpSurf)))
                goto wrapup;
            memcpy (tmpSurf.uKnots, segCurv.knots,
                    bspknot_numberKnots (tmpSurf.uParams.numPoles, tmpSurf.uParams.order,
                                         tmpSurf.uParams.closed) * sizeof(double));
            memcpy (tmpSurf.vKnots, surface->vKnots,
                    bspknot_numberKnots (tmpSurf.vParams.numPoles, tmpSurf.vParams.order,
                                         tmpSurf.vParams.closed) * sizeof(double));
            pP = tmpSurf.poles;
            wP = tmpSurf.weights;
            }
        memcpy (pP, segCurv.poles, tmpSurf.uParams.numPoles * sizeof(DPoint3d));
        if (tmpSurf.rational)
            {
            memcpy (wP, segCurv.weights, tmpSurf.uParams.numPoles * sizeof(double));
            weights += surface->uParams.numPoles;
            wP += tmpSurf.uParams.numPoles;
            }
        bspcurv_freeCurve (&segCurv);
        }

    bspcurv_freeCurve (&curve);

    /* Segment in the V direction */
    curve.rational = tmpSurf.rational;
    curve.params   = tmpSurf.vParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, tmpSurf.vKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    for (i=0; i < tmpSurf.uParams.numPoles; i++)
        {
        for (pP=endP=curve.poles, poles=tmpSurf.poles+i,
             endP += curve.params.numPoles;
             pP < endP; pP++, poles += tmpSurf.uParams.numPoles)
            *pP = *poles;

        if (curve.rational)
            for (wP=endW=curve.weights, weights=tmpSurf.weights+i,
                endW += curve.params.numPoles;
                wP < endW; wP++, weights += tmpSurf.uParams.numPoles)
                *wP = *weights;

        if (SUCCESS !=
            (status = bspcurv_segmentCurve (&segCurv, &curve, initial->y, final->y)))
            goto wrapup;

        if (i==0)
            {
            segSurf.rational = tmpSurf.rational;
            segSurf.uParams  = tmpSurf.uParams;
            segSurf.vParams  = segCurv.params;
            if (SUCCESS != (status = bspsurf_allocateSurface (&segSurf)))
                goto wrapup;
            memcpy (segSurf.uKnots, tmpSurf.uKnots,
                    bspknot_numberKnots (segSurf.uParams.numPoles, segSurf.uParams.order,
                                         segSurf.uParams.closed) * sizeof(double));
            memcpy (segSurf.vKnots, segCurv.knots,
                    bspknot_numberKnots (segSurf.vParams.numPoles, segSurf.vParams.order,
                                         segSurf.vParams.closed) * sizeof(double));
            }

        if (segCurv.params.numPoles < segSurf.vParams.numPoles)
            {
            status = ERROR;
            goto wrapup;
            }

        for (poles=segSurf.poles+i, pP=endP=segCurv.poles,
             endP += segSurf.vParams.numPoles;
             pP < endP; poles += segSurf.uParams.numPoles, pP++)
            *poles = *pP;

        if (segSurf.rational)
            for (weights=segSurf.weights+i, wP=endW=segCurv.weights,
                endW += segSurf.vParams.numPoles;
                wP < endW; weights += segSurf.uParams.numPoles, wP++)
                *weights = *wP;

        bspcurv_freeCurve (&segCurv);
        }
    bspcurv_freeCurve (&curve);

    segSurf.type = BSSURF_GENERAL;
    segSurf.display = surface->display;
    segSurf.uParams.numRules = segSurf.vParams.numPoles;
    segSurf.vParams.numRules = segSurf.uParams.numPoles;

    /* Add trim boundary if there is any */
    bspsurf_countLoops (surface, &numLinear, &numPCurve);
    //static int s_clipPCurves = 1;
    if (surface->numBounds != 0)
        {
        static double s_noGapTol = 1.0e-13; // aggressively close gaps.
        static double s_bigGapDirect = 1.0e-8;
        static double s_gapAlong = 1.0e-6;
        CurveGapOptions gapOptions (s_noGapTol, s_bigGapDirect, s_gapAlong);
        static bool s_preferCurves = true;
        CurveVectorPtr   oldBoundaries = surface->GetUVBoundaryCurves (true, s_preferCurves);
        Transform transform;
        double sx = 1.0 / (final->x - initial->x);
        double sy = 1.0 / (final->y - initial->y);
        transform.InitFromRowValues (
                 sx, 0.0, 0.0, - initial->x * sx,
                0.0,  sy, 0.0, - initial->y * sy,
                0.0, 0.0, 1.0, 0.0
                );
        DRange2d sortedRange;
        sortedRange.Init ();
        sortedRange.Extend (*initial);
        sortedRange.Extend (*final);
        CurveVectorPtr boxCurves = CurveVector::CreateRectangle (
            sortedRange.low.x, sortedRange.low.y, sortedRange.high.x, sortedRange.high.y, 0.0,
            CurveVector::BOUNDARY_TYPE_Outer);
        CurveVectorPtr fixedOldBoundaries = oldBoundaries->CloneWithGapsClosed (gapOptions);
        CurveVectorPtr newBoundaries = CurveVector::AreaIntersection (*boxCurves, *oldBoundaries);
        if (newBoundaries.IsValid ())
            {
            newBoundaries->TransformInPlace (transform);
            DPoint3d centroid;
            DVec3d normal;
            double area;
            newBoundaries->CentroidNormalArea (centroid, normal, area);
            static double s_unitAreaTolerance = 1.0e-8;
            if (newBoundaries->size () == 1 && fabs (area - 1.0) < s_unitAreaTolerance)
                {
                // The entire clipped area is live.  Let it all alone as an untrimmed surface.
                }
            else
                {
                segSurf.SetTrim (*newBoundaries);
                }
            }
        else
            {
            // hm.. the segment is all clipped out
            segSurf.ReleaseMem ();
            status = ERROR;     // ugh... it's an empty result, not necessarily an error.  But empty surfaces make apps choke, so call it an error.
            }
        }



    if (segment == surface)
        bspsurf_freeSurface (segment);
    *segment = segSurf;
    segSurf.Zero ();

wrapup:
    bspcurv_freeCurve (&curve);
    bspsurf_freeSurface (&tmpSurf);
    if (status)
        bspsurf_freeSurface (&segSurf);
    return status;
    }



/*----------------------------------------------------------------------+
|                                                                       |
|   Impose Boundary Routines                                            |
|                                                                       |
+----------------------------------------------------------------------*/
#ifdef CompileBoresite
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_addBndPoint                                     |
|                                                                       |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspsurf_addBndPoint
(
DPoint2d        **paramDest,
DPoint3d        **pointDest,
int             *numPoints,
DPoint2d        *param,
DPoint3d        *point,
DPoint2d        *paramBuffer,
DPoint3d        *pointBuffer,
int             *bufferSize,
int             flushBuffer
)
    {
    int         allocSize;
    double      *ptr;

#if defined (debug_imp)
    {
    DPoint3d    line[2];
    ElementUnion u;

    printf ("(U,V) = %f, %f\n", param->x, param->y);
    line[0] = line[1] = *point;
    mdlLine_create (&u, NULL, line);
    u.line_3d.dhdr.symb.b.weight = 5;
    mdlElement_display (&u, 0);
    }
#endif
    if (!flushBuffer)
        {
        paramBuffer[*bufferSize] = *param;
        pointBuffer[*bufferSize] = *point;
        }

    if (flushBuffer || (*bufferSize)++ == MAX_BATCH)
        {
        allocSize = *numPoints + *bufferSize;
        if (*numPoints)
            {
            if (NULL == (ptr =
                (double *) (DPoint2d*)BSIBaseGeom::Realloc (*paramDest, allocSize * sizeof(DPoint2d))))
                return ERROR;
            else
                *paramDest = (DPoint2d *) ptr;
            }
        else
            {
            if (NULL == (*paramDest = (DPoint2d*)BSIBaseGeom::Malloc (allocSize * sizeof(DPoint2d))))
                return ERROR;
            }

        memcpy (*paramDest + *numPoints, paramBuffer, *bufferSize * sizeof(DPoint2d));

        if (pointDest)
            {
            if (*pointDest)
                {
                if (NULL == (ptr =
                    (double *) BSIBaseGeom::Realloc (*pointDest, allocSize*sizeof(DPoint3d))))
                    return ERROR;
                else
                    *pointDest = (DPoint3d *) ptr;
                }
            else
                {
                if (NULL == (*pointDest = (DPoint3d*)BSIBaseGeom::Malloc (allocSize * sizeof(DPoint3d))))
                    return ERROR;
                }
            memcpy (*pointDest + *numPoints, pointBuffer, *bufferSize * sizeof(DPoint3d));
            }

        *numPoints += *bufferSize;
        *bufferSize = 0;
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_edgePtCode                                      |
|                                                                       |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspsurf_edgePtCode
(
DPoint2d        *pt
)
    {
    double      near1 = 1.0-fc_epsilon;

    if (pt->x < fc_epsilon)
        {
        if (pt->y < 0.5)
            return (17);
        else
            return (13);
        }
    else if (pt->x > near1)
        {
        if (pt->y < 0.5)
            return (3);
        else
            return (5);
        }
    else if (pt->y < fc_epsilon)
        {
        if (pt->x < 0.5)
            return (1);
        else
            return (2);
        }
    else
        {
        if (pt->x < 0.5)
            return (11);
        else
            return (7);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_nextBndPoint                                    |
|               derived from bspline_strokeSurfaceCurve                 |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspsurf_nextBndPoint
(
DPoint2d            **paramDest,
DPoint3d            **pointDest,
int                 *numPoints,
DPoint2d            *param,
DPoint2d            *lastParam,
DPoint2d            *paramBuffer,
DPoint3d            *pointBuffer,
int                 *bufferSize,
MSBsplineSurface    *surface,
double              tolerance
)
    {
    int         i, j, status, insertNum;
    double      t, tMin, tMax, tLast, tNext, tDelta;
    DPoint2d    delta, c, m, uvLast, uvNext;
    DPoint3d    pLast, pNext, tanLast, tanNext, dPdU, dPdV, point;

    delta.x = param->x - lastParam->x;
    delta.y = param->y - lastParam->y;
    if (fabs(delta.x) > fabs(delta.y))
        {
        tMin = lastParam->x;
        tMax = param->x;
        }
    else
        {
        tMin = lastParam->y;
        tMax = param->y;
        }
    if (bspknot_sameKnot (tMin, tMax))
        return SUCCESS;

    else if (tMin > tMax)
        DoubleOps::Swap (tMin, tMax);

    tDelta = tMax - tMin;
    m.x = (param->x - lastParam->x)/tDelta;
    m.y = (param->y - lastParam->y)/tDelta;
    c.x = lastParam->x - m.x * tMin;
    c.y = lastParam->y - m.y * tMin;

    tLast = tMin;
    uvLast.x = c.x + m.x * tLast;
    uvLast.y = c.y + m.y * tLast;
    bspsurf_evaluateSurfacePoint (&pLast, NULL, &dPdU, &dPdV, uvLast.x, uvLast.y,
                                  surface);

    tanLast.x = dPdU.x * m.x + dPdV.x * m.y;
    tanLast.y = dPdU.y * m.x + dPdV.y * m.y;
    tanLast.z = dPdU.z * m.x + dPdV.z * m.y;

    for (i=(int) tMin; i < tMax; i++)
        {
        tNext = i+1.0;
        if (tNext > tMax) tNext = tMax;

        uvNext.x = c.x + m.x * tNext;
        uvNext.y = c.y + m.y * tNext;

        bspsurf_evaluateSurfacePoint (&pNext, NULL, &dPdU, &dPdV, uvNext.x, uvNext.y,
                                      surface);

        tanNext.x = dPdU.x * m.x + dPdV.x * m.y;
        tanNext.y = dPdU.y * m.x + dPdV.y * m.y;
        tanNext.z = dPdU.z * m.x + dPdV.z * m.y;

        insertNum = bspline_spansReqd (&tanLast, &pLast, &pNext, tolerance);

        if (insertNum)
            {
            tDelta = (tNext - tLast)/((double) (insertNum+1));
            for (j=0, t=tLast+tDelta; j < insertNum; j++, t += tDelta)
                {
                uvLast.x = c.x + m.x * t;
                uvLast.y = c.y + m.y * t;
                bspsurf_evaluateSurfacePoint (&point, NULL, NULL, NULL, uvLast.x, uvLast.y,
                                              surface);

                if (SUCCESS != (status = bspsurf_addBndPoint (paramDest, pointDest, numPoints,
                                                 &uvLast, &point, paramBuffer, pointBuffer,
                                                 bufferSize, false)))
                    return status;
                }
            }

        if (SUCCESS != (status = bspsurf_addBndPoint (paramDest, pointDest, numPoints,
                                                      &uvNext, &pNext, paramBuffer,
                                                      pointBuffer, bufferSize, false)))
            return status;

        pLast = pNext;
        tLast = tNext;
        tanLast = tanNext;
        uvLast = uvNext;
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_longPath                                        |
|                                                                       |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_longPath
(
DPoint2d        *path,
int             *length,
DPoint2d        *param,
DPoint2d        *lastParam
)
    {
    int         codeA, codeB, prod;
    DPoint2d    uvA, uvB, corn00, corn01, corn10, corn11;

    corn00.x = 0.0;         corn00.y = 0.0;
    corn01.x = 0.0;         corn01.y = 1.0;
    corn10.x = 1.0;         corn10.y = 0.0;
    corn11.x = 1.0;         corn11.y = 1.0;

    /* Project current point to nearest edge */
    uvA = *lastParam;
    bsputil_closestEdge (&uvB, param);
    codeA = bspsurf_edgePtCode (&uvA);
    codeB = bspsurf_edgePtCode (&uvB);
    *length=0;
    path[0] = uvA;

    prod = codeA * codeB;

    switch (prod)
        {
        case 1:
        case 2:
        case 4:
            if (uvA.x < uvB.x)
                {
                path[1] = corn00; path[2] = corn01;
                path[3] = corn11; path[4] = corn10;
                }
            else
                {
                path[4] = corn00; path[3] = corn01;
                path[2] = corn11; path[1] = corn10;
                }
            *length=6;
            break;
        case 9:
        case 15:
        case 25:
            if (uvA.y < uvB.y)
                {
                path[1] = corn10; path[2] = corn00;
                path[3] = corn01; path[4] = corn11;
                }
            else
                {
                path[4] = corn10; path[3] = corn00;
                path[2] = corn01; path[1] = corn11;
                }
            *length=6;
            break;
        case 49:
        case 77:
        case 121:
            if (uvA.x < uvB.x)
                {
                path[1] = corn01; path[2] = corn00;
                path[3] = corn10; path[4] = corn11;
                }
            else
                {
                path[4] = corn01; path[3] = corn00;
                path[2] = corn10; path[1] = corn11;
                }
            *length=6;
            break;
        case 169:
        case 221:
        case 289:
            if (uvA.y < uvB.y)
                {
                path[1] = corn00; path[2] = corn10;
                path[3] = corn11; path[4] = corn01;
                }
            else
                {
                path[4] = corn00; path[3] = corn10;
                path[2] = corn11; path[1] = corn01;
                }
            *length=6;
            break;

        case 13:
        case 17:
        case 26:
        case 34:
            if (codeA == 1 || codeA == 2)
                {
                path[1] = corn10; path[2] = corn11; path[3] = corn01;
                }
            else
                {
                path[3] = corn10; path[2] = corn11; path[1] = corn01;
                }
            *length=5;
            break;
        case 3:
        case 5:
        case 6:
        case 10:
            if (codeA == 1 || codeA == 2)
                {
                path[1] = corn00; path[2] = corn01; path[3] = corn11;
                }
            else
                {
                path[3] = corn00; path[2] = corn01; path[1] = corn11;
                }
            *length=5;
            break;
        case 21:
        case 33:
        case 35:
        case 55:
            if (codeA == 3 || codeA == 5)
                {
                path[1] = corn10; path[2] = corn00; path[3] = corn01;
                }
            else
                {
                path[3] = corn10; path[2] = corn00; path[1] = corn01;
                }
            *length=5;
            break;
        case 91:
        case 119:
        case 143:
        case 187:
            if (codeA == 7 || codeA == 11)
                {
                path[1] = corn11; path[2] = corn10; path[3] = corn00;
                }
            else
                {
                path[3] = corn11; path[2] = corn10; path[1] = corn00;
                }
            *length=5;
            break;

        case 7:
        case 11:
            if (codeA == 1)
                {
                path[1] = corn10; path[2] = corn11;
                }
            else
                {
                path[2] = corn10; path[1] = corn11;
                }
            *length=4;
            break;
        case 14:
        case 22:
            if (codeA == 2)
                {
                path[1] = corn00; path[2] = corn01;
                }
            else
                {
                path[2] = corn00; path[1] = corn01;
                }
            *length=4;
            break;
        case 39:
        case 51:
            if (codeA == 3)
                {
                path[1] = corn11; path[2] = corn01;
                }
            else
                {
                path[2] = corn11; path[1] = corn01;
                }
            *length=4;
            break;
        case 65:
        case 85:
            if (codeA == 5)
                {
                path[1] = corn10; path[2] = corn00;
                }
            else
                {
                path[2] = corn10; path[1] = corn00;
                }
            *length=4;
            break;
        }

    path[*length-1] = uvB;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_shortPath                                       |
|                                                                       |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_shortPath
(
DPoint2d        *path,
int             *length,
DPoint2d        *param,
DPoint2d        *lastParam
)
    {
    int         codeA, codeB, prod;
    DPoint2d    uvA, uvB, corn00, corn01, corn10, corn11;

    corn00.x = 0.0;         corn00.y = 0.0;
    corn01.x = 0.0;         corn01.y = 1.0;
    corn10.x = 1.0;         corn10.y = 0.0;
    corn11.x = 1.0;         corn11.y = 1.0;

    /* Project current point to nearest edge */
    uvA = *lastParam;
    bsputil_closestEdge (&uvB, param);
    codeA = bspsurf_edgePtCode (&uvA);
    codeB = bspsurf_edgePtCode (&uvB);
    *length=0;
    path[0] = uvA;

    prod = codeA * codeB;

    switch (prod)
        {
        case 1:
        case 2:
        case 4:
        case 9:
        case 15:
        case 25:
        case 49:
        case 77:
        case 121:
        case 169:
        case 221:
        case 289:
            *length=2;
            break;

        case 13:
        case 17:
        case 26:
        case 34:
            path[1] = corn00;
            *length=3;
            break;
        case 3:
        case 5:
        case 6:
        case 10:
            path[1] = corn10;
            *length=3;
            break;
        case 21:
        case 33:
        case 35:
        case 55:
            path[1] = corn11;
            *length=3;
            break;
        case 91:
        case 119:
        case 143:
        case 187:
            path[1] = corn01;
            *length=3;
            break;

        case 7:
        case 11:
            if (codeA == 1)
                {
                path[1] = corn00; path[2] = corn01;
                }
            else
                {
                path[2] = corn00; path[1] = corn01;
                }
            *length=4;
            break;
        case 14:
        case 22:
            if (codeA == 2)
                {
                path[1] = corn10; path[2] = corn11;
                }
            else
                {
                path[2] = corn10; path[1] = corn11;
                }
            *length=4;
            break;
        case 39:
        case 51:
            if (codeA == 3)
                {
                path[1] = corn10; path[2] = corn00;
                }
            else
                {
                path[2] = corn10; path[1] = corn00;
                }
            *length=4;
            break;
        case 65:
        case 85:
            if (codeA == 5)
                {
                path[1] = corn11; path[2] = corn01;
                }
            else
                {
                path[2] = corn11; path[1] = corn01;
                }
            *length=4;
            break;
        }

    path[*length-1] = uvB;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_insertEdgePath                                  |
|                                                                       |
| author        BSI                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspsurf_insertEdgePath
(
DPoint2d            **paramDest,
DPoint3d            **pointDest,
int                 *numPoints,
DPoint2d            *param,
DPoint2d            *lastParam,
DPoint2d            *paramBuffer,
DPoint3d            *pointBuffer,
int                 *bufferSize,
int                 *holeOrigin,
MSBsplineSurface    *surface,
double              tolerance,
int                 shortPath          /* => true means get shortest path */
)
    {
    int         i, status, lngth;
    DPoint2d    path[7];

    if (shortPath)
        bspsurf_shortPath (path, &lngth, param, lastParam);
    else
        bspsurf_longPath (path, &lngth, param, lastParam);

    for (i=1; i < lngth; i++)
        {
        if (SUCCESS != (status = bspsurf_nextBndPoint (paramDest, pointDest, numPoints,
                                                       path+i, path+i-1,
                                                       paramBuffer, pointBuffer, bufferSize,
                                                       surface, tolerance)))
            return status;
        }

    *lastParam = path[lngth-1];

    /* Do NOT change the holeOrigin flag of the original surface as the
        bsptuil_pointOnSurface test then fails in bsprsurf_boresiteToSurface. */
    *holeOrigin = ! *holeOrigin;
    return SUCCESS;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_imposeBoundary                                  |
|                                                                       |
| author        BSI                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_imposeBoundaryBySweptCurve
(
MSBsplineSurface    *surface,
MSBsplineCurveCP    curve,
double              tolerance,
DVec3dCP            direction,
DPoint3d            **surfPoints,       /* <= points on surface, or NULL */
int                 *numSurfPts
)
    {
    int             status, numSamplePts, allocSize, bufCount, saveCount, offSurface,
                    flipSurface, holeOrigin, saveNumBounds;
    double          tol, dist, uvTol, incr, u;
    DPoint2d        param, lastParam, edgeParam, uvBuffer[MAX_BATCH+1],
                    *allParam;
    DPoint3d        point, ptBuffer[MAX_BATCH+1], crvPt, *allPt;
    DVec3d          norm;
    BsurfBoundary   bound, *tmpBound;
    Evaluator       eval;

    if (tolerance < fc_epsilon)
        return ERROR;

    lastParam.Zero ();
    /* initialize variables */
    tol = tolerance;
    allPt = NULL; allParam = NULL;
    numSamplePts = bufCount = 0;
    eval.surf     = surface;
    eval.offset   = 0;
    eval.distance = 0.0;
    bspUtil_initializeBsurfBoundary (&bound);

    /* Don't let the user get surfPoints without knowing its size */
    if (surfPoints)
        {
        if (! numSurfPts)
            return ERROR;
        *surfPoints = NULL;
        }
    if (numSurfPts)    *numSurfPts = 0;


    if (direction)
        norm.Normalize (*direction);

    DRange3d surfaceRange;
    surface->GetPoleRange (surfaceRange);
    uvTol = tolerance / (10.0 * surfaceRange.low.Distance (surfaceRange.high));

    KnotData knotData;
    knotData.LoadCurveKnots (*curve);


    /* Ignore bounds in the surface because the boresite routine does
        a bsputil_pointOnSurface test before returning. */
    saveNumBounds = surface->numBounds;
    surface->numBounds = 0;

    dist = curve->PolygonLength ();

    flipSurface = 0;
    offSurface = false;
    holeOrigin = surface->holeOrigin;
    incr = tol / dist;
    param.x = param.y = 0.5;
    double nextKnot;
    size_t activeKnotIndex = 0;
    size_t knotMultiplicity;
    knotData.GetKnotByActiveIndex (activeKnotIndex, nextKnot, knotMultiplicity);
    for (u = 0.0; u < 1.0; u += incr)
        {
        if (u >= nextKnot)
            {
            u = nextKnot;
            activeKnotIndex++;
            if (knotData.GetKnotByActiveIndex (activeKnotIndex, nextKnot, knotMultiplicity))
                {
                }
            else
                {
                nextKnot = DBL_MAX;
                }
            }
        bspcurv_evaluateCurvePoint (&crvPt, NULL, curve, u);

        if (direction)
            {
            bsprsurf_boresiteToSurface (&point, &param, &crvPt, &norm,
                                        surface, &tol);
            }
        else
            {
            /* Try to relax from the last point on surface */
            if (STATUS_CONVERGED != (status = bspssi_relaxToSurface (&point, &norm, &dist,
                                                                     &param, &crvPt, &eval,
                                                                     5, uvTol)))
                bsprsurf_minDistToSurface (&dist, &point, &param, &crvPt, surface, &tol);
            }

        saveCount = bound.numPoints + bufCount;

        if (param.x < 0.0)
            {
            /* projected point missed surface */
            if (!offSurface && (bound.numPoints || bufCount))
                {
                /* point missed surface but there are some valid points,
                    so extend boundary to nearest edge from last bnd point */
                bsputil_closestEdge (&param, &lastParam);
                if (SUCCESS != (status = bspsurf_nextBndPoint (&bound.points, surfPoints, &bound.numPoints, &param,
                                                               &lastParam, uvBuffer, ptBuffer, &bufCount, surface, tol)))
                    goto wrapup;
                }

            offSurface = true;
            }
        else
            {
            /* NEED TO TEST THAT THE PARAMETER POINTS ARE NOT FARTHER
                APART THAN THE surface points         */

            /* projected point hit surface */
            if (offSurface)
                {
                if (bound.numPoints || bufCount)
                    {
                    /* back onto surface after missing */
                    if (SUCCESS !=
                        (status = bspsurf_insertEdgePath (&bound.points, surfPoints,
                                          &bound.numPoints, &param, &lastParam, uvBuffer,
                                          ptBuffer, &bufCount, &holeOrigin, surface, tol,
                                          false)))
                        goto wrapup;
                    }
                else
                    {
                    /* first hit on surface after some misses */
                    bsputil_closestEdge (&lastParam, &param);
                    bspsurf_evaluateSurfacePoint (&point, NULL, NULL, NULL,
                                                  lastParam.x, lastParam.y, surface);

                    if (SUCCESS != (status = bspsurf_addBndPoint (&bound.points, surfPoints,
                                                     &bound.numPoints, &lastParam, &point,
                                                     uvBuffer, ptBuffer, &bufCount, false)))
                        goto wrapup;
                    }
                }

            if (bound.numPoints + bufCount == 0)
                {
                /* first hits surface, no prior misses */
                if (SUCCESS != (status = bspsurf_addBndPoint (&bound.points, surfPoints,
                                                 &bound.numPoints, &param, &point,
                                                 uvBuffer, ptBuffer, &bufCount, false)))
                    goto wrapup;
                }
            else
                {
                /* hit on surface, previous point did not miss */
                dist = fabs (lastParam.x - param.x) +
                       fabs (lastParam.y - param.y);
                if (dist > BOUNDARY_JUMP_TOLERANCE)
                    {
                    edgeParam = lastParam;
                    bsputil_closestEdge (&edgeParam, &lastParam);
                    if (SUCCESS !=
                        (status = bspsurf_nextBndPoint (&bound.points, surfPoints,
                                          &bound.numPoints, &edgeParam, &lastParam,
                                          uvBuffer, ptBuffer, &bufCount, surface, tol)))
                        goto wrapup;
                    lastParam = edgeParam;
                    if (SUCCESS !=
                        (status = bspsurf_insertEdgePath (&bound.points, surfPoints,
                                          &bound.numPoints, &param, &lastParam, uvBuffer,
                                          ptBuffer, &bufCount, &holeOrigin, surface, tol,
                                          false)))
                        goto wrapup;
                    flipSurface++;
                    }
                if (SUCCESS !=
                    (status = bspsurf_nextBndPoint (&bound.points, surfPoints,
                                      &bound.numPoints, &param, &lastParam,
                                      uvBuffer, ptBuffer, &bufCount, surface, tol)))
                    goto wrapup;
                }

            offSurface = false;
            }

        // Advance lastParam *only if* the boundary point count has advanced.
        // Before this change, lastParam was always advanced, and in the case where each pair of successive params is within fc_p0001,
        //    bspsurf_nextBndPoint would always return without adding the next point (but lastParam nevertheless reflected the skipped
        //    point): this led to ERROR being returned and no boundary being imposed.  -DA4
        if (saveCount < bound.numPoints + bufCount)
            lastParam = param;
        }

    /* restore numBounds (see comment above) */
    surface->numBounds = saveNumBounds;

    if (bound.numPoints + bufCount < 2)
        {
        status = ERROR;
        goto wrapup;
        }

    /* Close the boundary */
    if (bound.numPoints == 0)
        param = uvBuffer[0];
    else
        param = bound.points[0];

    if (offSurface)
        {
        /* Boundary curve ended off the surface, so must have also started off surface */
        if (SUCCESS !=
            (status = bspsurf_insertEdgePath (&bound.points, surfPoints, &bound.numPoints,
                                              &param, &lastParam, uvBuffer, ptBuffer,
                                              &bufCount, &holeOrigin, surface, tol,
                                              false)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS !=
            (status = bspsurf_nextBndPoint (&bound.points, surfPoints, &bound.numPoints,
                                            &param, &lastParam, uvBuffer, ptBuffer,
                                            &bufCount, surface, tol)))
            goto wrapup;
        }


    /* Flush buffer, point & param are ignored because last arg is true */
    if (SUCCESS !=
        (status = bspsurf_addBndPoint (&bound.points, surfPoints, &bound.numPoints,
                                       &param, &point, uvBuffer, ptBuffer, &bufCount, true)))
        goto wrapup;

    allocSize = (surface->numBounds + 1) * sizeof(BsurfBoundary);
    if (surface->numBounds)
        {
        if (NULL == (tmpBound = (BsurfBoundary*)BSIBaseGeom::Realloc (surface->boundaries, allocSize)))
            {
            status = ERROR;
            goto wrapup;
            }
        surface->boundaries = tmpBound;
        }
    else
        {
        if (NULL == (surface->boundaries = (BsurfBoundary*)BSIBaseGeom::Malloc (allocSize)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    memcpy (&surface->boundaries[surface->numBounds], &bound, sizeof(BsurfBoundary));
    surface->numBounds += 1;
    surface->holeOrigin = holeOrigin;

    if (numSurfPts)
        *numSurfPts = surface->boundaries[surface->numBounds - 1].numPoints;

    if (flipSurface % 4)
        surface->holeOrigin = ! surface->holeOrigin;

wrapup:
    if (status)
        {
        if (bound.points)       BSIBaseGeom::Free (bound.points);
        if (surfPoints)
            {
            if (*surfPoints)    BSIBaseGeom::Free (*surfPoints);
            *numSurfPts = 0;
            }
        }


    return status;
    }
#endif


/*----------------------------------------------------------------------+
|                                                                       |
|   Routines from bspline.c                                             |
|   These routines should be put back where they belong when            |
|   impose/extract boundary can be redone better                        |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspline_spansReqd                                       |
|                                                                       |
| author        RBB                                     9/89            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     bspline_spansReqd
(
DPoint3d        *tangent,
DPoint3d        *p1,
DPoint3d        *p2,
double          tolerance
)
    {
    int         insertNum;
    double      distance, radius, cosAlpha, alpha, newAlpha;
    DPoint3d    chord, normalizedTangent;

    /* Compute the number of chords necessary to stroke to tolerance */
    chord.DifferenceOf (*p2, *p1);
    distance = chord.Magnitude ();

    insertNum = 0;
    if (distance > tolerance)
        {
        chord.Scale (chord, 1.0/distance);
        normalizedTangent = *tangent;
        normalizedTangent.Normalize ();

        cosAlpha = chord.DotProduct (normalizedTangent);
        if (fabs(cosAlpha) < COSINE_TOLERANCE)
            {
            alpha = acos (cosAlpha);
            radius = distance/(2.0*sin(alpha));
            if (radius > tolerance)
                {
                newAlpha = acos (1.0 - tolerance/radius);
                insertNum = (int)(1.1 * (alpha/newAlpha + 1));
                }
            }
        }
    return insertNum;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_segmentDisjointSurface                          |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspsurf_segmentDisjointSurface
(
MSBsplineSurface    **segSurfs,     /* <= continuous surfaces */
int                 *nSegSurfs,     /* <= number of continuous surfaces */
MSBsplineSurface    *surfP          /* => possibly disjoint surface */
)
    {
    MSBsplineSurface    *segSurfP;
    bvector<double> uBreak;
    bvector<double> vBreak;
    KnotData uKnots, vKnots;
    uKnots.LoadSurfaceUKnots (*surfP);
    vKnots.LoadSurfaceVKnots (*surfP);

    uKnots.CollectHighMultiplicityActiveKnots ((size_t)surfP->uParams.order, true, uBreak);
    vKnots.CollectHighMultiplicityActiveKnots ((size_t)surfP->vParams.order, true, vBreak);

    int numAllocated = ((int)uBreak.size () - 1) * ((int)vBreak.size () - 1);
    segSurfP = *segSurfs = (MSBsplineSurface*)BSIBaseGeom::Malloc (numAllocated * sizeof(MSBsplineSurface));
    int numOut = 0;
    if (uBreak.size () == 2 && vBreak.size () == 2)
        {
        bspsurf_copySurface (segSurfP, surfP);
        numOut = 1;
        }
    else
        {
        for (size_t iV = 0; iV + 1 < vBreak.size (); iV++)
            {
            for (size_t iU = 0; iU + 1 < uBreak.size (); iU++)
                {
                DPoint2d uv00 = DPoint2d::From (uBreak[iU], vBreak[iV]);
                DPoint2d uv11 = DPoint2d::From (uBreak[iU+1], vBreak[iV+1]);
                if (SUCCESS == bspsurf_segmentSurface (&(segSurfP[numOut]), surfP, &uv00, &uv11))
                        numOut++;
                }
            }
        }
    *nSegSurfs = numOut;

    return SUCCESS;
    }

static int sNumSampleCurve = 200;

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_trimmedPlaneFromCurves                          |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspsurf_trimmedPlaneFromCurves
(
MSBsplineSurface    *surfP,             /* <= trimmed plane */
MSBsplineCurve      *curves,            /* => input curves */
int                 nCurves,            /* => number of input curves */
double              tolerance           /* => stroke tolerance for bounds */
)
    {
    int             i, status, k = -1, j;
    double          yMagnitude, dt = 1.0/(sNumSampleCurve - 1);
    DPoint2d        min, max, delta;
    DVec3d          planeNormal, xVector, yVector;
    DPoint3d        planePoint, *pnt3dP, *endP;
    RotMatrix       rotMatrix;
    MSBsplineCurve  *curveP;
    DPoint3d        *samplePts = (DPoint3d*)BSIBaseGeom::Malloc (sNumSampleCurve * sizeof(DPoint3d));
    DPoint2d        *minBox = (DPoint2d*)BSIBaseGeom::Malloc (nCurves * sizeof(DPoint2d));
    DPoint2d        *maxBox = (DPoint2d*)BSIBaseGeom::Malloc (nCurves * sizeof(DPoint2d));
    double minDistTol = 0.0;
    for (i = 0; i < nCurves; i++)
        {
        double a = curves[i].Resolution ();
        if (a > minDistTol)
            minDistTol = a;
        }

    bspcurv_extractNormal (&planeNormal, &planePoint, NULL, &curves[0]);

    ScopedArray<MSBsplineCurve> curveArray (nCurves);   MSBsplineCurve *tmpCurves = curveArray.GetData ();

    min.x = min.y =   1.0E20;
    max.x = max.y = - 1.0E20;
    for (i=0, curveP=tmpCurves; i<nCurves; i++, curveP++)
        {
        bspcurv_copyCurve (curveP, &curves[i]);

        if (curveP->rational)
            bsputil_unWeightPoles (curveP->poles, curveP->poles,
                                   curveP->weights,
                                   curveP->params.numPoles);
        
        minBox[i].x = minBox[i].y =   1.0E20;
        maxBox[i].x = maxBox[i].y = - 1.0E20;
        
        if (i == 0)
            {
            /* Extract the xVector from the first curve segment */
            if (curveP->params.closed)
                xVector.NormalizedDifference (*(&curveP->poles[0]), *(&curveP->poles[curveP->params.numPoles-1]));
            else
                xVector.NormalizedDifference (*(&curveP->poles[1]), *(&curveP->poles[0]));

            yVector.CrossProduct (planeNormal, xVector);
            if ((yMagnitude = yVector.Magnitude ()) < fc_epsilon)
                {
                rotMatrix_orthogonalFromZRow (&rotMatrix, &planeNormal);
                }
            else
                {
                xVector.CrossProduct (yVector, planeNormal);
                xVector.Normalize ();
                yVector.Scale (yVector, 1.0/yMagnitude);
                yVector.Normalize ();
                rotMatrix.InitFromRowVectors (xVector, yVector, planeNormal);
                }
            }
        rotMatrix.Multiply (curveP->poles, curveP->poles, curveP->params.numPoles);
        
        // Get linestring range directly before reweighting. defect #13915
        if (curveP->params.order == 2)
            {
            for (pnt3dP = curveP->poles, endP = pnt3dP + curveP->params.numPoles; pnt3dP < endP; pnt3dP++)
                {
                if (pnt3dP->x < min.x) min.x = pnt3dP->x;
                if (pnt3dP->x > max.x) max.x = pnt3dP->x;
                if (pnt3dP->y < min.y) min.y = pnt3dP->y;
                if (pnt3dP->y > max.y) max.y = pnt3dP->y;

                if (pnt3dP->x < minBox[i].x) minBox[i].x = pnt3dP->x;
                if (pnt3dP->x > maxBox[i].x) maxBox[i].x = pnt3dP->x;
                if (pnt3dP->y < minBox[i].y) minBox[i].y = pnt3dP->y;
                if (pnt3dP->y > maxBox[i].y) maxBox[i].y = pnt3dP->y;
                }
            }

        if (curveP->rational)
            bsputil_weightPoles (curveP->poles, curveP->poles, curveP->weights, curveP->params.numPoles);

        if (curveP->params.order != 2)
        {
            for (int jj = 0; jj < sNumSampleCurve; jj++)
                bspcurv_evaluateCurvePoint(&samplePts[jj], NULL, curveP, jj*dt);

            for (pnt3dP = samplePts, endP = pnt3dP + sNumSampleCurve; pnt3dP < endP; pnt3dP++)
            {
                if (pnt3dP->x < min.x) min.x = pnt3dP->x;
                if (pnt3dP->x > max.x) max.x = pnt3dP->x;
                if (pnt3dP->y < min.y) min.y = pnt3dP->y;
                if (pnt3dP->y > max.y) max.y = pnt3dP->y;

                if (pnt3dP->x < minBox[i].x) minBox[i].x = pnt3dP->x;
                if (pnt3dP->x > maxBox[i].x) maxBox[i].x = pnt3dP->x;
                if (pnt3dP->y < minBox[i].y) minBox[i].y = pnt3dP->y;
                if (pnt3dP->y > maxBox[i].y) maxBox[i].y = pnt3dP->y;
            }
        }

        if ((maxBox[i].x - minBox[i].x) < 1.0) maxBox[i].x = minBox[i].x + 1.0;
        if ((maxBox[i].y - minBox[i].y) < 1.0) maxBox[i].y = minBox[i].y + 1.0;
        }
        
    if ((max.x - min.x) < 1.0) max.x = min.x + 1.0;
    if ((max.y - min.y) < 1.0) max.y = min.y + 1.0;
    
    for (i=0; i<nCurves; i++)
        {
        if (fabs(maxBox[i].x - max.x) + fabs(maxBox[i].y - max.y) + fabs(minBox[i].x - min.x) 
                        + fabs(minBox[i].y - min.y) < 1E-8 && 
                        tmpCurves[i].params.order == 2 &&
                        tmpCurves[i].params.numPoles == 4 &&
                        tmpCurves[i].params.closed &&
                      ! tmpCurves[i].rational)
                k = i;
        }
    BSIBaseGeom::Free (samplePts);
    BSIBaseGeom::Free (minBox);
    BSIBaseGeom::Free (maxBox);
    
    memset (surfP, 0, sizeof(*surfP));
    surfP->uParams.numPoles = surfP->vParams.numPoles = 2;
    surfP->uParams.order    = surfP->vParams.order    = 2;
    surfP->uParams.numKnots = surfP->vParams.numKnots = 0;
    surfP->uParams.numRules = surfP->vParams.numRules = 2;
    surfP->display.curveDisplay = true;
    surfP->holeOrigin = true;
    surfP->rational = false;
    surfP->type = BSSURF_PLANE;

    if (SUCCESS != (status = bspsurf_allocateSurface (surfP)))
        return status;


    bspknot_computeKnotVector (surfP->uKnots, &surfP->uParams, NULL);
    bspknot_computeKnotVector (surfP->vKnots, &surfP->vParams, NULL);
    if (nCurves == 1 && tmpCurves[0].params.order == 2 &&
                        tmpCurves[0].params.numPoles == 4 &&
                        tmpCurves[0].params.closed &&
                      ! tmpCurves[0].rational &&
                        polyutil_isPolygonConvex ((double*)tmpCurves[0].poles, 4, 3, false))
        {
        /* Special case for a convex 4 sided polygon, can be done with no boundary */
        surfP->poles[0] = tmpCurves[0].poles[0];
        surfP->poles[1] = tmpCurves[0].poles[1];
        surfP->poles[2] = tmpCurves[0].poles[3];
        surfP->poles[3] = tmpCurves[0].poles[2];

        rotMatrix.MultiplyTranspose (surfP->poles, surfP->poles, 4);
        bspcurv_freeCurve (&tmpCurves[0]);
        }
    else
        {
        rotMatrix.Multiply (planePoint);
        delta.x = max.x - min.x;
        delta.y = max.y - min.y;

        surfP->poles[0].x = surfP->poles[2].x = min.x;
        surfP->poles[1].x = surfP->poles[3].x = max.x;
        surfP->poles[0].y = surfP->poles[1].y = min.y;
        surfP->poles[2].y = surfP->poles[3].y = max.y;

        for (i=0; i<4; i++)
            {
            surfP->poles[i].z = planePoint.z;
            rotMatrix.MultiplyTranspose (*(&surfP->poles[i]));
            }
            
        if (k > -1)
            {
            surfP->poles[0] = tmpCurves[k].poles[0];
            surfP->poles[1] = tmpCurves[k].poles[1];
            surfP->poles[2] = tmpCurves[k].poles[3];
            surfP->poles[3] = tmpCurves[k].poles[2];
            rotMatrix.MultiplyTranspose (surfP->poles, surfP->poles, 4);
            }
            
        surfP->boundaries = (BsurfBoundary*)BSIBaseGeom::Calloc (nCurves, sizeof(BsurfBoundary));
        if (tolerance <= 0.0)
            tolerance = (delta.x > delta.y ? delta.x : delta.y)/100.0;

        if (!bspsurf_isPreciseTrimEnabled ())
            {
            DPoint3d *points;
            DPoint2d *pnt2dP;
            BsurfBoundary *boundP = surfP->boundaries;

            for (i=0; i<nCurves; i++)
                {
                boundP->numPoints = 0;
                points = NULL;

                if (bspcurv_evaluateCurve (&points, &tolerance, &boundP->numPoints,
                                        &tmpCurves[i]) == SUCCESS &&
                    boundP->numPoints > 0)
                    {
                    pnt2dP = boundP->points = (DPoint2d*)BSIBaseGeom::Malloc
                                        (boundP->numPoints * sizeof(DPoint2d));
                    pnt3dP = points;
                    for (endP = pnt3dP + boundP->numPoints;
                            pnt3dP < endP;
                                pnt2dP++, pnt3dP++)
                        {
                        if (k == -1)
                            {
                            pnt2dP->x = (pnt3dP->x - min.x)/delta.x;
                            pnt2dP->y = (pnt3dP->y - min.y)/delta.y;
                            }
                        else
                            {
                            rotMatrix.MultiplyTranspose (*pnt3dP);
                            bsprsurf_minDistToSurface (NULL, NULL, pnt2dP, pnt3dP, surfP, &minDistTol);
                            }
                        }
                    /* Force closure */
                    boundP->points[boundP->numPoints-1] = boundP->points[0];
                    surfP->numBounds++;
                    boundP++;
                    BSIBaseGeom::Free (points);
                    bspcurv_freeCurve (&tmpCurves[i]);
                    }
                }
            }
        else
            {
            Transform globalToUV;
            double    sx, sy;
            sx = 1.0 / delta.x;
            sy = 1.0 / delta.y;

            globalToUV.InitFromRowValues (
                    sx,  0, 0, -sx * min.x,
                    0, sy, 0,  -sy * min.y,
                    0,  0, 0,   0
                    );
            
            DPoint3d            *points;
            DPoint2d            *pnt2dP;
            int                 numPts, nSegCurves;
            MSBsplineCurve      *segCurves, *comCurve;
            
            for (i=0; i<nCurves; i++)
                {
                BsurfBoundary *boundP = &surfP->boundaries[i];
                
                nSegCurves = 0;
                points = NULL;
                segCurves = NULL;

                if (k != -1)
                    {
                    if (tmpCurves[i].params.order == 2 &&
                            tmpCurves[i].params.closed &&
                          ! tmpCurves[i].rational)
                        {
                        int  num = tmpCurves[i].params.numPoles;
                        pnt2dP = (DPoint2d*)BSIBaseGeom::Malloc (num * sizeof(DPoint2d));
                        rotMatrix.MultiplyTranspose (tmpCurves[i].poles, tmpCurves[i].poles, num);

                        for (j=0; j<num; j++)
                            {
                            DPoint3d xyzA = tmpCurves[i].poles[j];
                            DPoint3d xyz0;
                            DPoint2d uv0;
                            surfP->ClosestPoint (xyz0, uv0, xyzA);
                            pnt2dP[j] = uv0;
                            }
                        for (j=0; j<num; j++)
                            {
                            tmpCurves[i].poles[j].x = pnt2dP[j].x;
                            tmpCurves[i].poles[j].y = pnt2dP[j].y;
                            tmpCurves[i].poles[j].z = 0.0;
                            }
                        BSIBaseGeom::Free (pnt2dP);
                        }
                    else
                        {
                        if (SUCCESS == bsputil_segmentC1DiscontinuousCurve (&segCurves, &nSegCurves, &tmpCurves[i]))
                            {
                            comCurve = NULL;
                            for (int l=0; l<nSegCurves; l++)
                                {
                                numPts = 0;
                                if (SUCCESS == bspcurv_evaluateCurve (&points, &tolerance, &numPts, &segCurves[l]))
                                    {
                                    pnt2dP = (DPoint2d*)BSIBaseGeom::Malloc (numPts * sizeof(DPoint2d));
                                    rotMatrix.MultiplyTranspose (points, points, numPts);

                                    for (j=0; j<numPts; j++)
                                        {
                                        DPoint3d xyz0;
                                        DPoint2d uv0;
                                        DPoint3d xyzA = points[j];
                                        surfP->ClosestPoint (xyz0, uv0, xyzA);
                                        pnt2dP[j] = uv0;
                                        points[j].x = pnt2dP[j].x;
                                        points[j].y = pnt2dP[j].y;
                                        points[j].z = 0.0;
                                        }
                                    bspcurv_c2CubicInterpolateCurveExt (&segCurves[l], points, NULL, numPts, true, fc_epsilon, NULL, 0 != segCurves[l].params.closed, true, false, true, true);
                                    if (NULL == comCurve)
                                        comCurve = &segCurves[l];
                                    else
                                        bspcurv_appendCurves (comCurve, comCurve, &segCurves[l], false, false);
                                    BSIBaseGeom::Free (pnt2dP);
                                    }
                                }
                            bspcurv_copyCurve  (&tmpCurves[i], comCurve);
                            bspcurv_freeCurve (comCurve);
                            }
                        }
                    BSIBaseGeom::Free (points);
                    BSIBaseGeom::Free (segCurves);
                    }
                else
                    {
                    if (tmpCurves[i].params.order == 2 && tmpCurves[i].rational)
                        {
                        bsputil_unWeightPoles (tmpCurves[i].poles, tmpCurves[i].poles, tmpCurves[i].weights, tmpCurves[i].params.numPoles);
                        for (j=0; j<tmpCurves[i].params.numPoles; j++)
                            tmpCurves[i].weights[j] = 1.0;
                        }
                    bspcurv_transformCurve (&tmpCurves[i], &tmpCurves[i], &globalToUV);
                    }
                bspTrimCurve_allocateAndInsertCyclicC1Segments (&boundP->pFirst, &tmpCurves[i], true);
                bspcurv_freeCurve (&tmpCurves[i]);
                bspTrimCurve_breakCyclicList (&boundP->pFirst);
                bspsurf_strokeSingleTrimLoop (boundP, boundP, 0.0, tolerance, tmpCurves[i].params.numPoles, surfP);
                surfP->numBounds++;
                }
            }
        }

    if (0 == surfP->numBounds)
        surfP->holeOrigin = false;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_transformSurface                                |
|                                                                       |
| author        RayBentley                               1/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspsurf_transformSurface
(
MSBsplineSurface        *outSurfP,          /* <= transformed surface */
MSBsplineSurfaceCP      inSurfP,           /* => input surface */
Transform const         *transformP         /* => transform */
)
    {
    int         status, totalPoles;

    if (outSurfP != inSurfP)
        if (SUCCESS != (status = bspsurf_copySurface (outSurfP, inSurfP)))
            return status;

    if (transformP && ! transformP->IsIdentity ())
        {
        totalPoles = outSurfP->uParams.numPoles * outSurfP->vParams.numPoles;
        if (outSurfP->rational)
            {
            transformP->MultiplyWeighted (outSurfP->poles, outSurfP->poles, outSurfP->weights, totalPoles);
            }
        else
            {
            transformP->Multiply (outSurfP->poles, totalPoles);
            }
        }
    return SUCCESS;
    }



//REMOVE BBB
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_c1Discontinuities                               |
|                                                                       |
| author        RayBentley                               4/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_c1Discontinuities
(
double              **paramPP,
int                 *nParamsP,
MSBsplineSurface    *surfP,
int                 direction,          /* BSSURF_U or BSPCURV_V */
double              tolerance
)
    {
    int             nCurve, status;
    MSBsplineCurve  curve;

    bvector<double>sliceParams;
    bvector<double>allParams;
    /* Note...This routine does not currently  mdl_malloc the memory for paramPP...
        if it is made mdl-visible this is necessary, but it was easier this way. */

    nCurve = direction == BSSURF_U ? surfP->vParams.numPoles : surfP->uParams.numPoles;

    for (int i = 0; i < nCurve; i++)
        {
        if (SUCCESS != (status = bspconv_getCurveFromSurface (&curve, surfP, direction, i)))
            break;
        sliceParams.clear ();
        if (bspcurv_polygonLength (&curve) > tolerance &&
            bspcurv_c1Discontinuities (sliceParams, &curve) == SUCCESS &&
            sliceParams.size () > 0)
            {
            DoubleOps::Append (&allParams, &sliceParams);
            }
        bspcurv_freeCurve (&curve);
        }

    
    size_t nParams = allParams.size ();
    if (nParams > 0)
        {
        /* Weed out duplicates */
        std::sort (allParams.begin (), allParams.end ());
        size_t numAccept = 0;
        for (size_t i=0; i<nParams; i++)
            if (i==0 || (allParams[i] - allParams[numAccept]) > KNOT_TOLERANCE_BASIS)
                allParams[numAccept++] = allParams[i];
        allParams.resize (numAccept);
        
        *nParamsP = (int)numAccept;
        if (paramPP != NULL)
            BSIBaseGeom::MallocAndCopy (paramPP, allParams);
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_freeArray                                       |
|                                                                       |
| author        earlinLutz                               10/94          |
|                                                                       |
| Free an array allocated by other bspline operations.                  |
| The pointer is cleared after freeing.                                 |
| This is so DLM procedures can release spline memory without knowning  |
| which allocation mechanism was used.                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_freeArray
(
void **arrayPP          /* <=> array to be freed */
)
    {
    if( *arrayPP ) msbspline_free( *arrayPP );
    *arrayPP = 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_minAveRowLength                                 |
|                                                                       |
| author        RayBentley                               5/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bspsurf_minAveRowLength
(
MSBsplineSurface    *surfaceP
)
    {
    int         i, j, index;
    double      uDistance, vDistance;
    DPoint3d    pole, lastPole;

    /* Test Rows */
    for (i=0, uDistance=0.0; i<surfaceP->vParams.numPoles; i++)
        {
        for (j=0; j<surfaceP->uParams.numPoles; j++)
            {
            index = i * surfaceP->uParams.numPoles + j;
            if (surfaceP->rational)
                bsputil_unWeightPoles (&pole, surfaceP->poles+index,
                                                 surfaceP->weights+index, 1);
            else
                pole = surfaceP->poles[index];

            if (j)  uDistance += pole.Distance (lastPole);

            lastPole = pole;
            }
        }
    uDistance /= (double) surfaceP->vParams.numPoles;

    /* Test Columns */
    for (i=0, vDistance=0.0; i<surfaceP->uParams.numPoles; i++)
        {
        for (j=0; j<surfaceP->vParams.numPoles; j++)
            {
            index = i + j * surfaceP->uParams.numPoles;
            if (surfaceP->rational)
                bsputil_unWeightPoles (&pole,  surfaceP->poles+index,
                                                  surfaceP->weights+index, 1);
            else
                pole = surfaceP->poles[index];

            if (j)  vDistance += pole.Distance (lastPole);
            lastPole = pole;
            }
        }
    vDistance /= (double) surfaceP->uParams.numPoles;

    return uDistance < vDistance ? uDistance : vDistance;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_isPlane                                         |
|                                                                       |
| author        RayBentley                               8/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool   bspsurf_isPlane
(
MSBsplineSurface    *surfaceP
)
    {
    if (surfaceP->uParams.order == 2 && surfaceP->vParams.order == 2 &&
        surfaceP->uParams.numPoles == 2 && surfaceP->vParams.numPoles == 2 &&
        !surfaceP->rational)
        {
        DPoint3d        delta1, delta2, delta3, delta4;

        /* Check for planar rows */
        delta1.NormalizedDifference (*(&surfaceP->poles[0]), *(&surfaceP->poles[1]));
        delta2.NormalizedDifference (*(&surfaceP->poles[2]), *(&surfaceP->poles[3]));
        if (1.0 - delta1.DotProduct (delta2) > fc_epsilon)
            return false;

        /* Check for planar columns */
        delta3.NormalizedDifference (*(&surfaceP->poles[0]), *(&surfaceP->poles[2]));
        delta4.NormalizedDifference (*(&surfaceP->poles[1]), *(&surfaceP->poles[3]));

        if (1.0 - delta3.DotProduct (delta4) > fc_epsilon)
            return  false;

        /* Check for orthogonal rows & columns */
        return  ((fabs (delta2.DotProduct (delta4)) < fc_epsilon) &&
                 (fabs (delta1.DotProduct (delta3)) < fc_epsilon));
        }
    else
        {
        return false;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_swapSurfaceUV                                   |
|                                                                       |
| author        LuHan                                   4/95            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_swapSurfaceUV
(
MSBsplineSurface    *outP,
MSBsplineSurface    *inP
)
    {
    // EDL 10/06 bspsurf_swapSurfaceUV and bspsurf_swapUV seem to have the same purpose.
    //   bspsurf_swapSurfaceUV implementation (a) didn't touch the boundaries and
    //  (b) made but failed to free an extra copy on heap.
    return bspsurf_swapUV (outP, inP);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_surfacePhysicallyClosed                         |
|                                                                       |
| author        BrianPeters                             11/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_surfacePhysicallyClosed
(
bool                *uClosed,
bool                *vClosed,
MSBsplineSurface    *surf,
double              tolerance
)
    {
    int         totalPoles, uPoles;
    DPoint3d    *pt0, *pt1, *endP;

    uPoles = surf->uParams.numPoles;
    totalPoles = surf->uParams.numPoles * surf->vParams.numPoles;

    if (surf->rational)
        bsputil_unWeightPoles (surf->poles, surf->poles, surf->weights, totalPoles);

    if (surf->vParams.closed)
        *vClosed = true;
    else
        {
        for (*vClosed = true, pt0 = endP = surf->poles,
             pt1 = surf->poles + uPoles * (surf->vParams.numPoles - 1), endP += uPoles;
                *vClosed && (pt0 < endP);
                    pt0++, pt1++)
            *vClosed = pt0->Distance (*pt1) < tolerance;
        }

    if (surf->uParams.closed)
        *uClosed = true;
    else
        {
        for (*uClosed = true, pt0 = endP = surf->poles,
             pt1 = surf->poles + uPoles - 1, endP += totalPoles;
                *uClosed && (pt0 < endP);
                    pt0 += uPoles, pt1 += uPoles)
            *uClosed = pt0->Distance (*pt1) < tolerance;
        }

    if (surf->rational)
        bsputil_weightPoles (surf->poles, surf->poles, surf->weights, totalPoles);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_normalizeSurface                                |
|                                                                       |
| author        RayBentley                                1/94          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspsurf_normalizeSurface
(
MSBsplineSurface    *surfaceP
)
    {
    DPoint2d        min, max;

    bsputil_extractParameterRange (&min.x, &max.x, surfaceP->uKnots, &surfaceP->uParams);
    bsputil_extractParameterRange (&min.y, &max.y, surfaceP->vKnots, &surfaceP->vParams);

    if (min.x != 0.0 || min.y != 0.0 || max.x != 1.0 || max.y != 1.0)
        {
        Transform transform;
        double sx = 1.0 / (max.x - min.x);
        double sy = 1.0 / (max.y - min.y);
        transform.InitFromRowValues (
                 sx, 0.0, 0.0, - min.x * sx,
                0.0,  sy, 0.0, - min.y * sy,
                0.0, 0.0, 1.0, 0.0
                );
        bspsurf_transformAllBoundaries (surfaceP, &transform);
        }


    bspknot_normalizeKnotVector (surfaceP->uKnots,
                                    surfaceP->uParams.numPoles,
                                    surfaceP->uParams.order,
                                    surfaceP->uParams.closed);
    bspknot_normalizeKnotVector (surfaceP->vKnots,
                                    surfaceP->vParams.numPoles,
                                    surfaceP->vParams.order,
                                    surfaceP->vParams.closed);
    }


typedef struct addknotparams
    {
    double          value;
    double          knotTol;
    int             newMult;
    int             addToCurrent;
    } AddKnotParams;



#if !defined (FONTIMPORTER)
#if defined (debug)
/*----------------------------------------------------------------------+
|                                                                       |
| name          displaySurf                                             |
|                                                                       |
| author        BFP                                     3/91            |
|                                                                       |
+----------------------------------------------------------------------*/
void     displaySurf
(
MSBsplineSurface    *surface,
int                 drawMode
)
    {
    int                 i;
    double              knots[2 * MAX_ORDER];
    MSBsplineSurface    srf;
    ElementDescr        *edP;

    memset (knots, 0, MAX_ORDER * sizeof(double));
    for (i = MAX_ORDER; i < 2 * MAX_ORDER; i++)
        knots[i] = 1.0;

    memset (&srf, 0 , sizeof(srf));
    srf.display.curveDisplay = true;
    srf.rational = surface->rational;
    srf.uParams.numPoles = srf.uParams.order = surface->uParams.order;
    srf.vParams.numPoles = srf.vParams.order = surface->vParams.order;
    srf.vParams.numRules = srf.uParams.numPoles;
    srf.uParams.numRules = srf.vParams.numPoles;
    srf.uKnots = knots + MAX_ORDER - srf.uParams.order;
    srf.vKnots = knots + MAX_ORDER - srf.vParams.order;
    srf.poles = surface->poles;
    srf.weights = surface->weights;

    bspsurf_createSurface (&edP, NULL, &srf);
    mdlElmdscr_display (edP, 0, drawMode);
    mdlElmdscr_freeAll (&edP);
    }
#endif
#endif /* !FONTIMPORTER */

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnotToSurf                                   |
|                                                                       |
| author        BFP                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspknot_addKnotToSurf
(
MSBsplineCurve  *curve,
AddKnotParams *akp,
int i
)
    {
    return bspknot_addKnot (curve, akp->value, akp->knotTol, akp->newMult,
                            akp->addToCurrent);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnotSurface                                  |
|                                                                       |
| author        BFP                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_addKnotSurface
(
MSBsplineSurface    *surface,
double              uv,
double              knotTolerance,
int                 newMult,
int                 addToCurrent,
int                 direction
)
    {
    int                 status;
    AddKnotParams       akp;
    MSBsplineSurface    surf;

    akp.value        = uv;
    akp.knotTol      = knotTolerance;
    akp.newMult      = newMult;
    akp.addToCurrent = addToCurrent;

    if (bspsurf_copySurface (&surf, surface))
        return ERROR;

    if (SUCCESS != (status = bspproc_processSurfaceByCurves (&surf, (PFBCurveVoidPInt)bspknot_addKnotToSurf,
                                                             &akp, direction)))
        goto wrapup;

    bspsurf_freeSurface (surface);
    *surface = surf;

wrapup:
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_pointOnSurface                                  |
|               returns true if point is on displayed part of surface   |
| author        BrianPeters                             5/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_pointOnSurface
(
DPoint2d            *uv,
MSBsplineSurfaceCP surf
)
    {
    if (! bsputil_validParameterSurface (uv, surf->uKnots, &surf->uParams,
                                             surf->vKnots, &surf->vParams))
        return false;

    return bsputil_pointInBounds (uv, surf->boundaries, surf->numBounds, (0 != surf->holeOrigin));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_addBoundaries                                   |
|                                                                       |
| author        BrianPeters                             4/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_addBoundaries
(
MSBsplineSurface    *surface,
BsurfBoundary       **bounds,
int                 numBounds
)
    {
    BsurfBoundary   *bndP;

    if (surface->numBounds)
        {
        if (NULL == (bndP = (BsurfBoundary*)BSIBaseGeom::Realloc (surface->boundaries,
                            (surface->numBounds + numBounds) * sizeof(BsurfBoundary))))
            return ERROR;
        surface->boundaries = bndP;
        memcpy (surface->boundaries + surface->numBounds, *bounds,
                numBounds * sizeof(BsurfBoundary));
        BSIBaseGeom::Free (*bounds);
        }
    else
        {
        surface->boundaries = *bounds;
        }

    *bounds = NULL;
    surface->numBounds += numBounds;
    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_addSingleBoundary                               |
|                                                                       |
| author        RayBentley                              01/99           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt     bsputil_addSingleBoundary
(
MSBsplineSurface    *pSurface,
int                 numPoints,
DPoint2d            *pBoundPoints
)
    {
    BsurfBoundary   boundary, *pBoundaries;

    if (NULL == (boundary.points = (DPoint2dP)BSIBaseGeom::Malloc (numPoints * sizeof(DPoint2d))))
        return      ERROR;

    if (NULL == (pBoundaries = (BsurfBoundaryP)BSIBaseGeom::Realloc (pSurface->boundaries, (pSurface->numBounds + 1) * sizeof(BsurfBoundary))))
        {
        BSIBaseGeom::Free (boundary.points);
        return ERROR;
        }

    if (SUCCESS != bspUtil_initializeBsurfBoundary(&boundary))
	return ERROR;
    boundary.numPoints = numPoints;
    memcpy (boundary.points, pBoundPoints, numPoints * sizeof(DPoint2d));

    pSurface->boundaries = pBoundaries;
    pSurface->boundaries[pSurface->numBounds++] = boundary;

    return SUCCESS;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          angleBetween                                            |
|                                                                       |
| author        RayBentley                              05/98           |
|                                                                       |
+----------------------------------------------------------------------*/
static double     angleBetween
(
DPoint3d        *normal1P,
DPoint3d        *normal2P
)
    {
    if ((normal1P->x != 0.0 || normal1P->y != 0.0 || normal1P->z != 0.0) &&
        (normal2P->x != 0.0 || normal2P->y != 0.0 || normal2P->z != 0.0))
        return fabs (((DVec3d*)normal1P)->AngleTo (*((DVec3d*)normal2P)));
    else
        return 0.0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_nIsoParamsFromAngle                             |
|                                                                       |
| author        RayBentley                              05/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     bsputil_nIsoParamsFromAngle
(
double          angle,
int             nIsoParams
)
    {
    int         n = (int)floor (0.5 + (double) nIsoParams * (fabs (angle) - fc_epsilon) / msGeomConst_2pi );

    return n > 0 ?  (n + 1) : 2;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_calculateNumRules                               |
|                                                                       |
| author        RayBentley                              05/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsputil_calculateNumRules
(
MSBsplineSurface        *surfaceP,
int                     uFullCircleIsoparametrics,
int                     vFullCircleIsoparametrics,
double                  closureTolerance
)
    {
    int                 i, j, nUPoles = surfaceP->uParams.numPoles, nVPoles = surfaceP->vParams.numPoles, nPoles = nUPoles * nVPoles;
    double              rowAngle, uAngle, vAngle;
    bool                first;
    DPoint3d            *poleP, *lastPoleP, *endP, normal, lastNormal;

    if (surfaceP->rational)
        bsputil_unWeightPoles (surfaceP->poles, surfaceP->poles, surfaceP->weights, nPoles);

    uAngle = 0.0;
    for (i=0; i<nVPoles; i++)
        {
        rowAngle =0.0;
        lastPoleP = surfaceP->poles + i * nUPoles;
        for (first = true, poleP = lastPoleP + 1, endP = lastPoleP + nUPoles; poleP < endP; poleP++, first = false)
            {
            if (normal.NormalizedDifference (*lastPoleP, *poleP) < fc_epsilon)
                normal.x = normal.y = normal.z = 0.0;

            if (!first)
                rowAngle += angleBetween (&normal, &lastNormal);

            lastPoleP = poleP;
            lastNormal = normal;
            }
        if (rowAngle > uAngle)
            uAngle = rowAngle;
        }
    vAngle = 0.0;
    for (i=0; i<nUPoles; i++)
        {
        rowAngle =0.0;
        lastPoleP = surfaceP->poles + i;
        for (first = true, j=1, poleP = lastPoleP + nUPoles; j<nVPoles;  j++, poleP += nUPoles, first = false)
            {
            if (normal.NormalizedDifference (*lastPoleP, *poleP) < fc_epsilon)
                normal.x = normal.y = normal.z = 0.0;

            if (!first)
                rowAngle += angleBetween (&normal, &lastNormal);

            lastPoleP = poleP;
            lastNormal = normal;
            }
        if (rowAngle > vAngle)
            vAngle = rowAngle;
        }
    if (surfaceP->rational)
        bsputil_weightPoles (surfaceP->poles, surfaceP->poles, surfaceP->weights, nPoles);

    surfaceP->uParams.numRules = bsputil_nIsoParamsFromAngle (vAngle, uFullCircleIsoparametrics);
    surfaceP->vParams.numRules = bsputil_nIsoParamsFromAngle (uAngle, vFullCircleIsoparametrics);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_ruledSurfaceFromCompatibleCurves                |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_ruledSurfaceFromCompatibleCurves
(
MSBsplineSurface    *surface,
MSBsplineCurve      *curve1,
MSBsplineCurve      *curve2
)
    {
    int         i, uPoles;

    /* Clear the entire structure so only the nonZero values need be set */
    memset (surface, 0, sizeof(*surface));

    surface->display.curveDisplay = true;
    surface->uParams = curve1->params;
    surface->vParams.order = 2;
    surface->vParams.numPoles = 2;
    surface->rational = curve1->rational || curve2->rational;

    /* Set the number of rule lines equal to poles in opposite direction */
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    if (bspsurf_allocateSurface (surface))
        return ERROR;

    for (i=0, uPoles=surface->uParams.numPoles; i < curve1->params.numPoles;
         i++, uPoles++)
        {
        surface->poles[i] = curve1->poles[i];
        surface->poles[uPoles] = curve2->poles[i];
        if (surface->rational)
            {
            surface->weights[i] = curve1->rational ? curve1->weights[i] : 1.0;
            surface->weights[uPoles] = curve2->rational ? curve2->weights[i] : 1.0;
            }
        }

    /* Set the full knot vectors */
    memcpy (surface->uKnots, curve1->knots,
            bspknot_numberKnots (surface->uParams.numPoles, surface->uParams.order,
                                    surface->uParams.closed) * sizeof(double));

    surface->vKnots[0] = surface->vKnots[1] = 0.0;
    surface->vKnots[2] = surface->vKnots[3] = 1.0;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_ruledSurface                                    |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bspsurf_ruledSurface
(
MSBsplineSurface    *surface,
MSBsplineCurve      *curve1,
MSBsplineCurve      *curve2
)
    {
    return bsputil_ruledSurfaceFromCompatibleCurves (surface, curve1, curve2);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_ruledSurfaceFromCompatibleCopiesOfCurves
(
MSBsplineSurface    *surface,
MSBsplineCurve      *inCurve1,
MSBsplineCurve      *inCurve2
)
    {
    int             status;
    MSBsplineCurve  curve1, curve2;

    memset (&curve1, 0, sizeof(curve1));
    memset (&curve2, 0, sizeof(curve2));

    if (SUCCESS != (status = bspcurv_copyCurve (&curve1, inCurve1)) ||
        SUCCESS != (status = bspcurv_copyCurve (&curve2, inCurve2)))
        goto wrapup;

    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&curve1, &curve2)))
        goto wrapup;

    status = bspsurf_ruledSurface (surface, &curve1, &curve2);

wrapup:
    bspcurv_freeCurve (&curve1);
    bspcurv_freeCurve (&curve2);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractIsoIntersects                            |
|                                                                       |
| author        RayBentley                                2/94          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_extractIsoIntersects
(
bvector <double> &params,
double              value,
MSBsplineSurface    *surfaceP,
bool                horizontal
)

    {
    int             i, side, initialSide;
    DPoint2d        *pointP, *prevP = NULL, *endP, *startP;

    params.clear ();
    // NOTE: if hole includes edge of parameter space, then this may return a false intersection! (cf. TR #127137)
    if (0 == surfaceP->numBounds || ! surfaceP->holeOrigin)
        {
        params.push_back (0.0);
        params.push_back (1.0);
        }

    for (i=0; i<surfaceP->numBounds; i++)
        {
        initialSide = side = 0;

        for (startP = pointP = surfaceP->boundaries[i].points,
             endP = pointP + surfaceP->boundaries[i].numPoints - 2;
                pointP <= endP;
                    pointP++)
            {
            if (horizontal)
                {
                if (pointP->y < value)
                    {
                    if (side == 0)
                        {
                        side = initialSide = -1;
                        }
                    else if (side > 0)
                        {
                        params.push_back (prevP->x + (value - prevP->y) *
                                                      (pointP->x - prevP->x) /
                                                      (pointP->y - prevP->y));
                        side = -1;
                        }
                    }
                else if (pointP->y > value)
                    {
                    if (side == 0)
                        {
                        side = initialSide = 1;
                        }
                    else if (side < 0)
                        {
                        params.push_back (prevP->x + (value - prevP->y) *
                                                      (pointP->x - prevP->x) /
                                                      (pointP->y - prevP->y));
                        side = 1;
                        }
                    }
                }
            else
                {
                if (pointP->x < value)
                    {
                    if (side == 0)
                        {
                        side = initialSide = -1;
                        }
                    else if (side > 0)
                        {
                        params.push_back (prevP->y + (value - prevP->x) *
                                                      (pointP->y - prevP->y) /
                                                      (pointP->x - prevP->x));
                        side = -1;
                        }
                    }
                else if (pointP->x > value)
                    {
                    if (side == 0)
                        {
                        side = initialSide = 1;
                        }
                    else if (side < 0)
                        {
                        params.push_back (prevP->y + (value - prevP->x) *
                                                      (pointP->y - prevP->y) /
                                                      (pointP->x - prevP->x));
                        side = 1;
                        }
                    }
                }
            prevP = pointP;
            }
        if (initialSide * side < 0)
            {
            if (horizontal)
                params.push_back (startP->y == endP->y ? startP->x :
                                   endP->x + (value - endP->y) *
                                   (startP->x - endP->x) /
                                   (startP->y - endP->y));
            else
                params.push_back (startP->x == endP->x ? startP->y :
                                   endP->y + (value - endP->x) *
                                   (startP->y - endP->y) /
                                   (startP->x - endP->x));
            }
        }
    std::sort (params.begin (), params.end ());

    return (int) params.size ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractIsoIntersects                            |
|                                                                       |
| author        RayBentley                                2/94          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_extractIsoIntersects
(
double              **paramPP,
double              value,
MSBsplineSurface    *surfaceP,
bool                horizontal
)
    {
    bvector<double>params;
    bsputil_extractIsoIntersects (params, value, surfaceP, horizontal);
    return BSIBaseGeom::MallocAndCopy (paramPP, params);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_segmentC1DiscontinuousCurve                     |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_segmentC1DiscontinuousCurve
(
MSBsplineCurve  **segCurves,    /* <= continuous curves */
int             *nSegCurves,    /* <= number of continuous curves */
MSBsplineCurveCP curveP         /* => possibly disjoint curve */
)
    {
    bvector<MSBsplineCurvePtr> segments;
    bvector<double> fractions;
    curveP->GetC1DiscontinuousCurves (fractions, segments);
    *nSegCurves = 0;
    *segCurves = NULL;
    MSBsplineCurve *segCurveP;
    if (segments.size () > 0)
        {
        segCurveP = *segCurves =
            (MSBsplineCurve *) BSIBaseGeom::Malloc (segments.size ()  * sizeof(MSBsplineCurve));
        if (NULL == segCurveP)
            {
            *nSegCurves = 0;
            return ERROR;
            }
        for (size_t i = 0; i < segments.size (); i++)
            {
            segCurveP[i] = *segments[i];
            segments[i]->Zero ();       // We've snarfed the pointers, just zero it out !!!
            *nSegCurves += 1;
            }
        }
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in the
*       u-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as periodic.
*       The remedy is usually a call to bsprsurf_openSurface.
* @remarks If this function returns true, then the first/last columns of poles are equal,
*       e.g., the surface is geometrically closed but not periodically defined in the
*       u-parameter direction.
* @see mdlBspline_surfaceShouldBeOpenedInV, mdlBspline_curveShouldBeOpened
* @bsimethod                                                    DavidAssaf      04/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_surfaceShouldBeOpenedInU
(
const MSBsplineSurface  *pSurface
)
    {
    if (pSurface && pSurface->uParams.closed)
        {
        int nKnots = bspknot_numberKnots (pSurface->uParams.numPoles, pSurface->uParams.order, true);

        return mdlBspline_knotsShouldBeOpenedInU (pSurface->uKnots, nKnots, pSurface->poles, pSurface->rational ? pSurface->weights : NULL,
                                                  pSurface->uParams.numPoles, pSurface->vParams.numPoles, pSurface->uParams.order, true);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in
*       the v-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as
*       periodic.  The remedy is usually a call to bsprsurf_openSurface.
* @remarks If pPoles is not NULL and this function returns true, then the first/last
*       rows of poles are equal, e.g., the surface is geometrically closed but not
*       periodically defined in the v-parameter direction.  If pPoles is NULL and this
*       function returns true, then the first/last rows of poles are *usually* equal,
*       but not always.
* @param pKnots     IN      full v-knot vector (or at least the first 2*order v-knots)
* @param numKnots   IN      number of knots given
* @param pPoles     IN      full pole grid (optional)
* @param pWeights   IN      full weight grid if rational (optional)
* @param numPolesU  IN      (if pPoles) number of poles in u-parameter direction (#poles per row)
* @param numPolesV  IN      (if pPoles) number of poles in v-parameter direction (#poles per column)
* @param order      IN      v-order of B-spline surface
* @param closed     IN      v-closure of B-spline surface
* @see mdlBspline_knotsShouldBeOpenedInU, mdlBspline_knotsShouldBeOpened
* @bsimethod                                                    DavidAssaf      04/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_knotsShouldBeOpenedInV
(
const double*   pKnots,
int             numKnots,
const DPoint3d* pPoles,
const double*   pWeights,
int             numPolesU,
int             numPolesV,
int             order,
int             closed
)
    {
    // If closed, look for start knot = 0 with multiplicity order (using small tolerance)
    // and containing order / 2 interior knots, and verify equality of first/last rows of poles.
    // Assume nondecreasing knot vector.
    int start = order - 1;

    if (pKnots && numKnots >= start && closed)
        {
        int i, numInteriorStartKnots, mult;

        // V7 nonperiodic closure only occurs on normalized knots
        if (pKnots[start] != 0.0)
            return false;

        // check knots after start
        for (i = start + 1, mult = 1; i < numKnots; i++, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        numInteriorStartKnots = mult - 1;
        if (numInteriorStartKnots != order / 2)
            return false;

        // check knots before start
        for (i = start - 1; i >= 0; i--, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        if (mult != order)
            return false;

        // check first/last rows of poles as in bsprsurf_closeSurface
        if (pPoles && numPolesU > 0 && numPolesV > 1)
            {
            int col, colEnd;
            double  tol = bsputil_pointTolerance (pPoles, pWeights, numPolesU * numPolesV);

            for (col = 0, colEnd = numPolesU * (numPolesV - 1); col < numPolesU; col++, colEnd++)
                {
                if (NULL != pWeights)
                    {
                    if (!bsputil_isSameRationalPointTolerance (&pPoles[col], pWeights[col],
                                                      &pPoles[colEnd], pWeights[colEnd], tol))
                        return false;
                    }
                else
                    {
                    if (!bsputil_isSamePointTolerance (&pPoles[col], &pPoles[colEnd], tol))
                        return false;
                    }

                }
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in
*       the u-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as
*       periodic.  The remedy is usually a call to bsprsurf_openSurface.
* @remarks If pPoles is not NULL and this function returns true, then the first/last
*       columns of poles are equal, e.g., the surface is geometrically closed but not
*       periodically defined in the u-parameter direction.  If pPoles is NULL and this
*       function returns true, then the first/last columns of poles are *usually* equal,
*       but not always.
* @param pKnots     IN      full u-knot vector (or at least the first 2*order u-knots)
* @param numKnots   IN      number of knots given
* @param pPoles     IN      full pole grid (optional)
* @param pWeights   IN      full weight grid if rational (optional)
* @param numPolesU  IN      (if pPoles) number of poles in u-parameter direction (#poles per row)
* @param numPolesV  IN      (if pPoles) number of poles in v-parameter direction (#poles per column)
* @param order      IN      u-order of B-spline surface
* @param closed     IN      u-closure of B-spline surface
* @see mdlBspline_knotsShouldBeOpenedInV, mdlBspline_knotsShouldBeOpened
* @bsimethod                                                    DavidAssaf      04/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_knotsShouldBeOpenedInU
(
const double*   pKnots,
int             numKnots,
const DPoint3d* pPoles,
const double*   pWeights,
int             numPolesU,
int             numPolesV,
int             order,
int             closed
)
    {
    // If closed, look for start knot = 0 with multiplicity order (using small tolerance)
    // and containing order / 2 interior knots, and verify equality of first/last columns of poles.
    // Assume nondecreasing knot vector.
    int start = order - 1;

    if (pKnots && numKnots >= start && closed)
        {
        int i, numInteriorStartKnots, mult;

        // V7 nonperiodic closure only occurs on normalized knots
        if (pKnots[start] != 0.0)
            return false;

        // check knots after start
        for (i = start + 1, mult = 1; i < numKnots; i++, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        numInteriorStartKnots = mult - 1;
        if (numInteriorStartKnots != order / 2)
            return false;

        // check knots before start
        for (i = start - 1; i >= 0; i--, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        if (mult != order)
            return false;

        // check first/last columns of poles as in bsprsurf_closeSurface
        if (pPoles && numPolesU > 1 && numPolesV > 0)
            {
            int row, rowStart, nextRowStart;
            double  tol = bsputil_pointTolerance (pPoles, pWeights, numPolesU * numPolesV);

            for (row = rowStart = 0, nextRowStart = numPolesU; row < numPolesV; row++, rowStart = nextRowStart, nextRowStart += numPolesU)
                {
                if (NULL != pWeights)
                    {
                    if (!bsputil_isSameRationalPointTolerance (&pPoles[rowStart], pWeights[rowStart],
                                                          &pPoles[nextRowStart - 1], pWeights[nextRowStart - 1], tol))
                        return false;
                    }
                else
                    {
                    if (!bsputil_isSamePointTolerance (&pPoles[rowStart],
                                                      &pPoles[nextRowStart - 1], tol))
                        return false;
                    }
                }
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in the
*       v-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as periodic.
*       The remedy is usually a call to bsprsurf_openSurface.
* @remarks If this function returns true, then the first/last rows of poles are equal,
*       e.g., the surface is geometrically closed but not periodically defined in the
*       v-parameter direction.
* @see mdlBspline_surfaceShouldBeOpenedInU, mdlBspline_curveShouldBeOpened
* @bsimethod                                                    DavidAssaf      04/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_surfaceShouldBeOpenedInV
(
const MSBsplineSurface  *pSurface
)
    {
    if (pSurface && pSurface->vParams.closed)
        {
        int nKnots = bspknot_numberKnots (pSurface->vParams.numPoles, pSurface->vParams.order, true);

        return mdlBspline_knotsShouldBeOpenedInV (pSurface->vKnots, nKnots, pSurface->poles, pSurface->rational ? pSurface->weights : NULL,
                                                  pSurface->uParams.numPoles, pSurface->vParams.numPoles, pSurface->vParams.order, true);
        }

    return false;
    }

typedef int (*PFFlushDPoint3dPDoublePIntVoidP)(DPoint3d*,double*,int,void*);
typedef int (*PFFlushDPoint3dPIntVoidP)(DPoint3d*,int,void*);
#define MAX_CLIPBATCH_MINUS_1 (MAX_CLIPBATCH - 1)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    01/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspstrok_vectorOutput
(
DSegment3d       *stroke,               /* => stroke vector */
PFFlushDPoint3dPIntVoidP         flushFunc,             /* => function to flush the buffers */
int             *numStrokes,           /* => number of strokes in buffers */
DPoint3d        *buffer,               /* => buffer of stroke points */
void            *argsP                 /* => passed through to flushFunc */
)
    {
    /* Leave this test in as it supports discontinuity in a B-spline (which arises
        from the conversion of group holes). */
    if (*numStrokes && ! bsputil_isSamePoint (&stroke->point[0], buffer + *numStrokes - 1))
        {
        (*flushFunc) (buffer, *numStrokes, argsP);
        *numStrokes = 0;
        }

    /* If the buffer is empty ... */
    if (! *numStrokes)
        {
        buffer[0] = stroke->point[0];
        *numStrokes = 1;
        }

    buffer[*numStrokes] = stroke->point[1];
    *numStrokes += 1;

    if (*numStrokes >= MAX_CLIPBATCH_MINUS_1)
        {
        (*flushFunc) (buffer, *numStrokes, argsP);
        buffer[0] = buffer[*numStrokes - 1];
        *numStrokes = 1;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspstrok_displayBoundaries
(
MSBsplineSurface    *surf,             /* => surface with boundaries to stroke */
int                 (*stopFunc)(),     /* => function to abort stroking */
int                 (*strokeFunc)(),   /* => function to process strokes -- PFDPoint3dPIntVoidP*/
void                *argP              /* => passed through to strokeFunc */
)
    {
    int              lastEdge, thisEdge, numStrokes;
    DPoint2d         *bP, *endP;

    DPoint3d         strokeBuffer[MAX_CLIPBATCH+1];
    DSegment3d      chord;
    BsurfBoundary    *bndP, *endB;

    if (surf->numBounds == 0)
        return SUCCESS;

    numStrokes = 0;


    for (bndP = endB = surf->boundaries, endB += surf->numBounds; bndP < endB; bndP++)
        {
        if (stopFunc && (*stopFunc) ())
            return ERROR;

        bP = endP = bndP->points;
        bspsurf_evaluateSurfacePoint (&chord.point[1], NULL, NULL, NULL, bP->x, bP->y, surf);
        thisEdge = bsputil_edgeCode (bP, 0.0);
        for (endP += bndP->numPoints, bP++; bP < endP; bP++)
            {
            chord.point[0] = chord.point[1];
            lastEdge  = thisEdge;
            bspsurf_evaluateSurfacePoint (&chord.point[1], NULL, NULL, NULL, bP->x, bP->y, surf);
            thisEdge = bsputil_edgeCode (bP, 0.0);

            /* If both points are on the same edge then do not stroke this segment. */
            if (! (lastEdge & thisEdge))
                bspstrok_vectorOutput (&chord, (PFDPoint3dPIntVoidP)strokeFunc, &numStrokes, strokeBuffer, argP);
            }

        ((PFDPoint3dPIntVoidP)strokeFunc) (strokeBuffer, numStrokes, argP);
        numStrokes = 0;
        }

    return SUCCESS;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
