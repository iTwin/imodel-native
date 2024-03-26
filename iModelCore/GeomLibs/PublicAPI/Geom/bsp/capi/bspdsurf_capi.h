/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_leastSquaresToSurface
(
MSBsplineSurface    *surface,
double              *avgDistance,
double              *maxDistance,
DPoint3d            *points,
DPoint2d            *uvValues,
int                 *uNumPoints,
int                 vNumPoints
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_surfaceOfRevolution
(
MSBsplineSurface    *surface,       /* OUT     Surface of revolution */
MSBsplineCurveCP    curve,         /* IN      Boundary curve */
DPoint3dCP          center,        /* IN      Center axis point */
DPoint3dCP          axis,          /* IN      Axis direction vector */
double              start,          /* IN      Start angle */
double              sweep           /* IN      Sweep angle */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_surfaceOfProjection
(
MSBsplineSurface    *surface,
MSBsplineCurveCP    curve,
DPoint3dCP          delta
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspsurf_orderCoonsCurves
(
MSBsplineCurve  *curves,
int             numCv
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_coonsPatch
(
MSBsplineSurface    *surface,          /* B-spline Coons patch surface */
MSBsplineCurve      *curves            /* bounding B-spline curve */
);

/*---------------------------------------------------------------------------------**//**
* (1,0)---q[1]---(1,1) | | (u) p[0] p[1] | | | | (0,0)---q[0]---(0,1) (v)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_gregoryPatch
(
MSBsplineSurface    *surface,          /* B-spline Coons patch surface */
MSBsplineCurve      *curves            /* bounding B-spline curve */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_gordonSurface
(
MSBsplineSurface    *surface,          /* OUT     B-spline Gordon surface */
MSBsplineCurve      *uCurves,          /* IN      network of B-spline curves */
MSBsplineCurve      *vCurves,          /* IN      network of B-spline curves */
int                 numUCurves,        /* IN      number of curves in U direction */
int                 numVCurves         /* IN      number of curves in V direction */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_approxByCubicNonRational
(
MSBsplineCurve      *outCv,             /* IN      non-rational curve of order 4 */
MSBsplineCurve      *inCv               /* IN      rational curve of order 3 */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_tubeSurface
(
MSBsplineSurface    *surface,          /* OUT     surface */
MSBsplineCurveCP    trace,            /* IN      B-spline curve */
MSBsplineCurveCP    section,          /* IN      section to sweep */
DSegment3d          *orientation,      /* IN  unused -- but cannot be null */
double              *cuspRadius,       /* IN unused -- can be null */
bool                rigidSweep         /* IN true to just translate section, false to rotate with trace */
);

/*---------------------------------------------------------------------------------**//**
* ----------- | | (u) section0 | | section1 | | -- trace0 - (v)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_skinPatch
(
MSBsplineSurface    *surface,
MSBsplineCurve      *trace0,
MSBsplineCurve      *trace1,
MSBsplineCurve      *section0,
MSBsplineCurve      *section1,
DSegment3d          *orientation0,
DSegment3d          *orientation1,
int                 checkInflectionPt
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_skinSurface
(
MSBsplineSurface    *surface,
MSBsplineCurve      *trace,
MSBsplineCurve      *trace1,       /* IN      second trace, or NULL */
MSBsplineCurve      **sections,    /* IN      pre-aligned in the xy plane */
double              *params,
int                 numSections
);

/*---------------------------------------------------------------------------------**//**
* Replace the point arrays in the PointLists with arrays holding two more points each (from c2Cubic fit). The number of list does not change.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsproll_generateArcPoles
(
PointList       *ptList,               /* OUT     middle pole of arc, IN      center of arc */
PointList       *n0List,               /* IN OUT  first pole of arc */
PointList       *n1List,               /* IN OUT  last pole of arc */
int             numList,               /* IN      number of list */
int             (*rhoFunc) (),         /* IN      varying rho function, or NULL */
void            *userDataP             /* IN      arguments passed through to rhoFunc */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bsproll_extractListsFromData
(
PointList           **ptList,          /* IN OUT  OSIC points */
PointList           **n0List,          /* IN OUT  normals to surface0 */
PointList           **n1List,          /* IN OUT  normals to surface1 */
int                 *numList,          /* IN OUT  number of lists returned */
BsurfBoundary       **bnd0,            /* IN OUT  trim boiundaries for surface0, or NULL */
int                 *numBnd0,          /* IN OUT  number of trim boundaries for surface0 */
BsurfBoundary       **bnd1,            /* IN OUT  trim boiundaries for surface1, or NULL */
int                 *numBnd1,          /* IN OUT  number of trim boundaries for surface1 */
IntLink             *chainP,           /* IN      data adjusted for varying radius */
MSBsplineSurface    *surface0,         /* IN      first surface */
MSBsplineSurface    *surface1,         /* IN      second surface */
SsiTolerance        *ssiTolP           /* IN      tolerance used in SSI calculations */
);

/*---------------------------------------------------------------------------------**//**
* see Choi, B.K. Surface Modeling for CAD/CAM, pp275-278.
* The data comes in in the IntLink as follows: xyz = the actual intersection point at avgRadius norm0 = normal direction of eval0 (NOT
* normalized) norm1 = normal direction of eval1 (NOT normalized) uv0 = parameters of xyz on eval0->surface uv1 = parameters of xyz on
* eval1->surface
* The data goes out in the IntLink as follows: xyz = the actual ball center point norm0 = ball contact point on eval0->surface norm1 = ball
* contact point on eval1->surface uv0 = parameters of norm0 on eval0->surface uv1 = parameters of norm1 on eval1->surface
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsproll_adjustOSIC
(
IntLink         *chain,                /* IN OUT  full OSIC data */
Evaluator       *eval0,                /* IN      first surface */
Evaluator       *eval1,                /* IN      second surface */
int             (*radFunc) (),         /* IN      varying radius function, or NULL */
void            *userDataP,            /* IN      arguments passed trhough to radFunc */
double          avgRadius,             /* IN      average radius */
double          tolerance              /* IN      3d convergence tolerance */
);


END_BENTLEY_GEOMETRY_NAMESPACE

