/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/FPoint3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#ifndef FPoint3d_H_
#define FPoint3d_H_

BEGIN_BENTLEY_NAMESPACE
/**
3d point coordinates.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP FPoint3d
{
public:
//! x coordinate
float x;
//! y coordinate
float y;
//! z coordinate
float z;
private:
// private members expected to be inlined
    static FPoint3d from (double xx, double yy, double zz)
        {
        FPoint3d xyz;
        xyz.x = (float)xx;
        xyz.y = (float)yy;
        xyz.z = (float)zz;
        return xyz;
        }
    void init (double xx, double yy, double zz)
        {
        x = (float)xx;
        y = (float)yy;
        z = (float)zz;
        }
// private members expected to be inlined
    static FPoint3d from (DPoint3dCR in)
        {
        FPoint3d xyz;
        xyz.x = (float)in.x;
        xyz.y = (float)in.y;
        xyz.z = (float)in.z;
        return xyz;
        }


public:
#ifdef __cplusplus
//BEGIN_FROM_METHODS

//! Return a FPoint3d with given xyz.
static FPoint3d From (double x, double y, double z = 0.0);

//! Return a FPoint3d with given 2d point and z
static FPoint3d From (DPoint2dCR xy, double z);

//! Return a FPoint3d from DPoint3d
static FPoint3d From (DPoint3dCR xyz);


//! Return a FPoint3d from xy parts of given point, with z from optional parameter. (And z of the input point is ignored)
static FPoint3d FromXY (FPoint3dCR xy, double z = 0.0);

//! Return a FPoint3d from xy parts of given point, with z from optional parameter. (And z of the input point is ignored)
static FPoint3d FromXY (DPoint3dCR xy, double z = 0.0);

//! Return a FPoint3d with xyz = 0.
static FPoint3d FromZero();
//! Return a FPoint3d with xyz = 1.
static FPoint3d FromOne();


//! @description Simple initialization from base point and shifts
static FPoint3d FromShift
(
FPoint3dCR xyz0,    //!< [in] reference point
double dx = 0.0,    //!< [in] shift to apply to x direction
double dy = 0.0,    //!< [in] shift to apply to y direction
double dz = 0.0     //!< [in] shift to apply to z direction
);

//END_FROM_METHODS

/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/

//! Swap contents of instance, other.
//! @param [in,out] other second point.
void Swap (FPoint3dR other);

//! @description Returns the (scalar) cross product of the xy parts of two vectors.
//!   The vectors are computed from the Origin to target1 and target2.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
double CrossProductToPointsXY (FPoint3dCR target1, FPoint3dCR target2) const;

//! @description Returns the (scalar) dot product of two vectors.
//!   The vectors are computed from the Origin to target1 and target2.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
double DotProductToPoints (FPoint3dCR target1, FPoint3dCR target2) const;

//! @description Returns the (scalar) dot product of xy parts of two vectors.
//!   The vectors are computed from the BasePoint to target1 and target2.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
double DotProductToPointsXY (FPoint3dCR target1, FPoint3dCR target2) const;

//!
//! @description Returns the (scalar) dot product of a two vectors.
//!  One vector is computed internally as the difference of the TargetPoint
//!   and Origin. (TargetPoint-Origin)
//!  The other is given directly as a single argument.
//!
//! @param [in] origin  The start (orign) point of the first vector of the cross product.
//! @param [in] vector  The second
//!
double DotDifference (FPoint3dCR origin, DVec3dCR vector) const;

//!
//! @description Computes the triple product of vectors from a base point three target points.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
//! @param [in] target3 The target point for the third vector.
//! @return The triple product
//!
double TripleProductToPoints (FPoint3dCR target1, FPoint3dCR target2, FPoint3dCR target3) const;

//!
//! @description Sets all components of a point or vector to zero.
//!
void Zero ();

//!
//! @description Returns a point or vector with all components 1.0.
//!
void One ();

//!
//! @description Copies doubles from a 3 component array to the x,y, and z components
//! of a FPoint3d
//!
//! @param [in] pXyz x, y, z components
//!
void InitFromArray (const double *pXyz);


//!
//! @description Copy from a 2d point setting z to zero.
//!
//! @param [in] source source point for xy values
//! @param [in] z z value
//!
void Init (DPoint2dCR source, double z = 0.0);


//!
//! @description Sets the x,y, and z components of a point
//!
//! @param [in] ax The x component.
//! @param [in] ay The y component.
//! @param [in] az The z component.
//!
void Init (double ax, double ay, double az = 0.0);

//!
//! @description Sets the x,y, and z components of a FPoint3d structure from the
//! corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
//!
//! @param [in] hPoint The homogeneous point
//!
void XyzOf (DPoint4dCR hPoint);

//!
//! @description Set one of three components (x,y,z) of the point.
//!
//! @param [in] a The component value.
//! @param [in] index Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
//!
void SetComponent (double a, int index);

//!
//! @description Gets a single component of a point.  If the index is out of
//! range 0,1,2, it is interpreted cyclically.
//!
//! @param [in] index Indicates which component is accessed.  The values
//!                       are 0=x, 1=y, 2=z.  Other values are treated cyclically.
//! @return The specified component of the point or vector.
//!
double GetComponent (int index) const;

//!
//! @description Copies x,y,z components from a point to individual variables.
//!
//! @param [out] xCoord x component
//! @param [out] yCoord y component
//! @param [out] zCoord z component
//!
void GetComponents (double &xCoord, double &yCoord, double &zCoord) const;

//!
//! @description Computes a point whose position is given by a fractional
//! argument and two endpoints.
//!
//! @param [in] point0 The point corresponding to fractionParameter of 0.
//! @param [in] fractionParameter The fractional parametric coordinate.
//!               0.0 is the start of the segment, 1.0 is the end, 0.5 is midpoint.
//! @param [in] point1 The point corresponding to fractionParameter of 1.
//!
void Interpolate (FPoint3dCR point0, double fractionParameter, FPoint3dCR point1);

//!
//! @description Form vectors from the origin to the test point and the two boundary vectors.
//! Test if the test vector is within the smaller angle between the other two vectors.
//! @param [in] origin The point to test.
//! @param [in] target1 The first target point.
//! @param [in] target2 The second target point.
//! @return true if the test point is within the angle.
//!
bool IsPointInSmallerSector (FPoint3dCR origin, FPoint3dCR target1, FPoint3dCR target2) const;

//!
//! @description Test if a point is within the counter-clockwise sector defined by
//! an origin and two boundary points, with an up vector to determine which
//! direction is counter clockwise.
//! @param [in] origin The point to test.
//! @param [in] target0 The first target point.
//! @param [in] target1 The second target point.
//! @param [in] upVector vector towards eye to resolve direction.
//! @return true if the test point is within the angle.
//!
bool IsPointInCCWector (FPoint3dCR origin, FPoint3dCR target0, FPoint3dCR target1, DVec3dCR upVector) const;

//!
//! @description Computes the (cartesian) distance between two points
//!
//! @param [in] point2 The second point
//! @return The distance between points.
//!
double Distance (FPoint3dCR point2) const;

//!
//! @description Computes the squared distance between two points.
//!
//! @param [in] point2 The second point.
//! @return The squared distance between the points.
//!
double DistanceSquared (FPoint3dCR point2) const;

//!
//! @description Computes the squared distance between two points, using only the
//!       xy parts.
//!
//! @param [in] point2 The second point
//! @return The squared distance between the XY projections of the two points.
//!               (i.e. any z difference is ignored)
//!
double DistanceSquaredXY (FPoint3dCR point2) const;

//!
//! @description Computes the distance between two points, using
//!   only x and y components.
//!
//! @param [in] point2 The second point
//! @return The distance between the XY projections of the two points.
//!               (i.e. any z difference is ignored)
//!
double DistanceXY (FPoint3dCR point2) const;

//! @description Computes the distance between two points, using
//!   only x and y components, optionally applying a transform into view space.
//!
//! @param [in] otherPoint The second point
//! @param [in] matrix optional transform
//! @param [out] distance computed distance.
//! @return true if both points normalized properly after the transform.
bool DistanceXY (FPoint3dCR otherPoint, DMatrix4dCP matrix, double &distance) const;

//! @description Finds the largest absolute value among the components of a point or vector.
//! @return The largest absolute value among point coordinates.
double MaxAbs () const;

//! @description Finds the largest absolute coordinate difference between two points.
//! @return The largest absolute value among point coordinates.
double MaxDiff (FPoint3dCR other) const;

//! @description Finds the smallest absolute value among the components of a point or vector.
//! @return The smallest absolute value among point coordinates.
double MinAbs () const;

//! return in the index of the MaxAbs () value.
int MaxAbsIndex () const;

//! return in the index of the MinAbs () value
int MinAbsIndex () const;


//! @return min and max of componetns.
DRange1d ComponentRange () const;

//!
//! @description Test for exact equality between all components of two points or vectors.
//! @param [in] point2 The second point or vector
//! @return true if the points are identical.
//!
bool IsEqual (FPoint3dCR point2) const;

//!
//! @description Test if the x, y, and z components of two points or vectors are
//!   equal within tolerance.
//! Tests are done independently using the absolute value of each component differences
//! (i.e. not the magnitude or sum of squared differences)
//!
//! @param [in] point2 The second point or vector.
//! @param [in] tolerance The tolerance.
//! @return true if all components are within given tolerance of each other.
//!
bool IsEqual (FPoint3dCR point2, double tolerance) const;


//!
//! @description Initialize a point by copying x,y,z from a vector.
//! @param [in] vector  The vecotr
//!
void Init (DVec3dCR vector);

//!
//! @description Subtract a vector from a point.
//!
//! @param [in] base  The the first point or vector
//! @param [in] vector  The second point or vector
//!
void Subtract (FPoint3dCR base, DVec3dCR vector);

//!
//! @description Adds a vector to a pointer or vector, returns the result in place.
//!
//! @param [in] vector  The vector to add.
//!
void Add (DVec3dCR vector);

//! Add translation to all points.
static void AddToArray (FPoint3dP points, int n, FPoint3dCR delta);
//!
//! @description Adds an origin and a scaled vector.
//!
//! @param [in] origin  Origin for the sum.
//! @param [in] vector  The vector to be added.
//! @param [in] scale  The scale factor.
//!
void SumOf (FPoint3dCR origin, DVec3dCR vector, double scale);

//!
//! @description Adds an origin and two scaled vectors.
//!
//! @param [in] origin  The origin.
//! @param [in] point1  The first direction vector
//! @param [in] scale1  The first scale factor
//! @param [in] point2  The second direction vector
//! @param [in] scale2  The second scale factor
//!
void SumOf (FPoint3dCR origin, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2);

//!
//! @description Adds an origin and three scaled vectors.
//!
//! @param [in] origin  The origin.
//! @param [in] point1  The first direction vector
//! @param [in] scale1  The first scale factor
//! @param [in] point2  The second direction vector
//! @param [in] scale2  The second scale factor
//! @param [in] point3  The third direction vector
//! @param [in] scale3  The third scale factor
//!
void SumOf (FPoint3dCR origin, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2, DVec3dCR vector3, double scale3);

//!
//! @description Returns the cross (vector) cross product of two vectors.
//! @param [in] point1 The first vector
//! @param [in] point2 The second vector
//!
void CrossProduct (FPoint3dCR point1, FPoint3dCR point2);

//!
//! @description Returns the (vector) cross product of two vectors.
//!   The vectors are computed from the Origin to target1 and target2.
//! @param [in] origin The base point for computing vectors.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
//!
void CrossProductToPoints (FPoint3dCR origin, FPoint3dCR target1, FPoint3dCR target2);

//!
//! @description Return the (scalar) cross product of the xy parts of two vectors.
//! @param [in] point2 The second vector
//! @return The 2d cross product.
//!
double CrossProductXY (FPoint3dCR point2) const;

//!
//! @description Compute the normalized cross product of two vectors
//! and return the length of the unnormalized cross product.
//!
//! @param [in] point1 The first vector
//! @param [in] point2 The second vector
//! @return The length of the original (prenormalization) cross product vector
//!
double NormalizedCrossProduct (FPoint3dCR point1, FPoint3dCR point2);

//!
//! @description Computes the cross product of the two parameter vectors and scales it to a given
//! length.  The scaled vector is stored as the product vector, and the length of the original
//! cross product vector is returned.
//!
//! @param [in] point1 The first vector
//! @param [in] point2 The second vector
//! @param [in] productLength The Desired length
//! @return The length of original vector.
//!
double SizedCrossProduct (FPoint3dCR point1, FPoint3dCR point2, double productLength);

//!
//! @description Computes the cross product of two vectors and scales it to the
//! geometric mean of the lengths of the two vectors.  This is useful
//! because it has the direction of the cross product (i.e. normal to the plane
//! of the two vectors) and a size in between the two vectors.
//!
//! @param [in] point1 The first vector
//! @param [in] point2 The second vector
//! @return The length of original vector.
//!
double GeometricMeanCrossProduct (FPoint3dCR point1, FPoint3dCR point2);

//!
//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] point2 The second vector
//! @return The dot product of the two vectors
//!
double DotProduct (FPoint3dCR point2) const;

//!
//! @description Returns the (scalar) dot product of xy parts of two vectors.
//! @param [in] point2 The second vector
//! @return The dot product of the xy parts of the two vectors
//!
double DotProductXY (FPoint3dCR point2) const;

//!
//! @description Computes the dot product of one vector given as a point structure and another given as
//! xyz components.
//! @param [in] ax The x component of second vector.
//! @param [in] ay The y component of second vector.
//! @param [in] az The z component of second vector.
//! @return The dot product of the vector with a vector with the given components
//!
double DotProduct (double ax, double ay, double az) const;


//!
//! @description Computes a unit vector  in the direction of the difference of the points
//! or vectors (Second parameter vector is subtracted from the first parameter vector,
//! exactly as in the subtract function.)
//!
//! @param [in] target The target point.
//! @param [in] origin The origin point.
//! @return The length of original difference vector.
//!
double NormalizedDifference (FPoint3dCR target, FPoint3dCR origin);

//!
//! @description Returns the angle between two vectors.  This angle is between 0 and
//! pi.  Rotating the first vector by this angle around the cross product
//! between the vectors aligns it with the second vector.
//!
//! @param [in] point2 The second vector
//! @return The angle between the vectors.
//!
double AngleTo (FPoint3dCR point2) const;

//!
//! @description Computes the (signed) angle from xy axis to the vector, using only xy parts.
//!
double AngleXY () const;
//!
//! @description Returns the angle between two vectors, choosing the smaller
//!   of the two possible angles when both the vectors and their negations are considered.
//!    This angle is between 0 and pi/2.
//!
//! @param [in] point2 The second vector
//! @return The angle between the vectors.
//!
double SmallerUnorientedAngleTo (FPoint3dCR point2) const;

//!
//! @description Test a vector is "between" point0 and point1.
//! If the vectors are coplanar and point0 is neither parallel nor antiparallel
//! to point1, betweenness has the expected meaning: there are two angles between
//! point0 and point1; one is less than 180; the test vector is tested to
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
//! @param [in] point0 The first boundary vector.
//! @param [in] point1 The second boundary vector.
//! @return true if the test vector is within the angle.
//!
bool IsVectorInSmallerSector (FPoint3dCR point0, FPoint3dCR point1) const;

//!
//! @description Test if the test vector vector is "between" point0 and point1, with CCW direction
//! resolved by an up vector.  The cross product of point0 and point1 is
//! considered the positive plane normal if its dot product with the up vector
//! is positive.
//!
//! @param [in] point0 The boundary vector.
//! @param [in] point1 The boundary vector.
//! @param [in] upVector The out of plane vector.
//! @return true if test vector is within the angle.
//!
bool IsVectorInCCWSector (FPoint3dCR point0, FPoint3dCR point1, FPoint3dCR upVector) const;

//!
//! @description Returns the angle from Vector1 to Vector2 using only xy parts.
//!  This angle is between -pi and +pi.
//!
//! @param [in] point2 The second vector
//! @return The angle between vectors.
//!
double AngleToXY (FPoint3dCR point2) const;

//!
//! @description Returns the angle between two vectors, considering both
//!   the vectors and their negations and choosing the smaller.
//!   This angle is between 0 and pi/2.
//!
//! @param [in] point2 The second vector
//! @return The angle between vectors.
//!
double SmallerUnorientedAngleToXY (FPoint3dCR point2) const;

//!
//! @description Rotate vector around the z axis, return as calling instance.
//! @param [in] vector vector to rotate.
//! @param [in] theta The rotation angle.
//!
void RotateXY (FPoint3dCR vector, double theta);

//!
//! @description Rotate the calling instance around the z axis.
//! @param [in] theta The rotation angle.
//!
void RotateXY (double theta);

//!
//! @description Computes the signed from one vector to another, in the plane
//!       of the two vectors.   Initial computation using only the two vectors
//!       yields two possible angles depending on which side of the plane of the
//!       vectors is viewed.  To choose which side to view, go on the side whose
//!       normal has a positive dot product with the orientation vector.
//! This angle can be between -pi and +pi.
//!
//! @param [in] point2 The second vector
//! @param [in] orientationVector The vector used to determine orientation.
//! @return The signed angle
//!
double SignedAngleTo (FPoint3dCR point2, FPoint3dCR orientationVector) const;

//!
//! @description Computes the signed angle between the projection of two vectors
//!       onto a plane with given normal.
//!
//! @param [in] point2 The second vector
//! @param [in] planeNormal The plane normal vector
//! @return The angle in plane
//!
double PlanarAngleTo (FPoint3dCR point2, FPoint3dCR planeNormal) const;

//!
//! @description Scale each point by the other's weight and subtract, i.e. form
//! (point1 * point2.w - point2 * point1.w).  The weight term
//! vanishes.   Copy the xyz parts back as a vector.
//!
//! @param [in] point1 The first point
//! @param [in] point2 The second pont.
//!
void WeightedDifferenceOf (DPoint4dCR point1, DPoint4dCR point2);

//!
//! @description Form the cross product of the weighted differences from point0 to point1 and point2.
//!
//! @param [in] basePoint The common base point (second point for differences)
//! @param [in] target1 The first target point.
//! @param [in] target2 The second target point.
//!
void WeightedDifferenceCrossProduct (DPoint4dCR basePoint, DPoint4dCR target1, DPoint4dCR target2);

//!
//! @description Computes the squared magnitude of a vector.
//!
//! @return The squared magnitude of the vector.
//!
double MagnitudeSquared () const;

//!
//! @description Computes the magnitude of the xy part of a vector.
//! @return The magnitude of the xy parts of the given vector.
//!
double MagnitudeXY () const;

//!
//! @description Computes the squared magnitude of the xy part of a vector.
//! @return The squared magnitude of the xy parts of the given vector.
//!
double MagnitudeSquaredXY () const;

//!
//! @description Compute a unit vector perpendicular to the xy parts of given vector.
//! @param [in] vector The source vector
//! @return true if the input vector has nonzero length
//!
bool UnitPerpendicularXY (FPoint3dCR vector);

//!
//! @description Computes the magnitude of a vector.
//! @return The length of the vector
//!
double Magnitude () const;

//!
//! @description Multiplies a vector by a scale factor.
//! @param [in] vector The vector to be scaled.
//! @param [in] scale The scale factor.
//!
void Scale (FPoint3dCR vector, double scale);

//!
//! @description Multiplies a vector (in place) by a scale factor.
//! @param [in] scale The scale
//!
void Scale (double scale);

//!
//! @description Computes a negated (opposite) vector.
//!
//! @param [in] vector The vector to be negated.
//!
void Negate (FPoint3dCR vector);

//!
//! @description Negate a vector in place.
//!
//!
void Negate ();

//!
//! @description Normalizes (scales) a vector to length 1.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param [in] vector The vector to be normalized.
//! @return The length prior to normalization
//!
double Normalize (FPoint3dCR vector);

//!
//! @description Scales a vector to specified length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param [in] vector The original vector.
//! @param [in] length The requested length.
//! @return The length prior to scaling.
//!
double ScaleToLength (FPoint3dCR vector, double length);

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

//!
//! @description Replaces a vector by a unit vector in the same direction, and returns
//! the original length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @return The length prior to normalization
//!
double Normalize ();

//!
//! @description Tests if two vectors are parallel.
//!
//! @param [in] point2 The second vector
//! @return true if the vectors are parallel within default tolerance
//!
bool IsParallelTo (FPoint3dCR point2) const;

//!
//! @description Tests if two vectors are perpendicular.
//!
//! @param [in] point2 The second vector
//! @return true if vectors are perpendicular within default tolerance
//!
bool IsPerpendicularTo (FPoint3dCR point2) const;

//!
//! @description Try to divide each component of a vector by a scalar.  If the denominator
//! near zero compared to any numerator component, return the original
//! vector.
//! @param [in] vector The initial vector.
//! @param [in] denominator The divisor.
//! @return true if division is numerically safe.
//!
bool SafeDivide (FPoint3dCR vector, double denominator);

//!
//! @description Computes the triple product of three vectors.
//! The following are equivalent definitions of the triple product of three
//! vectors V1, V2, and V3:
//!
//! <UL>
//! <LI> (V1 cross V2) dot V3
//! <LI> V1 dot (V2 cross V3)
//! <LI>The determinant of the 3x3 matrix with the three vectors as its
//!               columns.
//! <LI>The determinant of the 3x3 matrix with the three vectors as its
//!               rows.
//! <LI>The (signed)volume of the parallelepiped whose 4 vertices are at the
//!               origin and at the ends of the 3 vectors placed at the
//!               origin.
//! </UL>
//!
//! @param [in] point2 The second vector.
//! @param [in] point3 The third vector.
//! @return The triple product
//!
double TripleProduct (FPoint3dCR point2, FPoint3dCR point3) const;

//!
//! @description Subtract two points or vectors, and return the result in
//!           place of the first.
//!
//! @param [in] point2 The vector to subtract.
//!
void Subtract (FPoint3dCR point2);

//!
//! @description Subtract coordinates of two vectors or points. (Compute Point1 - Point2)
//!
//! @param [in] point1 The first point
//! @param [in] point2 The second (subtracted) point.
//!
void DifferenceOf (FPoint3dCR point1, FPoint3dCR point2);


//! Componentwise linear combination.
//! @remark Becasue point is base class for vector, this may be used for linear combinations of vectors.
//! @param [in] point1 first point
//! @param [in] a1 scale for first point.
//! @param [in] point2 second point
//! @param [in] a2 scale for second point.

void SumOf (FPoint3dCR point1, double a1, FPoint3dCR point2, double a2);

//! Componentwise linear combination.
//! @remark Becasue point is base class for vector, this may be used for linear combinations of vectors.
//! @param [in] point1 first point
//! @param [in] a1 scale for first point.
//! @param [in] point2 second point
//! @param [in] a2 scale for second point.
//! @param [in] point3 third point
//! @param [in] a3 scale for third point.
void SumOf (FPoint3dCR point1, double a1, FPoint3dCR point2, double a2, FPoint3dCR point3, double a3);


//!
//! @description Compute the sum of two points or vectors.
//!
//! @param [in] point1 The the first point or vector
//! @param [in] point2 The second point or vector
//!
void SumOf (FPoint3dCR point1, FPoint3dCR point2);

//!
//! @description Multiply each point in an array by its corresponding scale factor.
//! @param [out] pDest destination array.
//! @param [in] pSource source array.
//! @param [in] pScales scale factors
//! @param [in] n number of points.
//!
static void MultiplyArrayByScales (FPoint3dP pDest, FPoint3dCP pSource, double *pScales, int n);


//!
//! @description Divide each point in an array by its corresponding scale factor.
//!    Leave any point with near zero weight unchanged.
//! @param [out] pDest destination array.
//! @param [in] pSource source array.
//! @param [in] pScales scale factors
//! @param [in] n number of points.
//!
static void DivideArrayByScales (FPoint3dP pDest, FPoint3dCP pSource, double *pScales, int n);

//! return product of transform times point given as components.
//! @param [in] transform affine transform.
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
static FPoint3d FromProduct (TransformCR transform, double x, double y, double z);

//! return product of transform times point
//! @param [in] transform affine transform.
//! @param [in] point point to transform.
static FPoint3d FromProduct (TransformCR transform, FPoint3dCR point);

//! return product of transform times point
//! @param [in] transform affine transform.
//! @param [in] point point to transform.
static FPoint3d FromProduct (TransformCR transform, DPoint3dCR point);

//! interpolate between points.
//! @param [in] pointA start point
//! @param [in] fraction fractional parameter
//! @param [in] pointB end point
static FPoint3d FromInterpolate (FPoint3dCR pointA, double fraction, FPoint3dCR pointB);

//! interpolate between points.  Then add a shift in the xy plane by a fraction of the XY projection perpendicular
//! @param [in] pointA start point
//! @param [in] fraction fractional parameter along the line from A to B
//! @param [in] pointB end point
//! @param [in] fractionXYPerp fractional parameter applied to vector that is the XY parts of (B-A), rotated CCW in plane.
static FPoint3d FromInterpolateAndPerpendicularXY (FPoint3dCR pointA, double fraction, FPoint3dCR pointB, double fractionXYPerp);


//! return the centroid for points with specified weights
static FPoint3d FromWeightedAverage
(
FPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
FPoint3dCR pointB,  //!< [in] second point
double weightB      //!< [in] weight of second point
);

//! return the centroid for points with specified weights
static FPoint3d FromWeightedAverage
(
FPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
FPoint3dCR pointB,  //!< [in] second point
double weightB,     //!< [in] weight of second point.
FPoint3dCR pointC,  //!< [in] third point
double weightC      //!< [in] weight of third point
);


//! @description Returns a bilinear interpolation from corners (00)(10)(01)(11)
//! @param [in] point00 point at (0,0)
//! @param [in] point10 point at (1,0)
//! @param [in] point01 point at (0,1)
//! @param [in] point11 point at (1,1)
//! @param [in] u interpolation fraction for edges (point00,point10) and (point01,point11)
//! @param [in] v interpolation fraction for edges (point00,point10) and (point10,point11)
static FPoint3d FromInterpolateBilinear (FPoint3dCR point00, FPoint3dCR point10, FPoint3dCR point01, FPoint3dCR point11, double u, double v);

//! Add a point and a vector.
//! @param [in] origin start point
//! @param [in] vector vector add
static FPoint3d FromSumOf (FPoint3dCR origin, DVec3dCR vector);

//! Add scaled vector from origin.
//! @param [in] origin start point
//! @param [in] vector vector to scale.
//! @param [in] scaleFactor multiplier.
static FPoint3d FromSumOf (FPoint3dCR origin, DVec3dCR vector, double scaleFactor);

//! Add scaled vectors from an origin.
//! @param [in] origin start point
//! @param [in] point0 vector to scale.
//! @param [in] scaleFactor0 multiplier.
//! @param [in] point1 vector to scale.
//! @param [in] scaleFactor1 multiplier.
static FPoint3d FromSumOf (FPoint3dCR origin, DVec3dCR vector0, double scaleFactor0, DVec3dCR vector1, double scaleFactor1);

//! Add scaled vectors from origin.
//! @param [in] origin start point
//! @param [in] point0 vector to scale.
//! @param [in] scaleFactor0 multiplier.
//! @param [in] point1 vector to scale.
//! @param [in] scaleFactor1 multiplier.
//! @param [in] point2 vector to scale.
//! @param [in] scaleFactor2 multiplier.
static FPoint3d FromSumOf (FPoint3dCR origin, DVec3dCR vector0, double scaleFactor0, DVec3dCR vector1, double scaleFactor1, DVec3dCR vector2, double scaleFactor2);

//! @description Returns a scalar multiple of a FPoint3d
//! @param [in] point input point
//! @param [in] scale scale factor
static FPoint3d FromScale (FPoint3d point, double scale);

//! @description Returns a linear combination of points
//! @param [in] point0 first point
//! @param [in] scale0  first scale
//! @param [in] point1 second point
//! @param [in] scale1 second scale
static FPoint3d FromSumOf (FPoint3dCR point0, double scale0, FPoint3dCR point1, double scale1);

//! @description Returns a linear combination of points
//! @param [in] point0 first point
//! @param [in] scale0  first scale
//! @param [in] point1 second point
//! @param [in] scale1 second scale
//! @param [in] point2 third point
//! @param [in] scale2 third scale
static FPoint3d FromSumOf (FPoint3dCR point0, double scale0, FPoint3dCR point1, double scale1, FPoint3dCR point2, double scale2);


//! Return {point + matrix * (x,y,z)}
//! @param [in] point base point
//! @param [in] matrix 
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
static FPoint3d FromProduct (FPoint3dCR point, RotMatrixCR matrix, double x, double y, double z);

//! Return {point + matrix * vector}
//! @param [in] point base point
//! @param [in] matrix 
//! @param [in] vector vector part
static FPoint3d FromProduct (FPoint3dCR point, RotMatrixCR matrix, DVec3dCR vector);

//! Return (if possible) the intesection of xy perpendiculars from fractional points on rays from a basePoint.  Returned point has z from basePoint
static ValidatedFPoint3d FromIntersectPerpendicularsXY
(
FPoint3dCR basePoint,   //!< [in] common point of rays
FPoint3dCR targetA,     //!< [in] target point of first ray.
double fractionA,       //!< [in] fractional position for perpendicular to first ray
FPoint3dCR targetB,     //!< [in] target point of second ray
double fractionB        //!< [in] fractional position for perpenedicular to second ray
);

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second FPoint3d
//! @return true if within tolerance.
bool AlmostEqual (FPoint3d const & dataB) const;

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second FPoint3d
//! @return true if within tolerance.
bool AlmostEqualXY (FPoint3d const & dataB) const;

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second FPoint3d
//! @param [in] abstol absolute tolerance.  If 0, use defaults.
//! @return true if within tolerance.
bool AlmostEqual (FPoint3d const & dataB, double abstol) const;

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second FPoint3d
//! @param [in] abstol absolute tolerance.  If 0, use defaults.
//! @return true if within tolerance.
bool AlmostEqualXY (FPoint3d const & dataB, double abstol) const;





//! apply AlmostEqual test to corresponding points
//! @param [in] left first array.
//! @param [in] right first vector.
//! @param [in] tolerance optional tolerance.  If 0, use defaults from AlmostEqual
static bool AlmostEqual (bvector<FPoint3d> const &left, bvector<FPoint3d> const &right, double tolerance = 0.0);

//! apply AlmostEqualXY test to corresponding points
//! @param [in] left first array.
//! @param [in] right first vector.
//! @param [in] tolerance optional tolerance.  If 0, use defaults from AlmostEqualXY
static bool AlmostEqualXY (bvector<FPoint3d> const &left, bvector<FPoint3d> const &right, double tolerance = 0.0);


#endif // __cplusplus

};

END_BENTLEY_NAMESPACE

#endif // FPoint3d_H_

/*__PUBLISH_SECTION_END__*/

