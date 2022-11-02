/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_freeSurface
(
MSBsplineSurface *surface
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_allocateSurface
(
MSBsplineSurface    *surface
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_copyBoundaries
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_copySurface
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP  input
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_computeBoundarySpans
(
double                    **spans,
double                    value,
const MSBsplineSurface    *bspline,
int                       horizontal
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_matchCurveParams
(
MSBsplineSurface    *surface,
MSBsplineCurveCP    curve,
int                 direction
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_make2SurfacesCompatible
(
MSBsplineSurface    *surface0,
MSBsplineSurface    *surface1,
int                 direction
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP bool     bspsurf_isDegenerateEdge
(
int                 edgeCode,
MSBsplineSurfaceCP  surf,
double              tolerance
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP bool      bspsurf_isSolid
(
MSBsplineSurface    *surf,
double              tolerance
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_isPhysicallyClosed
(
bool                *uClosed,
bool                *vClosed,
MSBsplineSurface    *surf
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
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
const MSBsplineSurface *surfP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_evaluateSurfacePoint
(
DPoint3d                    *pointP,
double                      *weightP,
DPoint3d                    *dPdU,
DPoint3d                    *dPdV,
double                      u,
double                      v,
const MSBsplineSurface      *surfP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_makeRationalSurface
(
MSBsplineSurface    *out,
const MSBsplineSurface    *in
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_swapUV
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP in
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_reverseSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP  in,
int                 direction
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_evaluateSurface
(
DPoint3d                    **pts,
DPoint2d                    *data,
int                         *numPts,
MSBsplineSurfaceCP  in
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_segmentSurface
(
MSBsplineSurface    *segment,
MSBsplineSurfaceCP   surface,
DPoint2dP          initial,
DPoint2dP          final
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_longPath
(
DPoint2d        *path,
int             *length,
DPoint2d        *param,
DPoint2d        *lastParam
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_shortPath
(
DPoint2d        *path,
int             *length,
DPoint2d        *param,
DPoint2d        *lastParam
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_imposeBoundaryBySweptCurve
(
MSBsplineSurface    *surface,
MSBsplineCurveCP    curve,
double              tolerance,
DVec3dCP            direction,
DPoint3d            **surfPoints,       /* OUT     points on surface, or NULL */
int                 *numSurfPts
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int     bspline_spansReqd
(
DPoint3d        *tangent,
DPoint3d        *p1,
DPoint3d        *p2,
double          tolerance
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int bspsurf_segmentDisjointSurface
(
MSBsplineSurface    **segSurfs,     /* OUT     continuous surfaces */
int                 *nSegSurfs,     /* OUT     number of continuous surfaces */
MSBsplineSurface    *surfP          /* IN      possibly disjoint surface */
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int bspsurf_trimmedPlaneFromCurves
(
MSBsplineSurface    *surfP,             /* OUT     trimmed plane */
MSBsplineCurve      *curves,            /* IN      input curves */
int                 nCurves,            /* IN      number of input curves */
double              tolerance           /* IN      stroke tolerance for bounds */
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int bspsurf_transformSurface
(
MSBsplineSurface        *outSurfP,          /* OUT     transformed surface */
MSBsplineSurfaceCP      inSurfP,           /* IN      input surface */
Transform const         *transformP         /* IN      transform */
);


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_c1Discontinuities
(
double              **paramPP,
int                 *nParamsP,
MSBsplineSurface    *surfP,
int                 direction,          /* BSSURF_U or BSPCURV_V */
double              tolerance
);

//--------------------------------------------------------------------------------------
// Free an array allocated by other bspline operations.                  |
// The pointer is cleared after freeing.                                 |
// This is so DLM procedures can release spline memory without knowning  |
// which allocation mechanism was used.                                  |
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void bspsurf_freeArray
(
void **arrayPP          /* IN OUT  array to be freed */
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP double bspsurf_minAveRowLength
(
MSBsplineSurface    *surfaceP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP bool   bspsurf_isPlane
(
MSBsplineSurface    *surfaceP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_swapSurfaceUV
(
MSBsplineSurface    *outP,
MSBsplineSurface    *inP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_surfacePhysicallyClosed
(
bool                *uClosed,
bool                *vClosed,
MSBsplineSurface    *surf,
double              tolerance
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void     bspsurf_normalizeSurface
(
MSBsplineSurface    *surfaceP
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspknot_addKnotSurface
(
MSBsplineSurface    *surface,
double              uv,
double              knotTolerance,
int                 newMult,
int                 addToCurrent,
int                 direction
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP bool     bsputil_pointOnSurface
(
DPoint2d            *uv,
MSBsplineSurfaceCP surf
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bsputil_addBoundaries
(
MSBsplineSurface    *surface,
BsurfBoundary       **bounds,
int                 numBounds
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP StatusInt     bsputil_addSingleBoundary
(
MSBsplineSurface    *pSurface,
BsurfBoundary       *pBoundary
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void bsputil_calculateNumRules
(
MSBsplineSurface        *surfaceP,
int                     uFullCircleIsoparametrics,
int                     vFullCircleIsoparametrics,
double                  closureTolerance
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bsputil_ruledSurfaceFromCompatibleCurves
(
MSBsplineSurface    *surface,
MSBsplineCurve      *curve1,
MSBsplineCurve      *curve2
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bspsurf_ruledSurfaceFromCompatibleCopiesOfCurves
(
MSBsplineSurface    *surface,
MSBsplineCurve      *inCurve1,
MSBsplineCurve      *inCurve2
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bsputil_extractIsoIntersects
(
bvector <double> &params,
double              value,
MSBsplineSurface    *surfaceP,
bool                horizontal
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bsputil_extractIsoIntersects
(
double              **paramPP,
double              value,
MSBsplineSurface    *surfaceP,
bool                horizontal
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      bsputil_segmentC1DiscontinuousCurve
(
MSBsplineCurve  **segCurves,    /* OUT     continuous curves */
int             *nSegCurves,    /* OUT     number of continuous curves */
MSBsplineCurveCP curveP         /* IN      possibly disjoint curve */
);

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in the
*       u-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as periodic.
*       The remedy is usually a call to bsprsurf_openSurface.
* @remarks If this function returns true, then the first/last columns of poles are equal,
*       e.g., the surface is geometrically closed but not periodically defined in the
*       u-parameter direction.
* @see mdlBspline_surfaceShouldBeOpenedInV, mdlBspline_curveShouldBeOpened
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_surfaceShouldBeOpenedInU
(
const MSBsplineSurface  *pSurface
);

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
* @bsimethod
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
);

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
* @bsimethod
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
);

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline surface is "closed" in the
*       v-parameter direction in the nonperiodic V7 sense (cf. bsprsurf_closeSurface).
*       Such knots are problematic when the B-spline surface is processed as periodic.
*       The remedy is usually a call to bsprsurf_openSurface.
* @remarks If this function returns true, then the first/last rows of poles are equal,
*       e.g., the surface is geometrically closed but not periodically defined in the
*       v-parameter direction.
* @see mdlBspline_surfaceShouldBeOpenedInU, mdlBspline_curveShouldBeOpened
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_surfaceShouldBeOpenedInV
(
const MSBsplineSurface  *pSurface
);

END_BENTLEY_GEOMETRY_NAMESPACE

