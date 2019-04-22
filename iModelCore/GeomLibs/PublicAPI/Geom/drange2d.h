/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


/*
#ifdef flexwiki
:Title: struct Bentley::DRange2d

Summary: A DRange2d is a range cube described by two fields low and hi.

#endif
*/

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MS_DRANGE2D_DEFINED
/**
A 2d low and high corner pair for range boxes.

Useful typedefs for DRange2d
\code
    typdedef struct const &DRange2d DRange2dCR;
    typdedef struct &DRange2d DRange2dR;
    typdedef struct const *DRange2d DRange2dCP;
    typdedef struct *DRange2d DRange2dP;
\endcode

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DRange2d
{
//! low point of box
DPoint2d low;
//! high point of box
DPoint2d high;

#ifdef __cplusplus
//BEGIN_FROM_METHODS
//flex!! Initialization
//flex|| Description   || Direct assignment || instance initialize || instance update ||

//flex|| Null state    || outRange = DRange2d::FromNull () || outRange.Init () ||

//! Initializes a range cube with (inverted) large positive and negative values.
static DRange2d NullRange ();
//! Initializes a range cube with (inverted) large positive and negative  values.
void Init ();

//flex|| single point  || outRange = DRange2d::From (point)   || outRange.InitFrom (point)   || inoutRange.Extend (point) ||
//flex|| || || || inoutRange.Extend(point3d) ||
//flex|| || || || inoutRange.Extend(point4d) ||

//! Initializes the range to contain the single given point.
static DRange2d From (DPoint2dCR point);
//! Initializes the range to contain the xy parts of 3d range
static DRange2d From (DRange3dCR source);
//! Return the range of an array.
static DRange2d From (bvector<DPoint3d> const &point);

//! Initializes the range to contain the single given point.
void InitFrom (DPoint2dCR point);

//!
//!
//! Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point point.
//!
//! @param [in] point new point to be included in the range.
//!
void Extend (DPoint2dCR point);

//!
//!
//! Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point point.
//!
//! @param [in] point new point to be included in the range.
//!
void Extend (DPoint3dCR point);

//flex|| single point  || outRange = DRange2d::From (x, y)   || outRange.InitFrom (x, y)   || inoutRange.Extend (x, y) ||
//!
//! Initialize the range from a single point given by components
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//!
static DRange2d From  (double x, double y);

//!
//! Initialize the range from a single point given by components
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//!
void InitFrom  (double x, double y);

//! extends the coordinates of the range cube points in pRange so as
//! to include the single additional point at x,y.
//! @param [in] x extended range coordinate
//! @param [in] y extended range coordinate
void Extend (double x, double y);

//! extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of) the xy projection of the 4D point.
//! @param [in] point4d new point to be included in minmax ranges
void Extend (DPoint4dCR point4d);


//flex|| two points || outRange = DRange2d::From (pointA, pointB)   || outRange.InitFrom (pointA, pointB)   || inoutRange.Extend (pointA, pointB) ||
//! Initializes the range to contain the two given points.
//! @param [in] point0 first point
//! @param [in] point1 second point
static DRange2d From (DPoint2dCR point0, DPoint2dCR point1);

//! Initializes the range to contain the two given points.
//! @param [in] point0 first point
//! @param [in] point1 second point
void InitFrom (DPoint2dCR point0, DPoint2dCR point1);


//flex|| two points || outRange = DRange2d::From (xA, yA, xB, yB)   || outRange.InitFrom (xA, yA, xB, yB)   || inoutRange.Extend (xA, yA, xB, yB) ||
//!
//! Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param [in] x0 first x
//! @param [in] y0 first y
//! @param [in] x1 second x
//! @param [in] y1 second y
//!
static DRange2d From  (double x0, double y0, double x1, double y1);

//!
//! Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param [in] x0 first x
//! @param [in] y0 first y
//! @param [in] x1 second x
//! @param [in] y1 second y
//!
void InitFrom  (double x0, double y0, double x1, double y1);

//flex|| three points || outRange = DRange2d::From (pointA, pointB, pointC)   || outRange.InitFrom (pointA, pointB, pointC)   || inoutRange.Extend (pointA, pointB, pointC) ||
//! Initialize the range to contain the three given points.
//! @param [in] point0 first point
//! @param [in] point1 second point
//! @param [in] point2 third point
//!
static DRange2d From  (DPoint2dCR point0, DPoint2dCR point1, DPoint2dCR point2);

//!
//! Initialize the range to contain the three given points.
//! @param [in] point0 first point
//! @param [in] point1 second point
//! @param [in] point2 third point
//!
void InitFrom  (DPoint2dCR point0, DPoint2dCR point1, DPoint2dCR point2);



//flex|| arc of unit circle || outRange = DRange2d::FromUnitArcSweep (thetaRadians, sweepRadians) || outRange.InitFromUnitArcSweep (thetaRadians, sweepRadians) ||

//! Initialize the range from an arc of the unit circle
//! @param [in] theta0 start angle
//! @param [in] sweep angular sweep
static DRange2d FromUnitArcSweep  (double theta0, double sweep);
//! Initialize the range from an arc of the unit circle
//! @param [in] theta0 start angle
//! @param [in] sweep angular sweep
void InitFromUnitArcSweep  (double theta0, double sweep);

//flex|| arrays  || || ||
//flex|| || outRange = DRange2d::From (points[], n) || outRange.InitFrom (points[], n) || inoutRange.Extend (points[], n) ||
//flex|| || outRange = DRange2d::From (bvector<DPoint2d> &points, n) || outRange.InitFrom (bvector<DPoint2d> &points, n) ||
//flex|| || outRange = DRange2d::From (points3d[], n) || outRange.InitFrom (points3d[], n) ||

//! Initializes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! @param [in] point array of points to search
//! @param [in] n number of points in array
static DRange2d From  (DPoint2dCP point, int n);
//! Initializes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! @param [in] point array of points to search
//! @param [in] n number of points in array
void InitFrom  (DPoint2dCP point, int n);

//! extends the coordinates of the range cube points in pRange so as to include range of an array of points.
//! @param [in] array new points to be included in minmax ranges
//! @param [in] n number of points
void Extend  (DPoint2dCP array, int n);

//! Initializes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange2d.init()
//! @param [in] points coordinates
static DRange2d From (bvector<DPoint2d> const &points);

//! Initializes the range to contain the range of the xy parts of the array of 3D points.
//! @param [in] point array of points to search
//! @param [in] n number of points in array
static DRange2d From  (DPoint3dCP point, int n);
//! Initializes the range to contain the range of the xy parts of the array of 3D points.
//! @param [in] point array of points to search
//! @param [in] n number of points in array
void InitFrom  (DPoint3dCP point, int n);

//flex|| expand in all directions || || || inOutRange.Extend (distance) ||
//flex|| expand (union) to second range || || || inOutRange.Extend (rangeB) ||

//!
//!
//! Extend each axis by the given distance on both ends of the range.
//!
//! @param [in] extend distance to extend
//!
void Extend (double extend);

//! extends the coordinates of the range cube points to
//! include the range cube range1P.
//! @param [in] rangeB second range
void Extend (DRange2dCR rangeB);

//flex|| intersection of ranges || || outRange.IntersectionOf (range1, range2) ||
//flex|| intersection of ranges || || outRange.UnionOf (range1, range2) ||

//! Compute the intersection of two ranges and test if it is nonempty.
//! If empty (non overlap), result range null range.
//! @remark In earlier API (bsiDRange2d_intersect, DRange2d::intersectionOf (DRange2dCP, DRange2dCP)
//!    the result was uninitialized in the no-intersection case)
//!
//! @param [in] range1 first range
//! @param [in] range2 second range
//! @return same result as checkOverlap(range1,range2).
//!
bool IntersectionOf  (DRange2dR range1, DRange2dR range2);

//!
//!
//! Form the union of two ranges.
//!
//! @param [in] range1 first range.
//! @param [in] range2 second range.
//!
void UnionOf  (DRange2dCR range1, DRange2dCR range2);

//!
//!
//! Returns a range which is the intersection of two ranges.  The first
//! range is treated as a signed range, i.e. decreasing values from low
//! to high are a nonempty range, and the output will maintain the
//! direction.
//! In a direction where there is no overlap, pRange high and low values
//! are identical and are at the limit of pRange1 that is nearer to the
//! values in range0.
//! (Intended use: range0 is the 'actual' stroking range of a surface
//!   i.e. may go 'backwards'.  pRange1 is the nominal full surface range,
//!   i.e. is known a priori to be 'forwards'.  The clipping restricts
//!   unreliable range0 to the nominal surface range pRange1.
//! range0 and pRange may be the same address.  minMax must be different.
//!
//! @param [in] range0 range to be restricted
//! @param [in] minMax allowable minmax range.  Assumed to have low < high
//!
void RestrictToMinMax  (DRange2dCR range0, DRange2dCR minMax);

//!
//! @description scale a range about its center point.
//! @param [in] rangeIn  original range
//! @param [in] scale  scale factor
//!
void ScaleAboutCenter
(
DRange2dCR      rangeIn,
double          scale
);

//flex!! Queries
//flex|| Exactly matches the null range    || bool range.IsNull () ||
//flex|| 0 for the null range, otherwise area || a = range.ExtendSqaured () ||
//flex|| area of intersection range || a = rangeA.IntersectionExtentSquared (rangeB) ||
//flex|| Has high < low (strictly) in any direction || bool range.IsEmpty () ||
//flex|| exactly equal hi,low in each direction || bool range.IsPoint () ||

//! Check if the range is exactly the same as the null ranges returned
//! by bsiDRange2d_init.  (Note that ranges with other values with low > high
//! are not necessarily null by this condition.)
bool IsNull () const;

//! @return 0 if null range (as decided by IsNull ()), otherwise
//!       sum of squared axis extents.
double ExtentSquared () const;

//flex|| area of intersection range || a = rangeA.IntersectionExtentSquared (rangeB) ||
//!
//!
//! Compute the intersection of given range with another range and return the
//! extentSquared of the intersection range.
//!
//! @param [in] range2 second range
//! @return extentSquared() for the intersection range.
//!
double IntersectionExtentSquared (DRange2dCR range2) const;

//! Test if high component is (strictly) less than low in any direction.
//! Note that equal components do not indicate empty.
//! returns true if any low component is less than the corresponding high component
bool IsEmpty () const;

//! @return true if high is less than or equal to low in every direction.
bool IsPoint () const;

//! returns product of axis extents.  No test for zero or negative axes.
double Area () const;

//flex|| large coordinate (e.g. for tolerance reference) || a = range.LargestCoordinate () ||
//! @return the largest individual coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
double LargestCoordinate () const;

//flex|| individual direction sizes  || a = range.XLength () || a = range.YLength () ||

//! Returns difference of high and low in x direction -- 0 if high < low.
double XLength () const;
//! Returns difference of high and low in y direction -- 0 if high < low.
double YLength () const;


//flex|| origins and outward normals of 4 lines || range.Get4Lines (origins[], normals[]) ||
//flex|| 4 corner coordinates, in 00,10,01,11 lexical order || range.Get4Corners (points[]) ||

//! Extract the 4 bounding lines for a range rectangle, in origin normal form
//! @param [out] originArray array of line origins
//! @param [out] normalArray array of plane normals. Directions down, left, right, up.
void Get4Lines  (DPoint2dP originArray, DPoint2dP normalArray) const;
//! Generates a 4 point box around around a range cube.  Point ordering is by "x varies fastest" --- 00, 10, 01, 11 for the unit range.
//! @param [out] box array of 4 points of the box
void Get4Corners (DPoint2dP box) const;

//flex|| Axis index (0 or 1) with larger range || axisIndex = range.IndexOfMaximalAxis () ||

//! Return the index of the axis with largest absolute range.
int IndexOfMaximalAxis () const;

//flex|| Intersect with (unbounded) ray || bool range.IntersectRay (outFraction0, outFraction1, outPoint0, outPoint1, origin, direction) ||
//! Compute the intersection of a range cube and a ray.
//!
//! If there is not a finite intersection, both params are set to 0 and
//! and both points to point0.
//!
//! @param [out] param0 ray parameter where cube is entered
//! @param [out] param1 ray parameter where cube is left
//! @param [out] point0 entry point
//! @param [out] point1 exit point
//! @param [in] start start point of ray
//! @param [in] direction direction of ray
//! @return true if non-empty intersection.
//!
bool IntersectRay  (double &param0, double &param1, DPoint2dR point0, DPoint2dR point1, DPoint2dCR start, DPoint2dCR direction) const;


//flex|| Exact and toleranced equality tests || bool rangeA.IsEqual (rangeB) || bool rangeA.IsEqual (rangeB, tolerance) ||
//!
//! @description Test if two ranges are exactly equal.
//! @param [in] range1  second range
//! @return true if ranges are identical in all components.
//! 
//!
bool IsEqual (DRange2dCR range1) const;

//!
//! @description Test if two ranges are equal within a tolerance applied componentwise.
//! @param [in] range1  second range
//! @param [in] tolerance  toleranc to be applied to each component
//! @return true if ranges are within tolerance in all components.
//! 
//!
bool IsEqual  (DRange2dCR range1, double tolerance) const;

//flex|| fraction to xy || bool TryFractionsToRangePoint (fractionUV, outXY) ||
//flex|| xy to fraction || bool TryRangePointToFracitons (xy, outFractionUV) ||
//! map a fractional point to the range coordinates. (0,0) is low point, (1,1) is high point.
//! @param [in] fractions fractional coordinates
//! @param [out] xy computed coordinates.
//! @return false if range is null range.
bool TryFractionsToRangePoint (DPoint2dCR fractions, DPoint2dR xy) const;

//! map a range point to the fractional coordinates. (0,0) is low point, (1,1) is high point.
//! @param [out] fractions fractional coordinates
//! @param [in] xy computed coordinates.
//! @return false if range is null range or single point.
bool TryRangePointToFractions (DPoint2dCR xy, DPoint2dR fractions) const;



//flex!! Containment and comparison tests

//flex|| relationship with another range || bool rangeA.IsContained (outerRange) || bool rangeA.IntersectsWith(rangeB) ||
//flex|| point in range || bool range.Contains (point) || bool range.Contains (point3d) || bool range.Contains (x, y) ||

//! Test if the given range is a (possible improper) subset of outerRange.
//! @param [in] outerRange outer range
//! @return true if the given range is a (possibly improper) subset of outerRange.
bool IsContained (DRange2dCR outerRange) const;

//! @description Test if a point is contained in a range.
//! @param [in] point  point to test. (z is ignored)
//! @return true if the point is in (or on boundary of)
bool Contains (DPoint3dCR point) const;

//! @description Test if a point is contained in a range.
//! @param [in] point  point to test. (z is ignored)
//! @return true if the point is in (or on boundary of)
bool Contains (DPoint2dCR point) const;

//! @description Test if a point given as x,y,z is contained in a range.
//! @param [in] x  x coordinate
//! @param [in] y  y coordinate
//! @return true if the point is in (or on boundary of)
bool Contains  (double x, double y) const;

//! Test if two ranges have strictly non-null overlap (intersection)
//! @param [in] range2 second range
//! @return true if ranges overlap, false if not.
bool IntersectsWith (DRange2dCR range2) const;

//!
//!
//! Test if a modification of the given (instance) range would have a different
//! touching relationship with outerRange.
//!
//! @remark This may only be meaningful in context of range tree tests where
//!   some prior relationship among ranges is known to apply.
//!
//! @param [in] newRange candidate for modified range relationship.
//! @param [in] outerRange containing range
//! @return true if touching condition occurs.
//!
bool MoveChangesIntersection  (DRange2dCR newRange, DRange2dCR outerRange) const;


#endif


};
END_BENTLEY_GEOMETRY_NAMESPACE
