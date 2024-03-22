/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Scan the edges of a polygon and determine the closest approach to a test point.
//! @param pNearestPoint OUT     coordinates of closest point.
//! @param pClassificationVector OUT     cross product of the vector along the closest
//!               edge with the vector from the edge base to the test point.
//!               If the polygon has a known orientation and return type is 2 or 3, the
//!               dot product of this vector and the polygon normal is positive iff the
//!               point is inside the polygon.
//! @param pParam OUT     parametric coordinate along nearest edge
//! @param pBaseVertexId OUT     index of base vertex of nearest edge
//! @param pPointArray IN      polygon vertices
//! @param nPoint IN      number of vertices
//! @param pPoint IN      point to test
//! @param tolDistSquared IN      squared absolute tolerance for deciding if the point is on an edge or vertex.
//! @return 0 if the test point is on a vertex,
//!         1 if along an edge interior,
//!         2 if not on any edge and projects to an edge interior,
//!         3 if not on any edge and projects to a vertex.
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiPolygon_closestEdgeToPoint
(
DPoint3dP pNearestPoint,
DPoint3dP pClassificationVector,
double      *pParam,
int         *pBaseVertexId,
DPoint3dCP pPointArray,
int         nPoint,
DPoint3dCP pPoint,
double      tolDistSquared
);

//!
//! @description Return the point where a ray pierces a polygon, and classify it as interior, exterior, edge or vertex hit.
//!
//! @param pPierce        OUT     point where the ray pierces the plane
//! @param pParam         OUT     parametric coordinate of nearest edge point
//! @param pBaseVertexId  OUT     index of base vertex of edge
//! @param pPointArray    IN      vertices of polygon
//! @param nPoint         IN      number of polygon vertices
//! @param pNormal        IN      polygon normal.  If null pointer is given, the normal is recomputed.
//! @param pOrigin        IN      origin of ray
//! @param pDirection     IN      ray direction
//! @param tolDistSquared IN      squared absolute tolerance for on-edge decision
//! @return -3 if ray passes through polygon exterior and projects to a vertex,
//!         -2 if ray passes through polygon exterior and projects to the interior of an edge,
//!         -1 if no intersection,
//!          0 if ray passes through a vertex,
//!          1 if ray passes through the interior of an edge,
//!          2 if ray passes through polygon interior and projects to the interior of an edge, or
//!          3 if ray passes through polygon interior and projects to a vertex.
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiPolygon_piercePoint
(
DPoint3dP pPierce,
double      *pParam,
int         *pBaseVertexId,
DPoint3dCP  pPointArray,
int         nPoint,
DPoint3dCP  pNormal,
DPoint3dCP  pOrigin,
DPoint3dCP  pDirection,
double      tolDistSquared
);

//!
//! @description Compute the volume of the tent formed by a polygon base and a given "pole" vector.
//! @remarks The tent is a collection of tetrahedra.  Each tetrahedron is formed from the first
//!       point of the polygon, the top of the pole vector, and two points of a polygon edge.
//! @param pPointArray IN      polygon vertices
//! @param numPoint IN      number of polygon vertices
//! @param pDirection IN      the vector along the pole
//! @return summed (possibly negative) tent volume
//! @group Polygons
//!
Public GEOMDLLIMPEXP double           bsiPolygon_tentVolume
(
DPoint3dCP pPointArray,
int             numPoint,
DVec3dCP    pDirection
);

//!
//! @description Look for intersections of a ray with a swept polygon.
//! @remarks In most common usage, the caller expects the polygon to be planar, and passes the plane normal as the sweep direction.
//!   Defining the results in terms of a swept polygon volume clarifies what the calculation means if the caller unexpectedly has a
//!   non-planar polygon.
//! @param pNumPositiveRayCrossings OUT number of edges which cross the (strictly) negative half-ray
//! @param pNumNegativeRayCrossings OUT number of edges which cross the (strictly) positive half-ray
//! @param pbAnyEdgePassesThroughPoint OUT true if one or more edges pass exactly throught the point
//! @param pPoint IN start point of ray
//! @param pRayDirection IN direction of ray
//! @param pPointArray IN array of polygon vertices
//! @param numPoint IN number of vertices in polygon
//! @param pSweepDirection IN direction of sweep, typically the normal to the polygon
//! @return true if the ray and sweep directions are are non-parallel
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiGeom_sweptPolygonCrossingCounts
(
int *pNumPositiveRayCrossings,
int *pNumNegativeRayCrossings,
bool    *pbAnyEdgePassesThroughPoint,
DPoint3dCP pPoint,
DPoint3dCP pRayDirection,
DPoint3dCP pPointArray,
int       numPoint,
DPoint3dCP pSweepDirection
);

//!
//! @description Classify a point with respect to a polygon, ignoring z-coordinates.
//! @param pPoint IN      point to test
//! @param pPointArray IN      polygon vertices
//! @param numPoint IN      number of polygon vertices
//! @param tol IN      absolute tolerance for ON classification
//! @return 0 if pPoint is on the polygon within tolerance, 1 if in, -1 if out, -2 if nothing worked
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiGeom_XYPolygonParity
(
DPoint3dCP pPoint,
DPoint3dCP pPointArray,
int         numPoint,
double      tol
);

//!
//! @description Compute the signed area of the polygon, using only xy-coordinates.
//! @remarks Positive area is a counterclocwise polygon, negative is clockwise.
//! @param    pPointArray IN      polygon vertices
//! @param    numPoint    IN      number of vertices
//! @return signed area of polygon
//! @group Polygons
//!
Public GEOMDLLIMPEXP double bsiGeom_getXYPolygonArea
(
DPoint3dCP pPointArray,
int             numPoint
);

//!
//! @description Copy vertices of a polygon into the output array, suppressing immediately
//!       adjacent duplicate points.
//! @remarks This function also processes a given integer array in a parallel fashion.
//! @remarks The first point/integer of a run of duplicates is saved in the compressed output arrays.
//! @remarks Input/output arrays may be the same.
//! @remarks Working tolerance is abstol + reltol * maxCoordinate, where maxCoordinate is the maximum coordinate found in any point.
//! @param pXYZOut    OUT     compressed point array (or NULL)
//! @param pIntOut    OUT     compressed int array (or NULL)
//! @param pIntOut2   OUT     copy of pIntIn with duplicate entries corresponding to redundant points (or NULL)
//! @param pNumOut    OUT     compressed point/int count (or NULL)
//! @param pXYZIn     IN      point array
//! @param pIntIn     IN      integer array (or NULL)
//! @param numIn      IN      point/int count
//! @param abstol     IN      absolute tolerance for identical point test.
//! @param reltol     IN      relative tolerance for identical point test.
//! @return true if compression was successful
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_compressDuplicateVertices
(
DPoint3dP pXYZOut,
int             *pIntOut,
int             *pIntOut2,
int             *pNumOut,
DPoint3dCP pXYZIn,
const int       *pIntIn,
int             numIn,
double          abstol,
double          reltol
);

//!
//! @description Test if pXYZ0 is "below" pXYZ1 in xy-lexical order.
//! @param pXYZ0  IN  point to test
//! @param pXYZ1  IN  comparison point
//! @return true iff (a) the y-coordinate of pXYZ1 is strictly greater than the y-coordinate of pXYZ0, or
//!                  (b) the y-coordinates are identical but x-coordinate of pXYZ1 is strictly greater than the x-coordinate of pXYZ0.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_lexicalXYBelow
(
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
);

//!
//! @description Test if pXYZ0 is "to the left of" pXYZ1 in xy-lexical order.
//! @param pXYZ0  IN  point to test
//! @param pXYZ1  IN  comparison point
//! @return true iff (a) the x-coordinate of pXYZ1 is strictly greater than the x-coordinate of pXYZ0, or
//!                  (b) the x coordinates are identical but the y-coordinate of pXYZ1 is strictly greater than the y-coordinate of pXYZ0.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_lexicalXYLeftOf
(
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
);

//! Test if a polygon has 3 or more points and all cross products are in the same direction as the largest cross product.
//! @param [in] pPointArray polygon vertices
//! @param [in] numPoint number of vertices. Trailing duplicates of point 0 are ignored.
//! @return true if convex
//!
Public GEOMDLLIMPEXP bool bsiGeom_testPolygonConvex
(
DPoint3dCP pPointArray,
int             numPoint
);

//!
//! @description Test if a polygon is convex, ignoring z-coordinates.
//!
//! @param pPointArray IN      polygon vertices
//! @param numPoint    IN      number of vertices.  Trailing duplicates of the first vertex are ignored.
//! @return 0 if polygon is not convex, 1 if convex with all turns to left, -1 if convex with all turns to right.
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiGeom_testXYPolygonConvex
(
DPoint3dCP pPointArray,
int             numPoint
);

//!
//! @description Test the direction of turn at the vertices of the polygon, ignoring z-coordinates.
//! @remarks For a polygon without self intersections, this is a convexity and orientation test:
//!       all positive is convex and counterclockwise, all negative is convex and clockwise.
//!       Beware that a polygon which turns through more than a full turn can cross itself
//!       and close, but is not convex.
//! @param pPointArray IN      polygon vertices
//! @param numPoint    IN      number of vertices.  Trailing duplicates of the first vertex are ignored.
//! @return 1 if all turns are to the left, -1 if all to the right, and 0 if there are any zero turns
//!       (successive colinear edges) or a mixture of right and left.
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiGeom_testXYPolygonTurningDirections
(
DPoint3dCP pPointArray,
int             numPoint
);

//!
//! @description Test if a point is within a convex polygon, ignoring z-coordinates.
//!
//! @param pPoint      IN      point to test
//! @param pPointArray IN      boundary points of convex region
//! @param numPoint    IN      number of points
//! @param sense       IN      0 if polygon sense is unknown (it will be determined internally by area calculation),
//!                       1 if polygon is known to be counterclockwise,
//!                      -1 if polygon is known to be clockwise
//! @return true if the point is in the convex polygon
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiGeom_isXYPointInConvexPolygon
(
DPoint3dCP pPoint,
DPoint3dCP pPointArray,
int             numPoint,
int             sense
);

//!
//! @description Compute the normal and the area of the polygon.
//! @remarks This function computes vectors from the first point to all other points, and sums
//!    the cross product of each successive pair of these vectors.  For a planar polygon, this
//!    vector's length is the area of the polygon, and the direction is the plane normal.
//! @param pNormal OUT polygon unit normal
//! @param pOrigin OUT origin (first point in array)
//! @param pXYZ IN polygon points
//! @param numXYZ IN number of polygon points
//! @return half the pre-normalization magnitude of the cross product sum.
//! @group Polygons
//!
Public GEOMDLLIMPEXP double bsiPolygon_polygonNormalAndArea
(
DVec3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pXYZ,
int numXYZ
);

//!
//! @description Compute the normal of the polygon.
//! @param pNormal OUT     polygon normal, or approximate normal if nonplanar points (or NULL)
//! @param pOrigin OUT     origin for plane (or NULL)
//! @param pVert IN      vertex array
//! @param numPoints IN      number of vertices
//! @return true if the polygon defines a clear plane
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiGeom_polygonNormal
(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pVert,
int              numPoints
);

//!
//! @description Computes the normal of the given triangle at its largest angle.
//!
//! @param pNormal    OUT     unnormalized normal (or NULL)
//! @param pMaxIndex  OUT     index of vertex at largest angle (or NULL)
//! @param pXYZ       IN      array of 3 vertices
//! @param eps2       IN      minimum value of sin^2(angle) allowable in squared magnitude of cross product
//! @return squared magnitude of normal, or zero if triangle is degenerate
//! @group Polygons
//!
Public GEOMDLLIMPEXP double   bsiGeom_triangleNormal
(
DPoint3dP pNormal,
int                 *pMaxIndex,
DPoint3dCP pXYZ,
double              eps2
);

//!
//! @description Computes the circumcenter of the given triangle.
//!
//! @param pCenter        OUT     circumcenter (or NULL)
//! @param pBaryCenter    OUT     barycentric coordinates of the circumcenter (or NULL)
//! @param pXYZ           IN      array of 3 vertices
//! @param eps2           IN      minimum value of sin^2(angle) allowable in squared magnitude of cross product
//! @return squared circumradius, or zero if triangle is degenerate
//! @group Polygons
//!
Public GEOMDLLIMPEXP double   bsiGeom_triangleCircumcenter
(
DPoint3dP    pCenter,
DPoint3dP    pBaryCenter,
DPoint3dCP    pXYZ,
double      eps2
);

//!
//! @description Test if a polyline pierces the plane of a polygon at a point interior to the polygon.
//! @param pPointArrayA IN points of polyline.
//! @param numA IN number of points for polyline.
//! @param bAddClosureEdgeA IN whether to use an additional edge from the end to the start of the polyline.
//!         Note that this closure edge does not make the polyline bound area for clash purposes.
//! @param pPointArrayB IN points of polygon
//! @param numB IN number of points for polygon
//! @return true if polyline and polygon clash
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool      bsiDPoint3dArray_polylineClashPolygonXYZ
(
DPoint3dCP   pPointArrayA,
int numA,
bool    bAddClosureEdgeA,
const   DPoint3d *  pPointArrayB,
int numB
);

//!
//! @description Test if there is any point of contact between two polygons on arbitrary planes.
//! @remarks If polygons are coplanar, look for any edge intersection or total containment;
//!       if they are not coplanar, look for an edge of one polygon piercing the plane of the
//!       other polygon at an interior point of the other polygon.
//! @param pPointArrayA IN points of polygon A
//! @param numA IN number of points for polygon A
//! @param pPointArrayB IN points of polygon B
//! @param numB IN number of points for polygon B
//! @return true if polygons clash
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool      bsiDPoint3dArray_polygonClashXYZ
(
DPoint3dCP   pPointArrayA,
int numA,
const   DPoint3d *  pPointArrayB,
int numB
);

//!
//! @description For a space point and polygon, find the polygon vertex, edge, or interior point closest to the space point.
//! @param pClosestPoint    OUT     closest point
//! @param pXYZ             IN      polygon vertices
//! @param numXYZ           IN      number of polygon vertices
//! @param pSpacePoint      IN      point to project to polygon
//! @return Indicator of where the closest point lies:
//!    <ul>
//!    <li>-1 no data
//!    <li>0 at polygon vertex
//!    <li>1 on polygon edge
//!    <li>2 in polygon interior
//!    </ul>
//! @group Polygons
//!
Public GEOMDLLIMPEXP int bsiPolygon_closestPoint
(
DPoint3dP pClosestPoint,
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dCP pSpacePoint
);

/*---------------------------------------------------------------------------------**//**
@description For a space point and polygon, find the polygon vertex, edge, or interior point closest to the space point.
@param closestPoint    OUT     closest point
@param edgeIndex       OUT     index of base index of closest edge.
@param edgeFraction    OUT      fraction along edge.  (0.0 if vertex is closest)
@param pXYZ             IN      polygon vertices
@param numXYZ           IN      number of polygon vertices
@param spacePoint      IN      point to project to polygon
@return Indicator of where the closest point lies:
    <ul>
    <li>-1 no data</li>
    <li>0 at polygon vertex</li>
    <li>1 on polygon edge</li>
    <li>2 in polygon interior</li>
    </ul>
@group Polygons
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiPolygon_closestPointExt
(
DPoint3dR closestPoint,
ptrdiff_t &edgeIndex,
double   &edgeFraction,
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dCR spacePoint
);


//!
//! @description Compute centroid, unit normal, and area of a polygon.
//! @remarks Duplication of first/last points is optional and will not affect results.
//! @param pXYZ IN array of points
//! @param numXYZ IN number of points
//! @param pCentroid OUT centroid of polygon
//! @param pUnitNormal OUT unit normal to best fit plane
//! @param pArea OUT area of polygon
//! @param pPerimeter OUT perimeter of polygon
//! @param pMaxPlanarError OUT max height difference between polygon points above and below the best fit plane
//! @return true if polygon has nonzero area
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_centroidAreaPerimeter
(
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dP pCentroid,
DVec3dP pUnitNormal,
double      *pArea,
double      *pPerimeter,
double      *pMaxPlanarError
);

//!
//! @description Clip an unbounded ray to a polygon as viewed perpendicular to a given vector.
//! @param pClipPoints      OUT     array of pairs of points, alternating between start and end of "in" segments.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
//! @param pClipParams      OUT     array of ray parameters of intersection points.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
//! @param pNumClipPoints   OUT     number of clip points returned
//! @param maxClipPoints    IN      size of preallocated output arrays
//! @param pXYZArray        IN      polygon points
//! @param numXYZ           IN      number of polygon points
//! @param pPolygonPerp     IN      normal of plane in which (virtual) intersections are computed
//! @param pRay             IN      unbounded ray to clip to polygon in plane
//! @return false if ray has zero magnitude or if maxClipPoints is too small to house all computed clip points.
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_clipDRay3d
(
DPoint3dP pClipPoints,
double      *pClipParams,
int         *pNumClipPoints,
int         maxClipPoints,
DPoint3dCP pXYZArray,
int             numXYZ,
DVec3dCP pPolygonPerp,
DRay3dCP pRay
);

//!
//! @description Compute the intersection segment(s) between two non-coplanar polygons.
//!
//! @param pSegmentArray OUT array (ALLOCATED BY CALLER) of intersection segments
//! @param pNumSegment OUT number of intersection segments returned
//! @param pbParallelPlanes OUT true if polygon planes are parallel.  INTERSECTION NOT COMPUTED IN THIS CASE.
//! @param pNormalA OUT normal to plane of polygon A
//! @param pNormalB OUT normal to plane of polygon B
//! @param maxClipSegments IN allocated size of segment array
//! @param pXYZArrayA IN polygon A
//! @param numXYZA IN number of points on polygon A
//! @param pXYZArrayB IN polygon B
//! @param numXYZB IN number of points on polygon B
//! @return false if maxClipPoints is too small to house all computed intersections.
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_transverseIntersection
(
DSegment3dP pSegmentArray,
int         *pNumSegment,
bool        *pbParallelPlanes,
DVec3dP pNormalA,
DVec3dP pNormalB,
int         maxClipSegments,
DPoint3dCP pXYZArrayA,
int             numXYZA,
DPoint3dCP pXYZArrayB,
int             numXYZB
);

//!
//! @description Search for the points where a pair of polygons makes closest approach.
//! @param pPointA OUT point on polygon A
//! @param pPointB OUT point on polygon B
//! @param pPolygonA IN vertices of polygon A
//! @param numA IN number of vertices of polygon A
//! @param pPolygonB IN vertices of polygon B
//! @param numB IN number of vertices of polygon B
//! @return true if polygon vertex counts are positive
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygons
(
DPoint3dP pPointA,
DPoint3dP pPointB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pPolygonB,
int numB
);


//!
//! @description Search for the points where a pair of polygons makes closest approach.
//! @param pPointA OUT point on polygon A
//! @param pPointB OUT point on polygon B
//! @param pNormalA OUT normal to polygon A
//! @param pNormalB OUT normal to polygon B
//! @param pPolygonA IN vertices of polygon A
//! @param numA IN number of vertices of polygon A
//! @param pPolygonB IN vertices of polygon B
//! @param numB IN number of vertices of polygon B
//! @return true if polygon vertex counts are positive
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygons
(
DPoint3dP pPointA,
DPoint3dP pPointB,
DVec3dP   pNormalA,
DVec3dP   pNormalB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pPolygonB,
int numB
);


/*---------------------------------------------------------------------------------**//**
@description Search for the points where a polygon and linestring makes closest approach.
@param pPointA OUT point on polygon A
@param pPointB OUT point on linestring
@param pPolygonA IN vertices of polygon A
@param numA IN number of vertices of polygon A
@param pLinestringB IN vertices of linestring
@param numB IN number of vertices of linestring
@return true if polygon vertex counts are positive
@group Polygons
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygonAndLineString

(
DPoint3dR pointA,
DPoint3dR pointB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pLinestringB,
int numB
);



//!
//! Find transverse plane polygon intersections via paritiy crossings.
//! Call bsiPolygon_intersectDPlane3dExt to additional determine the all-on case.
//! @param    pXYZOut    OUT intersection segments, in start end pairs.
//! @param    pNumXYZOut OUT number of points (2x number of segments)
//! @param pbAllOn OUT true if all points are on the plane.   Points are NOTE copied to the output buffer.
//! @param    maxOut OUT max number of points (including disconnects) that can be returned.
//! @param    pXYZ        IN polygon vertices.
//! @param    numXYZ      IN number of vertices.
//! @return false if output buffer too small.  Note that zero crossings is a true return.
//! @param    pPlane      IN      plane
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_intersectDPlane3d
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane
);

//!
//! Find plane polygon intersections via paritiy crossings.
//! @param    pXYZOut    OUT intersection segments, in start end pairs.
//! @param    pNumXYZOut OUT number of points (2x number of segments)
//! @param pbAllOn OUT true if all points are on the plane.   Points are NOTE copied to the output buffer.
//! @param    maxOut OUT max number of points (including disconnects) that can be returned.
//! @param    pXYZ        IN polygon vertices.
//! @param    numXYZ      IN number of vertices.
//! @param tolerance IN tolerance for deciding if vertices are all on plane.
//! @return false if output buffer too small.  Note that zero crossings is a true return.
//! @param    pPlane      IN      plane
//! @group Polygons
//!
Public GEOMDLLIMPEXP bool    bsiPolygon_intersectDPlane3dExt
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
bool            *pbAllOn,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane,
double          tolerance
);

//!
//! Clip loops to a plane.  NO OPTIONAL POINTERS.
//! @param    pXYZOut    OUT "interior" loops, separated by disconnects.
//! @param    pNumXYZOut OUT number of points (including disconnects)
//! @param    pNumLoop OUT number of loops
//! @param    maxOut OUT max number of points (including disconnects) that can be returned.
//! @param    pXYZ        IN polygon vertices.  Separate multiple loops with disconnect points.
//! @param    numXYZ      IN number of vertices.
//! @param    pPlane      IN plane with normal pointing to outside.
//! @group Polygons
//!
Public GEOMDLLIMPEXP void bsiPolygon_clipToPlane
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
int             *pNumLoop,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane
);

END_BENTLEY_GEOMETRY_NAMESPACE

