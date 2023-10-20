/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */



BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
@description Inquire the static enabling flag for precise trim.
@returns static flag value.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspsurf_isPreciseTrimEnabled
(
);

/*----------------------------------------------------------------------+
@description Set the static enabling flag for precise trim.
@param bEnabled IN flag to save.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_setIsPreciseTrimEnabled
(
bool    bEnabled
);

#ifdef __cplusplus
/*----------------------------------------------------------------------+
@param pCurvePoints OUT points on curve
@param pSurfacePoints OUT points on surface
@param curveTol IN absolute tolerance on curve.
@param surfaceTol IN absolute tolerance on surface.
@param minPoints IN minimum number of points.  Evaluation begins with this number of
        points at uniform parameter steps.
@param pCurve IN paramter curve.
@param surface IN target surface.
@param param0 IN start param on curve
@param param1 IN end param on curve
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_appendPCurveStrokes
(
EmbeddedDPoint3dArray *pCurvePoints,    /* OUT     evaluated points. MAY NOT BE NULL */
EmbeddedDPoint3dArray *pSurfacePoints,  /* OUT     evaluated points. MAY NOT BE NULL */
double          curveTol,               /* IN      Curve-space tolerance. */
double          surfaceTol,             /* IN      Surface-space tolerance. */
int             minPoints,              /* IN      minimum number of points.  Evaluation begins with */
                                        /*     this number of points at uniform parameter steps. */
const MSBsplineCurve  *pCurve,           /* IN      curve structure */
const MSBsplineSurface *pSurface,        /* IN      surface structure */
double param0,                          /* IN      start parameter on curve */
double param1                           /* IN      end parameter on curve */
);
#endif
/*----------------------------------------------------------------------+
@param pDestBoundary IN destination boundary.  Prior linear loop is freed.
            Count and points are assigned.  Trim list is NOT addressed.
@param pSourceBoundary IN source boundary.  Trim list is addressed.  Linear loop
            is not addressed through pSourceBoundary.
@param curveTol IN tolerance on curves
@param surfaceTol IN tolerance on surface
@param minPoints IN min points per curve.
@param surface IN target surface.
@returns SUCCESS if all trims evaluated.  If any evaluation fails,
        evaluation halts without changing the pDestBoundary.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_strokeSingleTrimLoop
(
BsurfBoundary   *pDestBoundary,
BsurfBoundary   *pSourceBoundary,
double     curveTol,
double     surfaceTol,
int        minPoints,
const MSBsplineSurface *pSurface
);

/*----------------------------------------------------------------------+
@param pSurface IN OUT surface whose exact trims are to be restroked.
@param curveTol IN tolerance on curves.
@param surfaceTol IN tolerance on surface
@param surface IN target surface.
@remark if both tolerances are negative, default values are supplied.
@returns SUCCESS if all trims evaluated.  If any evaluation fails, the surface
        will have a mixture of old and new polyline loops.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_restrokeTrimLoops
(
MSBsplineSurface    *pSurface,
double     curveTol,
double     surfaceTol
);

/*----------------------------------------------------------------------+
@deprecated in 4.x. Use the overload with const input surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_openTempTrimmedSurface
(
MSBsplineSurface    *pTempSurface,
MSBsplineSurface    *pSourceSurface,
double     curveTol,
double     surfaceTol
);

/*----------------------------------------------------------------------+
Copy non-boundary data bitwise from source header to dest header.
Build reevaluated boundaries (lines) with no trim curves in the dest.
This is used by bspmesh, which needs trim boundaries at the same tolerance
as it is using for faceting the body of the face.
@param pTempSurface OUT surface to use for temporary operation.
@param pSourceSurface IN original surface.
@param curveTol IN tolerance for parameter space evaluation of curves.
@param surfaceTol IN tolerance for surface error.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_openTempTrimmedSurface
(
MSBsplineSurfaceR   tempSurface,
MSBsplineSurfaceCR  sourceSurface,
double              curveTol,
double              surfaceTol
);

/*----------------------------------------------------------------------+
Free boundary data in surface.  NULL out remaining parts (i.e. pole pointers
    that were copied by bspcurv_openTempTrimmedSurface)
@param pTempSurface IN OUT surface to close.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_closeTempTrimmedSurface
(
MSBsplineSurface    *pTempSurface
);

/*----------------------------------------------------------------------+
Stroke a parameter space curve and map to surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_strokePCurve
(
DPoint3d **ppCurveBuffer,    /* OUT     evaluated points. MAY NOT BE NULL */
DPoint3d **ppSurfaceBuffer,    /* OUT     evaluated points. MAY NOT BE NULL */
int       *pNumPoints,                /* OUT     number evaluated points */
double     curveTol,               /* IN      Curve-space tolerance. */
double     surfaceTol,             /* IN      Surface-space tolerance. */
int        minPoints,              /* IN      minimum number of points.  Evaluation begins with */
                                        /*     this number of points at uniform parameter steps. */
const MSBsplineCurve  *curve,           /* IN      curve structure */
const MSBsplineSurface *surface,        /* IN      surface structure */
double param0,                          /* IN      start parameter */
double param1                           /* IN      end parameter */
);

/*----------------------------------------------------------------------+
@param pSurface IN subject surface
@param pNumLinearBoundaries OUT number of polyline loops.
@param pNumPCurveLoops OUT number of precise curves.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_countLoops
(
const MSBsplineSurface *pSurface,
int *pNumLinearBoundaries,
int *pNumPCurveLoops
);

/*----------------------------------------------------------------------+
Allocate a trim curve link
Fill in its curve point.
Insert as the tail of CYLCIC linked list.
@param ppHead IN OUT head link
@param pCurve IN curve pointer to place in new link.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_allocateAndInsertCyclic
(
TrimCurve **ppHead,
MSBsplineCurve *pCurve
);

/*----------------------------------------------------------------------+
@description Split the curve at C1 discontinuities.  Pass each segment
to bspTrimCurve_allocateAndInsertCyclic.
The segments (or copy of linear curve) are retained in the trim.  Caller is
always responsible for freeing the original curve.
@param ppHead IN OUT head link
@param pCurve IN curve pointer to place in new link.
@param bPreserveLinear IN true to copy linear curves directly.  (Linear curve segments
    are handled efficiently by strokers.)
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_allocateAndInsertCyclicC1Segments
(
TrimCurve **ppHead,
MSBsplineCurve *pCurve,
bool    bPreserveLinear
);

/*----------------------------------------------------------------------+
Reduce a cyclic trim curve list to linear list.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_breakCyclicList
(
TrimCurve **ppHead
);

/*----------------------------------------------------------------------+
Free the links and curves in a TrimCurve chain.
Both cyclic and linear chains are allowed.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspTrimCurve_freeList
(
TrimCurve **ppHead
);

/*----------------------------------------------------------------------+
@description remove PCurve trim but leave polylines unchanged.
@param pSurf IN OUT subject surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_removePCurveTrim
(
MSBsplineSurface *pSurf
);

/*----------------------------------------------------------------------+
@param pSurf IN OUT subject surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_freeBoundaries
(
MSBsplineSurface *pSurf
);

/*----------------------------------------------------------------------+
Append all transformed copy of boundaries from the source surface to the destination.
@param out IN OUT destination surface. Non-boundary data is not altered.
@param in  IN source surface
@param pTransform IN transform (parameter space to parameter space) to apply to trim as copied.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspsurf_appendCopyBoundaries
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in,
Transform const *pTransform
);

/*----------------------------------------------------------------------+
@description Transform both linear and curve data in a single boundary.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformBoundary
(
BsurfBoundary *pBoundary,
Transform const *pTransform
);

/*----------------------------------------------------------------------+
@description Transform all trim data (linear, curve) in place.  This is a
   parameter-space to parameter space transformation.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformAllBoundaries
(
MSBsplineSurface *pSurface,
Transform const  *pTransform
);

/*----------------------------------------------------------------------+
@description Transform selected trim data (linear, curve) in place.  This is a
   parameter-space to parameter space transformation.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
@param index0 IN first index to be transformed
@param num    IN number to transform
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspsurf_transformBoundaries
(
MSBsplineSurface *pSurface,
Transform const  *pTransform,
int             index0,
int             num
);

/*----------------------------------------------------------------------+
@description test if a parametric point is within trim boundaries.  Uses precise trims first.
@param uvP IN parametric point.
@param boundP IN array of boundaries
@param numBounds IN number of boundaries
@param holeOrigin IN false inverts sense of parity logic.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsputil_pointInBounds
(
DPoint2d        *uvP,
BsurfBoundary   *boundaryArrayP,
int             numBounds,
bool            holeOrigin
);

/*---------------------------------------------------------------------------------**//**

+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspUtil_initializeBsurfBoundary
(
BsurfBoundary* pBBoundary
);
#ifdef __cplusplus
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspsurf_intersectBoundariesWithUVLine
(
bvector<double>             *pFractionArray,
double                      value,
const MSBsplineSurface      *pSurface,
int                         horizontal
);
#endif

/*----------------------------------------------------------------------+
@description Intersect trim curve region with clip polygon
@param pBoundaries IN boundaries to split
@param numBoundary IN num input boundaries.
@param ppNewBoundaries OUT new boundaries (allocated via dlmSystem)
@param pNumNewBoundaries OUT number of new boundaries
@param pPolygonPoints IN array of polygon points.   ASSUMED to have closure point.
@param numPolygonPoints IN number of points in clip polygon.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspsurf_clipTrimLoops
(
BsurfBoundary    *pBoundaries,
int              numBoundary,
BsurfBoundary    **ppNewBoundaries,
int              *pNumNewBoundaries,
DPoint3d         *pPolygonPoints,
int              numPolygonPoints
);


END_BENTLEY_GEOMETRY_NAMESPACE

