/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct pointList PointList;

/*----------------------------------------------------------------------+
|                                                                       |
| name          sortUtil_returnMinimum                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      sortUtil_returnMinimum
(
double          *minimum,
double          *list,
int             number
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          angleBtwDvector3d                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   angleBtwDvector3d
(
DPoint3d        *org1,
DPoint3d        *end1,
DPoint3d        *org2,
DPoint3d        *end2
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_computeBlendingFuncs                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_knotToBlendingFuncs
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          u,
double          uMax,
int             order,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_computeBlendingFuncs                            |
|                                                                       |
|   DO NOT USE THIS METHOD -- It contains incorrect uMax calculation    |
|   USE bsputil_computeBlendingFuncs instead.                           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_computeBlendingFuncs
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          fraction,
double          uMax,
int             order,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_computeBlendingFuncs                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_computeBlendingFunctions
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          fraction,
int             numPoles,
int             order,
int             closed
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_blendingsForSecondPars                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_blendingsForSecondPars
(
double          *coefs,
double          *dCoefs,
double          *ddCoefs,
int             *left,
double const    *knots,
double          u,
double          uMax,
int             order,
int             closed
);

//!
//! Return a tolerance appropriate to an array of points.
//! @param pXYZ IN array of points.
//! @param pWeights IN array of weights.  REMARK: library policy may choose to ignore weights
//!    and compute the tolerance using the raw xyz.
//! @return computed tolerance based on largest coordinate in poles.
//!
Public GEOMDLLIMPEXP double bsputil_pointTolerance
(
DPoint3dCP  pXYZ,
double const *pWeights,
int         numXYZ
);

//!
//! @param curveP IN subject curve
//! @return computed tolerance based on largest coordinate in poles.
//!
Public GEOMDLLIMPEXP double bsputil_curveTolerance (MSBsplineCurveCP curve);

//!
//! @param surface IN subject surface
//! @return computed tolerance based on largest coordinate in poles.
//!
Public GEOMDLLIMPEXP double bsputil_surfaceTolerance (MSBsplineSurfaceCP surface);

//!
//! Test if two weights are equal within system tolerance.
//! @param wA IN first weight
//! @param wB IN second weight
//! @return true if same within tolerance.
//!
Public GEOMDLLIMPEXP bool    bsputil_isSameWeight
(
double  wA,
double  wB
);

//!
//! Test if two points are equal, using caller's tolerance.
//! @param pPointA IN first point
//! @param pPointB IN second point
//! @return true if max absolute coordinate difference is within computed tolerance.
//!
Public GEOMDLLIMPEXP bool    bsputil_isSamePointTolerance
(
DPoint3dCP pPointA,
DPoint3dCP pPointB,
double     tolerance
);

//!
//! Test if two points are equal, using global absolute and relative tolerances.
//! @param pPointA IN first point
//! @param pPointB IN second point
//! @return true if max absolute coordinate difference is within computed tolerance.
//!
Public GEOMDLLIMPEXP bool    bsputil_isSamePoint
(
DPoint3dCP pPointA,
DPoint3dCP pPointB
);

//!
//! Test if two points and weights are equal, using global absolute and relative tolerances on xyz parts.
//! Weights must match to absolute tolerance.
//! @param p0 IN first point
//! @param w0 IN first weight.
//! @param p1 IN second point
//! @param w1 IN second weight.
//! @return true if weights and xyz parts both match.
//!
Public GEOMDLLIMPEXP bool     bsputil_isSameRationalPoint
(
DPoint3dCP p0,
double     w0,
DPoint3dCP p1,
double     w1
);

//!
//! Test if two points and weights are equal, using caller's xyz tolerance
//! Weights must match to absolute tolerance.
//! @param p0 IN first point
//! @param w0 IN first weight.
//! @param p1 IN second point
//! @param w1 IN second weight.
//! @return true if weights and xyz parts both match.
//!
Public GEOMDLLIMPEXP bool     bsputil_isSameRationalPointTolerance
(
DPoint3dCP p0,
double     w0,
DPoint3dCP p1,
double     w1,
double     xyzTolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_edgeCode                                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_edgeCode
(
DPoint2dCP      uv,
double          tolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_edgeCode                                        |
|                                                                       |
| return NO_EDGE, U0_EDGE, U1_EDGE, V0_EDGE, V1_EDGE if both points are |
|  on respective edge.  If both are 00 or 11, U edge will win.          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_edgeCode
(
DPoint2dCR uv0,
DPoint2dCR uv1,
double          tolerance
);

// count segments that are on an edge, or individual points up to an edge touch.
// The block of points i0..i0+numPoint-1 is the chain.
Public GEOMDLLIMPEXP bool     bsputil_countPointsToEdgeBreak
(
DPoint2dCP   uv,
int i0,
int numPoint,
double tolerance,
int    &edgeCode,
int    &numOnEdge
);
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_onEdge                                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_onEdge
(
DPoint2dCP uv,
double     tolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_closestEdge                                     |
|               Go to closest edge in UV space                          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_closestEdge
(
DPoint2d        *edgePt,
DPoint2d        *testPt
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractScaledValues                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractScaledValues
(
double          *weights,
double          *igdsWeights,
int             numPoles
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_loadPoles                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_loadPoles
(
DPoint3d        *poles,                /* OUT     poles that define curve at u */
double          *weights,              /* OUT     weights (if rational) */
DPoint3d        *fullPoles,            /* IN      all poles of curve */
double          *fullWeights,          /* IN      all  weights (if rational) */
int             start,                 /* IN      index of first pole */
int             numPoles,
int             order,
int             closed,
int             rational
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_swap                                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_swap
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_flushStrokes                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_flushStrokes
(
DPoint3d        *out,
int             strokes,
DPoint3d        **buffer,
int             *bufSize
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_returnStrokes                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_returnStrokes
(
DPoint3d       *rvec,
DPoint3d        out[],
int             *strokes,
DPoint3d        **destination,
int             *destSize
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_weightPoles                                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_weightPoles
(
DPoint3d        *weightedPoles,
DPoint3d        *poles,
double          *weights,
int             numPoles
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_unWeightPoles                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_unWeightPoles
(
DPoint3d        *poles,
DPoint3d        *weightedPoles,
double          *weights,
int             numPoles
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_regularizeWeights                               |
|               sets end weights both to the value 1.0                  |
|               WARNING: interior weights may exceed the value 1.0      |
|               See Farin, 3rd edition, page 236.                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_regularizeWeights
(
MSBsplineCurve  *curve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_cumulativeDistance                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   bsputil_cumulativeDistance
(
DPoint3d        *points,
double          *weights,
int             rational,
int             numPoints
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_isLinearArray                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_isLinearArray
(
DPoint3d        *poles,
double          *weights,
int             rational,
int             numPoles,
double          cosineTol
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_containsInflection                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_containsInflection
(
DPoint3d        *poles,
double          *weights,
int             rational,
int             numPoles
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonTangent                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_polygonTangent
(
DPoint3d        *tangent,
MSBsplineCurve  *bezier,
int             endFlag
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonBiNormal                                 |
|               Returns the first non-degenerate calculated bi-normal   |
|               by getting the cross product of succesive polygon       |
|               legs. Starts from the specified end of the control      |
|               polygon.                                                |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_polygonBiNormal
(
DPoint3d        *normal,
MSBsplineCurve  *curve,
int             endFlag
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractNormal                                   |
|               from extract_elemDescrNormal                            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractNormal
(
DPoint3d        *normalP,               /* OUT     curve normal */
DPoint3d        *positionP,             /* OUT     point on plane (taken directly from points) */
double          *planarDeviationP,      /* OUT     deviation from planar */
DPoint3d        *points,
int             numPoints,
DPoint3d        *defaultNormalP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_bezierKnotVector                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   *bspknot_bezierKnotVector (int order);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_numberKnots                                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_numberKnots
(
int             numPoles,
int             order,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_sameKnot                                        |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspknot_sameKnot
(
double knotA,
double knotB
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_knotTolerance                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   bspknot_knotTolerance (MSBsplineCurveCP curve);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeKnotVector                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeKnotVector
(
double          *knotVector,        /* OUT     Full knot vector */
BsplineParam    *params,            /* IN      B-Spline parameters */
double          *interiorKnots      /* IN      interior knots (if nonuniform) */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeKnotVectorNotNormalized                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeKnotVectorNotNormalized
(
double          *knotVector,        /* OUT     Full knot vector */
BsplineParam    *params,            /* IN      B-Spline parameters */
double          *interiorKnots      /* IN      interior knots (if nonuniform) */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_normalizeKnotVector                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_normalizeKnotVector
(
double          *knotVector,
int             numPoles,
int             order,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_findSpan                                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_findSpan
(
int             *spanIndex,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          t
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeGrevilleAbscissa                         |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeGrevilleAbscissa
(
double          *nodeValues,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          tolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_getKnotMultiplicity                             |
|                                                                       |
|                                                                       |
| Note: bsputil_getKnotMultiplicityExt does not decrement count         |
|       for a closed curve.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_getKnotMultiplicity
(
double          *distinctKnots,
int             *knotMultiplicity,
int             *numDistinct,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          knotTolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_increaseKnotDegree                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_increaseKnotDegree
(
double          **newKnots,
int             *newNumPolesM1,
int             newDegree,
double          *oldKnots,
int             oldNumPoles,
int             oldDegree,
int             closed,
double          knotTolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_insertKnot                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_insertKnot
(
double          u,
int             addMult,
int             currentMult,
DPoint3d        *oldPoles,
double          *oldKnots,
double          *oldWeights,
DPoint3d        *newPoles,
double          *newKnots,
double          *newWeights,
int             numPoles,
int             order,
int             closed,
int             rational
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnotWithHeapDescr                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_addKnotWithHeapDescr
(
MSBsplineCurve  *curve,
double          u,
double          knotTolerance,
int             newMult,
int             addToCurrent,
void            *pHeapDescr
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnot                                         |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_addKnot
(
MSBsplineCurve  *curve,
double          u,
double          knotTolerance,
int             newMult,
int             addToCurrent
);
/*----------------------------------------------------------------------+
|  If the curve is open, clamp the knots at both ends.                  |
|  Additional corrections may be applied in the future.                 |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspknot_correctEndKnots
(
MSBsplineCurve  *curve
);
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_scaleKnotVector                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspknot_scaleCurveKnots
(
MSBsplineCurve  *curveP,
double          scale
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_scaleKnotVector                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspknot_scaleKnotVector
(
double          *knots,
BsplineParam    *params,
double          scale
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_validParameter                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_validParameter
(
double          u,
double const    *knots,
BsplineParam const   *params
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_validParameterSurface                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_validParameterSurface
(
DPoint2d        *uv,
double const    *uKnots,
BsplineParam const    *uParams,
double const *vKnots,
BsplineParam const   *vParams
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_xySegmentIntersection                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_xySegmentIntersection
(
int             *intersect0,            /* OUT     intersection code for segment 0 */
int             *intersect1,            /* OUT     intersection code for segment 1 */
double          *distance0,             /* OUT     distance (0-1.0) along segment 0 */
double          *distance1,             /* OUT     distance (0-1.0) along segment 1 */
DRange2dP       seg0P,                  /* IN      segment 0 */
DRange2dP       seg1P,                  /* IN      segment 1 */
double          tolerance               /* IN      tolerance for endpoint checks */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_segmentIntersection                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsputil_segmentIntersection
(
int         *code0P,
int         *code1P,
DPoint3d    *pointP0,
DPoint3d    *pointP1,
double      *param0P,
double      *param1P,
DPoint3d    *segment0P,
DPoint3d    *segment1P,
double      endTol
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_lineStringIntersect                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsputil_lineStringIntersect
(
DPoint3d    *pointP,
double      *u0P,
double      *u1P,
DPoint3d    *stringP0,
int         nPoints0,
DPoint3d    *stringP1,
int         nPoints1,
double      tolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_freeBoundary                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_freeBoundary
(
BsurfBoundary   **bounds,
int             numBounds
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_free                                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_free
(
void            *pMem
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_freePointList                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_freePointList
(
PointList       **list,
int             numLists
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_getKnotMultiplicityExt                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_getKnotMultiplicityExt
(
double          *distinctKnots,
int             *knotMultiplicity,
int             *numDistinct,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          knotTolerance
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractParameterRange                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractParameterRange
(
double          *minP,
double          *maxP,
double          *knotP,
BsplineParam    *paramP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_nIsoParamsFromAngle                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     bsputil_nIsoParamsFromAngle
(
double          angle,
int             nIsoParams
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_calculateNumRulesFromCurve                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsputil_calculateNumRulesFromCurve
(
MSBsplineCurve          *curveP,
int                     nFullCircleIsoparametrics
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_getNaturalParameterRange                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_getNaturalParameterRange
(
double          *pStart,
double          *pEnd,
MSBsplineCurveCP curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_normalizeCurveKnots                          |
|                                                                       |
| Return true if normalization is performed                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_normalizeCurveKnots
(
double          *pStartNaturalParam,
double          *pEndNaturalParam,
MSBsplineCurve  *curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_unNormalizeCurveKnots                        |
|                                                                       |
| Return true if non-normalization is preformed                         |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_unNormalizeCurveKnots
(
MSBsplineCurve  *curveP,        /* IN OUT  curve with normalized knots */
double          startNaturalParam,
double          endNaturalParam
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_getParameterRange                            |
|                                                                       |
|                                                                       |
| DEPRECATED -- Use getNaturalParameterRange                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_getParameterRange
(
double          *pStart,
double          *pEnd,
MSBsplineCurveCP curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasNormalizedKnots                      |
|                                                                       |
| This function tells if a curve has parameter range between [0.0, 1.0] |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasNormalizedKnots (MSBsplineCurveCP curveP);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_toFractionParams                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_toFractionParams
(
double          *pOutParams,    /* OUT     fraction params between 0.0 and 1.0 */
double          *pInParams,     /* IN      natural params */
int             num,            /* IN      size of array */
double          start,          /* IN      start of natural range */
double          end             /* IN      end of natural range */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_toNaturalParams                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_toNaturalParams
(
double          *pOutParams,    /* OUT     natural params between start and end */
double          *pInParams,     /* IN      fraction params between 0.0 and 1.0 */
int             num,            /* IN      size of array */
double          start,          /* IN      start of natural range */
double          end             /* IN      end of natural range */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_naturalParameterToFractionParameter          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double mdlBspline_naturalParameterToFractionParameter
(
MSBsplineCurveCP pCurve,
double          naturalParam
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_fractionParameterToNaturalParameter          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double mdlBspline_fractionParameterToNaturalParameter
(
MSBsplineCurveCP pCurve,
double          fractionalParam
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_clampCurveKnots                              |
|                                                                       |
| This function will clamp periodic V7 or V8 knots                      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_clampCurveKnots
(
MSBsplineCurve  *curOutP,
MSBsplineCurve  *curInP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_convertPeriodicKnots                         |
|                                                                       |
| This function converts V7.0 periodic knots to V8.0 format             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_convertPeriodicKnots
(
MSBsplineCurve  *curveOut,              /* OUT     expanded periodci knots, but curve.params.closed == false */
MSBsplineCurve  *curveIn                /* IN      curve.params.closed == true */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasClampedKnots                         |
|                                                                       |
| This function tells if a curve has the clamped knots in V8.0 or V7.0  |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasClampedKnots (MSBsplineCurveCP curveP);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasPeriodicKnots                        |
|                                                                       |
| This function tells if a curve has the periodic knots in V8.0 or V7.0 |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasPeriodicKnots
(
MSBsplineCurveCP curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveStoreFullKnots                          |
|                                                                       |
| This function tells if a curve has the new knot format in V8.0        |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveStoreFullKnots (MSBsplineCurveCP curveP);

//!
//! @description Returns true if the associated B-spline curve is "closed" in
//!       the nonperiodic V7 sense (cf. bspcurv_closeCurve, bspconv_computeCurveFromArc).
//!       Such knots are problematic when the B-spline curve is processed as
//!       periodic.  The remedy is usually a call to bspcurv_openCurve.
//! @remarks If pPoles is not NULL and this function returns true, then the first/last
//!       poles are equal, e.g., the B-spline curve is geometrically closed, but
//!       not periodically defined.  If pPoles is NULL and this function returns true,
//!       then the first/last poles are *usually* equal, but not always.
//! @param pKnots     IN      full knot vector (or at least the first 2*order knots)
//! @param numKnots   IN      number of knots given
//! @param pPoles     IN      full pole vector (optional)
//! @param pWeights   IN      full weight vector if rational (optional)
//! @param numPoles   IN      (if pPoles) number of poles
//! @param order      IN      order of B-spline curve
//! @param closed     IN      closure of B-spline curve
//! , mdlBspline_knotsShouldBeOpenedInV
//!
Public GEOMDLLIMPEXP bool     mdlBspline_knotsShouldBeOpened
(
const double*   pKnots,
int             numKnots,
const DPoint3d* pPoles,
const double*   pWeights,
int             numPoles,
int             order,
int             closed
);

//!
//! @description Returns true if the associated B-spline curve is "closed" in the
//!       nonperiodic V7 sense (cf. bspcurv_closeCurve and bspconv_computeCurveFromArc).
//!       Such knots are problematic when the B-spline curve is processed as periodic.
//!       The remedy is usually a call to bspcurv_openCurve.
//! @remarks If this function returns true, then the first/last poles are equal, e.g.,
//!       the curve is geometrically closed, but not periodically defined.
//! , mdlBspline_surfaceShouldBeOpenedInV
//!
Public GEOMDLLIMPEXP bool     mdlBspline_curveShouldBeOpened
(
const MSBsplineCurve    *pCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     mdlBspline_knotsAreValid                                    |
|                                                                       |
|                                                                       |
| Always looks for nontrivial range and nondecreasing knots.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_areKnotsValid
(
const   double*         pKnots,             /* full knot vector */
const   BsplineParam*   pParams,
        bool            bCheckExteriorKnots /* clamped if open, periodically extended if closed */
);

Public GEOMDLLIMPEXP StatusInt mdlBspline_computeNormal
(
DPoint3d*       pNormal,
DPoint3d*       pOrigin,
bool*           pIsPlanar,
DPoint3d*       pPoles,
const double*   pWeights,
int             numPoles,
const DPoint3d* pDefaultNormal,
double          absoluteTolerance,
double          relativeTolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

