/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//!
//! Initializes a range cube with (inverted) large positive and negative
//! values.
//!
//! @param
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_init (FRange3dP pRange);

//!
//!
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDRange3d
(
FRange3dP pRange,
DRange3dP pSource
);

//!
//!
//! Check if the range is exactly the same as the null ranges returned
//! by ~mbsiFRange3d_init.  (Note that ranges with other values with low > high
//! are not necessarily null by this condition.)
//!
//!
//!
//!
Public GEOMDLLIMPEXP bool    bsiFRange3d_isNull (FRange3dCP pRange);

//!
//! @return 0 if null range (as decided by ~mbsiFRange3d_isNull), otherwise
//!       sum of squared axis extents.
//!
//!
Public GEOMDLLIMPEXP double    bsiFRange3d_extentSquared (FRange3dCP pRange);

//!
//!
//! Test if low component is (strictly) less than high in any direction.
//! Note that equal components do not indicate empty.
//!
//! returns true if any low component is less than the corresponding high component
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange3d_isEmpty (FRange3dCP pRange);

//!
//!
//! @return true if high is less than or equal to low in every direction.
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange3d_isPoint (FRange3dCP pRange);

//!
//! returns product of axis extents.  No test for zero or negative axes.
//!
//!
Public GEOMDLLIMPEXP double      bsiFRange3d_volume (FRange3dCP pRange);

//!
//! Initializes the range to contain the single given point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromFPoint3d
(
FRange3dP pRange,
FPoint3dCP pPoint
);

//!
//! Initializes the range to contain the single given point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint3d
(
FRange3dP pRange,
DPoint3dCP pPoint
);

//!
//! Initializes the range to contain the two given points.
//! @param pPoint0 IN      first point
//! @param pPoint1 IN      second point
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2FPoint3d
(
FRange3dP pRange,
FPoint3dCP pPoint0,
FPoint3dCP pPoint1
);

//!
//! Initializes the range to contain the two given points.
//! @param pPoint0 IN      first point
//! @param pPoint1 IN      second point
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2DPoint3d
(
FRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
);

//!
//! Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param x0 IN      first x
//! @param y0 IN      first y
//! @param z0 IN      first z
//! @param x1 IN      second x
//! @param y1 IN      second y
//! @param z1 IN      second z
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2Components
(
FRange3dP pRange,
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);

//!
//! Initialize the range from a single point given by components
//! @param x0 IN      x coordinate
//! @param y0 IN      y coordinate
//! @param z0 IN      z coordinate
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromComponents
(
FRange3dP pRange,
double          x,
double          y,
double          z
);

//!
//! Initialize the range from given min and max in all directions.
//! @param v0 IN      min (or max)
//! @param v1 IN      max (or min)
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromCubeLimits
(
FRange3dP pRange,
double          v0,
double          v1
);

//!
//! Initialize the range to contain the three given points.
//! @param pPoint0 IN      first point
//! @param pPoint1 IN      second point
//! @param pPoint2 IN      third point
//! @param
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom3DPoint3d
(
FRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
);

//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! ~mbsiFRange3d_init.
//!
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint3dArray
(
FRange3dP pRange,
DPoint3dCP pPoint,
int             n
);

//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! ~mbsiFRange3d_init.
//!
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromFPoint3dArray
(
FRange3dP pRange,
FPoint3dCP pPoint,
int             n
);

//!
//!
//! Initializes the range to contain the range of the given array of 2D points,
//! with a single given z value for both the min and max points.
//! If there are no points in the array, the range is initialized by
//! ~mbsiFRange3d_init.
//!
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//! @param zVal IN      default z value
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint2dArray
(
FRange3dP pRange,
DPoint2dCP pPoint,
int              n,
double           zVal
);

//!
//!
//! Extend each axis by the given distance on both ends of the range.
//!
//! @param extend IN      distance to extend
//!
//!
Public GEOMDLLIMPEXP void bsiFRange3d_extendByDistance
(
FRange3dP pRange,
double           extend
);

//!
//!
//! Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point pPoint.
//!
//! @param pPoint IN      new point to be included in the range.
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint3d
(
FRange3dP pRange,
DPoint3dCP pPoint
);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include the single additional point at x,y,z.
//!
//! @param pRange IN OUT  range to be extended
//! @param x IN      extended range coordinate
//! @param y IN      extended range coordinate
//! @param z IN      extended range coordinate
//! @param
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByComponents
(
FRange3dP pRange,
double      x,
double      y,
double      z
);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of) the given 4D point.
//!
//! @param pPoint4d IN      new point to be included in minmax ranges
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint4d
(
FRange3dP pRange,
DPoint4dCP pPoint4d
);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of the) array of DPoint4d
//!
//! @param pPoint4d IN      array of  to be included in minmax ranges
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint4dArray
(
FRange3dP pRange,
DPoint4dCP pPoint4d,
int             numPoint
);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include range of an array of points.
//!
//! @param pArray IN      new points to be included in minmax ranges
//! @param n IN      number of points
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint3dArray
(
FRange3dP pRange,
FPoint3dCP pArray,
int             n
);

//!
//!
//! extends the coordinates of the range cube points to
//! include the range cube range1P.
//!
//! @param pRange1 IN      second range
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByFRange3d
(
FRange3dP pRange0,
FRange3dCP pRange1
);

//!
//!
//! extends the coordinates of the range cube points to
//! include the range cube range1P.
//!
//! @param pRange1 IN      second range
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDRange3d
(
FRange3dP pRange0,
DRange3dCP pRange1
);

//!
//! Compute the intersection of two ranges and test if it is nonempty.
//! If empty (non overlap), result range is not set!!!!
//!
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @return same result as checkOverlap(pRange1,pRange2).
//!
//!
Public GEOMDLLIMPEXP bool       bsiFRange3d_intersect
(
FRange3dP pOutRange,
FRange3dP pRange1,
FRange3dP pRange2
);

//!
//!
//! Form the union of two ranges.
//!
//! @param pRange1 IN      first range.
//! @param pRange2 IN      second range.
//!
//!
Public GEOMDLLIMPEXP void bsiFRange3d_combineRange
(
FRange3dP pCombRange,
FRange3dCP pRange1,
FRange3dCP pRange2
);

//!
//!
//! Test if the given range is a (possible improper) subset of pOuterRange.
//!
//! @param pOuterRange IN      outer range
//! @return true if the given range is a (possibly improper) subset of
//!   pOuterRange.
//!
//!
Public GEOMDLLIMPEXP bool    bsiFRange3d_isContained
(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
);

//!
//!
//! Test if the given range is a proper subset of pOuterRange, using only xy parts
//!
//! @param pOuterRange IN      outer range
//! @return true if the given range is a proper subset of
//!   pOuterRange.
//!
//!
Public GEOMDLLIMPEXP bool    bsiFRange3d_isStrictlyContainedXY
(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
);

//!
//!
//! test if any min or max of the given range touches a limit (min or max)
//! of a non-trivial direction of pOuterRange.
//!
//! @param pOuterRange IN      outer range
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange3d_touchesEdge
(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
);

//!
//!
//! Returns a range which is the intersection of two ranges.  The first
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
//!
//! @param pRange0 IN      range to be restricted
//! @param pMinMax IN      allowable minmax range.  Assumed to have low < high
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_restrictToMinMax
(
FRange3dP pRange,
FRange3dCP pRange0,
FRange3dCP pMinMax
);

//!
//!
//! Scale pRangeIn about its center point
//!
//! @param pRangeIn IN      original range
//! @param scale IN      scale factor
//!
//!
Public GEOMDLLIMPEXP void            bsiFRange3d_scaleAboutCenter
(
FRange3dP pRange,
FRange3dCP pRangeIn,
double           scale
);

//!
//!
//! Extract the 6 bounding planes for a range cube.
//!
//! @param pOriginArray OUT     array of plane origins
//! @param pNormalArray OUT     array of plane normals
//!
//!
Public GEOMDLLIMPEXP void            bsiFRange3d_extractPlanes
(
FRange3dCP pRange,
DPoint3dP pOriginArray,
DPoint3dP pNormalArray
);

//!
//!
//! Return the index of the axis with largest absolute range.
//!
//!
//!
Public GEOMDLLIMPEXP int      bsiFRange3d_indexOfMaximalAxis (FRange3dCP pRange);

//!
//!
//! Compute the intersection of a range cube and a ray.
//!
//! If there is not a finite intersection, both params are set to 0 and
//! and both points to pPoint0.
//!
//! @param pParam0 OUT     ray parameter where cube is entered
//! @param pParam1 OUT     ray parameter where cube is left
//! @param pPoint0 OUT     entry point
//! @param pPoint1 OUT     exit point
//! @param pStart IN      start point of ray
//! @param pDirection IN      direction of ray
//! @return true if non-empty intersection.
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange3d_intersectRay
(
FRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dCP pStart,
DPoint3dCP pDirection
);

//!
//!
//! @return the largest individual coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
//!
//!
Public GEOMDLLIMPEXP double  bsiFRange3d_getLargestCoordinate (FRange3dCP pRange);

//!
//!
//! Compute the intersection of a range cube with a plane.
//! If sorting is requested, the n point polygon is returned as n+1 points
//!   with first and last duplicated.
//! If no sorting is requested, the polygon is returned as up to 12 points
//!   arranged as start-end pairs, in arbitrary order, i.e. probably not chained.
//!
//! @param pPointArray OUT     Array to receive points.  MUST be dimensioned to at
//!                       least 12.
//! @param pNumPoints OUT     number of points returned.
//! @param maxPoints IN      dimensioned size of receiver buffer.
//! @param pOrigin IN      any point on the plane
//! @param pNormal IN      plane normal vector (not necessarily unit)
//!
//!
Public GEOMDLLIMPEXP void bsiFRange3d_intersectPlane
(
DPoint3dP pPointArray,
int         *pNumPoints,
int         maxPoints,
FRange3dCP pRange,
DPoint3dCP pOrigin,
DPoint3dCP pNormal,
int         sort
);

//!
//!
//! Generates an 8point box around around a range cube.  Point ordering is
//! maintained from the cube.
//!
//! @param pBox OUT     array of 8 points of the box
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange3d_box2Points
(
FRange3dCP pRange,
DPoint3dP pBox
);

//!
//!
//! Starting at the beginning of the array, test if points from pPointArray are "in" the range.
//!
//! @param pPointArray IN      points
//! @param numPoint IN      number of points
//! @param pRange IN      range cube
//! @param mask   IN      selects faces to consider. Valid values are the constants
//!       RangePlane_XMin
//! @return number of points that were "in" before the first "out" or end of array.
//!
//!
Public GEOMDLLIMPEXP int      bsiFRange3d_numLeadingPointsInRange
(
FRange3dCP pRange,
DPoint3dCP pPointArray,
int         numPoint,
RangePlaneMask mask
);

//!
//!
//! Compute the intersection of given range with another range and return the
//! extentSquared of the intersection range.
//!
//! @param pRange2 IN      second range
//! @return extentSquared() for the intersection range.
//!
//!
Public GEOMDLLIMPEXP double  bsiFRange3d_getOverlap
(
FRange3dCP pRange1,
FRange3dCP pRange2
);

//!
//!
//! Test if two ranges have strictly non-null overlap (intersection)
//!
//! @param pRange1 IN      first range
//! @param pRange2 IN      second range
//! @return true if ranges overlap, false if not.
//!
//!
Public GEOMDLLIMPEXP bool    bsiFRange3d_checkOverlap
(
FRange3dCP pRange1,
FRange3dCP pRange2
);

//!
//!
//! Test if a modification of the given (instance) range would have a different
//! touching relationship with pOuterRange.
//!
//! @remark This may only be meaningful in context of range tree tests where
//!   some prior relationship among ranges is known to apply.
//!
//! @param pNewRange IN      candidate for modified range relationship.
//! @param pOuterRnage IN      containing range
//! @return true if touching condition occurs.
//!
//!
Public GEOMDLLIMPEXP bool     bsiFRange3d_moveChangesRange
(
FRange3dCP pOldRange,
FRange3dCP pNewRange,
FRange3dCP pOuterRange
);

END_BENTLEY_GEOMETRY_NAMESPACE

