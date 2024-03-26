/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Copy the given number of DPoint3d structures from the pSource array to the pDest array.
//!
//! @param pDest OUT     destination array
//! @param pSource IN      source array
//! @param n IN      number of points
//! @group "DPoint3d Copy"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_copyArray
(
DPoint3dP pDest,
DPoint3dCP pSource,
int          n
);

//!
//! @description Reverse the order of points in the array.
//! @param pXYZ IN      source array
//! @param n IN      number of points
//! @group "DPoint3d Copy"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_reverseArrayInPlace
(
DPoint3dP pXYZ,
int          n
);

//!
//! @description Copy the given number of DPoint2d structures, setting all z-coordinates to zero.
//! @param pDest OUT     destination array
//! @param pSource IN      source array
//! @param n IN      number of points
//! @group "DPoint3d Copy"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_copyDPoint2dArray
(
DPoint3dP pDest,
DPoint2dCP pSource,
int          n
);

//!
//! @description Copy DPoint3d structures from the pSource array to pDest, using an index array to rearrange the copy order.
//! @remarks The indexing assigns pDest[i] = pSource[indexP[i]], and is not necessarily 1 to 1.
//! @remarks This function does not perform in-place rearrangement.
//!
//! @param pDest OUT     destination array (must be different from pSource)
//! @param pSource IN      source array
//! @param pIndex IN      array of indices into source array
//! @param nIndex IN      number of points
//! @group "DPoint3d Copy"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_copyIndexedArray
(
DPoint3dP pDest,
DPoint3dCP pSource,
int         *pIndex,
int          nIndex
);

//!
//! @description Find a disconnect point in the array after a given start index.
//! @param pBuffer    IN      array of points
//! @param i0         IN      start index
//! @param n          IN      number of points
//! @param value      IN      unused
//! @return Index of disconnect point, or n if no disconnects.
//!
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP int  bsiDPoint3d_findDisconnectIndex
(
DPoint3dCP pBuffer,
int         i0,
int          n,
double      value
);

//!
//! @description Add a given point to each of the points of an array.
//!
//! @param pArray IN OUT  array whose points are to be incremented
//! @param pDelta IN      point to add to each point of the array
//! @param numPoints IN      number of points
//! @group "DPoint3d Addition"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dArray
(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numPoints
);

//!
//! @description Sets pOutVec[i] to pOrigin + scale*pInVec[i], for 0 &le; i &lt; numPoint.
//! @param pOutVec OUT     output array
//! @param pOrigin IN      origin for points
//! @param pInVec IN      input array
//! @param numPoint IN      number of points in arrays
//! @param scale IN      scale
//! @group "DPoint3d Addition"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDPoint3dArray
(
DPoint3dP pOutVec,
DPoint3dCP pOrigin,
DPoint3dCP pInVec,
int          numPoint,
double       scale
);

//!
//! @description Subtract a given point from each of the points of an array.
//!
//! @param pArray IN OUT  Array whose points are to be decremented
//! @param pDelta IN      point to subtract from each point of the array
//! @param numVerts IN      number of points
//! @group "DPoint3d Addition"
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dArray
(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numVerts
);

//!
//! @description Normalize an array of vectors in place.
//! @param pArray IN OUT  array of vectors to be normalized
//! @param numVector IN      number of vectors
//! @return number of zero length vectors encountered
//! @group "DPoint3d Normalize"
//!
Public GEOMDLLIMPEXP int bsiDPoint3d_normalizeArray
(
DPoint3dP pArray,
int         numVector
);

//!
//! @description Returns an upper bound for both the largest absolute value x, y or z
//! coordinate and the greatest distance between any two x,y or z coordinates
//! in an array of points.
//!
//! @param pPointArray IN      array of points to test
//! @param numPoint IN      number of points
//! @return upper bound as described above. or zero if no points
//! , bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinate
(
DPoint3dCP pPointArray,
int         numPoint
);

//!
//! @description Returns an upper bound for the greatest distance between any two x, y or z
//! coordinates in an array of points.
//!
//! @param pPointArray IN      array of points to test
//! @param numPoint IN      number of points
//! @return upper bound as described above, or zero if no points
//! , bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinateDifference
(
DPoint3dCP pPointArray,
int         numPoint
);

//!
//! @description Returns an upper bound for the greatest distance between any two x, y or z
//! coordinates in an array of weighted points.
//! @remarks Points with zero weight are ignored.
//!
//! @param pPointArray IN      array of weighted points to test
//! @param pWeightArray IN      array of weights
//! @param numPoint IN      number of points and weights
//! @return upper bound as described above, or zero if no points
//! @group "DPoint3d Queries"
//! , bsiDPoint3d_getLargestCoordinate, bsiDPoint3d_getLargestXYCoordinate
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestWeightedCoordinateDifference
(
DPoint3dCP    pPointArray,
const   double*     pWeightArray,
int         numPoint
);

//!
//! @description Returns an upper bound for both the largest absolute value x or y coordinate
//! and the greatest distance between any two x or y coordinates in an array of points.
//!
//! @param pPointArray IN      array of points to test
//! @param numPoint IN      number of points
//! @return upper bound as described above, or zero if no points
//! @group "DPoint3d Queries"
//! , bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestCoordinate
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestXYCoordinate
(
DPoint3dCP pPointArray,
int         numPoint
);

//!
//! @description Apply a perspective transformation to an array of 3D points.
//! @remarks The transformation is defined by a single fraction which controls the taper rate of the view frustum.
//! @remarks Some geometric effects of this perspective transformation are:
//!    <ul>
//!    <li>The eyepoint is at z = 1/(1-f)
//!    <li>The z=0 plane is unchanged.
//!    <li>The z=1 plane maps into itself; xy-components of such points are scaled by factor 1/f.
//!        That is, the unit square {(0,0,1), (1,0,1), (1,1,1), (0,1,1)} maps to {(0,0,1), (1/f,0,1), (1/f,1/f,1), (0,1/f,1)}.
//!    <li>Original z-direction lines maintain their original z=0 points but now pass through the point (0,0,f/(f-1)).
//!    <li>All planes map to planes.  This requires that z-coordinates vary in a not necessarily intuitive way.  The general behavior of
//!        mapped z for 0 < f < 1 as a point moves along a (premapped) z-direction line is:
//!        <ul>
//!        <li>Start at z=0.  Move toward z=1/(1-f).  Mapped z increases to infinity.
//!        <li>Start at z=0.  Move in the negative z direction.  Mapped z decreases to -f/(f-1).
//!        <li>Start at z=infinity and move back towards z=1/(f-1).  Mapped z starts at -f/(f-1) and decreases to -infinity.
//!        <li>Start at z=-infinity and move forward towards z=1/(f-1).  Mapped z starts at -f/(f-1) and increases to infinity.
//!        </ul>
//!    </ul>
//! @param pOutArray OUT     points in perspective space
//! @param pInArray IN      points in real space
//! @param numPoint IN      number of points
//! @param fraction IN      perspective effects parameter.
//!
//! @group "DPoint3d Modification"
//!
Public GEOMDLLIMPEXP void bsiGeom_applyPerspective
(
DPoint3dP pOutArray,
DPoint3dCP pInArray,
int      numPoint,
double   fraction
);

//!
//! @description Find two points (and their indices) in the given array of points that are relatively far from each other.
//! @remarks The returned points are not guaranteed to be the points with farthest separation.
//!
//! @param pMinPoint  OUT     first of the two widely separated points (or null)
//! @param pMinIndex  OUT     index of first point (or null)
//! @param pMaxPoint  OUT     second of the two widely separated points (or null)
//! @param pMaxIndex  OUT     index of second point (or null)
//! @param pPoints    IN      array of points
//! @param numPts     IN      number of points
//! @return false if numPts < 2
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiGeom_findWidelySeparatedPoints
(
DPoint3dP pMinPoint,
int                 *pMinIndex,
DPoint3dP pMaxPoint,
int                 *pMaxIndex,
const DPoint3d      *pPoints,
int                 numPts
);

//!
//! @description Compute a line segment approximating an array of points, with endpoints selected from the array.
//! @remarks The returned distances are useful for testing if the points are colinear.  A typical followup test
//!       would be to test if maxDist is less than a tolerance and dist01 is larger than (say) 1000 times the tolerance.
//!       This test is implemented by ~mbsiGeom_isUnorderedDPoint3dArrayColinear.
//! @remarks Note that the returned start and end points are selected from the given points; they are not points on
//!       the least squares approximation that might have a smaller maxDist but not pass through any points.
//! @param pPoint0 OUT     suggested starting point of the line segment.
//! @param pPoint1 OUT     suggested end point.
//! @param pDist01 OUT     distance from pPoint0 to pPoint1
//! @param pMaxDist OUT     largest distance of any point to the (infinite) line
//! @param pPointArray IN      array of points
//! @param numPoint IN      number of points
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP void        bsiGeom_approximateLineThroughPoints
(
DPoint3dP pPoint0,
DPoint3dP pPoint1,
double      *pDist01,
double      *pMaxDist,
const   DPoint3d    *pPointArray,
int         numPoint
);

//!
//! @description Test if an array of points is effectively a straight line from the first to the last.
//! @param pOnLine OUT     true if all points are all within tolerance of the (bounded) line segment from the first point to the last point.
//! @param pPointArray IN      array of points
//! @param numPoint IN      number of points
//! @param tolerance IN      absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
//! @return same as pOnLine
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiGeom_pointArrayColinearTest
(
bool        *pOnLine,
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
);

//!
//! @description Test if an array of points is effectively a straight line from the first to the last.
//! @param pPointArray IN      array of points
//! @param numPoint IN      number of points
//! @param tolerance IN      absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
//! @return true if all points are within tolerance of the (bounded) line segment from the first point to the last point.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiGeom_isDPoint3dArrayColinear
(
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
);

//!
//! @description Test if the points are colinear.
//! @param pPointArray    IN      array of points
//! @param numPoint       IN      number of points
//! @param relativeTol    IN      fraction of the distance <i>d</i> between extremal points to use as colinearity threshhold
//! @return true if all points are within the distance relativeTol * <i>d</i> of the line through the extremal points.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiGeom_isUnorderedDPoint3dArrayColinear
(
DPoint3dCP    pPointArray,
int         numPoint,
double      relativeTol
);

//!
//! @description Approximate a plane through a set of points.
//! @remarks The method used is:
//!    <ul>
//!    <li>Find the bounding box.
//!    <li>Choose the axis with greatest range.
//!    <li>Take two points that are on the min and max of this axis.
//!    <li>Also take as a third point the point that is most distant from the line connecting the two extremal points.
//!    <li>Form plane through these 3 points.
//!    </ul>
//! @param pNormal OUT     plane normal
//! @param pOrigin OUT     origin for plane
//! @param pPoint IN      point array
//! @param numPoint IN      number of points
//! @param tolerance IN      max allowable deviation from colinearity (or nonpositive to compute minimal tolerance)
//! @return true if the points define a clear plane; false if every point lies on the line (within tolerance) joining the two extremal points.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPointsTol
(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint,
double          tolerance
);

//!
//! @description Approximate a plane through a set of points.
//! @remarks This function calls ~mbsiGeom_planeThroughPointsTol with tolerance = 0.0 to force usage of smallest colinearity tolerance.
//! @param pNormal OUT     plane normal
//! @param pOrigin OUT     origin for plane
//! @param pPoint IN      point array
//! @param numPoint IN      number of points
//! @return true if the points define a clear plane; false if every point lies on the line joining the two extremal points.
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPoints
(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint
);

//!
//! @description Remove the effects of a perspective transformation of 3D points.
//! @remarks The transformation is as described in ~mbsiGeom_applyPerspective.
//!
//! @param pOutArray OUT     points in real space
//! @param pInArray IN      points in perspective space
//! @param numPoint IN      number of points
//! @param fraction IN      perspective effects parameter
//!
//! @group "DPoint3d Modification"
//!
Public GEOMDLLIMPEXP void bsiGeom_invertPerspective
(
DPoint3dP pOutArray,
DPoint3dCP pInArray,
int      numPoint,
double   fraction
);

//!
//! @description Find the closest point in an array to the given point.
//! @param pDist2 OUT     squared distance of closest point to test point (or NULL)
//! @param pPointArray IN      point array
//! @param n IN      number of points
//! @param pTestPoint IN      point to test
//! @return index of nearest point, or negative if n is nonpositive
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP int bsiGeom_nearestPointinDPoint3dArray
(
double          *pDist2,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
);

//!
//! @description Find the closest point in an array to the given point, using only xy-coordinates for distance calculations.
//! @param pPoint IN      point to test
//! @param pArray IN      point array
//! @param nPoint IN      number of points
//! @return index of nearest point, or negative if nPoint is nonpositive
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP int bsiGeom_closestXYPoint
(
DPoint3dCP pPoint,
DPoint3dCP pArray,
int             nPoint
);

//!
//! @description Evaluate trig functions at regularly spaced angles and store the results in the xy-coordinates of a point array.
//! @param pPointArray OUT     filled array, z-coordinates untouched
//! @param numPoint IN      number of points to evaluate
//! @param theta0 IN      initial radian angle
//! @param theta1 IN      end radian angle
//! @group Trigonometry
//!
Public GEOMDLLIMPEXP void bsiEllipse_evaluateCosSin
(
DPoint3dP pPointArray,
int             numPoint,
double          theta0,
double          theta1
);

//!
//! @description Compute the simple average of points in the array.
//! @param pAverage OUT     average of points
//! @param pPoint IN      point array
//! @param numPoint IN      number of points
//! @return true if 1 or more points in the array
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_averageDPoint3dArray
(
DPoint3dP pAverage,
DPoint3dCP pPoint,
int             numPoint
);

//!
//! @description Generate a compressed linestring, with points removed if they are within tolerance of the output linestring.
//! @remarks For a closed polygon, the caller must ensure the first/last points are equal; this
//!       point will also be the first/last of the output.
//! @param pOut OUT output point buffer (must be allocated by caller)
//! @param pNumOut OUT number of output points (at most numIn)
//! @param pIn IN input point buffer
//! @param numIn IN number of input points
//! @param tol IN absolute distance tolerance from input point to output segment
//! @group Polylines
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_compressXYLinestring
(
DPoint3dP pOut,
int     *pNumOut,
DPoint3dCP pIn,
int     numIn,
double tol
);

//!
//! @description Compute a convex hull of a point array, ignoring z-coordinates.
//! @remarks THIS FUNCTION IS DEPRECATED.  Please use DPoint3dOps::ConvexHullXY (bvector<DPoint3d> const &inputPoints, bvector<DPoint3d> &hullPoints);
//! @remarks Each output point is one of the inputs, including its z-coordinate.
//! @param pOutBuffer OUT convex hull points, first/last point <em>not</em> duplicated.
//!                       This must be allocated by the caller, large enough to contain numIn points.
//! @param pNumOut    OUT number of points on hull
//! @param pInBuffer  IN  input points
//! @param numIn      IN  number of input points
//! @return false if numin is nonpositive or memory allocation failure
//! @group "DPoint3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_convexHullXY
(
DPoint3dP pOutBuffer,
int         *pNumOut,
DPoint3dP pInBuffer,
int         numIn
);

//!
//! @description Compute a transformation which, if the points are coplanar in 3D, transforms all to the z=0 plane.
//! @remarks Optionally returns post-transform range data so the caller can assess planarity.   If non-coplanar points are given,
//!    the plane will be chosen to pass through 3 widely separated points.   If the points are "close" to coplanar, the choice of
//!    "widely separated" will give an intuitively reasonable plane, but is not a formal "best" plane by any particular condition.
//! @param pTransformedPoints OUT the points after transformation.  May be NULL.
//! @param pWorldToPlane OUT transformation from world to plane.  May be NULL.
//! @param pPlaneToWorld OUT transformation from plane to world.  May be NULL.
//! @param pRange OUT range of the points in the transformed system.  May be NULL.
//! @param pPoints IN pretransformed points
//! @param numPoint IN number of points
//! @return true if a plane was computed.  This does not guarantee that the points are coplanar.
//!    The false condition is for highly degenerate (colinear or single point) data, not
//!    an assessment of deviation from the proposed plane.
//! @group "DPoint3d Modification"
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_transformToPlane
(
DPoint3dP pTransformedPoints,
TransformP pWorldToPlane,
TransformP pPlaneToWorld,
DRange3dP pRange,
DPoint3dCP pPoints,
int numPoint
);

//!
//! @description Compute the linear combination of the given points and scalars.
//!
//! @param pPoint         OUT     linear combination of points
//! @param pPoints        IN      array of points
//! @param pScalars       IN      array of scalars to multiply each point
//! @param numPoints      IN      number of points = number of scalars
//! @group "DPoint3d Addition"
//!
Public GEOMDLLIMPEXP void     bsiDPoint3dArray_linearCombination
(
DPoint3dP        pPoint,
DPoint3dCP        pPoints,
const   double*         pScalars,
int             numPoints
);

//!
//! @description Compute the linear combination of the given points (XY only) and scalars.
//!
//! @param pPoint         OUT     linear combination of points
//! @param pPoints        IN      array of points
//! @param pScalars       IN      array of scalars to multiply each point
//! @param numPoints      IN      number of points = number of scalars
//! @group "DPoint3d Addition"
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_linearCombinationXY
(
DPoint2dP        pPoint,
DPoint3dCP        pPoints,
const   double*         pScalars,
int             numPoints
);

//!
//! @description Return the range of dot product values in the expression (XYZ[i] - Origin) dot Vector.
//! @param pOrigin IN origin for vectors
//! @param pVector IN fixed vector
//! @param pXYZ IN array of points
//! @param numXYZ IN number of points
//! @param pAMin OUT minimum dot product
//! @param pAMax OUT minimum dot product
//! @return true if numXYZ is positive
//! @group "DPoint3d Dot and Cross"
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_dotDifferenceRange
(
DPoint3dCP pOrigin,
DVec3dCP pVector,
DPoint3dCP pXYZ,
int         numXYZ,
double      *pAMin,
double      *pAMax
);

//!
//! @description Sort points along any direction with clear variation.
//! @param pXYZ IN OUT points to sort.
//! @param numXYZ IN number of points.
//! @group "DPoint3d Sorting"
//!
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortAlongAnyDirection
(
DPoint3d *pXYZ,
int      numXYZ
);

END_BENTLEY_GEOMETRY_NAMESPACE

