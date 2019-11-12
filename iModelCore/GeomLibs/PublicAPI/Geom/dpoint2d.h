/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*
#ifdef flexwiki
:Title: struct Bentley::DPoint2d

Summary: 2d point with x,y coordinates as public members.

#endif
*/


/*__PUBLISH_SECTION_START__*/

#ifndef dpoint2d_H_
#define dpoint2d_H_

BEGIN_BENTLEY_NAMESPACE
/**
2d point coordinates.

Useful typedefs for DPoint2d
\code
    typdedef struct const &DPoint2d DPoint2dCR;
    typdedef struct &DPoint2d DPoint2dR;
    typdedef struct const *DPoint2d DPoint2dCP;
    typdedef struct *DPoint2d DPoint2dP;
\endcode

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DPoint2d
{
//! x coordinate
double x;
//! y coordinate
double y;

#ifdef __cplusplus


/*__PUBLISH_SECTION_END__*/
//! @description Returns a DPoint2d with given fPoint.
//! @param [in] fPoint point
static DPoint2d From (FPoint2dCR fPoint);

//!
//! @description Returns a DPoint2d with 2 components (xy) from a double array
//!
//! @param [in] pXy x, y components
//!
static DPoint2d FromArray
(
const   double  *pXy
);

/*__PUBLISH_SECTION_START__*/
//flex!!Construction and Initialization

//flex|| Description || result returned from static method || result placed in instance.  No use of prior instance contents. || modify instance in place||
//flex|| by component  || point = DPoint2d::From (x, y)       || ||
//flex|| all zeros ||     || point.Zero () ||
//flex|| all ones  ||     || point.One () ||
//flex|| all DISCONNECT    || || point.InitDisconnect () ||
//flex|| from point3d  || || point2d.Init (point3d) ||
//! @description Returns a DPoint2d with 2 components (xy) from given components
//! @param [in] ax x coordinate
//! @param [in] ay y coordinate
static DPoint2d From (double ax, double ay);
//! Return a DPoint2d with xy = 0.
static DPoint2d FromZero();
//! Return a DPoint2d with xy = 1.
static DPoint2d FromOne();
//! Initialize with given components
//! @param [in] x x component
//! @param [in] y y component
void Init (double x, double y);

//! @description Returns a DPoint2d from DPoint3d
//! @param [in] source 
static DPoint2d From (DPoint3dCR source);

//! @description Sets all components of a point or vector to zero.
void Zero ();
//! @description Returns a point or vector with all components 1.0.
void One ();

//! @param [in] source x, y components
void Init (DPoint3dCR source);
//! Initialize a point with all coordinates as the disconnect value.
void InitDisconnect ();



/*__PUBLISH_SECTION_END__*/
void InitFrom (FPoint2dCR fPoint);
/*__PUBLISH_SECTION_START__*/

//flex|| Sum and difference        ||     || point.SumOf (pointA, pointB)    || point.Add (pointB) ||
//flex|| (pointA - pointB)         ||     || point.DifferenceOf (pointA, pointB) || pointA.Subtract (pointB) ||
//flex|| (pointA - pointB) normalized  || || a = point.NormalizedDifferenceOf (pointA, pointB) ||
//flex|| scale prior point         || point = DPoint2d::FromScale (pointA, scaleA)    || point.Scale (pointA, scaleA) || point.Scale (scaleA) ||
//flex|| negate                    || || point.Negate (pointA)   ||
//flex|| scale to target length    ||  || a = point.ScaleToLength (pointA, length)   || point.ScaleToLength (length) ||
//flex|| Sums with scale factors  || || ||
//flex||                           || point = DPoint2d::FromSumOf (pointA, scaleA, pointB, scaleB) || ||
//flex||                           || point = DPoint2d::FromSumOf (pointA, scaleA, pointB, scaleB, pointC, scaleC) || ||
//flex||                           || point = DPoint2d::FromSumOf (origin, vectorA, scale)                                || point.SumOf (origin, vectorA, scaleA) ||
//flex||                           || point = DPoint2d::FromSumOf (origin, vectorA, scale, vectorB, scaleB)               || point.SumOf (origin, vectorA, scaleA, vectorB, scaleB) ||
//flex||                           || point = DPoint2d::FromSumOf (origin, vectorA, scale, vectorB, scaleB, vectorC, scaleC) || point.SumOf (origin, vectorA, scale, vectorB, scaleB, vectorC, scaleC) ||

//! @description Compute the sum of two points or vectors.
//! @param [in] point1 First point or vector
//! @param [in] point2 Second point or vector
void SumOf  (DPoint2dCR point1, DPoint2dCR point2);
//! Add a vector to the instance.
//! @param [in] vector vector to add
void Add (DPoint2dCR vector);

//! @description return point shifted by forward and CCW multiples of vector.
DPoint2d AddForwardLeft (DVec2dCR vector, double forwardFraction, double leftFraction) const;

//! @description Subtract one vector from another in place.
//! @param [in] vector vector to subtract
void Subtract (DPoint2dCR vector);

//! @description Return the difference of two points or vectors.
//! @param [in] point1 First point or vector.
//! @param [in] point2 Second (subtracted) point or vector.
void DifferenceOf  (DPoint2dCR point1, DPoint2dCR point2);

//! Sets pNormal to the unit vector in the direction of point1  point2
//! @param [in] point1 point 1
//! @param [in] point2 point 2
//! @return double distance between input points
double NormalizedDifferenceOf  (DPoint2dCR point1, DPoint2dCR point2);


//! @description Returns a scalar multiple of a DPoint2d
//! @param [in] point input point
//! @param [in] scale scale factor
static DPoint2d FromScale (DPoint2d point, double scale);
//! Scale the instance coordinates from source
//! @param [in] source input point
//! @param [in] s scale factor
//!
void Scale (DPoint2dCR source, double s);

//! Scale the instance coordinates (in place)
//! @param [in] s scale factor
void Scale (double s);

//! @description returns the negative of a vector.
//! @param [in] vector input
void Negate (DPoint2dCR vector);

//! @description Scales a vector to specified length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param [in] source The original vector.
//! @param [in] length The requested length.
//! @return The length prior to scaling.
//!
double ScaleToLength (DPoint2dCR source, double length);

//!
//! @description Scales a vector to a specified length, and returns
//! the prior length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param [in] length The requested length
//! @return The length prior to scaling.
//!
double ScaleToLength (double length);

//! normalizes pVector1 in place, and returns the original magnitude.  If the original magnitude is 0 the vector is left unchanged.
//! @return original length
double Normalize ();
//! normalizes vector1, stores as instance, and returns the original magnitude.  If the original magnitude is 0 the vector is copied.
//! @return original length
double Normalize (DPoint2dCR vector1);

//! @description Returns a linear combination of points
//! @param [in] point0 first point
//! @param [in] scale0  first scale
//! @param [in] point1 second point
//! @param [in] scale1 second scale
static DPoint2d FromSumOf (DPoint2dCR point0, double scale0, DPoint2dCR point1, double scale1);


//! @description Returns a linear combination of points
//! @param [in] point0 first point
//! @param [in] scale0  first scale
//! @param [in] point1 second point
//! @param [in] scale1 second scale
//! @param [in] point2 third point
//! @param [in] scale2 third scale
static DPoint2d FromSumOf (DPoint2dCR point0, double scale0, DPoint2dCR point1, double scale1, DPoint2dCR point2, double scale2);
//!
//! @description Adds three scaled points.
//!
//! @param [in] vector1 direction vector
//! @param [in] scale1 scale factor
//! @param [in] vector2 direction vector
//! @param [in] scale2 scale factor
//! @param [in] vector3 direction vector
//! @param [in] scale3 scale factor
//!
void SumOf  (DPoint2dCR vector1, double scale1, DPoint2dCR vector2, double scale2, DPoint2dCR vector3, double scale3);

//!
//! @description Adds three scaled points.
//!
//! @param [in] vector1 direction vector
//! @param [in] scale1 scale factor
//! @param [in] vector2 direction vector
//! @param [in] scale2 scale factor
//!
void SumOf  (DPoint2dCR vector1, double scale1, DPoint2dCR vector2, double scale2);


//! Add scaled vector from origin.
//! @param [in] origin start point
//! @param [in] vector vector to scale.
//! @param [in] scaleFactor multiplier.
static DPoint2d FromSumOf (DPoint2dCR origin, DVec2dCR vector, double scaleFactor);

//! @description Adds an origin and a scaled vector.
//! @param [in] point  origin.
//! @param [in] vector direction vector
//! @param [in] s scale factor
void SumOf  (DPoint2dCR point, DPoint2dCR vector, double s);


//! Add scaled vectors from an origin.
//! @param [in] origin start point
//! @param [in] vector0 vector to scale.
//! @param [in] scaleFactor0 multiplier.
//! @param [in] vector1 vector to scale.
//! @param [in] scaleFactor1 multiplier.
static DPoint2d FromSumOf (DPoint2dCR origin, DVec2dCR vector0, double scaleFactor0, DVec2dCR vector1, double scaleFactor1);

//!
//! @description Adds an origin and two scaled vectors.
//!
//! @param [in] origin origin.
//! @param [in] vector1 direction vector
//! @param [in] scale1 scale factor
//! @param [in] vector2 direction vector
//! @param [in] scale2 scale factor
//!
void SumOf  (DPoint2dCR origin, DPoint2dCR vector1, double scale1, DPoint2dCR vector2, double scale2);

//! Add scaled vectors from origin.
//! @param [in] origin start point
//! @param [in] vector0 vector to scale.
//! @param [in] scaleFactor0 multiplier.
//! @param [in] vector1 vector to scale.
//! @param [in] scaleFactor1 multiplier.
//! @param [in] vector2 vector to scale.
//! @param [in] scaleFactor2 multiplier.
static DPoint2d FromSumOf (DPoint2dCR origin, DVec2dCR vector0, double scaleFactor0, DVec2dCR vector1, double scaleFactor1, DVec2dCR vector2, double scaleFactor2);


//!
//! @description Adds an origin and three scaled vectors.
//!
//! @param [in] origin origin.
//! @param [in] vector1 direction vector
//! @param [in] scale1 scale factor
//! @param [in] vector2 direction vector
//! @param [in] scale2 scale factor
//! @param [in] vector3 direction vector
//! @param [in] scale3 scale factor
//!
void SumOf  (DPoint2dCR origin, DPoint2dCR vector1, double scale1, DPoint2dCR vector2, double scale2, DPoint2dCR vector3, double scale3);



//flex|| Interpolate between points    || point = DPoint2d::FromInterpolate (pointA, fraction, pointB)    || point.Interpolate (pointA, fraction, pointB) ||
//flex|| Bilinear patch interpolation || point = DPoint2d::FromBilinearInterpolate (point00, point10, point01, point11, u, v) ||
//! @description Returns an interpolated point.
//! @param [in] point0 point at fraction 0
//! @param [in] fraction fraction from interpolation.
//! @param [in] point1 point at fraction 1
static DPoint2d FromInterpolate (DPoint2dCR point0, double fraction, DPoint2dCR point1);

//! @description Returns an interpolated point.
//! @param [in] point0 point at fraction 0
//! @param [in] tangentFraction fraction for interpolation from point0 towards point1
//! @param [in] leftFraction fraction to move along the 90 CCW perpendicular
//! @param [in] point1 point at fraction 1
static DPoint2d FromForwardLeftInterpolate (DPoint2dCR point0, double tangentFraction, double leftFraction, DPoint2dCR point1);

//! Compute the point at an interpolated (fractional) position
//!   between a start and end point.
//! @param [in] point0 start point (at parameter s=0)
//! @param [in] s  interpolation parameter
//! @param [in] point1 end point (at parameter s=1)
void Interpolate  (DPoint2dCR point0, double s, DPoint2dCR point1);


//! @description Returns a bilinear interpolation from corners (00)(10)(01)(11)
//! @param [in] point00 point at (0,0)
//! @param [in] point10 point at (1,0)
//! @param [in] point01 point at (0,1)
//! @param [in] point11 point at (1,1)
//! @param [in] u interpolation fraction for edges (point00,point10) and (point01,point11)
//! @param [in] v interpolation fraction for edges (point00,point10) and (point10,point11)
static DPoint2d FromInterpolateBilinear (DPoint2dCR point00, DPoint2dCR point10, DPoint2dCR point01, DPoint2dCR point11, double u, double v);

//flex|| rotate 90 degrees CCW ||     || point.Rotate90 (pointA) ||
//flex|| rotate                ||     || pointCCW.Rotate (pointA, radians) ||

//! @param [in] vec original vector
void Rotate90 (DPoint2dCR vec);
//! @param [in] vec original vector
//! @param [in] radians rotation angle
//! @remark Note that this is a counterclockwise rotation.   The "rotate" method in prior api was clockwise.
void RotateCCW (DPoint2dCR vec, double radians);


//! Swap contents of instance, other.
//! @param [in,out] other second point.
void Swap (DPoint2dR other);

//flex!! Scalar Measurements

//flex|| magnitude of vector from origin       || a = point.Magnitude ()      || a = point.magnitudeSquared ||
//flex|| distance between points               || a = pointA.Distance (pointB)    || A = Point.DistanceSquared (pointB) ||
//flex|| prodcuts, points are vectors from origin          || a = pointA.CrossProduct (pointB)      || a = pointA.DotProduct (pointB) ||
//flex|| cross proudcut (vectors from intsance)  || a = pointA.CrossProductToPoints (pointB, pointC)  || a = pointA.DotProductToPoints (pointB, pointC) ||
//flex|| components                            || a = point.GetComponent (index)      || point.SetComponent (value, index) ||
//flex||                                       || point.GetComponents (x, y)  ||
//flex!! Angles and comparisons
//flex|| Angle (vectors from origin)         ||  radians [-pi..+pi) = pointA.AngleTo (pointB)  ||
//flex||                                   || bool pointA.IsPerpendicularTo (pointB)   || bool pointA.IsParallelTo (pointB) ||
//flex|| equality tests                    || bool pointA.IsEqual (pointB)        || bool pointA.IsEqual (pointB, tolerance) ||
//flex||                                   || bool pointA.AlmostEqual (pointB) ||
//flex|| identify disconnect               || bool pointA.IsDisconnect () ||
//flex|| largest component                 || a = pointA.MaxAbs () ||
//flex|| largest component difference      || a = pointA.MaxDiff (pointB) ||


//! @description Returns the magnitude (length) of a vector.
//! @return Length of the vector.
double Magnitude () const;
//! @return squared magnitude of the vector
double MagnitudeSquared () const;

//! @description Returns the largest absolute difference between coordinates
//! @param [in] point1 second point
//! @return distance from point 0 to point 1
double MaxDiff (DPoint2dCR point1) const;


//! @description Returns the distance between 2 points
//! @param [in] point1 second point
//! @return distance from point 0 to point 1
double Distance (DPoint2dCR point1) const;
//!
//! @description Return the squared distance between two points or vectors.
//! @param [in] point2 end point
//! @return squared distance between points
//!
double DistanceSquared (DPoint2dCR point2) const;

//! @description Returns the (scalar) cross product of two vectors.
//! @param [in] vector1 first vector
double CrossProduct (DPoint2dCR vector1) const;

//! @description Returns the (scalar) cross product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! @param [in] target1 target of first vector
//! @param [in] target2 target of second vector
double CrossProductToPoints
(
DPoint2dCR      target1,
DPoint2dCR      target2
) const;

//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] vector2 second vector.
//! @return dot product of the two vectors
double DotProduct (DPoint2dCR vector2) const;

//!
//! @description Returns the (scalar) dot product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! @param [in] target1 target of first vector
//! @param [in] target2 target of second vector
//!
double DotProductToPoints
(
DPoint2dCR      target1,
DPoint2dCR      target2
) const;



//! @description Sets a single component of a point.  If the index is out of
//! range 0,1, it is interpreted cyclically.
//! @param [in] a value for component
//! @param [in] index 0=x, 1=y, others cyclic
void SetComponent (double a, int index);

//! @description Gets a single component of a point.  If the index is out of range 0,1, it is interpreted cyclically.
//! @param [in] index 0=x, 1=y, others cyclic
//! @return specified component of the point or vector.
double GetComponent (int index) const;
//! @param [out] xCoord x component
//! @param [out] yCoord y component
void GetComponents (double &xCoord, double &yCoord) const;


//! copy 2 components (xy) from a double array to the DPoint2d
//! @param [in] pXy x, y components
void InitFromArray
(
const   double  *pXy
);

//! @description Returns (signed, counterclockwise) angle between two vectors.
//!   The angle is in radians. The angle range is from -pi to +pi; positive
//!   angles are counterclockwise, negative angles are clockwise.
//! @param [in] vector2 second vector
//! @return angle in radians
double AngleTo (DPoint2dCR vector2) const;


//! @description Test if two vectors are parallel.
//! @param [in] vector2 Second vector
//! @return true if vectors are (nearly) parallel.
bool IsParallelTo (DPoint2dCR vector2) const;

//! @description Test if two vectors are perpendicular.
//! @param [in] vector2 Second vector
//! @return true if vectors are (nearly) parallel.
bool IsPerpendicularTo (DPoint2dCR vector2) const;

//!
//! @description Test if two points or vectors are exactly equal.
//! @param [in] vector2 Second point or vector
//! @return true if the points are identical.
//!  (DPoint2dCR, double)
//!
bool IsEqual (DPoint2dCR vector2) const;

//!
//! @description Test if the x and y components of two points or vectors are
//!   equal within tolerance.
//! Tests are done independently using the absolute value of each component differences
//! (i.e. not the magnitude or sum of squared differences)
//!
//! @param [in] vector2 second point or vector
//! @param [in] tolerance tolerance
//! @return true if all components are within given tolerance of each other.
//!
bool IsEqual (DPoint2dCR vector2, double tolerance) const;

//! @description Finds the largest absolute value among the components of a point or vector.
//! @return largest absolute value among point coordinates.
double MaxAbs () const;

//!
//! @return true if the point has coordinates which indicate it is
//!   a disconnect (separator) ponit.
//!
bool IsDisconnect () const;

//! return true if the x,y (a) sum to within Angle::SmallAngle () of 1 and (optionally) (b) are both 0 or positive
//! <ul>
//! <li>Summing to 1 makes the pair valid weights for interpolation between 2 points (or other values)
//! <li>"allowExtrapolation" false restricts the interpolation to "interior" results
//! <li>"allowExtrapolation" true allows pairs that are both outside 1 but still sum to 1, i.e. represent extrapolation
//! </ul>
bool IsConvexPair (bool allowExtrapolation = false) const;

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second DPoint2d
//! @return true if within tolerance.
bool AlmostEqual (DPoint2d const & dataB) const;

static bool LexicalXYLessThan (DPoint2d const &a, DPoint2d const &b);

#endif // __cplusplus
};

END_BENTLEY_NAMESPACE
#endif // dpoint2d_H_
