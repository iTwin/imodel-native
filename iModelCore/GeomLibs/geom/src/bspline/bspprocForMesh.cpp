/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspprocForMesh.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspproc_prepareSurface
(
MSBsplineSurface    *bezier,           /* <= bezier surface */
int                 *uNumSegs,         /* <= number of segs U */
int                 *vNumSegs,         /* <= number of segs V */
int                 **uStarts,         /* <= offsets of segments U */
int                 **vStarts,         /* <= offsets of segments V */
MSBsplineSurface    *bspline           /* => Bspline surface */
)
    {
    int         i, *iP, status, numDist;
    double      *distKnot, knotTol=KNOT_TOLERANCE_BASIS;

    memset (bezier, 0, sizeof(*bezier));

    if (SUCCESS != (status = bsprsurf_openSurface (bezier, bspline, 0.0, 0)) ||
        SUCCESS != (status = bsprsurf_openSurface (bezier, bezier, 0.0, 1)) ||
        SUCCESS != (status = bsprsurf_makeBezierSurface (bezier, bezier)))
        return status;

    numDist = bspknot_numberKnots (bezier->uParams.numPoles, bezier->uParams.order,
                                   bezier->uParams.closed);
    distKnot = NULL;    *uStarts = NULL;
    if (NULL == (*uStarts = (int*)msbspline_malloc (numDist * sizeof(int),    HEAPSIG_BPRC)) ||
        NULL == (distKnot = (double*)msbspline_malloc (numDist * sizeof(double), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    bspknot_getKnotMultiplicity (distKnot, *uStarts, uNumSegs, bezier->uKnots,
                                 bezier->uParams.numPoles, bezier->uParams.order,
                                 bezier->uParams.closed, knotTol);
    *uNumSegs -= 1;

    numDist = bspknot_numberKnots (bezier->vParams.numPoles, bezier->vParams.order,
                                   bezier->vParams.closed);
    *vStarts = NULL;
    if (NULL == (*vStarts = (int*)msbspline_malloc  (numDist * sizeof(int), HEAPSIG_BPRC)) ||
        NULL == (distKnot = (double*)msbspline_realloc (distKnot, numDist*sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }
    bspknot_getKnotMultiplicity (distKnot, *vStarts, vNumSegs, bezier->vKnots,
                                 bezier->vParams.numPoles, bezier->vParams.order,
                                 bezier->vParams.closed, knotTol);
    *vNumSegs -= 1;

    /* Find offset of start of segs from knot multiplicity array */
    (*uStarts)[0] = 0;
    for (i=1, iP=(*uStarts)+1; i < *uNumSegs; i++, iP++)
        *(iP) += *(iP-1);
    (*vStarts)[0] = 0;
    for (i=1, iP=(*vStarts)+1; i < *vNumSegs; i++, iP++)
        *(iP) += *(iP-1);

wrapup:
    if (distKnot)       msbspline_free (distKnot);

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_initializeBezierPatch
(
MSBsplineSurface        *patchP,            /* <= bezier patch */
MSBsplineSurface        *surfP              /* => surface (bezier prepped) */
)
    {
    int                 numPatchPoles;

    /* bezier is the patch under consideration */
    numPatchPoles = surfP->uParams.order * surfP->vParams.order;
    if (NULL ==
        (patchP->poles = (DPoint3d*)msbspline_malloc (numPatchPoles * sizeof(DPoint3d), HEAPSIG_BPRC)))
        return ERROR;

    if (surfP->rational &&
        NULL ==
        (patchP->weights = (double*)msbspline_malloc (numPatchPoles * sizeof(double), HEAPSIG_BPRC)))
        return ERROR;

    patchP->rational         = surfP->rational;
    patchP->uParams.numPoles =
    patchP->uParams.order    = surfP->uParams.order;
    patchP->vParams.numPoles =
    patchP->vParams.order    = surfP->vParams.order;
    patchP->uParams.closed   =
    patchP->vParams.closed   =
    patchP->uParams.numKnots =
    patchP->vParams.numKnots = 0;

    patchP->uKnots           = bspknot_bezierKnotVector (patchP->uParams.order);
    patchP->vKnots           = bspknot_bezierKnotVector (patchP->vParams.order);

    patchP->holeOrigin       = surfP->holeOrigin;
    patchP->numBounds        = surfP->numBounds;
    patchP->boundaries       = surfP->boundaries;

    return SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE