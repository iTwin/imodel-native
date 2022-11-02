/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once



/*
#ifdef flexwiki
:Title: struct Bentley::DVec2d

Summary: A vector in xy space.
 
!!! Fields
 
|| Member  ||      ||
|| x       || x part ||
|| y       || y part ||
 
#endif
*/

#ifndef dvec2d_H_
#define dvec2d_H_

BEGIN_BENTLEY_NAMESPACE



#ifdef __cplusplus
/**
Vector with x,y components.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DVec2d : public DPoint2d
{
//flex!! Construction and initialization
//flex|| computed matrix || result returned from static method || result placed in instance.  No use of prior instance contents. || modify instance in place ||

//flex|| by component  || vector = DVec2d::From (ax, ay, az)  || vector.Init (ax, ay, az) ||
//flex|| all zeros  ||  || vector.Zero () ||
//flex|| all ones  ||  || vector.One () ||
//flex|| point components    || vector = DVec2d::From (point) @qD vector.Init (point) ||


//! Simple initializer from xy components.
static DVec2d From (double x, double y);
//! @description init from doubles.
void Init  (double ax, double ay);

//! @description Returns a DVec2d from a point (treating the point as a vector from its origin).
//! @param [in] point the point.
static DVec2d From (DPoint2dCR point);
//! @description Initialize a vector from a point (treating the point as a vector from its origin).
//! @param [in] point the point.
void Init (DPoint2dCR point);

//! @description Sets all components of a vector to zero.
void Zero ();

//! @description Returns a vector with all components 1.0.
void One ();




//! Return a DVec2d from start point to end point.
//! @param [in] start start point
//! @param [in] end   end point
static DVec2d FromStartEnd (DPoint2dCR start, DPoint2dCR end);

//! Return a DVec2d from start point to end point.
//! @param [in] start start point
//! @param [in] end   end point
static DVec2d FromStartEnd (DPoint3dCR start, DPoint3dCR end);


//! Return a vector rotated 90 degrees from the input.
static DVec2d FromRotate90CCW
(
DVec2dCR vector     //!< [in] starting vector
);

//! Return a vector rotated an integer multiple of 90 degrees from the input.
static DVec2d FromRotate90CCW
(
DVec2dCR vector,     //!< [in] starting vector
int      numSteps   //!< [in] number of 90 degree rotation steps.
);

//! Return the 90 degree CCW rotation of this vector.
DVec2d Rotate90CCW () const;

//! Return the 90 degree CW rotation of this vector.
DVec2d Rotate90CW() const;

//flex|| negated vector                ||                 || vector.Negate (vectorA)         || vector.Negate () ||
//! @description Computes a negated (opposite) vector.
//! @param [in] vector The vector to be negated.
void Negate (DVec2dCR vector);

//! @description Negate a vector in place.
void Negate ();



//flex|| vectorA + vectorB                                         || || vector.SumOf (vectorA, vectorB)                                 || inOutVectorA.Add (vectorB) ||
//flex|| vectotrA * scaleA                                         || vector = DVec2d::FromScale (vectorA, scaleA)                        || vector.Scale (vectorA, scaleA) || vector.Scale (scale) ||
//flex|| vectorA *scaleA +  vectorB * scaleB                       || vector = DVec2d::FromSumOf (vectorA, scaleA, vectorB, scaleB)       ||
//flex|| vectorA *scaleA +  vectorB * scaleB + vectorC * scaleC    || vector = DVec2d::FromSumOf (vectorA, scaleA, vectorB, scaleB, vectorC, scaleC) ||


//! @description Compute the sum of two vectors or vectors.
//! @param [in] vector1 The the first vector
//! @param [in] vector2 The second vector
void SumOf  (DVec2dCR vector1, DVec2dCR vector2);

//! @description Adds a vector to a pointer or vector, returns the result in place.
//! @param [in] vector The vector to add.
void Add (DVec2dCR vector);

//! @description Returns a scalar multiple of a DVec2d
//! @param [in] vector input vector
//! @param [in] scale scale factor
static DVec2d FromScale (DVec2dCR vector, double scale);

//! @description Multiplies a vector by a scale factor.
//! @param [in] vector The vector to be scaled.
//! @param [in] scale The scale factor.
void Scale  (DVec2dCR vector, double scale);

//!
//! @description Multiplies a vector (in place) by a scale factor.
//! @param [in] scale The scale
//!
void Scale (double scale);


//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] scale0  first scale
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
static DVec2d FromSumOf (DVec2dCR vector0, double scale0, DVec2dCR vector1, double scale1);

//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
static DVec2d FromSumOf (DVec2dCR vector0, DVec2dCR vector1, double scale1);


//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] scale0  first scale
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
//! @param [in] vector2 third vector
//! @param [in] scale2 third scale
static DVec2d FromSumOf (DVec2dCR vector0, double scale0, DVec2dCR vector1, double scale1, DVec2dCR vector2, double scale2);

//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
//! @param [in] vector2 third vector
//! @param [in] scale2 third scale
static DVec2d FromSumOf (DVec2dCR vector0, DVec2dCR vector1, double scale1, DVec2dCR vector2, double scale2);

//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
//! @param [in] vector2 third vector
//! @param [in] scale2 third scale
//! @param [in] vector3 fourth vector
//! @param [in] scale3 fourth scale
static DVec2d FromSumOf (DVec2dCR vector0, DVec2dCR vector1, double scale1, DVec2dCR vector2, double scale2, DVec2dCR vector3, double scale3);

//flex|| origin + vectorA *scaleA                                           ||    || vector.SumOf (origin, vectorA, scaleA) ||
//flex|| origin + vectorA *scaleA +  vectorB * scaleB                       ||    || vector.SumOf (origin, vectorA, scaleA, vectorB, scaleB) ||
//flex|| origin + vectorA *scaleA +  vectorB * scaleB + vectorC * scaleC    ||    || vector.SumOf (origin, vectorA, scaleA, vectorB, scaleB, vectorC, scaleC) ||

//! @description Adds an origin and a scaled vector.
//! @param [in] origin Origin for the sum.
//! @param [in] vector The vector to be added.
//! @param [in] scale The scale factor.
void SumOf  (DVec2dCR origin, DVec2dCR vector, double scale);

//! @description Adds an origin and two scaled vectors.
//! @param [in] origin The origin.
//! @param [in] vector1 The first direction vector
//! @param [in] scale1 The first scale factor
//! @param [in] vector2 The second direction vector
//! @param [in] scale2 The second scale factor
void SumOf  (DVec2dCR origin, DVec2dCR vector1, double scale1, DVec2dCR vector2, double scale2);

//! @description Adds an origin and three scaled vectors.
//! @param [in] origin The origin.
//! @param [in] vector1 The first direction vector
//! @param [in] scale1 The first scale factor
//! @param [in] vector2 The second direction vector
//! @param [in] scale2 The second scale factor
//! @param [in] vector3 The third direction vector
//! @param [in] scale3 The third scale factor
void SumOf  (DVec2dCR origin, DVec2dCR vector1, double scale1, DVec2dCR vector2, double scale2, DVec2dCR vector3, double scale3);


//flex|| vectorB-vectorA || || vector.DifferenceOf (vectorB, vectorA) || inOutVectorB.Subtract (vectorA) ||
//flex|| pointB-pointA   || || vector.DifferenceOf (pointB, pointA) ||
//flex|| unitp (pointB-pointA)   || || a = vector.NormalizedDifference (pointB, pointA) ||

//! @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second (subtracted) vector
void DifferenceOf  (DVec2dCR vector1, DVec2dCR vector2);

//! @description Subtract coordinates of two points. (Compute Point1 - Point2)
//! @param [in] target The target point
//! @param [in] base The base point
void DifferenceOf  (DPoint2dCR target, DPoint2dCR base);


//! @description Subtract two vectors, and return the result in
//!           place of the first.
//! @param [in] vector2 The vector to subtract.
void Subtract (DVec2dCR vector2);

//! @description Computes a unit vector  in the direction of the difference of the vectors
//! or vectors (Second parameter vector is subtracted from the first parameter vector,
//! exactly as in the subtract function.)
//! @remark In the 0-length error case, the vector is set to (1,0)
//! @param [in] target The target point.
//! @param [in] origin The origin point.
//! @return The length of original difference vector.
double NormalizedDifference  (DPoint2dCR target, DPoint2dCR origin);

//flex|| normalize existing vector     ||     || a = vector.Normalize (vectorA)      || a = vector.Normalize () ||
//flex|| scale existing vector to target length     ||     || a = vector.Normalize (vectorA, length)      || a = vector.scaleToLength (length) ||

//! @description Normalizes (scales) a vector to length 1.
//! @remark In the 0-length error case, the vector is set to (1,0).
//! @param [in] vector The vector to be normalized.
//! @return The length prior to normalization
double Normalize (DVec2dCR vector);

//!
//! @description initialize this vector to unit vector in the direction of source.
//! @remark In the 0-length error case, the vector is set to (1,0).
//! @param [in] source The vector to be normalized.
//! @param [out] magnitude The orignal magnitude of the source vector;
//! @return true if the other vector length is large enough for DoubleOps::SafeDivide to compute 1/length.
bool TryNormalize (DVec2dCR source, double &magnitude);
//!
//! @description Replaces a vector by a unit vector in the same direction, and returns
//! the original length.
//! @remark In the 0-length error case, the vector is set to (1,0).
//!
//! @return The length prior to normalization
//!
double Normalize ();


//! @description Scales a vector to specified length.
//! @remark In the 0-length error case, the vector is set to (1,0).
//!
//! @param [in] vector The original vector.
//! @param [in] length The requested length.
//! @return The length prior to scaling.
double ScaleToLength  (DVec2dCR vector, double length);

//!
//! @description Scales a vector to a specified length, and returns
//! the prior length.
//! @remark In the 0-length error case, the vector is set to (1,0).
//!
//! @param [in] length The requested length
//! @return The length prior to scaling.
//!
double ScaleToLength (double length);


//flex|| divide by scale ||       || vector.SafeDivide (vectorA, denominator) ||

//!
//! @description Try to divide each component of a vector by a scalar.  If the denominator
//! near zero compared to any numerator component, return the original
//! vector.
//! @param [in] vector The initial vector.
//! @param [in] denominator The divisor.
//! @return true if division is numerically safe.
//!
bool SafeDivide  (DVec2dCR vector, double denominator);



//flex|| interpolation between || vector = DVec2d::FromInterpolate (vectorA, fraction, vectorB) || vector.Interpolate (vectorA, fraction, vectorB) ||
//flex|| bilinear combination || vector = DVec2d::FromInterpolateBilinear (vector00, vector10, vector01, vector11, u, v) ||

//! @description Returns an interpolated vector.
//! @param [in] vector0 vector at fraction 0
//! @param [in] fraction fraction from interpolation.
//! @param [in] vector1 vector at fraction 1
static DVec2d FromInterpolate (DVec2dCR vector0, double fraction, DVec2dCR vector1);



//!
//! @description Computes a vector whose position is given by a fractional
//! argument and two vectors.
//!
//! @param [in] vector0 The vector corresponding to fractionParameter of 0.
//! @param [in] fractionParameter The fractional parametric coordinate.
//!               0.0 is the start of the segment, 1.0 is the end, 0.5 is middle
//! @param [in] vector1 The vector corresponding to fractionParameter of 1.
//!
void Interpolate  (DVec2dCR vector0, double fractionParameter, DVec2dCR vector1);

//! @description Returns a bilinear interpolation from corners (00)(10)(01)(11)
//! @param [in] vector00 vector at (0,0)
//! @param [in] vector10 vector at (1,0)
//! @param [in] vector01 vector at (0,1)
//! @param [in] vector11 vector at (1,1)
//! @param [in] u interpolation fraction for edges (vector00,vector10) and (vector01,vector11)
//! @param [in] v interpolation fraction for edges (vector00,vector10) and (vector10,vector11)
static DVec2d FromInterpolateBilinear (DVec2dCR vector00, DVec2dCR vector10, DVec2dCR vector01, DVec2dCR vector11, double u, double v);

//flex|| Rotated xy parts from vectorA || || vector.Rotate (vectorA, radians) || inoutVector.Rotate (radians) ||
//flex|| Unit Perpendicular  || || vector.UnitPerpendicular (vectorA) ||

//! @description Rotate a vector COUNTERCLOCKWISE around the z axis.
//! @param [in] vector vector to rotate.
//! @param [in] theta The rotation angle.
void RotateCCW  (DVec2dCR vector, double theta);
//! Return a vector rotated by angle theta from the input
static DVec2d FromRotateCCW (DVec2dCR vector, double theta);
//! @description Rotate the instance in place around the z axis.
//! @param [in] theta The rotation angle.
void RotateCCW (double theta);

//! @description Compute a unit vector perpendicular given vector.
//! @remarks Input may equal output.
//! @param [in] vector The source vector
//! @return true if the input vector has nonzero length
bool UnitPerpendicular (DVec2dCR vector);

//flex|| copy xy from 4d point (NOT DEWEIGHTED) || || vector.XyOf (point4d) ||

//! @description Sets the x,y, and z components of a DVec2d structure from the
//! corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
//! @param [in] hPoint The homogeneous point
void XyOf (DPoint4dCR hPoint);

//flex!! Scalar queries

//flex|| Magnitude || vector.Magntiude () || vector.MagnitudeSquared () ||

//! @description Computes the squared magnitude of a vector.
//! @return The squared magnitude of the vector.
double MagnitudeSquared () const;

//! @description Computes the magnitude of a vector.
//! @return The length of the vector
double Magnitude () const;

//flex|| largest component || a = vectorA.MaxAbs () ||
//!
//! @description Finds the largest absolute value among the components of a vector.
//! @return The largest absolute value among vector coordinates.
//!
double MaxAbs () const;

//flex|| distance between  || a = vectorA.Distance (vectorB) || a = vectorA.DistanceSquared (vectorB) ||

//!
//! @description Computes the (cartesian) distance between two vectors
//!
//! @param [in] vector2 The second vector
//! @return The distance between vector.
//!
double Distance (DVec2dCR vector2) const;

//!
//! @description Computes the squared distance between two vectors.
//!
//! @param [in] vector2 The second vector.
//! @return The squared distance between the vectors.
//!
double DistanceSquared (DVec2dCR vector2) const;





//flex|| Single component access       || vector.SetComopnent (value, index) || a = vector.GetComponent (index) ||
//flex||                               ||             || vector.GetComponents (outX, outY) ||


//!
//! @description Gets a single component of a vector.  If the index is out of
//! range 0,1 it is interpretted cyclically.
//!
//! @param [in] index Indicates which component is accessed.  The values
//!                       are 0=x, 1=y  Other values are treated cyclically.
//! @return The specified component of the vector.
//!
double GetComponent (int index) const;

//!
//! @description Set one of three components (x,y) of the vector.
//!
//! @param [in] a The component value.
//! @param [in] index Selects the the axis: 0=x, 1=y
//!
void SetComponent  (double a, int index);

//! @description Copies x,y components from a vector to individual variables.
//!
//! @param [out] xCoord x component
//! @param [out] yCoord y component
//!
void GetComponents  (double &xCoord, double &yCoord);




//flex|| dot product || a = vectorA.DotProduct (vectorB) || a = vectorA.DotProduct (x, y) ||

//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] vector2 The second vector
//! @return The dot product of the two vectors
double DotProduct (DVec2dCR vector2) const;
//! @description Computes the dot product of one vector given as a vector structure and another given as
//! xyz components.
//! @param [in] ax The x component of second vector.
//! @param [in] ay The y component of second vector.
//! @return The dot product of the vector with a vector with the given components
double DotProduct  (double ax, double ay) const;


//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] point2 The second vector, given as point.
//!        The point's xyz are understood to be a vector from the origin.
//! @return dot product of the two vectors.
double DotProduct (DPoint2dCR point2) const;

//flex|| cross product (scalar) || vectorA.CrossProduct (vectorB) || vectorA.CrossProductSquared (vectorB) ||

//! @description Returns the (vector) cross product of two vectors.
//! @param [in] vector2 The second vector
double CrossProduct (DVec2dCR vector2) const;

//! @description Returns the (vector) cross product of two vectors.
//! @param [in] vector2 The second vector
double CrossProductSquared (DVec2dCR vector2) const;
//flex!! Angles and Sector Inclusion

//flex|| angle from vectorA to vectorB || radians(-pi..+pi) = vectorA.AngleTo (vectorB) ||
//flex|| angle from vectorA to vectorB or its negative || radians(0..pi/2) = vectorA.SmallerUnorientedAngleTo (vectorB) ||

//! Return the fractional coordinate where the other vector projects onto the instance. (0 if instance has no length)
ValidatedDouble ProjectionFraction (DVec2d const &other) const;

//! @return the fractional position along the instance when the other vector is projected perpendicular to the instance.
//! @remark 0.0 is returned if instance length is zero !!!
ValidatedDouble PerpendicularProjectionFraction (DVec2d const &other) const;

//! Return a vector that 
static DVec2d OffsetBisector (DVec2d const &unitPerpA, DVec2d const &unitPerpB, double offset);

//! @description Returns the (signed) angle (in radians) between two vectors.  This angle is between -pi and
//! pi.  Rotating the first vector by this angle around Z aligns it with the second vector.
//! @param [in] vector2 The second vector
//! @return The angle between the vectors.
double AngleTo (DVec2dCR vector2) const;

//!
//! @description Returns the angle between two vectors, choosing the smaller
//!   of the two possible angles when both the vectors and their negations are considered.
//!    This angle is between 0 and pi/2.
//!
//! @param [in] vector2 The second vector
//! @return The angle between the vectors.
//!
double SmallerUnorientedAngleTo (DVec2dCR vector2) const;

//flex|| is instance in smaller sector sector of sweep from vectorA to vectorB || bool vector.IsVectorInSmallerSector (vectorA, vectorB, upVector) ||
//flex|| is instance in counterclockwise sector of sweep from vectorA to vectorB || bool vector.IsInCCWSector (vectorA, vectorB, upVector) ||

//!
//! @description Test a vector is "between" vector0 and vector1.
//! If the vectors are coplanar and vector0 is neither parallel nor antiparallel
//! to vector1, betweenness has the expected meaning: there are two angles between
//! vector0 and vector1; one is less than 180; the test vector is tested to
//! see if it is in the smaller angle.
//! If the vectors are not coplanar, the test is based on the projection of
//! the test vector into the plane of the other two vectors.
//!
//! Zero testing is untoleranced, and is biased to all parallel conditions "false".
//! That is, if any pair of the input vectors is parallel or antiparallel,
//! the mathematical answer is false.  Floating point tolerances
//! will cause "nearby" cases to be unpredictable.  It is assumed that if
//! the caller considers the "parallel" cases important they will be
//! checked explicitly.
//!
//! @param [in] vector0 The first boundary vector.
//! @param [in] vector1 The second boundary vector.
//! @return true if the test vector is within the angle.
//!
bool IsVectorInSmallerSector  (DVec2dCR vector0, DVec2dCR vector1) const;

//! @description test if the instance is "between" vector0 and vector1 when vector0 is rotated counterclockwise towards vector1.
//! @param [in] vector0 start vector for CCW sweep.
//! @param [in] vector1 end vector for CCW sweep.
bool IsVectorInCCWSector  (DVec2dCR vector0, DVec2dCR vector1) const;



//flex|| parallel and perpendicular tests || bool vectorA.IsParallelTo (vectorB)  || vectorA.IsPerpendicularTo (vectorB) ||

//!
//! @description Tests if two vectors are parallel.
//!
//! @param [in] vector2 The second vector
//! @return true if the vectors are parallel within tolerance
//!
bool IsParallelTo (DVec2dCR vector2) const;

//! @description Tests if two vectors are parallel and have positive dot product (opposites are NOT considered parallel)
//! @param [in] vector2 The second vector
//! @return true if the vectors are parallel within tolerance
bool IsPositiveParallelTo (DVec2dCR vector2) const;
//!
//! @description Tests if two vectors are perpendicular.
//!
//! @param [in] vector2 The second vector
//! @return true if vectors are perpendicular within tolerance
//!
bool IsPerpendicularTo (DVec2dCR vector2) const;


//flex|| all zeros || vector.IsZero () ||
//flex|| equality with system tolerance || vectorA.AlmostEqual (vectorB) ||
//!
//! @description Test for exact equality between all components of two vectors.
//! @param [in] vector2 The second vector
//! @return true if the vectors are identical.
//!
bool IsEqual (DVec2dCR vector2) const;

//!
//! @description Test if the x, y, and z components of two vectors are
//!   equal within tolerance.
//! Tests are done independently using the absolute value of each component differences
//! (i.e. not the magnitude or sum of squared differences)
//!
//! @param [in] vector2 The second vector.
//! @param [in] tolerance The tolerance.
//! @return true if all components are within given tolerance of each other.
//!
bool IsEqual
(
DVec2dCR        vector2,
double          tolerance
) const;

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second DVec2d
//! @return true if within tolerance.
bool AlmostEqual (DVec2d const & dataB) const;


};
#endif

END_BENTLEY_NAMESPACE

#endif // dvec2d_H_
