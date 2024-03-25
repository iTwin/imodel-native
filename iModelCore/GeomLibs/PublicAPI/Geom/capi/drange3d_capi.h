/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Initializes a range cube with (inverted) large positive and negative
//! values.
//! @param pRange OUT the initialized range.
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_init (DRange3dP pRange);

//!
//! @description Copy from float to double range.
//! @param pRange OUT initialized range.
//! @param pFRange OUT     "float" range.
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromFRange3d
(
DRange3dP pRange,        /* OUT     range to be initialized */
FRange3dCP pFRange
);

//!
//! @description Check if the range is exactly the same as the null ranges of a just-initialized
//! range.
//! @param pRange IN range to test.
//! @return true if the range is null.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isNull (DRange3dCP pRange);

//!
//! @description returns 0 if the range is null (Range3dIsNull), otherwise
//!       sum of squared axis extents.
//! @param pRange IN range to test
//! @return squared magnitude of the diagonal vector.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP double   bsiDRange3d_extentSquared (DRange3dCP pRange);

//!
//! @description Test if low component is (strictly) less than high in any direction.
//! Note that equal components do not indicate empty.
//! @param pRange IN range to test
//! @returns true if any low component is less than the corresponding high component
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_isEmpty (DRange3dCP pRange);

//!
//! @param pRange IN range to test
//! @return true if high is less than or equal to low in every direction.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_isPoint (DRange3dCP pRange);

//!
//! @description Returns product of axis extents.
//!    Returns 0 if the range matches the initial null range conditions.
//! @param pRange IN range to test
//! @return product of axis lengths.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP double     bsiDRange3d_volume (DRange3dCP pRange);

//!
//! @description Initializes the range to contain the single given point.
//! @param pRange OUT the initialized range.
//! @param pPoint IN the point
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromPoint
(
DRange3dP pRange,
DPoint3dCP pPoint
);

//!
//! @description Initializes the range to contain the two given points.
//! @param pRange OUT initialized range.
//! @param pPoint0 IN first point
//! @param pPoint1 IN second point
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom2Points
(
DRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
);

//!
//! @description Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param pRange OUT initialized range.
//! @param x0 IN first x
//! @param y0 IN first y
//! @param z0 IN first z
//! @param x1 IN second x
//! @param y1 IN second y
//! @param z1 IN second z
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom2Components
(
DRange3dP pRange,
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);

//!
//! @description Initialize the range from a single point given by components
//! @param pRange OUT initialized range
//! @param x IN x coordinate
//! @param y IN y coordinate
//! @param z IN z coordinate
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromComponents
(
DRange3dP pRange,
double          x,
double          y,
double          z
);

//!
//! @description Initialize the range from given min and max in all directions.
//! Given values will be swapped if needed.
//! @param pRange OUT the initialized range.
//! @param v0 IN      min (or max)
//! @param v1 IN      max (or min)
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromCubeLimits
(
DRange3dP pRange,
double          v0,
double          v1
);

//!
//! @description Initializes a range to contain three given points.
//! @param pRange OUT initialized range.
//! @param pPoint0 IN first point
//! @param pPoint1 IN second point
//! @param pPoint2 IN third point
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom3DPoint3d
(
DRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
);

//!
//! @description Initializes the range to contain the range of the given array of points.
//! If there are no points in the array, an empty initialized range is returned.
//!
//! @param pRange OUT initialized range
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromArray
(
DRange3dP pRange,
DPoint3dCP pPoint,
int             n
);

//!
//! @description Initializes the range to contain the range of the given array of points.
//! If there are no points in the array, an empty initialized range is returned.
//!
//! @param pRange OUT initialized range
//! @param pPoint => array of points to search
//! @param pWeight => optional array of weights.
//! @param n => number of points in array
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromWeightedArray
(
DRange3dP pRange,
DPoint3dCP pPoint,
const double *pWeight,
int             n
);

//!
//! @description Initializes the range to contain the range of the given array of 2D points,
//! with a single given z value for both the min and max points.
//! If there are no points in the array, the range is initialized as a null range.
//! @param pRange OUT initialized range.
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//! @param zVal IN      default z value
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromArray2d
(
DRange3dP pRange,
DPoint2dCP pPoint,
int              n,
double           zVal
);

//!
//! @description Extend each axis by the given distance on both ends of the range.
//! @param pRange IN OUT updated range
//! @param extend IN distance to extend
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void bsiDRange3d_extendByDistance
(
DRange3dP pRange,
double           extend
);

//!
//! @description Adds a specified margin in all directions.
//! @param pResultRange OUT initialized range
//! @param pRange IN initial range.
//! @param margin IN margin to add in all directions (positive and negative x, y, z)
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromDRange3dMargin
(
DRange3dP pResultRange,
DRange3dCP pRange,
double          margin
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point.
//! @param pResultRange OUT initialized range
//! @param pRange IN initial range.
//! @param pPoint IN new point to be included in the range.
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dDPoint3d
(
DRange3dP pResultRange,
DRange3dCP pRange,
DPoint3dCP pPoint
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point pPoint.
//! @param pRange OUT initialized range
//! @param pPoint IN new point to be included in the range.
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint3d
(
DRange3dP pRange,
DPoint3dCP pPoint
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point pPoint.
//! @instance pRange OUT initialized range
//! @param pPoint IN new point to be included in the range.
//! @param weight IN weight for new point.
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByWeightedDPoint3d
(
DRange3dP pRange,
DPoint3dCP pPoint,
double     weight
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point given as xyz coordinates.
//! @param pResultRange OUT initialized range
//! @param pRange IN initialized range.
//! @param x IN x coordinate of additional point.
//! @param y IN y coordinate of additional point.
//! @param z IN z coordinate of additional point.
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dXYZ
(
DRange3dP pResultRange,
DRange3dCP pRange,
double          x,
double          y,
double          z
);

//!
//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point at x,y,z.
//! @param pRange IN OUT  range to be extended
//! @param x IN      extended range coordinate
//! @param y IN      extended range coordinate
//! @param z IN      extended range coordinate
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByComponents
(
DRange3dP pRange,
double      x,
double      y,
double      z
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of) the given 4D point.
//! @param pRange IN OUT updated range
//! @param pPoint4d IN      new point to be included in minmax ranges
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint4d
(
DRange3dP pRange,
DPoint4dCP pPoint4d
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of the) array of DPoint4d
//! @param pRange IN OUT updated range
//! @param pPoint4d IN      array of  to be included in minmax ranges
//! @param numPoint IN number of points.
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint4dArray
(
DRange3dP pRange,
DPoint4dCP pPoint4d,
int             numPoint
);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include range of an array of points.
//! @param pRange IN OUT updated range
//! @param pArray IN      new points to be included in minmax ranges
//! @param n IN      number of points
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint3dArray
(
DRange3dP pRange,
DPoint3dCP pArray,
int             n
);

//!
//! @description Extends the coordinates of the range cube by transformed points
//! @param pRange IN OUT updated range
//! @param pTransform IN      transform to apply to points.
//! @param pArray IN      new points to be included in minmax ranges
//! @param n IN      number of points
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByTransformDPoint3dArray
(
DRange3dP pRange,
TransformCP pTransform,
DPoint3dCP pArray,
int             n
);

// REMARK -- Deprecated.
//!
//! @description Extends the coordinates of the range cube by transformed points
//! @param pRange IN OUT updated range
//! @param pTransform IN      transform to apply to points.
//! @param pArray IN      new points to be included in minmax ranges
//! @param n IN      number of points
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDTransform3dDPoint3dArray
(
DRange3dP pRange,
DTransform3dCP pTransform,
DPoint3dCP pArray,
int             n
);





//!
//! @description Extends the coordinates of the range cube points to
//! include the range cube range1P.
//! @param pRange0 IN OUT updated range
//!
//! @param pRange1 IN      second range
//! @group "DRange3d Extend"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByRange
(
DRange3dP pRange0,
DRange3dCP pRange1
);

//!
//! @description returns the union of two ranges.
//! @param pResultRange OUT result range
//! @param pRange0 IN first range
//! @param pRange1 IN second range
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dDRange3d
(
DRange3dP pResultRange,
DRange3dCP pRange0,
DRange3dCP pRange1
);

//!
//! @description Compute the intersection of two ranges if they overlap (even with
//!       zero thickness); otherwise, return false without setting the output range.
//! @param pResultRange OUT intersection range.
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @return same result as checkOverlap(pRange1,pRange2).
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP bool       bsiDRange3d_intersect
(
DRange3dP pResultRange,
DRange3dP pRange1,
DRange3dP pRange2
);

//!
//! @description Compute the intersection of two ranges.  If any direction has no intersection,
//!       or if the intersection has zero thickness, the result range is initialized to a null range.
//! @param pResultRange OUT intersection range.
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void    bsiDRange3d_intersectDRange3dDRange3d
(
DRange3dP pResultRange,
DRange3dCP pRange1,
DRange3dCP pRange2
);

//!
//! @description Form the union of two ranges.
//! @param pCombRange OUT combined range
//! @param pRange1 IN      first range.
//! @param pRange2 IN      second range.
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDRange3d_combineRange
(
DRange3dP pCombRange,
DRange3dCP pRange1,
DRange3dCP pRange2
);

//!
//! @description Test if the first range is contained in the second range.
//! @param pInnerRange IN candidate inner range.
//! @param pOuterRange IN candidate outer range.
//! @return true if the inner range is a (possibly improper) subset of the outer range.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isContained
(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
);

//!
//! @description Test if a point is contained in a range.
//! @param pRange IN candidate containing range.
//! @param pPoint IN point to test.
//! @return true if the point is in (or on boundary of)
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isDPoint3dContained
(
DRange3dCP pRange,
DPoint3dCP pPoint
);

//!
//! @description Test if a point given as x,y,z is contained in a range.
//! @param pRange IN candidate containing range.
//! @param x IN x coordinate
//! @param y IN y coordinate
//! @param z IN z coordinate
//! @return true if the point is in (or on boundary of)
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isXYZContained
(
DRange3dCP pRange,
double    x,
double    y,
double    z
);

//!
//! @description Test if two ranges are exactly equal.
//! @param pRange0 IN first range
//! @param pRange1 IN second range
//! @param tolerance IN toleranc to be applied to each component
//! @return true if ranges are identical in all components.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isEqual
(
DRange3dCP pRange0,
DRange3dCP pRange1
);

//!
//! @description Test if two ranges are equal within a tolerance applied componentwise.
//! @param pRange0 IN first range
//! @param pRange1 IN second range
//! @param tolerance IN toleranc to be applied to each component
//! @return true if ranges are within tolerance in all components.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isEqualTolerance
(
DRange3dCP pRange0,
DRange3dCP pRange1,
double tolerance
);

//!
//! @description Test if the given range is a proper subset of pOuterRange, using only xy parts
//! @param pInnerRange IN      inner range
//! @param pOuterRange IN      outer range
//! @return true if the given range is a proper subset of
//!   pOuterRange.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_isStrictlyContainedXY
(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
);

//!
//! @description test if any min or max of the given range touches a limit (min or max)
//! of a non-trivial direction of pOuterRange.
//! @param pInnerRange IN      inner range
//! @param pOuterRange IN      outer range
//! @return true if there is an edge touch.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_touchesEdge
(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
);

//!
//! @description Returns a range which is the intersection of two ranges.  The first
//! range is treated as a signed range, i.e. decreasing values from low
//! to high are a nonempty range, and the output will maintain the
//! direction.
//! In a direction where there is no overlap, pRange high and low values
//! are identical and are at the limit of pRange1 that is nearer to the
//! values in pRange0.
//! (Intended use: pRange0 is the 'actual' stroking range of a surface
//!   i.e. may go 'backwards'.  pRange1 is the nominal full surface range,
//!   i.e. is known a priori to be 'forwards'.  The clipping restricts
//!   unreliable pRange0 to the nominal surface range pRange1.
//! pRange0 and pRange may be the same address.  pMinMax must be different.
//! @param pRange OUT computed range
//! @param pRange0 IN      range to be restricted
//! @param pMinMax IN      allowable minmax range.  Assumed to have low < high
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_restrictToMinMax
(
DRange3dP pRange,
DRange3dCP pRange0,
DRange3dCP pMinMax
);

//!
//! @description scale a range about its center point.
//! @param pRange OUT scaled range.
//! @param pRangeIn IN original range
//! @param scale IN scale factor
//! @group "DRange3d Initialization"
//!
Public GEOMDLLIMPEXP void            bsiDRange3d_scaleAboutCenter
(
DRange3dP pRange,
DRange3dCP pRangeIn,
double          scale
);

//!
//! @description Extract the 6 bounding planes for a range cube.
//! @param pRange IN range to query
//! @param pOriginArray OUT     array of plane origins
//! @param pNormalArray OUT     array of plane normals
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP void            bsiDRange3d_extractPlanes
(
DRange3dCP pRange,
DPoint3dP pOriginArray,
DPoint3dP pNormalArray
);

//!
//! @description Return the index of the axis with largest absolute range.
//! @param pRange IN range to query
//! @return axis index
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP int      bsiDRange3d_indexOfMaximalAxis (DRange3dCP pRange);

//!
//! @description Compute the intersection of a range cube and a ray.
//!
//! If there is not a finite intersection, both params are set to 0 and
//! and both points to pPoint0.
//! @param pRange IN range to test
//! @param pParam0 OUT     ray parameter where cube is entered
//! @param pParam1 OUT     ray parameter where cube is left
//! @param pPoint0 OUT     entry point
//! @param pPoint1 OUT     exit point
//! @param pStart IN      start point of ray
//! @param pDirection IN      direction of ray
//! @return true if non-empty intersection.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_intersectRay
(
DRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dCP pStart,
DPoint3dCP pDirection
);

//!
//! @description Compute the intersection of a range cube and a line segment
//!
//! If there is not a finite intersection, both params are set to 0 and
//! and the output segment consists of only the start point.
//! @param pRange IN range to test
//! @param pParam0 OUT     ray parameter where cube is entered
//! @param pParam1 OUT     ray parameter where cube is left
//! @param pClipped OUT     clipped segment
//! @param pSegment IN segment to clip
//! @return true if non-empty intersection.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_intersectDSegment3dBounded
(
DRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DSegment3dP pClipped,
DSegment3dCP pSegment
);

//!
//! @description Convert fractional coordinates within a range cube to point coordinates.
//! @param pRange IN range to query
//! @param pPoint OUT the computed point
//! @param pFractionPoint OUT coordinates as fractions with 000 at range box low,
//!                   111 at range box high.
//! @return true if all conversions are completed.  A just-initialized range
//!       box fails.  A range box with zero thickness in one or more directions also fails.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_dPoint3dFractionsToDPoint3d
(
DRange3dCP pRange,
DPoint3dP pPoint,
DPoint3dCP pFractionPoint
);

//!
//! @description Convert fractional coordinates within a range cube to point coordinates.
//! @param pRange IN range to query
//! @param pPoint OUT the computed point
//! @param x IN fractional coordinate in x direction
//! @param y IN fractional coordinate in y direction
//! @param z IN fractional coordinate in z direction
//! @return true if all conversions are completed.  A just-initialized range
//!       box fails.  A range box with zero thickness in one or more directions also fails.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_fractionsToDPoint3d
(
DRange3dCP pRange,
DPoint3dP pPoint,
double      x,
double      y,
double      z
);

//!
//! @description Compute point coordinates to fractional coordinates within a range cube.
//! @param pRange IN range to query
//! @param pFractionPoint OUT coordinates as fractions with 000 at range box low,
//!                   111 at range box high.
//! @param pPoint IN original coordinates
//! @return true if all conversions are completed.  A just-initialized range
//!       box fails.  A range box with zero thickness in one or more directions is ok.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool        bsiDRange3d_dPoint3dToDPoint3dFractions
(
DRange3dCP pRange,
DPoint3dP pFractionPoint,
DPoint3dCP pPoint
);

//!
//! @param pRange IN range to query
//! @return the largest individual coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDRange3d_getLargestCoordinate (DRange3dCP pRange);

//!
//! @description Compute the intersection of a range cube with a plane.
//! If sorting is requested, the n point polygon is returned as n+1 points
//!   with first and last duplicated.
//! If no sorting is requested, the polygon is returned as up to 12 points
//!   arranged as start-end pairs, in arbitrary order, i.e. probably not chained.
//! @param pPointArray OUT     Array to receive points.  MUST be dimensioned to at
//!                       least 12.
//! @param pNumPoints OUT     number of points returned.
//! @param maxPoints IN      dimensioned size of receiver buffer.
//! @param pRange IN range to test
//! @param pOrigin IN      any point on the plane
//! @param pNormal IN      plane normal vector (not necessarily unit)
//! @param sort IN true to chain the intersection segments as a polygon
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDRange3d_intersectPlane
(
DPoint3dP pPointArray,
int         *pNumPoints,
int         maxPoints,
DRange3dCP pRange,
DPoint3dCP pOrigin,
DPoint3dCP pNormal,
int         sort
);

//!
//! @description Generates an 8point box around around a range cube.  Point ordering is
//! maintained from the cube.
//! @param pRange IN range to query
//! @param pBox OUT     array of 8 points of the box
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDRange3d_box2Points
(
DRange3dCP pRange,
DPoint3dP pBox
);

//!
//! @description Starting at the beginning of the array, test if points from pPointArray are "in" the range.
//!
//! @param pPointArray IN      points
//! @param numPoint IN      number of points
//! @param pRange IN      range cube
//! @param mask   IN      selects faces to consider. Valid values are the constants
//!       RangePlane_XMin
//! @return number of points that were "in" before the first "out" or end of array.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP int      bsiDRange3d_numLeadingPointsInRange
(
DRange3dCP pRange,
DPoint3dCP pPointArray,
int         numPoint,
RangePlaneMask mask
);

//!
//! @description Compute the intersection of given range with another range and return the
//! extentSquared of the intersection range.
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @return extentSquared() for the intersection range.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDRange3d_getOverlap
(
DRange3dCP pRange1,
DRange3dCP pRange2
);

//!
//! @description Test if two ranges have strictly non-null overlap (intersection)
//!
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @return true if ranges overlap, false if not.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDRange3d_checkOverlap
(
DRange3dCP pRange1,
DRange3dCP pRange2
);

//!
//! @description Compute vectors from origin to range corners.  Find extrema
//!  of their dot products with a vector.
//! @param pRange IN      range to test
//! @param pMin OUT     minimum dot value.
//! @param pMax OUT     maximum dot value.
//! @param pBasePoint IN      optional base point.  If null, 000 is assumed.
//! @param pVector IN      vector for dot product.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDRange3d_projectedExtrema
(
DRange3dCP pRange,
double    *pMin,
double    *pMax,
DPoint3dP pBasePoint,
DPoint3dP pVector
);

//!
//! @description Test if a modification of the given (instance) range would have a different
//! touching relationship with pOuterRange.
//!
//! @remarks This may only be meaningful in context of range tree tests where
//!   some prior relationship among ranges is known to apply.
//! @param pOldRange IN      original range
//! @param pNewRange IN      candidate for modified range relationship.
//! @param pOuterRange IN      containing range
//! @return true if touching condition occurs.
//! @group "DRange3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDRange3d_moveChangesRange
(
DRange3dCP pOldRange,
DRange3dCP pNewRange,
DRange3dCP pOuterRange
);


//! Initialize the range from an arc of the unit circle
//! @param theta0 => start angle
//! @param sweep  => angular sweep
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromUnitArcSweep

(
DRange3dP pRange,
double          theta0,
double          sweep
);
END_BENTLEY_GEOMETRY_NAMESPACE

