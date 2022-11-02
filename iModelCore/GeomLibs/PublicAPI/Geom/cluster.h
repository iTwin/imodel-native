/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Identify clusters of nearly-identical coordinates.
//! @param [in] xyzArray       array of xyz coordinatss.
//! @param [out]pClusterArray      array of (-1)-deliminted blocks of nodes within tolerance
//! @param [in] absTol       absolute tolerance for xy-coordinate comparison
//! @param [in] bReassignXYZ       true to immediately update points to identical coordinates.
//! @param [in] bXYZ       true for 3d, false for xy
//! @param [out] oldToNew   mapping from old index to new packedXYZ entry.
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
bvector<DPoint3d> & xyzArray,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ,
bvector<size_t> *oldToNew
);

/**
* @description Identify clusters of nearly-identical coordinates.
* @param xyzArray                   IN      array of xyz coordinatss.
* @param pClusterArray              OUT     array of (-1)-deliminted blocks of nodes within tolerance
* @param packedArray                OUT     optional array to receive single representatives
* @param absTol                     IN      absolute tolerance for xy-coordinate comparison
* @param bReassignXYZ               IN      true to immediately update points to identical coordinates.
* @param bXYZ                       IN      true for 3d, false for xy
* @param [out] oldToNew   mapping from old index to new packedXYZ entry.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
DPoint3dP xyzArray,
size_t numXYZ,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ,
bvector<size_t> *oldToNew
);

Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
bvector<DPoint3d> & xyzArray,
bvector<int> & blockedIndexArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ
);

Public GEOMDLLIMPEXP void bsiDPoint3dArray_findClusters
(
DPoint3dP xyzArray,
size_t numXYZ,
bvector<int> & blockedIndexArray,
bvector<DPoint3d> *packedArray,
double          absTol,
bool            bReassignXYZ,
bool            bXYZ
);


END_BENTLEY_GEOMETRY_NAMESPACE
