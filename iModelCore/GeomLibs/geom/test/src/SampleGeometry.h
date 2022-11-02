/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//
//
#include <Geom/GeomApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL



// Return a bspline surface whose control points are evaluated on a hyperbolic patch.
// The patch has three points (00) (10) (01) at unit squre points and a fourth specified by parameters.
// @param [in] uOrder u direction order
// @param [in] vOrder v direction order
// @param [in] numI number of poles in i direction
// @param [in] numJ number of poles in j direction
// @param [in] x11 x coordinate of patch 11 point
// @param [in] y11 y coordinate of patch 11 point
// @param [in] z11 z coordinate of patch 11 point.
// @param [in] u1 upper u to evaluate grid
// @param [in] v1 upper v to evaluate grid.
MSBsplineSurfacePtr HyperbolicGridSurface (size_t uOrder, size_t vOrder, size_t numI, size_t numJ, 
double x11, double y11, double z11, double u1, double v1);

// Return a 2x2 linear bspline surface with control points (000)(100)(010)(u1 v1 w1)
MSBsplineSurfacePtr SimpleBilinearPatch (double u1, double v1, double w1);

PolyfaceHeaderPtr Mesh_XYGrid (int numXEdge, int numYEdge, double edgeX, double edgeY, bool triangulate, bool params = false);

// various arc samples, initially intended for use in creating point clouds .....
extern bvector<DEllipse3d> s_arcSamples;

// return a strip with arc points on one side, offset on the other.
bvector<DPoint3d> CreateOffsetStrip (DEllipse3dCR arc, double offset0, double offset1, size_t numStroke, bool close = true);

// return a "T" shape
bvector<DPoint3d> CreateT (
double dxTotal, //!< [in] total width of range box
double dyTotal, //!< [in] total height of range box
double dxLeft,  //!< [in] x distance under the left bar
double dxRight, //!< [in] x distance under the right bar
double dyLeft,  //!< [in] vertical width of left bar (measured from top down
double dyRight,      //!< [in] vertical width of right bar (measuredFrom top down)
bool close  = true  //!< [in] true to add closure point.
);

// return a "T" shape
CurveVectorPtr CreateFilletedSymmetricT (
double dxTotal, //!< [in] total width of range box
double dyTotal, //!< [in] total height of range box
double dxLeft,  //!< [in] x distance under the left bar
double dyLeft,  //!< [in] vertical width of left bar (measured from top down
double dxFillet,    //!< [in] x size under elliptic fillet
double dyFillet,    //!< [in] y size under elliptic fillet
bool splitUpperEdge
);


void SaveEdgeChains (PolyfaceHeaderR facets, bool showNoChainX);

PolyfaceHeaderPtr DodecahedronMesh();