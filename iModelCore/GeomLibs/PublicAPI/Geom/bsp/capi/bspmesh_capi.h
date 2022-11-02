/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

Public GEOMDLLIMPEXP int bspmesh_getCurveSteps
(
int                 uNum,
int                 vNum,
int                 uLoNum,
int                 uHiNum,
int                 vLoNum,
int                 vHiNum,
DPoint2d            *orgP,
DPoint2d            *endP,
DPoint2d            *bezierScale,
double              bezierUMin,                 /* IN      u Minimum for bezier */
double              bezierUMax,                 /* IN      u Maximum for bezier */
double              bezierVMin,                 /* IN      v Minimum for bezier */
double              bezierVMax                  /* IN      v Maximum for bezier */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspmesh_numMeshSizeSurf
(
int                 *uNum,              /* OUT     # steps in U */
int                 *vNum,              /* OUT     # steps in V */
int                 *uLoNum,            /* OUT     # steps along u Lo boundary */
int                 *uHiNum,            /* OUT     # steps along u Hi boundary */
int                 *vLoNum,            /* OUT     # steps along v Lo boundary */
int                 *vHiNum,            /* OUT     # steps along v Hi boundary */
MSBsplineSurface    *patchBezP,         /* IN      bezier patch */
double              tol,                /* IN      tolerance */
int                 toleranceMode       /* IN      tolerance mode */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_getCurveSteps
(
int                 uNum,
int                 vNum,
int                 uLoNum,
int                 uHiNum,
int                 vLoNum,
int                 vHiNum,
DPoint2d            *orgP,
DPoint2d            *endP,
DPoint2d            *bezierScale,
double              bezierUMin,                 /* IN      u Minimum for bezier */
double              bezierUMax,                 /* IN      u Maximum for bezier */
double              bezierVMin,                 /* IN      v Minimum for bezier */
double              bezierVMax                  /* IN      v Maximum for bezier */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_evaluateMeshBezier
(
DPoint3d            *meshPointsP,   /* OUT     mesh pts with u direction goes fast */
DPoint3d            *meshNormP,     /* OUT     mesh normal or NULL */
MSBsplineSurface    *patchBezP,     /* IN      Bezier Patch */
double              uLo,            /* IN      lower bound in u direction */
double              uHi,            /* IN      upper bound in u direction */
double              vLo,            /* IN      lower bound in v direction */
double              vHi,            /* IN      upper bound in u direction */
int                 uNum,           /* IN      num of points in u direction */
int                 vNum,           /* IN      num of points in v direction */
bool                reverse         /* IN      true to reverse U direction and normal */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_evaluatePointsBezier
(
DPoint3d            *pointsP,       /* OUT     points on Bezier surface */
DPoint3d            *normalP,       /* OUT     normal points */
MSBsplineSurface    *patchBezP,     /* IN      Bezier patch */
int                 num,            /* IN      number of points */
DPoint2d            **uvPP,         /* IN      uv values on parameter space */
double              uMin,           /* IN      u Minimum (entire surface) */
double              uMax,           /* IN      u Maximum (entire surface) */
double              vMin,           /* IN      v Minimum (entire surface) */
double              vMax,           /* IN      v Maximum (entire surface) */
bool                reverseNormal   /* IN      if true negate normal (dPdV X DPdU) */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_calculateParameterLengths
(
DPoint2d            *lengthP,
MSBsplineSurface    *surfaceP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_calculateParameterLengthsExact
(
DPoint2d            *lengthP,
MSBsplineSurface    *surfaceP
);

void bspmesh_calculateControlPolygonLengthsAndTurn
(
MSBsplineSurfaceCR surface,
bvector <double> &uLength,  //!< u direction length of control polygon [i] edges
bvector<double> &uTurn,     //!< u direction turning angle summed accross polygon [i] edges
bvector<double> &vLength,
bvector<double> &vTurn
);


/*---------------------------------------------------------------------------------**//**
* This is invoked by rendering to signal the start and end of large blocks of meshing. The memory used for VU graphs on surfaces is recycled
* within these blocks and returned to the system at the end All other users of bspemsh_meshSurface recycle the memory after each surface.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_setMemoryCaching
(
int     mode            /* true for caching on, false for caching off */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurfaceExt2
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* IN      mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* IN      triangle strip function */
double                                      tolerance,                  /* IN      tolerance */
int                                         toleranceMode,              /* IN      tolerance mode */
TransformP                                  toleranceTransformP,        /* IN      tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* IN      tolerance camera position */
double                                      toleranceFocalLength,       /* IN      tolerance focal length */
double                                      angleTolerance,             /* IN      angle tolerance */
DPoint2dP                                   parameterScale,             /* IN      parameter scale */
MSBsplineSurface                            *surfaceP,                  /* IN      surface to mesh */
bool                                        normalsRequired,            /* IN      true to return normals */
bool                                        parametersRequired,         /* IN      true to return parameters */
bool                                        reverse,                    /* IN      true to reverse order */
bool                                        covePatchBoundaries,        /* IN      true to cove patch bounds */
CallbackArgP                                userDataP                   /* IN      user data */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurfaceExt
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* => mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* => triangle strip function */
double                                      tolerance,                  /* => tolerance */
int                                         toleranceMode,              /* => tolerance mode */
TransformP                                  toleranceTranformP,         /* => tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* => tolerance camera position */
double                                      toleranceFocalLength,       /* => tolerance focal length */
double                                      angleTolerance,             /* => angle tolerance */
DPoint2dP                                   parameterScale,             /* => parameter scale */
MSBsplineSurface*                           surfaceP,                   /* => surface to mesh */
bool                                        normalsRequired,            /* => true to return normals */
bool                                        parametersRequired,         /* => true to return parameters */
bool                                        reverse,                    /* => true to reverse order */
bool                                        covePatchBoundaries,        /* => true to cove patch bounds */
CallbackArgP                                userDataP                   /* => user data */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_fixBoundaries
(
BsurfBoundary   **boundsPP,
int             *numBoundsP,
bool            conserveParity
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_fixSurfaceBoundaries
(
MSBsplineSurface    *surfaceP,
bool                conserveParity
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspmesh_addBoundaries
(
MSBsplineSurface    *surfaceP,
BsurfBoundary       *boundsP,
int                 numBounds,
int                 flipHoleOrigin
);

/*
@description Use bspmesh_fixBoundaries to resolve self intersections in a polygon.
The output is an array of BsurfBoundary structures.
The array of BsurfBoundary structures is itself allocated by BSIBaseGeom::Malloc.
Each of the BsurfBoundary structures represents 1 loop of the resolved polygon, and has a pointer
    to an array of points.  That array is also allocated by BSIBaseGeom::Malloc.
Caller is responsible to call BSIBaseGeom::Free for the "points" in each loop, and for the array of
    headers.

@param ppBoundaries OUT array of headers.
@param pNumBoundaries OUT number of boundaries returned.
@param pXYZBuffer IN points on input loop.
@param numXYZ IN number of input points.
*/
Public GEOMDLLIMPEXP StatusInt fixPolygon
(
BsurfBoundary **ppBoundaries,
int         *pNumBoundaries,
DPoint3d *pXYZBuffer,
int numXYZ
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspmesh_meshSurface
(
BSplineCallback_AnnounceMeshQuadGrid        meshFunction,               /* IN      mesh function */
BSplineCallback_AnnounceMeshTriangleStrip   triangleFunction,           /* IN      triangle strip function */
double                                      tolerance,                  /* IN      tolerance */
int                                         toleranceMode,              /* IN      tolerance mode */
TransformP                                  toleranceTransformP,        /* IN      tolerance transform */
DPoint3dP                                   toleranceCameraP,           /* IN      tolerance camera position */
double                                      toleranceFocalLength,       /* IN      tolerance focal length */
DPoint2dP                                   parameterScale,             /* IN      parameter scale */
MSBsplineSurface const*                     surfaceP,                   /* IN      surface to mesh */
bool                                        normalsRequired,            /* IN      true to return normals */
bool                                        parametersRequired,         /* IN      true to return parameters */
bool                                        reverse,                    /* IN      true to reverse order */
bool                                        covePatchBoundaries,        /* IN      true to cove patch bounds */
CallbackArgP                                userDataP                   /* IN      user data */
);

/*--------------------------------------------------------------------*//**
* @description Compute range of a surface.  This range is based on a
*   mesh to specified tolerance.
* @param pRange OUT     computed range.
* @param pSurface IN      surface to examine.
* @param tolerance IN      mesh tolerance.
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_surfaceMeshRange
(
DRange3d *pRange,
MSBsplineSurface *pSurface,
double tolerance
);

/*--------------------------------------------------------------------*//**
* @description Compute min and max distances from a plane to a bspline surface.
* @param pSignedLow OUT     signed coordinate of minimum point when coordinates are
*               measured perpendicular to the surface.
* @param pSignedHigh OUT     signed coordinate of maximum point when coordinates are
*               measured perpendicular to the surface.
* @param pAbsLow OUT     absolute shortest distance from plane to any point on the surface.
*               (0 if plane cuts the surface)
* @param pAbsHigh OUT     absolute farthest distance from plane to any point on the surface.
* @param pOrigin IN      plane origin.
* @param pNormal IN      plane normal.
* @param pSurface IN      surface to examine.
* @param tolerance IN      tolerance for chordal approximation to surface.
*------------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_planeSurfaceDistance
(
double *pSignedLow,
double *pSignedHigh,
double *pAbsLow,
double *pAbsHigh,
DPoint3d *pOrigin,
DPoint3d *pNormal,
MSBsplineSurface *pSurface,
double tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

