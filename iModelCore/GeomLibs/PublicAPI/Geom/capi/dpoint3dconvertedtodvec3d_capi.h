/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_NAMESPACE

//!
//! @vbdescription Returns the cross (vector) cross product of two vectors.
//! @param pCrossProduct OUT     cross product of vector1 and vector2
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_crossProduct
(
DPoint3dP pCrossProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Returns the (vector) cross product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! @param pCrossProduct OUT     product
//! @param pOrigin IN      The base point for computing vectors.
//! @param pTarget1 IN      The target point for the first vector.
//! @param pTarget2 IN      The target point for the second vector.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_crossProduct3DPoint3d
(
DPoint3dP pCrossProduct,
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
);

//!
//! @description Return the (scalar) cross product of the xy parts of two vectors.
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The 2d cross product.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_crossProductXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Compute the normalized cross product of two vectors
//! and return the length of the unnormalized cross product.
//!
//! @param pCrossProduct OUT     normalized (unit) vector in the direction of the
//!           cross product of vector1 and vector2
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The length of the original (prenormalization) cross product vector
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_normalizedCrossProduct
(
DPoint3dP pCrossProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Computes the cross product of the two parameter vectors and scales it to a given
//! length.  The scaled vector is stored as the product vector, and the length of the original
//! cross product vector is returned.
//!
//! @param pProduct OUT     vector of given length in direction of the cross product of 2 vectors.
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @param productLength IN      The Desired length
//! @return The length of original vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_sizedCrossProduct
(
DPoint3dP   pProduct,
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      productLength
);

//!
//! @description Computes the cross product of two vectors and scales it to the
//! geometric mean of the lengths of the two vectors.  This is useful
//! because it has the direction of the cross product (i.e. normal to the plane
//! of the two vectors) and a size in between the two vectors.
//!
//! @param pProduct OUT     cross product vector
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The length of original vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_geometricMeanCrossProduct
(
DPoint3dP pProduct,
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Returns the (scalar) dot product of two vectors.
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The dot product of the two vectors
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProduct
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Returns the (scalar) dot product of xy parts of two vectors.
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The dot product of the xy parts of the two vectors
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProductXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Computes the dot product of one vector given as a point structure and another given as
//! xyz components.
//! @param pVector IN      The first vector.
//! @param ax IN      The x component of second vector.
//! @param ay IN      The y component of second vector.
//! @param az IN      The z component of second vector.
//! @return The dot product of the vector with a vector with the given components
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_dotXYZ
(
DPoint3dCP  pVector,
double      ax,
double      ay,
double      az
);

//!
//! @description Sets three vectors so that they are mutually
//! perpendicular, the third (Z) vector is identical to the
//! given axis vector, and all have the same length.
//! If the given axis vector contains only zeros, a (0,0,1) vector
//!   is used instead.
//!
//! @param pGivenAxis IN      input z direction vector
//! @param pXAxis OUT     x direction of the coordinate system
//! @param pYAxis OUT     y direction of the coordinate system
//! @param pZAxis OUT     z direction of the coordinate system
//! @return true unless given vector is z zero vector.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_getTriad
(
DPoint3dCP pGivenAxis,
DPoint3dP pXAxis,
DPoint3dP pYAxis,
DPoint3dP pZAxis
);

//!
//! @description Sets three vectors so that they are mutually
//! perpendicular unit vectors with the  third (Z) vector in the
//! direction of the given axis vector.
//! If the given axis vector contains only zeros, a (0,0,1) vector
//!   is used instead.
//!
//! @param pGivenAxis   IN      input z direction vector
//! @param    pXAxis      OUT     unit x direction vector
//! @param    pYAxis      OUT     unit y direction vector
//! @param    pZAxis      OUT     unit z direction vector
//! @return true unless given vector has zero length.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_getNormalizedTriad
(
DPoint3dCP pGivenAxis,
DPoint3dP pXAxis,
DPoint3dP pYAxis,
DPoint3dP pZAxis
);

//!
//! @description Apply a Givens to the two points, as if they are columns of a matrix to be postmultiplied
//! by the Givens matrx.
//! The Givens matrix is
//!       R(0,0)=R(1,1)=c
//!       R(0,1)=-s
//!       R(1,0)=s
//!
//! @param    pOut0   OUT     rotation of pIn0
//! @param    pOut1   OUT     rotation of pIn1
//! @param    pIn0    IN      first vector
//! @param    pIn1    IN      second vector
//! @param    c   IN      cosine of givens rotation.
//! @param    s   IN      sine of givens rotation.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_applyGivensRotation
(
DPoint3dP   pOut0,
DPoint3dP   pOut1,
DPoint3dCP  pIn0,
DPoint3dCP  pIn1,
double      c,
double      s
);

//!
//! @description Apply a hyperbolic rotation to the two points, as if they are columns of a matrix to be postmultiplied
//! by the hyperbolic matrix
//! The hyperbolic matrix is
//!       R(0,0)=R(1,1)=secant
//!       R(0,1)=R(1,0)= tangent
//!
//! @param    pOut0   OUT     rotation of pIn0
//! @param    pOut1   OUT     rotation of pIn1
//! @param    pIn0    IN      first vector
//! @param    pIn1    IN      second vector
//! @param    secant  IN      secant (diagonal) part of the hyperbolic transformation
//! @param    tangent IN      tangent (offdiagonal) part of the hyperbolic transformation
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_applyHyperbolicReflection
(
DPoint3dP   pOut0,
DPoint3dP   pOut1,
DPoint3dCP  pIn0,
DPoint3dCP  pIn1,
double      secant,
double      tangent
);

//!
//! @description Computes a unit vector  in the direction of the difference of the points
//! or vectors (Second parameter vector is subtracted from the first parameter vector,
//! exactly as in the subtract function.)
//!
//! @param pVector OUT     The normalized vector in direction (pTarget - pOrigin)
//! @param pTarget IN      The target point.
//! @param pOrigin IN      The origin point.
//! @return The length of original difference vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_computeNormal
(
DPoint3dP pVector,
DPoint3dCP pTarget,
DPoint3dCP pOrigin
);

//!
//! @description Returns the angle between two vectors.  This angle is between 0 and
//! pi.  Rotating the first vector by this angle around the cross product
//! between the vectors aligns it with the second vector.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The angle between the vectors.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_angleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Returns the angle between two vectors, choosing the smaller
//!   of the two possible angles when both the vectors and their negations are considered.
//!    This angle is between 0 and pi/2.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The angle between the vectors.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_smallerAngleBetweenUnorientedVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

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
//! @param pTestVector IN      The vector to test
//! @param pVector0 IN      The first boundary vector.
//! @param pVector1 IN      The second boundary vector.
//! @return true if the test vector is within the angle.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInSmallerSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1
);

//!
//! @description Test if the test vector vector is "between" vector0 and vector1, with CCW direction resolved by an up vector.
//! @remarks The cross product of vector0 and vector1 is considered the positive plane normal if its dot product with the up vector is positive.
//! @remarks The containment test is strict in the sense that if the test vector equals a boundary vector, this function returns false.
//! @param pTestVector IN      The vector to test
//! @param pVector0 IN      The boundary vector
//! @param pVector1 IN      The boundary vector
//! @param pUpVector IN      The out of plane vector.  If null, the z-axis vector (001) is used.
//! @return true if test vector is within the angle.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInCCWSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1,
DPoint3dCP pUpVector
);

//!
//! @description Test if the test vector vector is "between" vector0 and vector1, in CCW sense using only xy components of the 3 vectors.
//! @remarks The containment test is strict in the sense that if the xy-components of the test vector equal those of a boundary vector, this function returns false.
//! @param pTestVector IN      The vector to test
//! @param pVector0 IN      The boundary vector
//! @param pVector1 IN      The boundary vector
//! @return true if test vector is within the angle.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isVectorInCCWXYSector
(
DPoint3dCP pTestVector,
DPoint3dCP pVector0,
DPoint3dCP pVector1
);

//!
//! @description Returns the angle from Vector1 to Vector2 using only xy parts.
//!  This angle is between -pi and +pi.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The angle between vectors.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_angleBetweenVectorsXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Returns the angle between two vectors, considering both
//!   the vectors and their negations and choosing the smaller.
//!   This angle is between 0 and pi/2.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return The angle between vectors.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_smallerAngleBetweenUnorientedVectorsXY
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Rotate a vector around the z axis.
//! @param pRotatedVector OUT     rotated vector
//! @param theta   IN      The rotation angle.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_rotateXY
(
DPoint3dP   pRotatedVector,
DPoint3dCP  pVector,
double      theta
);

//!
//! @description Rotate a vector around the z axis.
//! @param pVector IN OUT  rotated vector
//! @param theta   IN      The rotation angle.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_rotateXYInPlace
(
DPoint3dP   pVector,
double      theta
);

//!
//! @description Computes the signed from one vector to another, in the plane
//!       of the two vectors.   Initial computation using only the two vectors
//!       yields two possible angles depending on which side of the plane of the
//!       vectors is viewed.  To choose which side to view, go on the side whose
//!       normal has a positive dot product with the orientation vector.
//! This angle can be between -pi and +pi.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @param pOrientationVector IN      The vector used to determine orientation.
//! @return The signed angle
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_signedAngleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pOrientationVector
);

//!
//! @description Computes the signed angle between the projection of two vectors
//!       onto a plane with given normal.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @param pNormal IN      The plane normal vector
//! @return The angle in plane
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_planarAngleBetweenVectors
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pPlaneNormal
);

//!
//! @description Scale each point by the other's weight and subtract, i.e. form
//! (point1 * point2.w - point2 * point1.w).  The weight term
//! vanishes.   Copy the xyz parts back as a vector.
//!
//! @param pDifference OUT     The difference vector
//! @param pPoint1 IN      The first point
//! @param pTarget2 IN      The second pont.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_weightedDifference
(
DPoint3dP   pDifference,
DPoint4dCP  pPoint1,
DPoint4dCP  pPoint2
);

//!
//! @description Form the cross product of the weighted differences from point0 to point1 and point2.
//!
//! @param pProduct OUT     cross product result
//! @param pBasePoint IN      The common base point (second point for differences)
//! @param pTarget1 IN      The first target point.
//! @param pTarget2 IN      The second target point.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_weightedDifferenceCrossProduct
(
DPoint3dP   pProduct,
DPoint4dCP  pBasePoint,
DPoint4dCP  pTarget1,
DPoint4dCP  pTarget2
);

//!
//! @description Computes the squared magnitude of a vector.
//!
//! @param pVector IN      The vector whose magnitude is computed.
//! @return The squared magnitude of the vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeSquared (DPoint3dCP pVector);

//!
//! @description Computes the magnitude of the xy part of a vector.
//! @param pVector IN      The vector
//! @return The magnitude of the xy parts of the given vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeXY (DPoint3dCP pVector);

//!
//! @description Computes the squared magnitude of the xy part of a vector.
//! @param pVector IN      The vector
//! @return The squared magnitude of the xy parts of the given vector.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitudeSquaredXY (DPoint3dCP pVector);

//!
//! @description Compute a unit vector perpendicular to the xy parts of given vector.
//!
//! @remarks Input may equal output.
//!
//! @param pRotatedVector OUT     The rotated and scaled vector.  Z-coordinate is zero.
//! @param pVector        IN      The source vector
//! @return true if the input vector has nonzero length
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_unitPerpendicularXY
(
DPoint3dP    pRotatedVector,
DPoint3dCP    pVector
);

//!
//! @description Compute the unit vector perpendicular to the xy parts of the given vector with the given handedness.
//!
//! @remarks Input may equal output.
//!
//! @param pRotatedVector OUT     The rotated and scaled vector.  Z-coordinate is zero.
//! @param pVector        IN      The source vector
//! @param bRightHanded   IN      Whether the returned vector points to the right of the xy parts of the given vector.
//! @return true if the input vector has nonzero length
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_unitPerpendicularXYWithHandedness
(
DPoint3dP   pRotatedVector,
DPoint3dCP  pVector,
bool        bRightHanded
);

//!
//! @description Computes the magnitude of a vector.
//! @param pVector IN      The vector
//! @return The length of the vector
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitude (DPoint3dCP pVector);

//!
//! Determines within angular tolerance if normals are parallel and pointing
//! in same direction.  Input -1 for dot0, dot1 to compute them in this function.
//!
//! @param pNormal0   IN      first normal
//! @param dot0       IN      squared magnitude of first normal (or -1)
//! @param pNormal1   IN      second normal
//! @param dot1       IN      squared magnitude of second normal (or -1)
//! @param eps2       IN      max sin^2(angle) for parallel normals
//! @return true if normals are same within tolerance
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_normalEqualTolerance
(
DPoint3dCP  pNormal0,
double      dot0,
DPoint3dCP  pNormal1,
double      dot1,
double      eps2
);

//!
//! @description Multiplies a vector by a scale factor.
//! @param pScaledVector OUT     The scaled vector.
//! @param pVector IN      The vector to be scaled.
//! @param scale IN      The scale factor.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_scale
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      scale
);

//!
//! @description Multiplies a vector (in place) by a scale factor.
//! @param pVector IN OUT  scaled vector
//! @param scale IN      The scale
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_scaleInPlace
(
DPoint3dP   pVector,
double      scale
);

//!
//! @description Computes a negated (opposite) vector.
//!
//! @param pNegated OUT     The negated vector.
//! @param pVector IN      The vector to be negated.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_negate
(
DPoint3dP pNegatedVector,
DPoint3dCP pVector
);

//!
//! @description Negate a vector in place.
//!
//! @param pVector IN OUT  negated vector
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_negateInPlace (DPoint3dP pVector);

//!
//! @description Normalizes (scales) a vector to length 1.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param pUnitVector OUT     The normalized vector.
//! @param pVector IN      The vector to be normalized.
//! @return The length prior to normalization
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_normalize
(
DPoint3dP pUnitVector,
DPoint3dCP pVector
);

//!
//! @description Scales a vector to specified length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param pScaledVector OUT     The scaled vector.
//! @param    pVector IN      The original vector.
//! @param    length IN      The requested length.
//! @return The length prior to scaling.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_scaleToLength
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      length
);

//!
//! @description Scales a vector to a specified length, and returns
//! the prior length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param pScaledVector OUT     scaled
//! @param    length IN      The requested length
//! @return The length prior to scaling.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_scaleToLengthInPlace
(
DPoint3dP   pScaledVector,
double      length
);

//!
//! @description Replaces a vector by a unit vector in the same direction, and returns
//! the original length.
//! If the input vector length is 0, the output vector is a zero vector
//! and the returned length is 0.
//!
//! @param pVector IN OUT  vector to be normalized
//! @return The length prior to normalization
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_normalizeInPlace (DPoint3dP pVector);

//!
//! @description Computes a unit vector in the direction of a given vector.
//! If the input vector length is 0, the output vector is a zero vector.
//!
//! @param pUnitVector OUT     The normalized vector.
//! @param pVector IN      The vector to normalize.
//!
Public GEOMDLLIMPEXP void     bsiDPoint3d_unitVector
(
DPoint3dP pUnitVector,
DPoint3dCP pVector
);

//!
//! @description Tests if two vectors are parallel.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return true if the vectors are parallel within tolerance
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_areParallel
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Tests if two vectors are perpendicular.
//!
//! @param pVector1 IN      The first vector
//! @param pVector2 IN      The second vector
//! @return true if vectors are perpendicular within tolerance
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_arePerpendicular
(
DPoint3dCP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Tests if two vectors are parallel.
//! @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDPoint3d_areParallel.
//!
//! @param pVector1   IN      the first vector
//! @param pVector2   IN      the second vector
//! @param tolerance  IN      radian tolerance for angle between vectors
//! @return true if the vectors are parallel within tolerance
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_areParallelTolerance
(
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      tolerance
);

//!
//! @description Tests if two vectors are perpendicular.
//! @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDPoint3d_arePerpendicular.
//!
//! @param pVector1   IN      the first vector
//! @param pVector2   IN      the second vector
//! @param tolerance  IN      radian tolerance for angle between vectors
//! @return true if the vectors are perpendicular within tolerance
//!
Public GEOMDLLIMPEXP bool     bsiDPoint3d_arePerpendicularTolerance
(
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      tolerance
);

//!
//! @description Try to divide each component of a vector by a scalar.  If the denominator
//! near zero compared to any numerator component, return the original
//! vector.
//! @param pScaledVector OUT     The vector after scaling.
//! @param pVector IN      The initial vector.
//! @param denominator IN      The divisor.
//! @return true if division is numerically safe.
//!
Public GEOMDLLIMPEXP bool    bsiDPoint3d_safeDivide
(
DPoint3dP   pScaledVector,
DPoint3dCP  pVector,
double      denominator
);

//!
//! @description Return the angle component of the polar coordinates
//!           form of a vector.  Z part of the vector is ignored.
//! @param pVector IN      vector in xyz form.
//! @return angle from x axis to this vector, using only xy parts.
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_polarAngle (DPoint3dCP pVector);

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
//! @param pVector1 IN      The first vector.
//! @param pVector2 IN      The second vector.
//! @param pVector3 IN      The third vector.
//! @return The triple product
//!
Public GEOMDLLIMPEXP double bsiDPoint3d_tripleProduct
(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
DPoint3dCP pVector3
);

//!
//! @description Subtract two points or vectors, and return the result in
//!           place of the first.
//!
//! @param pVector2 IN OUT  The point or vector to be modified.
//! @param pVector2 IN      The vector to subtract.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_subtract
(
DPoint3dP pVector1,
DPoint3dCP pVector2
);

//!
//! @description Subtract coordinates of two vectors or points. (Compute Point1 - Point2)
//!
//! @param pVector OUT     The difference vector
//! @param pPoint1 IN      The first point
//! @param pPoint2 IN      The second (subtracted) point.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dDPoint3d
(
DPoint3dP pVector,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
);

//!
//! @description Adds an origin and a scaled vector.
//!
//! @param pSum OUT     The sum
//! @param pOrigin IN      Origin for the sum.
//! @param pVector IN      The vector to be added.
//! @param scale IN      The scale factor.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector,
double      scale
);

//!
//! @description Adds an origin and two scaled vectors.
//!
//! @param pSum OUT     sum
//! @param pOrigin IN      The origin.  May be null.
//! @param pVector1 IN      The first direction vector
//! @param scale1 IN      The first scale factor
//! @param pVector2 IN      The second direction vector
//! @param scale2 IN      The second scale factor
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_add2ScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector1,
double      scale1,
DPoint3dCP  pVector2,
double      scale2
);

//!
//! @description Adds an origin and three scaled vectors.
//!
//! @param pSum OUT     The sum.
//! @param pOrigin IN      The origin. May be null
//! @param pVector1 IN      The first direction vector
//! @param scale1 IN      The first scale factor
//! @param pVector2 IN      The second direction vector
//! @param scale2 IN      The second scale factor
//! @param pVector3 IN      The third direction vector
//! @param scale3 IN      The third scale factor
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_add3ScaledDPoint3d
(
DPoint3dP   pSum,
DPoint3dCP  pOrigin,
DPoint3dCP  pVector1,
double      scale1,
DPoint3dCP  pVector2,
double      scale2,
DPoint3dCP  pVector3,
double      scale3
);

//!
//! @description Adds a vector to a pointer or vector, returns the result in place.
//!
//! @param pSum IN OUT  The point or vector to be modified.
//! @param pVector IN      The vector to add.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dInPlace
(
DPoint3dP pSum,
DPoint3dCP pVector
);

//!
//! @description Compute the sum of two points or vectors.
//!
//! @param pSum OUT     The computed sum.
//! @param pPoint1 IN      The the first point or vector
//! @param pPoint2 IN      The second point or vector
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dDPoint3d
(
DPoint3dP pSum,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
);

END_BENTLEY_NAMESPACE

