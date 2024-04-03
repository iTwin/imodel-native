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
Public GEOMDLLIMPEXP StatusInt    bspssi_projectToTangentSystem
(
DPoint3d            *paramP,            /* OUT     x,y are parameters of projection
                                                of pointP onto skewed plane.  z=0 */
double              *duduP,             /* OUT     squared magnitude of U vector */
double              *dvdvP,             /* OUT     squared magnitude of V vector */
DPoint3d            *vectorUP,          /* IN      parameter space U direction */
DPoint3d            *vectorVP,          /* IN      parameter space V direction */
DPoint3d            *originP,           /* IN      origin of parameter system */
DPoint3d            *pointP,            /* IN      space point to be projected */
double              shortVecTol         /* IN      tolerance to consider short vectors zero */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToDifficultSurface
(
DPoint3d            *nearPt,           /* OUT     closest point on surface */
DPoint3d            *normal,           /* OUT     normal to surf @ nearPt, scaled by sine angle */
double              *degeneracy,       /* OUT     sine of angle between surface partials */
DPoint2d            *uv,               /* IN OUT  initial guess -> param of pt */
DPoint3d            *testPt,           /* IN      want closest pt to this pt */
Evaluator           *eval,
int                 iterations,        /* IN      number of iterations to try */
double              convTol            /* IN      convergence tolerance in UV space */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToSurface
(
DPoint3d            *nearPt,           /* OUT     closest point on surface */
DPoint3d            *normal,           /* OUT     normal to surf @ nearPt, scaled by sine angle */
double              *degeneracy,       /* OUT     sine of angle between surface partials */
DPoint2d            *uv,               /* IN OUT  initial guess -> param of pt */
DPoint3d            *testPt,           /* IN      want closest pt to this pt */
Evaluator           *eval,
int                 iterations,        /* IN      number of iterations to try */
double              convTol            /* IN      convergence tolerance in UV space */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToPatch
(
DPoint3d            *nearPt,           /* OUT     closest point on surface */
DPoint3d            *normal,           /* OUT     normal to surf @ nearPt */
DPoint2d            *uv,               /* IN OUT  initial guess -> param of pt */
DPoint3d            *testPt,           /* IN      want closest pt to this pt */
MSBsplineSurface    *surface,
int                 iterates
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_addLink
(
IntLink         **chain,
IntLink         *link,
int             addToBackwards
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bspssi_ssiSort
(
int                 *rank,
BezierInfo          *infoP,
int                 *uStart0,
int                 *vStart0,
int                 *uStart1,
int                 *vStart1,
int                 uNumSegs0,
int                 vNumSegs0,
int                 uNumSegs1,
int                 vNumSegs1,
MSBsplineSurface    *surface0,
MSBsplineSurface    *surface1
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bspssi_ssiGo
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1,
int                 uSeg0,
int                 vSeg0,
int                 uSeg1,
int                 vSeg1,
int                 uStart0,        /* IN      knot offset of bez0 in U */
int                 vStart0,        /* IN      knot offset of bez0 in V */
int                 uStart1,        /* IN      knot offset of bez1 in U */
int                 vStart1,        /* IN      knot offset of bez1 in V */
int                 numU0,
int                 numV0,
int                 numU1,
int                 numV1
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_ssiBezier
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspssi_freeLink
(
IntLink         **linkPP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_extractLinks
(
IntLink             **out,               /* OUT     link of only one uv0, uv1, or xyz */
IntLink             *chain,              /* IN      chain containing all info */
int                 code,                /* IN      choice of uv0, uv1, or xyz */
SsiTolerance        *ssiTolP,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointList
(
PointList       **listPP,           /* IN OUT  point list to append to */
int             *numLists,          /* IN OUT  number of lists */
PointList       *newList            /* IN      list to append */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointList_clearInput
(
PointList       **listPP,           /* <=> point list to append to */
int             *numLists,          /* <=> number of lists */
bvector<DPoint3d> newPoints         /* new points.  This array is cleared after the copy.*/
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointToList
(
PointList       *pointList,         /* IN OUT  list to append to */
DPoint3d        *point              /* IN      point to append */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_assignLinks
(
void            **linkPP,
int             *numLinks,
IntLink         **chain,
int             code,
SsiTolerance    *ssiTolP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_compareIntersections
(
const BoundIntersect    *biP1,
const BoundIntersect    *biP2,
const void              *optArgsP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_extractPointLists
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
IntLink             **chain,
SsiTolerance        *ssiTolP,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              *offset0P,
double              *offset1P
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfacesExtra
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1,
int                 tightTolerance
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfaces
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfacesTight
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1
);

END_BENTLEY_GEOMETRY_NAMESPACE

