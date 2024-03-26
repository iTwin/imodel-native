/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------*//**
* @description Create a new (isolated) loop in the graph from an array of 3D points.
* @remarks short edges are culled by removing duplicate points within tol = abstol + reltol * max,
        where max is the largest xy-coordinate of the input points.
* @param graphP IN OUT graph to receive loop
* @param pPointBuffer IN coordinates around loop
* @param numPoints IN number of points
* @param abstol IN smallest allowed tolerance
* @param reltol IN relative tolerance (or nonpositive for default)
* @return pointer to some node around the new loop
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_loopFromDPoint3dArrayXYTol
(
VuSetP          graphP,
DPoint3dCP      pPointBuffer,
int             numPoints,
double          abstol,
double          reltol
);

/*--------------------------------------------------------------------*//**
* @description Create a new (isolated) loop in the graph from an array of 3D points.
* @remarks short edges are culled by removing duplicate points within tol = abstol + reltol * max,
        where max is the largest xy-coordinate of the input points.
* @param [in] graphPgraph to receive loop
* @param [in] worldToLocal
* @param [in] pPointBuffer coordinates around loop
* @param [in] numPoints number of points
* @param [in] abstol smallest allowed tolerance
* @param [in] reltol relative tolerance (or nonpositive for default)
* @return pointer to some node around the new loop
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_loopFromDPoint3dArrayXYTol
(
VuSetP          graphP,
TransformCP     worldTolLocal,
DPoint3dCP      pPointBuffer,
int             numPoints,
double          abstol,
double          reltol
);


/*---------------------------------------------------------------------------------**//**
* @description Create a new (isolated) loop in the graph from an array of points.
* @remarks Duplicate points are eliminated within a fixed tolerance.
* @param graphP         IN OUT  graph to receive loop
* @param xyP            IN      uv-coordinates around loop
* @param nxy            IN      number of points
* @param markExterior   IN      whether to mark the right side of the path EXTERIOR
* @param testArea       IN      (if markExterior is true) whether to do a signed area test to determine
*                               if the exterior is to the right or left.  If false, the right side is marked.
* @return pointer to some node around the new loop
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP vu_makeLoopFromArray
(
VuSetP graphP,
DPoint2d *xyP,
int nxy,
bool    markExterior,
bool    testArea
);

/*---------------------------------------------------------------------------------**//**
* @description Create a new (isolated) loop in the graph from an array of 3D points.
* @remarks Duplicate points are eliminated if all three coordinates fall within a tolerance based on a fixed fraction of the xy-range.
* @param graphP         IN OUT  graph to receive loop
* @param pointP         IN      uv-coordinates around loop
* @param nPoints        IN      number of points
* @param markExterior   IN      whether to mark the right side of the path EXTERIOR
* @param testArea       IN      (if markExterior is true) whether to do a signed area test to determine
*                               if the exterior is to the right or left.  If false, the right side is marked.
* @return pointer to some node around the new loop
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_makeLoopFromArray3d
(
VuSetP          graphP,
DPoint3d        *pointP,
int             nPoints,
int             markExterior,
int             testArea
);

/*---------------------------------------------------------------------------------**//**
* @description Create a chain of VU edges in the graph from an array of points.
* @remarks Optionally skips short edges using xy and/or xyz length as tolerance.
* @param graphP             IN OUT  graph to receive chain
* @param pointP             IN      coordinates around loop
* @param nPoints            IN      number of points
* @param leftMask           IN      mask bits to apply to the left of each edge (typically VU_BOUNDARY_EDGE)
* @param rightMask          IN      mask bits to apply to the right of each edge (typically VU_EXTERIOR_EDGE and/or VU_BOUNDARY_EDGE)
* @param minXYEdgeLength    IN      absolute tolerance for testing minimum 2D edge length (if negative, no test is done)
* @param minXYZEdgeLength   IN      absolute tolerance for testing minimum 3D edge length (if negative, no test is done)
* @return pointer to first node in the new chain
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP      vu_makeEdgesFromArray3d
(
VuSetP          graphP,
DPoint3d        *pointP,
int             nPoints,
int             leftMask,
int             rightMask,
double          minXYEdgeLength,
double          minXYZEdgeLength
);

//! Add a single edge between coordinates.
//! @return nullptr if coordinates nearly equal.
Public VuP vu_addEdgeXYTol (
VuSetP          graph,          //!< [in] graph being expanded
DPoint3dCR xyz0,                //!< [in] start coordinate
DPoint3dCR xyz1,                //!< [in] end coordinate
double abstol,                  //!< [in] absolute tolerance for short edge
VuMask leftMask,                //!< [in] mask for left side of edge
VuMask rightMask                //!< [in] mask for right side of edge
);

Public GEOMDLLIMPEXP  void vu_addEdgesXYTol
(
VuSetP          graph,          //!< [in] graph being expanded
TransformCP     worldToLocal,   //!< [in] optional world to local transform
bvector<DPoint3d> const &data,  //!< [in] path or loop
bool closed,                    //!< [in] true to close back to first point.
double abstol,                  //!< [in] absolute tolerance for short edge.  Applied AFTER transform
VuMask leftMask,                //!< [in] mask for left side of edge
VuMask rightMask                //!< [in] mask for right side of edge
);

Public GEOMDLLIMPEXP void vu_addEdgesXYTol
(
VuSetP          graph,                  //!< [in] graph being expanded
TransformCP     worldToLocal,           //!< [in] optional world to local transform
bvector<bvector<DPoint3d>> const &data, //!< [in] multiple paths or loops
bool closed,                            //!< [in] true to close back to first point.
double abstol,                          //!< [in] absolute tolerance for short edge.  Applied AFTER transform
VuMask leftMask,                        //!< [in] mask for left side of edge
VuMask rightMask                        //!< [in] mask for right side of edge
);

Public GEOMDLLIMPEXP void vu_addEdgesXYTol
(
VuSetP          graph,                  //!< [in] graph being expanded
TransformCP     worldToLocal,           //!< [in] optional world to local transform
bvector<bvector<bvector<DPoint3d>>> const &data, //!< [in] multiple paths or loops
bool closed,                            //!< [in] true to close back to first point.
double abstol,                          //!< [in] absolute tolerance for short edge.  Applied AFTER transform
VuMask leftMask,                        //!< [in] mask for left side of edge
VuMask rightMask                        //!< [in] mask for right side of edge
);
END_BENTLEY_GEOMETRY_NAMESPACE

