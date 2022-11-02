/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*
#ifdef flexwiki
:Title: struct Bentley::DRange3d

Summary: A DRange3d is a range cube described by two fields low and hi.

#endif
*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
A 3d low and high corner pair for range boxes.

Useful typedefs for DRange3d
\code
    typdedef struct const &DRange3d DRange3dCR;
    typdedef struct &DRange3d DRange3dR;
    typdedef struct const *DRange3d DRange3dCP;
    typdedef struct *DRange3d DRange3dP;
\endcode

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DRange3d
{
//! low point of box
DPoint3d        low;
//! high point of box
DPoint3d        high;

#ifdef __cplusplus
//BEGIN_REFMETHODS
//flex!! Initialization
//flex|| Description   || Direct assignment || instance initialize || instance update ||
//flex|| Null state    || outRange = DRange3d::NullRange ()   || outRange.Init () ||
//flex|| single point  || outRange = DRange3d::From (point)   || outRange.InitFrom (point)   || inoutRange.Extend (point) ||
//flex||               ||                                     ||                             || inoutRange.Extend (point, weight) ||
//flex||               || outRange = DRange3d::From (xA, yA, zA) || outRange.InitFrom (xA, yA, zA)   || inoutRange.Extend (xA, yA, zA) ||
//flex|| two points    || outRange = DRange3d::From (pointA, pointB)   || outRange.InitFrom (pointA, pointB) ||
//flex||               || outRange = DRange3d::From (xA, yA, zA, xB, yB, zB) || outRange.InitFrom (xA, yA, zA, xB, yB, zB) ||
//flex|| three points  || outRange = DRange3d::From (pointA, pointB, pointC)   || outRange.InitFrom (pointA, pointB, pointC) ||
//flex|| array         || outRange = DRange3d::From (point[], n)  || outRange.InitFrom (point[], n) || inoutRange.Extend (points[], n) ||
//flex||          || outRange = DRange3d::From (bvector<DPoint3d> &points)    || outRange.InitFrom (bvector<DPoint3d> &points) || inoutRange.Extend (bvector<DPoint3d> &points) ||
//flex||          ||                                                          || outRange.InitFrom (transform, bvector<DPoint3d> &points) || inoutRange.Extend(transform, bvector<DPoint3d> points) ||
//flex||          ||                                                          || || outRange.Extend (transform, points [], n) ||
//flex||          || outRange = DRange3d::From (point[], weights[], n)  || outRange.InitFrom (point[], weights[], n) || inoutRange.Extend (bvector<DPoint3d> &points, bvector<double> weights) ||
//flex||          || outRange = DRange3d::From (bvector<DPoint3d> &point, bvector<double> &weights)  || outRange.InitFrom (bvector<DPoint3d> &points, bvector<double> &weights) ||
//flex||          ||                                                          || outRange.InitFrom (points4d[], n) ||
//flex||          ||                                                          || outRange.InitFrom (bvector<DPoint4d> &points) || inoutRange.Extend (bvector<DPoint4d> &points) ||
//flex||          || outRange = DRange3d::From (point2d[], z, n)  || outRange.InitFrom (point2d[], z, n) ||
//flex||          || outRange = DRange3d::From (bvector<DPoint2d> &points, z)  || outRange.InitFrom (bvector<DPoint2d> &points, z) || inoutRange.Extend (bvector<DPoint2d> &points, double z) ||
//flex||           ||                                                 || outRange.InitFrom (bvector<DPoint4d> &points) ||
//flex|| same value all directions || outRange = DRange3d::FromMinMax (a, b) || outRange.InitFromMinMax (a, b) ||
//flex|| expand by same value in all directions || || || inoutRange.Extend (a) ||
//flex|| expand by respective vector component in each direction || || || inoutRange.Extend (vector) ||
//flex|| expand to include an ellipse  || || || inoutRange.Extend (ellipse) ||
//flex|| expand to include another range || || || inoutRange.Extend (rangeA) ||
//flex|| Union of ranges || || || outRange.UnionOf (rangeA, rangeB) ||
//flex|| Intersection of ranges.  Entire range is null if any direction is null. || || || outRange.InersectionOf (rangeA, rangeB) ||
//flex|| Intersection of ranges.  optionally allow zero extent in individual direction.. || || || outRange = DRange3d::FromIntersection (rangeA, rangeB, allowZeroExtent) ||
//flex|| Intersection of ranges.  Entire range is null if any direction is null. || || || outRange.IntersectIndependentComponentsOf (rangeA, rangeB) ||
//flex|| expand by scaling in around center || || outRange.ScaleAboutCenter (range, scale) ||

//! @description Initializes a range cube with (inverted) large positive and negative values.
static DRange3d NullRange ();
//! @description Initializes a range cube with (inverted) large positive and negative values.
void Init ();

//!
//! @description Initializes the range to contain the single given point.
//! @param [in] point  the point
//!
static DRange3d From (DPoint3dCR point);
//! @description Initializes the range to contain the single given point.
//! @param [in] point  the point
void InitFrom (DPoint3dCR point);
//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point point.
//! @param [in] point  new point to be included in the range.
//!
void Extend (DPoint3dCR point);
//! Extend to include a new point.

void Extend (FPoint3dCR point);
//! Extend to include two new points
void Extend (FPoint3dCR pointA, FPoint3dCR pointB);
//! Extend to include new points.
void Extend (bvector<FPoint3d> const &pointA);

//! Create as a single point.
static DRange3d From (FPoint3dCR point);
//! Create to include two points.
static DRange3d From (FPoint3dCR pointA, FPoint3dCR pointB);
//! Create to include a vector of points.
static DRange3d From (bvector<FPoint3d> const &points);

//! initialize from a float range.
static DRange3d From (FRange3dCR fRange);

//!
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional weighted point.
//! @param [in] point  new point to be included in the range.
//! @param [in] weight  weight.   Point coordinates are divided by the weight.
//!
void Extend
(
DPoint3dCR      point,
double          weight
);

//!
//! @description Initializes the range to contain the two given points.
//! @param [in] point0  first point
//! @param [in] point1  second point
//!
static DRange3d From (DPoint3dCR point0, DPoint3dCR      point1);
//! @description Initializes the range to contain the two given points.
//! @param [in] point0  first point
//! @param [in] point1  second point
void InitFrom (DPoint3dCR point0, DPoint3dCR point1);

//! Initialize the range.InitFrom a single point given by components
//! @param [in] x  x coordinate
//! @param [in] y  y coordinate
//! @param [in] z  z coordinate
//!
static DRange3d From
(
double          x,
double          y,
double          z
);

//!
//! Initialize the range from a single point given by components
//! @param [in] x  x coordinate
//! @param [in] y  y coordinate
//! @param [in] z  z coordinate
//!
void InitFrom
(
double          x,
double          y,
double          z
);


//!
//!                                                                                                
//! @description Extends the coordinates of the range cube points in pRange so as
//! to include the single additional point at x,y,z.
//! @param [in] x extended range coordinate
//! @param [in] y extended range coordinate
//! @param [in] z extended     range coordinate
//!
void Extend
(
double          x,
double          y,
double          z
);

//! @description Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param [in] x0  first x
//! @param [in] y0  first y
//! @param [in] z0  first z
//! @param [in] x1  second x
//! @param [in] y1  second y
//! @param [in] z1  second z
static DRange3d From
(
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);

//!
//! @description Initializes the range to contain two points given as components.
//! Minmax logic is applied to the given points.
//! @param [in] x0  first x
//! @param [in] y0  first y
//! @param [in] z0  first z
//! @param [in] x1  second x
//! @param [in] y1  second y
//! @param [in] z1  second z
//!
void InitFrom
(
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);


//!
//! @description Initialize the range.InitFrom given min and max in all directions.
//! Given values will be swapped if needed.
//! @param [in] v0 min (or max)
//! @param [in] v1 max (or min)
//!
static DRange3d FromMinMax
(
double          v0,
double          v1
);

//!
//! @description Initializes a range to contain three given points.
//! @param [in] point0  first point
//! @param [in] point1  second point
//! @param [in] point2  third point
//!
static DRange3d From
(
DPoint3dCR      point0,
DPoint3dCR      point1,
DPoint3dCR      point2
);


//!
//! @description Initializes a range to contain three given points.
//! @param [in] point0  first point
//! @param [in] point1  second point
//! @param [in] point2  third point
//!
void InitFrom
(
DPoint3dCR      point0,
DPoint3dCR      point1,
DPoint3dCR      point2
);



//!
//! @description Initialize the range from given min and max in all directions.
//! Given values will be swapped if needed.
//! @param [in] v0 min (or max)
//! @param [in] v1 max (or min)
//!
void InitFromMinMax
(
double          v0,
double          v1
);


//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] n number of points in array
//!
static DRange3d From (DPoint3dCP point, int n);



//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] pWeight array of corresponding weights
//! @param [in] n number of points in array
//!
static DRange3d From
(
DPoint3dCP      point,
const   double  *pWeight,
int             n
);


//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//! @param [in] transform transform to apply
//! @param [in] point array of points to search
//! @param [in] pWeight array of corresponding weights
//! @param [in] n number of points in array
//!
static DRange3d From
(
TransformCR transform,
DPoint3dCP      point,
const   double  *pWeight,
int             n
);

//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] n number of points in array
//!
void InitFrom (DPoint3dCP point, int n);
//! extends the coordinates of the range cube points in pRange so as
//! to include range of an array of points.
//! @param [in] array new points to be included in minmax ranges
//! @param [in] n number of points
void Extend (DPoint3dCP array, int n);


//! Return a range of points
//! @param [in] points new points to be included in minmax ranges
static DRange3d From (bvector<DPoint3d> const &points);

//! Return a range of points
//! @param [in] points new points to be included in minmax ranges
static DRange3d From (bvector<bvector<DPoint3d>> const &points);

//! Return a range of points
//! @param [in] points new points to be included in minmax ranges
static DRange3d From (bvector<bvector<bvector<DPoint3d>>> const &points);

//! Return a range of points in segments.
//! @param [in] segments segments to search.
static DRange3d From (bvector<DSegment3d> const &segments);

//! Initialize a range to include points
//! @param [in] points new points to be included in minmax ranges
void InitFrom (bvector<DPoint3d> const &points);

//! return an intersection range.
static DRange3d FromIntersection
(
DRange3dCR range1,      //!< [in] first operand
DRange3dCR range2,      //!< [in] second operand
bool zeroExtentsAreValid = false    //!< [in] If false, equal high and low in any direction is forced to null range.  If true, the equal extent is allowed as a nonnull range.
);

//! return union of ranges
static DRange3d FromUnion
(
DRange3dCR range1,      //!< [in] first operand
DRange3dCR range2       //!< [in] second operand
);


//! Expand a range to include points
//!
//! @param [in] points new points to be included in minmax ranges
//!
void Extend (bvector<DPoint3d> const &points);


//!
//!
//! extends the coordinates of the range cube by transformed points
//!
//! @param [in] transform transform to apply to points.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] n number of points
//!
void Extend
(
TransformCR  transform,
DPoint3dCP      points,
int             n
);

//!
//!
//! extends the coordinates of the range cube by transformed points
//!
//! @param [in] transform transform to apply to points.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] weights weights
//! @param [in] n number of points
//!
void Extend
(
TransformCR  transform,
DPoint3dCP      points,
double const*   weights,
int             n
);

//! Return a range of transformed points
//! @param [in] transform transform to apply
//! @param [in] points new points to be included in minmax ranges
static DRange3d From (TransformCR transform, bvector<DPoint3d> const &points);

//! Return a range of transformed points
//! @param [in] transform transform to apply
//! @param [in] points new points to be included in minmax ranges
static DRange3d From (TransformCR transform, bvector<DPoint4d> const &points);

//! Initialize a range to include transformed points
//! @param [in] transform transform to apply
//! @param [in] points new points to be included in minmax ranges
void InitFrom (TransformCR transform, bvector<DPoint3d> const &points);

//! Expand a range to include transformed points
//!
//! @param [in] transform transform to apply
//! @param [in] points new points to be included in minmax ranges
//!
void Extend (TransformCR transform, bvector<DPoint3d> const &points);

//! Expand a range to include transformed points
//!
//! @param [in] transform transform to apply
//! @param [in] points new points to be included in minmax ranges
//!
void Extend (TransformCR transform, bvector<DPoint4d> const &points);


//!
//!
//! Initizlizes the range to contain the range of the given array of points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] pWeight array of corresponding weights
//! @param [in] n number of points in array
//!
void InitFrom
(
DPoint3dCP      point,
const   double  *pWeight,
int             n
);

//! 
//! Return a range of points with optional weights.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] weights correspnding weights.
//!
static DRange3d From (bvector<DPoint3d> const &points, bvector<double> const *weights);

//! 
//! Initialize a range to include points with optional weights.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] weights correspnding weights.
//!
void InitFrom (bvector<DPoint3d> const &points, bvector<double> const *weights);

//! 
//! Expand a range to include points with optional weights.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] weights correspnding weights.
//!
void Extend (bvector<DPoint3d> const &points, bvector<double> const *weights);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of) the given 4D point.
//!
//! @param [in] point4d new point to be included in minmax ranges
//!
void Extend (DPoint4dCR point4d);

//!
//!
//! extends the coordinates of the range cube points in pRange so as
//! to include the (normalized image of the) array of DPoint4d
//!
//! @param [in] point4d array of  to be included in minmax ranges
//! @param [in] numPoint number of points in the array.
//!
void Extend
(
DPoint4dCP      point4d,
int             numPoint
);


//!
//! Return a range of points
//! @param [in] points array of  to be included in minmax ranges
//!
static DRange3d From (bvector<DPoint4d> const &points);

//! Initialize a range to include points
//! @param [in] points array of  to be included in minmax ranges
void InitFrom (bvector<DPoint4d> const &points);

//! Expand a range to include points
//! @param [in] points array of  to be included in minmax ranges
void Extend (bvector<DPoint4d> const &points);

//!
//!
//! Initializes the range to contain the range of the given array of 2D points,
//! with a single given z value for both the min and max points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] n number of points in array
//! @param [in] zVal default z value
//!
static DRange3d From
(
DPoint2dCP      point,
int             n,
double          zVal
);

//!
//!
//! Initializes the range to contain the range of the given array of 2D points,
//! with a single given z value for both the min and max points.
//! If there are no points in the array, the range is initialized by
//! DRange3d#init()
//!
//! @param [in] point array of points to search
//! @param [in] n number of points in array
//! @param [in] zVal default z value
//!
void InitFrom
(
DPoint2dCP      point,
int             n,
double          zVal
);

//! Return a range of points with fixed z.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] zValue z coordinate for all points.
//!
static DRange3d From (bvector<DPoint2d> const &points, double zValue);
//! Return a range of points with fixed z.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] zValue z coordinate for all points.
static DRange3d From (bvector<bvector<DPoint2d> > const &points, double zValue);

//! 
//! Initialize a range to include points with fixed z.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] zValue z coordinate for all points.
//!
void InitFrom (bvector<DPoint2d> const &points, double zValue);


//! 
//! Expand a range to include points with fixed z.
//! @param [in] points new points to be included in minmax ranges
//! @param [in] zValue z coordinate for all points.
//!
void Extend (bvector<DPoint2d> const &points, double zValue);

//!
//! @description Extend each axis by the given distance on both ends of the range.
//! @param [in] extend  distance to extend
//!
void Extend (double extend);

//!
//! @description Extend either low or high of each axis by corresponding vector entry for sweep of the range cube.
//! Any axis with low > high is left unchanged.
//! @param [in] vector sweep drection
//!
void ExtendBySweep (DVec3dCR vector);

//! extends the coordinates of the range cube points to include the range cube range1P.
//! @param [in] range1 second range
void Extend (DRange3dCR range1);

//! extends the coordinates of the range cube points to include an ellipse
//! @param [in] ellipse 
void Extend (DEllipse3dCR ellipse);


//!
//! @description returns the union of two ranges.
//! @param [in] range0  first range
//! @param [in] range1  second range
//!
void UnionOf
(
DRange3dCR      range0,
DRange3dCR      range1
);

//!
//! Compute the intersection of two ranges.  If any direction has no intersection
//!       the result range is initialized to a null range.  (Zero thickness
//!       intersection is null.)
//! @param [in] range1 first range
//! @param [in] range2 second range
//!
void IntersectionOf
(
DRange3dCR      range1,
DRange3dCR      range2
);

//! Compute intersection of x,y,z components independently.
//! @param [in] range1 first range
//! @param [in] range2 second range
void IntersectIndependentComponentsOf
(
DRange3dCR range1,
DRange3dCR range2
);

//!
//! @description scale a range about its center point.
//! @param [in] rangeIn  original range
//! @param [in] scale  scale factor
//!
void ScaleAboutCenter
(
DRange3dCR      rangeIn,
double          scale
);

//flex!! Queries

//flex|| Exactly matches the null range    || bool range.IsNull () ||
//flex|| 0 for the null range, otherwise squared diagonal length || a = range.ExtendSqaured () ||
//flex|| exactly equal hi,low in each direction || bool range.IsPoint () ||
//flex|| Has high < low (strictly) in any direction || bool range.IsEmpty () ||
//flex|| 0 for exact null range, otherwise product of axis sizes || range.Volume () ||
//flex|| x direction high-low, 0 if high < low  || a = range.XLength () ||
//flex|| y direction high-low, 0 if high < low || a = range.YLength () ||
//flex|| z direction high-low, 0 if high < low || a = range.ZLength () ||
//flex|| largest coordinate in any dimension || a = range.LargestCoordinate () ||
//flex|| 8 corners in xyz lexical order    || range.Get8Corners (points[]) ||
//flex|| 6 planes, outward normals || range.Get6Planes (planes[]) ||
//flex|| single component 1D range || range1d = range.GetComponentRange1d (index) ||
//flex|| index of axis with largest range  || index = range.IndexOfMaximalAxis () ||

//! @description Check if the range is exactly the same as the null ranges of a just-initialized
//! range.
//! @return true if the range is null.
//!
bool IsNull () const;

//!
//! @description returns 0 if the range is null (Range3dIsNull), otherwise
//!       sum of squared axis extents.
//! @return squared magnitude of the diagonal vector.
//!
double ExtentSquared () const;

//! Test if low component is (strictly) less than high in any direction.
//! Note that equal components do not indicate empty.
//! returns true if any low component is less than the corresponding high component
bool IsEmpty () const;

//! @return true if high is exactly equal to low in every direction.
bool IsPoint () const;

//! returns product of axis extents.  No test for zero or negative axes.
double Volume () const;

//! Returns difference of high and low in x direction -- 0 if high < low.
double XLength () const;
//! Returns difference of high and low in y direction -- 0 if high < low.
double YLength () const;
//! Returns difference of high and low in z direction -- 0 if high < low.
double ZLength () const;

//! Returns the length of the xyz diagonal -- 0 if null range.
double DiagonalDistance () const;
//! Returns the length of the xy diagonal -- 0 if null range.
double DiagonalDistanceXY () const;
//! Returns the diagonal vector -- 000 if null range.
DVec3d DiagonalVector () const;
//! Returns the XY part of the diagonal vector -- 000 if null range.
DVec3d DiagonalVectorXY () const;

//! Test if z size is small compared to x and y.
bool IsAlmostZeroZ () const;
//! Test if y size is small compared to x and z
bool IsAlmostZeroY () const;
//! Test if x size is small compared to y and z
bool IsAlmostZeroX () const;


//!
//!
//! @return the largest individual coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
//!
double LargestCoordinate () const;


//!
//! @return the largest individual coordinate value among range low and high, zero if empty range.
//!
double MaxAbs () const;


//!
//!
//! @return the largest individual XY coordinate value among (a) range min point,
//! (b) range max point, and (c) range diagonal vector.
//!
double LargestCoordinateXY () const;

//!
//!
//! Generates an 8point box around around a range cube.  Point ordering is
//! maintained from the cube.
//!
//! @param [out] box array of 8 points of the box
//!
void Get8Corners (DPoint3dP box) const;

//!
//!
//! Generates 6 planes for the faces of the box.
//!
//! @param [out] planes array of 6 planes.  (Declared and allocated by caller)
//! @param [in] normalLength scale factor for plane normals.  1.0 is outward unit normals, -1.0 is inward unit normals
//!
void Get6Planes (DPlane3d planes[6], double normalLength = 1.0) const;

//!
//!
//! Generates individual DSegment3d for the 12 edges of the box.
//!
//! @param [out] edges array of 12 edges
//!
void GetEdges (bvector<DSegment3d> &edges) const;

//!
//!
//! Extract the 6 bounding planes for a range cube.
//!
//! @param [out] originArray array of plane origins
//! @param [out] normalArray array of plane normals
//!
void Get6Planes
(
DPoint3dP       originArray,
DPoint3dP       normalArray
) const;

//!
//!
//! Extract a single component (x,y,z) as a DRange1d.
//!
//! @param [in] index component index interpretted cyclically.
//! @return single component range.  Ranges may be null independently.
DRange1d GetComponentDRange1d (int index) const;

//! Compute range of dot products of ray.direction with vectors from ray.origin to corners of the range.
//! (The ray direction is NOT renormalized on each call)
//! @param [in] ray origin and vector for dot products.
DRange1d GetCornerRange (DRay3dCR ray) const;

//! Compute range of dot products of plane.normal with vectors from plane.origin to corners of the range.
//! (The plane normal is NOT renormalized on each call)
//! @param [in] plane origin and vector for dot products.
DRange1d GetCornerRange (DPlane3dCR plane) const;

//!
//!
//! Return the index of the axis with largest absolute range.
//!
//!
int IndexOfMaximalAxis () const;

//flex|| map fractions to global coordinates || point = range.LocalToGlobal (xFraction, yFraction, zFraction) ||

//! Convert fractional coordinates in x,y,z directions to global coordinates.
//! @param [in] xFraction
//! @param [in] yFraction
//! @param [in] zFraction
//! @return interpolated point.
DPoint3d LocalToGlobal
(
double xFraction,
double yFraction,
double zFraction
) const;
//flex!! containment and comparison tests

//flex|| range contains point  || range.IsContained (point) ||
//flex|| range contains point  || range.IsContained (x, y, z) ||
//flex||                       || range.IsContainedXY (point) ||
//flex||                       || range.IsContained (point, numTestedDimensions) ||
//flex|| range fully contained by outerRange   || range.IsContained (outerRange) ||
//flex|| exact equality    || range.IsEqual (rangeB) ||
//flex|| approximate equality || range.IsEqual (rangeB, tolerance) ||
//flex|| strictly contained by outerRange, XY only || range.IsStrictlyContainedXY (outerRange) ||
//!
//! @description Test if the first range is contained in the second range.
//! @param [in] outerRange  candidate outer range.
//! @return true if the inner range is a (possibly improper) subset of the outer range.
//!
bool IsContained (DRange3dCR outerRange) const;

//flex!! containment and comparison tests

//flex|| range contains point  || range.IsContained (point) ||
//flex|| range contains point  || range.IsContained (x, y, z) ||
//flex||                       || range.IsContainedXY (point) ||
//flex||                       || range.IsContained (point, numTestedDimensions) ||
//flex|| range fully contained by outerRange   || range.IsContained (outerRange) ||
//flex|| exact equality    || range.IsEqual (rangeB) ||
//flex|| approximate equality || range.IsEqual (rangeB, tolerance) ||
//flex|| strictly contained by outerRange, XY only || range.IsStrictlyContainedXY (outerRange) ||
//!
//! @description Compute the smallest distance from xyz to the range.
//! This is 0 for any point inside the range.
//! @param [in] xyz space point.
//! @return distance from xyz to closest point of the range.
//!
double DistanceOutside (DPoint3dCR xyz) const;

//! @description Compute the smallest distance squared from xyz to the range.
//! This is 0 for any point inside the range.
//! @param [in] xyz space point.
//! @return distance squared from xyz to closest point of the range.
//!
double DistanceSquaredOutside (DPoint3dCR xyz) const;
//!
//! @description Compute the smallest distance to the other range.
//! This is 0 if the ranges overlap.
//! @param [in] other second range
//! @return distance between range cubes (i.e. edge to edge or vertex to edge)
//!
double DistanceSquaredTo (DRange3dCR other) const;

//!
//! @description Test if a point is contained in a range.
//! @param [in] point  point to test.
//! @return true if the point is in (or on boundary of)
//!
bool IsContained (DPoint3dCR point) const;

//!
//! @description Test if a point is contained in a range, using only xy parts.
//! @param [in] point  point to test.
//! @return true if the point is in (or on boundary of)
//!
bool IsContainedXY (DPoint3dCR point) const;

//!
//! @description Test if a point is contained in a range, using 1,2,or 3 dimensions
//! @param [in] point  point to test.
//! @param [in] numDimensions (1,2, or 3) number of dimensions to include in test.
//! @return true if the point is in (or on boundary of)
//!
bool IsContained (DPoint3dCR point, int numDimensions) const;


//!
//! @description Test if a point given as x,y,z is contained in a range.
//! @param [in] x  x coordinate
//! @param [in] y  y coordinate
//! @param [in] z  z coordinate
//! @return true if the point is in (or on boundary of)
//!
bool IsContained
(
double          x,
double          y,
double          z
) const;

//flex|| Test for intersection || bool range.IntersectsWith (rangeB) ||
//flex|| Test for intersection in 1,2, or 3 dimensions || bool range.IntersectsWIth (rangeB, numDimensions) ||
//flex|| Test for intersection in 1,2, or 3 dimensions with an expansion tolerance || bool range.IntersectsWIth (rangeB, double expansionTolerance, numDimensions) ||

//!
//!
//! Test if two ranges have strictly non-null overlap (intersection)
//!
//! @param [in] range2 second range
//! @return true if ranges overlap, false if not.
//!
bool IntersectsWith (DRange3dCR range2) const;

//!
//!
//! Test if two ranges have strictly non-null overlap (intersection) in 1,2, or 3 dimensions
//!
//! @param [in] range2 second range
//! @param [in] numDimensions 1,2, or 3
//! @return true if ranges overlap, false if not.
//!
bool IntersectsWith (DRange3dCR range2, int numDimensions) const;

//!
//!
//! Test if two ranges have strictly non-null overlap (intersection) in 1,2, or 3 dimensions, with a scalar gap added in all dimensions.
//!
//! @param [in] range2 second range
//! @param [in] gapSize approach within this distance in any direction is considered an intersection.
//! @param [in] numDimensions 1,2, or 3
//! @return true if ranges overlap, false if not.
//!
bool IntersectsWith (DRange3dCR range2, double gapSize, int numDimensions) const;




//!
//! Test if two ranges are exactly equal.
//! @param [in] range1  second range
//! @return true if ranges are identical in all components.
//!
bool IsEqual (DRange3dCR range1) const;

//!
//! Test if two ranges are equal within a tolerance applied componentwise.
//! @param [in] range1  second range
//! @param [in] tolerance  toleranc to be applied to each component
//! @return true if ranges are within tolerance in all components.
//!
bool IsEqual
(
DRange3dCR      range1,
double          tolerance
) const;

//!
//!
//! Test if the given range is a proper subset of outerRange, using only xy parts
//!
//! @param [in] outerRange outer range
//! @return true if the given range is a proper subset of
//!   outerRange.
//!
bool IsStrictlyContainedXY (DRange3dCR outerRange) const;

//!
//!
//! Returns a range which is the intersection of two ranges.  The first
//! range is treated as a signed range, i.e. decreasing values from low
//! to high are a nonempty range, and the output will maintain the
//! direction.
//! In a direction where there is no overlap, instance high and low values
//! are identical and are at the limit of minMax that is nearer to the
//! values in range0.
//! (Intended use: range0 is the 'actual' stroking range of a surface
//!   i.e. may go 'backwards'.  minMax is the nominal full surface range,
//!   i.e. is known a priori to be 'forwards'.  The clipping restricts
//!   unreliable range0 to the nominal surface range pRange1.
//! range0 and instance may be the same address.  minMax must be different.
//!
//! @param [in] range0 range to be restricted
//! @param [in] minMax allowable minmax range.  Assumed to have low < high
//!
void RestrictToMinMax
(
DRange3dCR      range0,
DRange3dCR      minMax
);

//flex|| intersection (line segment) of cube and unbounded ray. || bool range.IntersectRay (outFraction0, outFraction1, outPoint0, outPoint1, rayStartPoint, rayDirectionVector) ||
//!
//!
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
bool IntersectRay
(
double          &param0,
double          &param1,
DPoint3dR       point0,
DPoint3dR       point1,
DPoint3dCR      start,
DPoint3dCR      direction
) const;

//flex|| intersection (line segment) of cube and a line segment. || bool range.IntersectBounded (outFraction0, outFraction1, outClippedSegment, segment) ||
//!
//!
//! Compute the intersection of a range cube and a ray.
//!
//! If there is not a finite intersection, both params are set to 0 and
//! and the output segment consists of only the start point.
//!
//! @param [out] param0 ray parameter where cube is entered
//! @param [out] param1 ray parameter where cube is left
//! @param [out] clipped clipped segment
//! @param [out] segment line segment to intersect with range cube.
//! @return true if non-empty intersection.
//!
bool IntersectBounded
(
double          &param0,
double          &param1,
DSegment3dR     clipped,
DSegment3dCR    segment
) const;


//!
//!
//! Compute the intersection of given range with another range and return the
//! extentSquared of the intersection range.
//!
//! @param [in] range2 second range
//! @return extentSquared() for the intersection range.
//!
double IntersectionExtentSquared (DRange3dCR range2) const;


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
bool MoveChangesIntersection
(
DRange3dCR      newRange,
DRange3dCR      outerRange
) const;

#endif

};
END_BENTLEY_GEOMETRY_NAMESPACE
