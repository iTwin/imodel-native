/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Split a set of loops into smaller convex polygons.
* @param loops    [in]  array of loops
* @param convexFaces [out] array of faces.
* @param numWrap [in] number of wraparound points to add.  (0 for one point per edge, 1 to add single closure)
* @param isBoundary [out] optional array identifying boundary edges.
* @return SUCCESS unless graph allocation failed
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_splitToConvexParts
(
bvector<bvector<DPoint3d>>  const &loops,
uint32_t numWrap,
bvector<bvector<DPoint3d>>  &convexFaces,
bvector<bvector<bool>> *isBoundary
);

/*---------------------------------------------------------------------------------**//**
* @description Split a single loops into smaller convex polygons.
* @param loop    [in]  array of loops
* @param convexFaces [out] array of faces.
* @param numWrap [in] number of wraparound points to add.  (0 for one point per edge, 1 to add single closure)
* @param isBoundary [out] optional array identifying boundary edges.
* @return SUCCESS unless graph allocation failed
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_splitToConvexParts
(
bvector<DPoint3d>  const &loop,
uint32_t numWrap,
bvector<bvector<DPoint3d>>  &convexFaces,
bvector<bvector<bool>> *isBoundary
);

END_BENTLEY_GEOMETRY_NAMESPACE

