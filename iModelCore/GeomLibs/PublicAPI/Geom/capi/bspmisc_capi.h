/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


Public GEOMDLLIMPEXP  StatusInt mdlBspline_removeCoincidentPoints
(
DPoint3d*       outPts,
double*         outParams,
int*            numOut,
const DPoint3d* inPts,
const double*   inParams,
int             numIn,
double          tol2,
bool            stripDisconnects,
bool            returnAtLeastTwo
);

Public GEOMDLLIMPEXP  int       mdlBspline_c2CubicInterpolatePrepareFitPoints
(
std::vector<DPoint3d>& outPts,      /* <= new points */
std::vector<double>& outParams,     /* <= new params */
bool            *closed,            /* <=> periodic curve flag, flipped if invalid */
const DPoint3d  *inPts,             /* => points to be interpolated */
const double    *inParams,          /* => parameter at each point (or NULL) */
int const       numPts,             /* => number of items in inPts and inParams */
bool            remvData,           /* => true = remove coincident points */
double          tolerance           /* => max dist betw coincide pts or closed curve */
);

Public GEOMDLLIMPEXP  int       bspcurv_c2CubicInterpolatePolesExt
(
DPoint3d        *outPts,            /* <= poles */
double          *outWts,            /* <= weights or NULL */
double          *knots,             /* <= knots or NULL */
double          *inParams,          /* => u parameters or NULL */
DPoint3d        *points,            /* => points to be interpolated */
DPoint3d        *endTangents,       /* => normalized end tangents or NULL */
double          *weights,           /* => weights or NULL */
BsplineParam    *bsplineParams,     /* => B-Spline parameters */
int             numPoints,          /* => number of points (incl redundant endpt if closed) */
bool            chordLenKnots,      /* => T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
);

Public GEOMDLLIMPEXP  int       bspcurv_c2CubicInterpolateCurveExt
(
MSBsplineCurve  *curve,             /* <= cubic spline curve */
DPoint3d        *inPts,             /* => points to be interpolated */
double          *inParams,          /* => u parameters or NULL */
int             numPts,             /* => number of points */
bool            remvData,           /* => true = remove coincident points */
double          tolerance,          /* => max dist betw coincide pts or closed curve */
DPoint3d        *endTangents,       /* => normalized end tangents or NULL */
bool            closedCurve,        /* => if true, closed Bspline is created */
bool            chordLenKnots,      /* => T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
);

Public GEOMDLLIMPEXP  int       mdlBspline_setInterpolationTangents
(
DPoint3d                *pStartTangent,
DPoint3d                *pStartPoint,
DPoint3d                *pStartTangentPoint,
DPoint3d                *pEndTangent,
DPoint3d                *pEndPoint,
DPoint3d                *pEndTangentPoint
);

Public GEOMDLLIMPEXP  StatusInt mdlBspline_validateCurveKnots
(
double*         pKnots,         // <=> full knots
DPoint3d*       pPoles,         // <=> full weighted poles (optional)
double*         pWeights,       // <=> full weights (optional)
BsplineParam*   pParams         // <=> potentially modified only if poles given
);


Public GEOMDLLIMPEXP  int       bspproc_processSurfaceByCurves
(
MSBsplineSurface    *surface,          /* => b-spline surface to process */
PFBCurveVoidPInt    processFunc, /* => acts on row/column curves of surface */
void                *args,             /* <=> information passed through to process */
int                 direction          /* => surface direction, BSSURF_U or BSSURF_V */
);

Public GEOMDLLIMPEXP  int       bspsurf_ruledSurface
(
MSBsplineSurface    *surface,          /* <= b-spline ruled surface */
MSBsplineCurve      *curve0,           /* => first profile curve (v = 0.0) */
MSBsplineCurve      *curve1            /* => second profile curve (v = 1.0) */
);

Public GEOMDLLIMPEXP  int       bspconv_getCurveFromSurface
(
MSBsplineCurve      *curve,
MSBsplineSurfaceCP  surface,
int                 direction,
int                 which              /* => -1 mean last row/column */
);

Public GEOMDLLIMPEXP  int       bspconv_getEdgeFromSurface
(
MSBsplineCurve      *curve,
MSBsplineSurfaceCP   surf,
int                 edgeCode
);

Public GEOMDLLIMPEXP  void      bspconv_extractProfile
(
MSBsplineCurve      *curve,            /* <= extracted profile curve */
MSBsplineSurfaceCP   surface,         /* => input surface */
bool                 lastRow            /* => false gives start of surface */
);

Public GEOMDLLIMPEXP  void      bspconv_extractWeightValues
(
double          *weights,
double          *igdsWeights,
int             numPoles
);

/*----------------------------------------------------------------------+
@param pSurf IN OUT subject surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspsurf_freeBoundariesWithHeapDescr
(
MSBsplineSurface *pSurf,
void             *pHeapDescr
);

/*----------------------------------------------------------------------+
Free the links and curves in a TrimCurve chain.
Both cyclic and linear chains are allowed.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspTrimCurve_freeListWithHeapDescr
(
TrimCurve **ppHead,
void      *pHeapDescr
);


/*----------------------------------------------------------------------+
Reduce a cyclic trim curve list to linear list.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspTrimCurve_breakCyclicList
(
TrimCurve **ppHead
);

/*----------------------------------------------------------------------+
Free the links and curves in a TrimCurve chain.
Both cyclic and linear chains are allowed.
@param ppHEAD IN OUT handle for head-of-chain.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspTrimCurve_freeList
(
TrimCurve **ppHead
);

/*----------------------------------------------------------------------+
@description Transform both linear and curve data in a single boundary.
@param pSurface IN surface whose trim is to be transformed (in place)
@param pTransform IN parameter space transformation
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspsurf_transformBoundary
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
Public GEOMDLLIMPEXP  void      bspsurf_transformAllBoundaries
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
Public GEOMDLLIMPEXP  void      bspsurf_transformBoundaries
(
MSBsplineSurface *pSurface,
Transform const  *pTransform,
int             index0,
int             num
);

//!
//!
//!
Public GEOMDLLIMPEXP  StatusInt bspUtil_initializeBsurfBoundary
(
BsurfBoundary* pBBoundary
);

Public GEOMDLLIMPEXP  int       bsprsurf_closeSurface
(
MSBsplineSurface    *outSurface,       /* <= closed surface, periodic */
MSBsplineSurfaceCP   surface,        /* => input surface */
int                 edge               /* => direction to open, BSSURF_U or BSSURF_V */
);

Public GEOMDLLIMPEXP  int       bsprsurf_openSurface
(
MSBsplineSurface    *outSurface,       /* <= open surface, non-periodic */
MSBsplineSurfaceCP   surface,        /* => input surface */
double              uvValue,           /* => parameter at which to open */
int                 edge               /* => direction to open, BSSURF_U or BSSURF_V */
);

Public GEOMDLLIMPEXP  int       bsprsurf_makeBezierSurface
(
MSBsplineSurface    *outSurface,       /* <= Bezier form, all knots of degree mult */
MSBsplineSurfaceCP   surface        /* => input surface */
);

Public GEOMDLLIMPEXP  int       bsprsurf_elevateDegreeSurface
(
MSBsplineSurface    *outSurface,       /* <= degree elevated surface */
MSBsplineSurfaceCP   surface,        /* => input surface */
int                 newDegree,         /* => desired new degree */
int                 edge               /* => direction to use, BSSURF_U or BSSURF_V */
);

Public GEOMDLLIMPEXP  int       mdlBspline_removeRedundantKnotsFromSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   surface,
bool                uvDir   /* => true: u knots, false: v knots */
);

Public GEOMDLLIMPEXP  int       mdlBspline_cleanSurfaceKnots
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
);

Public GEOMDLLIMPEXP  int       bspproc_prepareCurve
(
MSBsplineCurve  *bezier,                /* <= Bezier curve */
int             *numSegments,           /* <= number of segs */
int             **starts,               /* <= offsets of segments */
MSBsplineCurve  *bspline                /* => Bspline curve */
);

Public GEOMDLLIMPEXP  int     bspproc_prepareCurveOld
(
MSBsplineCurve  *bezier,               /* <= Bezier curve */
int             *numSegments,          /* <= number of segs */
int             **starts,              /* <= offsets of segments */
MSBsplineCurve  *bspline               /* => Bspline curve */
);

Public GEOMDLLIMPEXP  void      bspcurv_intersectPlane
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startParam,         /* => starting parameter */
double          endParam,           /* => ending parameter */
DPoint4d        *pPlaneCoff,        /* => plane coefficients */
double          absTol,             /* => absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
);

/*----------------------------------------------------------------------+
@param pSurface IN subject surface
@param pNumLinearBoundaries OUT number of polyline loops.
@param pNumPCurveLoops OUT number of precise curves.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  void      bspsurf_countLoops
(
const MSBsplineSurface *pSurface,
int *pNumLinearBoundaries,
int *pNumPCurveLoops
);

Public GEOMDLLIMPEXP  int       bspsurf_computeBoundarySpansWithHeapDescr
(
double                       **spans,
double                       value,
const MSBsplineSurface      *bspline,
int                         horizontal,
void                        *pHeapDescr
);

Public GEOMDLLIMPEXP  int       bspsurf_computeBoundarySpans    /* <= number of spans of rule line */
(
double                      **spans,           /* <= break points of rule line */
double                      value,             /* => parameter of rule line */
const MSBsplineSurface      *bspline,          /* => input surface */
int                         addBoundaries      /* => if true origin is hole */
);


Public GEOMDLLIMPEXP  int       bspproc_surfaceWireframeByCurves
(
const MSBsplineSurface          *surface,
ProcessFuncSurfaceWireFrame     processFunc,        /* => acts on rule lines */
void                            *pProcessFuncArg,   /* => argument passed to processFunction */
bool                            processBounds       /* => process boundaries as curves? */
);

Public GEOMDLLIMPEXP  StatusInt bspproc_getBoundaryCurve
(
MSBsplineCurve          *curveP,
const MSBsplineSurface  *surfaceP,
int                     iBound
);

Public GEOMDLLIMPEXP  int       bspproc_processBoundaryCurves
(
const MSBsplineSurface          *bspline,           /* => Bspline surface to process */
ProcessFuncSurfaceWireFrame     processFunc,        /* => acts on rule lines */
void                            *pProcessFuncArg,   /* => argument passed to processFunction */
bool                            processEdge         /* => true to process parameter edges if unbounded */
);

Public GEOMDLLIMPEXP  bool      bspcurv_closestXYPoint
(
DPoint3d            *pClosePoint,
double              *pCloseParam,
double              *pCloseDist2,
MSBsplineCurve      *curveP,        /* => input curve */
double              startParam,     /* => starting parameter */
double              endParam,       /* => ending parameter */
double              xx,             /* => fixed point x */
double              yy              /* => fixed point y */
);

Public GEOMDLLIMPEXP  int       bspcurv_parameterFromArcLength
(
DPoint3d        *pointP,            /* <= point of paramP */
double          *paramP,            /* <= parameter to match the arc length */
double          *errorAchievedP,    /* <= the absolute error achieved */
double          arcLength,          /* => from start parameter to paramP */
double          startParam,         /* => start parameter */
MSBsplineCurve  *curveP,            /* => input curve */
double          *totalLengthP,      /* => total length if known, or NULL */
double          relativeTol,        /* => relative tolerance for integration */
double          paramTol            /* => parameter tolerance for iteration */
);

Public GEOMDLLIMPEXP  double    bspcurv_getResolutionExt
(
MSBsplineCurve const *pCurve,                /* => b-spline curve */
double absTol,
double relTol
);

Public GEOMDLLIMPEXP  double    bspsurf_getResolutionExt
(
MSBsplineSurface const *pSurface,          /* => b-spline surface */
double              absTol,
double              relTol
);

Public GEOMDLLIMPEXP  StatusInt mdlBspline_validateSurfaceKnots
(
double*         pUKnots,        // <=> full u-knots
double*         pVKnots,        // <=> full v-knots
DPoint3d*       pPoles,         // <=> full weighted poles (u-major order, optional)
double*         pWeights,       // <=> full weights (u-major order, optional)
BsplineParam*   pUParams,       // <=> potentially modified only if poles given
BsplineParam*   pVParams        // <=> potentially modified only if poles given
);


Public GEOMDLLIMPEXP  int       bspproc_processSurfacePolygon
(
const MSBsplineSurface              *surfaceP,         /* => curve to process */
ProcessFuncSurfaceControlPolygon    processFunc,       /* => process function */
void                                *args              /* => arguments */
);

#ifdef __cplusplus
Public GEOMDLLIMPEXP  int       bspcurv_curvePlaneIntersectsExt
(
bvector<double>&    pParamArray,
DPoint3dCP              pOrigin,
DPoint3dCP              pNormal,
MSBsplineCurveP         pCurve,
double                  tolerance
);

//!
//!
Public GEOMDLLIMPEXP  void      bspsurf_intersectBoundariesWithUVLineExt
(
bvector<double>&    pFractionArray,
double                  value,
MSBsplineSurfaceCP      pSurface,
int                     horizontal
);
#endif

Public GEOMDLLIMPEXP  double    bspproc_setParameterWithOrder
(
double          in,                    /* => parameter in Bezier segment [0.0,1.0] */
int             start,                 /* => knot offset of Bezier segment */
double          *knots,                /* => full knot vector of b-spline */
int             order
);


Public GEOMDLLIMPEXP  int       bspproc_processBspline
(
BezierInfo      *infoP,                /* <=> information desired of curve */
MSBsplineCurve  *bspline,              /* => b-spline curve to process */
FPBsplineSort    sortFunc,     /* => sorting function, or NULL */
FPBsplineGo      goFunc,       /* => go function, or NULL */
FPBsplineProcess processFunc,  /* => acts on Bezier segments */
FPBsplineSelect  selectFunc,   /* => loads ouput based on segments */
FPBsplineClean   cleanFunc     /* => called before returning */
);

Public GEOMDLLIMPEXP  int       bsprsurf_minDistToSurface
(
double*             distance,           /* <= distance to closest point on curve */
DPoint3dP           minPt,              /* <= closest point on curve */
DPoint2dP           uv,                 /* <= parameter of closest point */
DPoint3dCP          testPt,             /* => point to calculate dist from */
MSBsplineSurface*   surface,            /* => input surface */
double*             tolerance           /* => tolerance to use in calculation */
);

//!
//!
Public double    bound_sphereFromSurface
(
DPoint3d            *center,
double              *radius,
MSBsplineSurface    *surface
);

//!
//!
Public bool      bound_sphereMinMax
(
double          *min,
double          *max,
DPoint3d        *testPt,
DPoint3d        *center,
double          radius
);

//!
//!
Public void     bound_boxFromSurface
(
BoundBox            *box,
MSBsplineSurface    *surface
);

Public GEOMDLLIMPEXP  int       bspssi_relaxToPatch
(
DPoint3d            *nearPt,           /* <= closest point on surface */
DPoint3d            *normal,           /* <= normal to surf at nearPt */
DPoint2d            *uv,               /* <=> initial guess -> param of pt */
DPoint3d            *testPt,           /* => want closest pt to this pt */
MSBsplineSurface    *surface,          /* => input surface */
int                 iterates           /* => maximum number of times to iterate */
);


Public GEOMDLLIMPEXP  int       bspproc_processBezierPatch
(
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch,            /* => Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatch_Select selectFunc /* => loads ouput, or NULL */
);

Public GEOMDLLIMPEXP  void      bsprsurf_netPoles
(
DPoint3d            *poles,            /* <= poles of single Bezier patch of surface */
double              *weights,          /* <= weights of Bezier patch */
int                 uIndex,            /* => index of patch desired */
int                 vIndex,            /* => index of patch desired */
MSBsplineSurface    *surface           /* => input surface */
);

Public GEOMDLLIMPEXP  int       bspproc_processBsplineSurface
(
BezierInfo          *infoP,            /* <=> information desired of surf */
MSBsplineSurface    *bspline,          /* => b-spline surface to process */
PFBSurf_Sort       sortFunc,          /* => sorting function, or NULL */
PFBSurf_Go         goFunc,            /* => go function, or NULL */
PFBSurf_Process    processFunc,       /* => acts on Bezier patches */
PFBSurf_Select     selectFunc,        /* => loads ouput based on patches */
PFBSurf_Clean      cleanFunc          /* => called before returning */
);

//!
//!
Public void      bound_boxCompute
(
BoundBox        *box,
DPoint3d        *points,
int             numU,
int             numV
);

Public GEOMDLLIMPEXP  int       bspssi_relaxToSurface
(
DPoint3d        *nearPt,               /* <= closest point on surface */
DPoint3d        *normal,               /* <= normal to surf @ nearPt, scaled by sine angle */
double          *degeneracy,           /* <= sine of angle between surface partials */
DPoint2d        *uv,                   /* <=> initial guess -> param of pt */
DPoint3d        *testPt,               /* => want closest pt to this pt */
Evaluator       *eval,
int             iterations,            /* => number of iterations to try */
double          convTol                /* => convergence tolerance in UV space */
);

Public GEOMDLLIMPEXP  int       bspproc_prepareSurface
(
MSBsplineSurface    *bezier,            /* <= bezier surface */
int                 *uNumSegs,          /* <= number of segs U */
int                 *vNumSegs,          /* <= number of segs V */
int                 **uStarts,          /* <= offsets of segments U */
int                 **vStarts,          /* <= offsets of segments V */
MSBsplineSurface    *bspline            /* => Bspline surface */
);

Public GEOMDLLIMPEXP  int       bspproc_initializeBezierPatch
(
MSBsplineSurface    *patchP,            /* <= bezier patch */
MSBsplineSurface    *surfP              /* => surface (bezier prepped) */
);

Public GEOMDLLIMPEXP int        mdlBspline_removeRedundantKnotsFromCurve
(
MSBsplineCurve  *out,
MSBsplineCurve  *in
);

#ifdef __cpluspluss
Public GEOMDLLIMPEXP  StatusInt bspcurv_curvePlaneIntersectsExt
(
bvector<double>&    pParamArray,
DPoint3dCP              pOrigin,
DPoint3dCP              pNormal,
MSBsplineCurveP         pCurve,
double                  tolerance
);
#endif

Public GEOMDLLIMPEXP int        bspsurf_getNumBoundPointsCurve
(
int                 *numPointsP,
MSBsplineCurve      *curveP,
MSBsplineSurface    *surfaceP,
DRange2d            *surfUVRange,
double              tolerance
);

Public GEOMDLLIMPEXP int        bspsurf_imposeBoundary
(
MSBsplineSurface    *surface,
MSBsplineCurve      *curve,
double              tolerance,
DRange3d           *directionPoints,
DPoint3d            **surfPoints,       /* <= points on surface, or NULL */
int                 *numSurfPts
);

//!
//!
Public GEOMDLLIMPEXP void       bound_rectangleCompute
(
DRange3d    *rectP,             /* <= bounding rectangle */
DPoint3d    *pointP,            /* => points */
int         nPoints             /* => number of points */
);

//!
//!
Public GEOMDLLIMPEXP void       bspproc_setSubPatchParameter
(
DRange2d       *out,                  /* OUT     corners of sub-patch */
int             code,                  /* IN      sub-patch code */
DRange2d       *in                    /* IN      corners of parent patch */
);

//!
//!
Public GEOMDLLIMPEXP void       bsputil_selectQuarterPatch
(
DRange2dP   out,                  /* <= corners of sub-patch */
int         code,                  /* => sub-patch code */
DRange2dCP  in                    /* => corners of parent patch */
);

END_BENTLEY_GEOMETRY_NAMESPACE
