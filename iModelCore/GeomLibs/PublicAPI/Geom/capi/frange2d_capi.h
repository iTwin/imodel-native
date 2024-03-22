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
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_init (FRange2dP pRange);

//!
//!
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDRange2d
(
FRange2dP pRange,
DRange2dP pSource
);

//!
//!
//! Check if the range is exactly the same as the null ranges returned
//! by ~mbsiFRange2d_init.  (Note that ranges with other values with low > high
//! are not necessarily null by this condition.)
//!
//!
//!
//!
Public GEOMDLLIMPEXP bool    bsiFRange2d_isNull (FRange2dCP pRange);

//!
//! @return 0 if null range (as decided by isNull), otherwise
//!       sum of squared axis extents.
//!
//!
Public GEOMDLLIMPEXP double    bsiFRange2d_extentSquared (FRange2dCP pRange);

//!
//!
//! Test if low component is (strictly) less than high in any direction.
//! Note that equal components do not indicate empty.
//!
//! returns true if any low component is less than the corresponding high component
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange2d_isEmpty (FRange2dCP pRange);

//!
//!
//! @return true if high is less than or equal to low in every direction.
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange2d_isPoint (FRange2dCP pRange);

//!
//! returns product of axis extents.  No test for zero or negative axes.
//!
//!
Public GEOMDLLIMPEXP double      bsiFRange2d_volume (FRange2dCP pRange);

//!
//! Initializes the range to contain the single given point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromFPoint2d
(
FRange2dP pRange,
FPoint2dCP pPoint
);

//!
//! Initializes the range to contain the single given point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint2d
(
FRange2dP pRange,
DPoint2dCP pPoint
);

//!
//! Initializes the range to contain the two given points.
//! @param pPoint0 IN      first point
//! @param pPoint1 IN      second point
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2FPoint2d
(
FRange2dP pRange,
FPoint2dCP pPoint0,
FPoint2dCP pPoint1
);

//!
//! Initializes the range to contain the two given points.
//! @param pPoint0 IN      first point
//! @param pPoint1 IN      second point
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2DPoint2d
(
FRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
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
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2Components
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromComponents
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromCubeLimits
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom3DPoint2d
(
FRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
);

//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! ~mbsiFRange2d_init.
//!
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint3dArray
(
FRange2dP pRange,
DPoint3dCP pPoint,
int             n
);

//!
//!
//! Initializes the range to contain the range of the given array of 2D points,
//! with a single given z value for both the min and max points.
//! If there are no points in the array, the range is initialized by
//! ~mbsiFRange2d_init.
//!
//! @param pPoint IN      array of points to search
//! @param n IN      number of points in array
//! @param zVal IN      default z value
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint2dArray
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void bsiFRange2d_extendByDistance
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint2d
(
FRange2dP pRange,
DPoint2dCP pPoint
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByComponents
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint4d
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint4dArray
(
FRange2dP pRange,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint2dArray
(
FRange2dP pRange,
FPoint2dCP pArray,
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
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByFRange2d
(
FRange2dP pRange0,
FRange2dCP pRange1
);

//!
//!
//! extends the coordinates of the range cube points to
//! include the range cube range1P.
//!
//! @param pRange1 IN      second range
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDRange2d
(
FRange2dP pRange0,
DRange2dCP pRange1
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
Public GEOMDLLIMPEXP bool       bsiFRange2d_intersect
(
FRange2dP pOutRange,
FRange2dP pRange1,
FRange2dP pRange2
);

//!
//!
//! Form the union of two ranges.
//!
//! @param pRange1 IN      first range.
//! @param pRange2 IN      second range.
//!
//!
Public GEOMDLLIMPEXP void bsiFRange2d_combineRange
(
FRange2dP pCombRange,
FRange2dCP pRange1,
FRange2dCP pRange2
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
Public GEOMDLLIMPEXP bool    bsiFRange2d_isContained
(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
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
Public GEOMDLLIMPEXP bool    bsiFRange2d_isStrictlyContainedXY
(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
);

//!
//!
//! test if any min or max of the given range touches a limit (min or max)
//! of a non-trivial direction of pOuterRange.
//!
//! @param pOuterRange IN      outer range
//!
//!
Public GEOMDLLIMPEXP bool        bsiFRange2d_touchesEdge
(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
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
Public GEOMDLLIMPEXP void     bsiFRange2d_restrictToMinMax
(
FRange2dP pRange,
FRange2dCP pRange0,
FRange2dCP pMinMax
);

//!
//! Scale pRangeIn about its center point
//!
//! @param pRangeIn IN      original range
//! @param scale IN      scale factor
//!
//!
Public GEOMDLLIMPEXP void            bsiFRange2d_scaleAboutCenter
(
FRange2dP pRange,
FRange2dCP pRangeIn,
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
Public GEOMDLLIMPEXP void            bsiFRange2d_extractPlanes
(
FRange2dCP pRange,
DPoint2dP pOriginArray,
DPoint2dP pNormalArray
);

//!
//!
//! Return the index of the axis with largest absolute range.
//!
//!
//!
Public GEOMDLLIMPEXP int      bsiFRange2d_indexOfMaximalAxis (FRange2dCP pRange);

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
Public GEOMDLLIMPEXP bool        bsiFRange2d_intersectRay
(
FRange2dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint2dP pPoint0,
DPoint2dP pPoint1,
DPoint2dCP pStart,
DPoint2dCP pDirection
);

//!
//!
//! @return the largest individual coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
//!
//!
Public GEOMDLLIMPEXP double  bsiFRange2d_getLargestCoordinate (FRange2dCP pRange);

//!
//!
//! Generates a 4-point box around around a range cube.*
//! @param pBox OUT     array of 4 points of the box
//!
//!
Public GEOMDLLIMPEXP void     bsiFRange2d_box2Points
(
FRange2dCP pRange,
DPoint2dP pBox
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
Public GEOMDLLIMPEXP int      bsiFRange2d_numLeadingPointsInRange
(
FRange2dCP pRange,
DPoint2dCP pPointArray,
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
Public GEOMDLLIMPEXP double  bsiFRange2d_getOverlap
(
FRange2dCP pRange1,
FRange2dCP pRange2
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
Public GEOMDLLIMPEXP bool    bsiFRange2d_checkOverlap
(
FRange2dCP pRange1,
FRange2dCP pRange2
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
Public GEOMDLLIMPEXP bool     bsiFRange2d_moveChangesRange
(
FRange2dCP pOldRange,
FRange2dCP pNewRange,
FRange2dCP pOuterRange
);

END_BENTLEY_GEOMETRY_NAMESPACE

