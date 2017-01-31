//
//
#include <Geom/GeomApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL



// Return a bspline surface whose control points are on a doubly sinusoidal surface.
// @param [in] uOrder u direction order
// @param [in] vOrder v direction order
// @param [in] numI number of poles in i direction
// @param [in] numJ number of poles in j direction
// @param [in] q0I start angle for u direction sine calls
// @param [in] qI angle step between successive u direction sine calls.
// @param [in] q0J start angle for v direction sine calls
// @param [in] qJ angle step between successive v direction sine calls.
// @remarks Control point i,j is at (x,y,z) = (i,j,sin(q0I + aI * i) * sin (q0J + aJ * j))
MSBsplineSurfacePtr SurfaceWithSinusoidalControlPolygon
(int uOrder, int vOrder, size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ);

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

PolyfaceHeaderPtr Mesh_XYGrid (int numXEdge, int numYEdge, double edgeX, double edgeY, bool triangulate);

