/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_c1Discontinuities                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_c1Discontinuities
(
bvector<double> &params,
MSBsplineCurveCP curveP
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_freeCurve                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_freeCurve
(
MSBsplineCurve *curve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_freeInterpolationCurve                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_freeInterpolationCurve
(
MSInterpolationCurve *curve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_allocateCurve                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_allocateCurve
(
MSBsplineCurve *curve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_allocateInterpolationCurve                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_allocateInterpolationCurve
(
MSInterpolationCurve *curve
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_copyCurve                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_copyCurve
(
MSBsplineCurveP output,
MSBsplineCurveCP input
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_copyInterpolationCurve                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_copyInterpolationCurve
(
MSInterpolationCurve  *output,
MSInterpolationCurve  *input
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_isPhysicallyClosedBCurve                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool       bspcurv_isPhysicallyClosedBCurve
(
MSBsplineCurveCP    pCurve,        /* IN      input curve to be checked */
double              tolerance       /* IN      tolerance used to check dist in 3d space */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_chainRule                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    bspcurv_chainRule
(
DPoint3d        *out,
DPoint3d        *inPoles,
double          *inWeights
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_computeDerivatives                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_computeDerivatives
(
DPoint3d        *dervPoles,            /* must contain numDervs+1 Dpoint3ds */
double          *dervWts,              /* must contain numDervs+1 doubles */
MSBsplineCurveCP curve,
int             numDervs,              /* does not include point itself */
double          u,
int             leftHand               /* give left derivatives at knots */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_getTangent                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_getTangent
(
DPoint3d        *tangent,
double          param,
MSBsplineCurve  *curve,
int             leftHand
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonNormal                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_polygonNormal
(
DPoint3d        *normal,                        /* OUT     normal of polygon */
DPoint3d        *vertices,
int             nvertices
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_frenetFrame                                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_frenetFrame
(
DPoint3d        *frame,                /* OUT     Frenet frame t,m,b */
DPoint3d        *point,                /* OUT     origin of frame (on curve) */
double          *curvature,
double          *torsion,
MSBsplineCurveCP curve,                /* IN      curve structure */
double          u,                     /* IN      parameter value */
DPoint3d        *defaultM              /* IN      normal for linear, or NULL */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_inflectionPoints                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_inflectionPoints
(
double          **params,
int             *numPoints,
MSBsplineCurve  *curve
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_openCurve                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_openCurve
(
MSBsplineCurveP outCurve,
MSBsplineCurveCP inCurve,
double          u
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_closeCurve                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_closeCurve
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  const*inCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_makeBezier                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_makeBezier
(
MSBsplineCurve  *outCurve,
MSBsplineCurve const *inCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_makeRational                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_makeRational
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_reverseCurve                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_reverseCurve
(
MSBsplineCurve  const*inCurve,
MSBsplineCurve  *outCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_knotToPointAndTangent                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_knotToPointAndTangent
(
DPoint3d        *point,
DPoint3d        *tangent,
double          *weight,
double          x,
DPoint3d        *poles,
int             order,
int             numPoles,
double          *t,
double          *weights,
int             rational,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_computeCurvePoint                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_computeCurvePoint
(
DPoint3d        *point,
DPoint3d        *tangent,
double          *weight,
double          fraction,
DPoint3d        *poles,
int             order,
int             numPoles,
double          *t,
double          *weights,
int             rational,
int             closed
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_evaluateCurvePoint                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspcurv_evaluateCurvePoint
(
DPoint3d        *pointP,            /* OUT     point */
DPoint3d        *tangentP,          /* OUT     tangent */
MSBsplineCurve const *curveP,            /* IN      curve */
double          u                   /* IN      u */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_generateSymmetricFunctions                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     bspcurv_generateSymmetricFunctions
(
double          *f,
double          *x,
int             degree
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_elevateDegree                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_elevateDegree
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve,
int             newDegree
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_segmentCurve                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_segmentCurve
(
MSBsplineCurve  *segment,
MSBsplineCurveCP inCurve,
double          uInitial,
double          uFinal
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_combineCurves                                   |
|                                                                       |
|   contiguous determines how the pole arrays are combined              |
|                                                                       |
|   CONTINUITY_NONE:  all poles of curve1, all poles of curve2.         |
|                                                                       |
|   In all other cases, outCurve has one less pole than the two inputs  |
|   CONTINUITY_LEFT:  all poles of curve1, lose the first pole of curve2|
|                                                                       |
|   CONTINUITY_MID:   bisect the last pole of curve1 and the first      |
|                     pole of curve2                                    |
|   CONTINUITY_RIGHT: lose the last pole of curve1, all poles of curve2 |
|                                                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_combineCurves
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2,
bool            contiguous,
bool            reparam
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_contiguousCurves                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bspcurv_contiguousCurves
(
MSBsplineCurveCP curve1,
MSBsplineCurveCP curve2
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_appendCurves                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_appendCurves
(
MSBsplineCurve  *combinedCurve,
MSBsplineCurve  *inCurve1,
MSBsplineCurve  *inCurve2,
bool            forceContinuity,
bool            reparam
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_segmentDisjointCurve                            |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_segmentDisjointCurve
(
MSBsplineCurve    **segCurves,          /* OUT     nondisjoint curves */
int               *nSegCurves,          /* OUT     number of nondisjoint curves */
MSBsplineCurveCP    curve               /* IN      input curve */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_extractEndPoints                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspcurv_extractEndPoints
(
DPoint3d        *startP,
DPoint3d        *endP,
MSBsplineCurveCP curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_extractNormal                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_extractNormal
(
DPoint3d          *normalP,               /* OUT     curve normal */
DPoint3d          *positionP,             /* OUT     average pole position */
double            *planarDeviationP,      /* OUT     deviation from planar */
MSBsplineCurveCP  curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_rotateCurve                                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_rotateCurve
(
MSBsplineCurve          *outCurveP,         /* OUT     rotated curve */
MSBsplineCurve          *inCurveP,          /* IN      input curve */
RotMatrixCP             rMatrixP           /* IN      rotation matrix */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_transformCurve                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_transformCurve
(
MSBsplineCurveP         outCurveP,         /* <= transformed curve */
MSBsplineCurveCP        inCurveP,          /* => input curve */
Transform const         *transformP         /* IN      transform */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_transformCurve4d                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_transformCurve4d
(
MSBsplineCurveP         outCurveP,         /* <= transformed curve */
MSBsplineCurveCP        inCurveP,          /* => input curve */
DMatrix4dCP             transform4dP         /* => transform */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_isPhysicallyClosed                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspcurv_isPhysicallyClosed
(
MSBsplineCurveCP  curve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_c1Discontinuities                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_c1Discontinuities
(
double          **paramPP,
int             *nParamsP,
MSBsplineCurve  *curveP
);


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_areCurvesIdentical                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspcurv_areCurvesIdentical
(
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_polygonLength                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bspcurv_polygonLength
(
MSBsplineCurve const *curveP
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_computeInflectionPoints                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_computeInflectionPoints
(
DPoint3d        **ppPoints,    /* OUT     point on curve evaluated at param */
double          **ppParams,    /* OUT     point on curve evaluated at param */
int             *pNumPoints,
MSBsplineCurve  *pCurve
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_createBsplineFromPointsAndOrder
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_createBsplineFromPointsAndOrder
(
MSBsplineCurve  *pCurve,
DPoint3d        *pPoints,
int             numPoints,
int             order
);

END_BENTLEY_GEOMETRY_NAMESPACE

