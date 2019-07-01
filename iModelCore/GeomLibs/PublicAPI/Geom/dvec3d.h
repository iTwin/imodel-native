/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once



/*__PUBLISH_SECTION_START__*/

//! @file Dvec3d.h A vector in xyz space: Bentley::DVec3d
#ifndef dvec3d_H_
#define dvec3d_H_

BEGIN_BENTLEY_NAMESPACE
//!
//! Vector with x,y,z components.
//!
//!
//! <table border="1" cellpadding="5" cellspacing="0">
//! <caption ALIGN="TOP">>DVec3d Usage Summary
//! </caption>
//! <tr><th>Method Invocation</th><th>Remarks</th></tr>
//!
//! <tr><td>W.crossProduct(U,V)</td>    <td>W = U cross V</td></tr>
//! <tr><td>W.crossProductToPoints (A,B,C)</td>    <td>W = (B-A) cross (C-A)</td></tr>
//! <tr><td>W.normalizedCrossProduct (U,V)</td>    <td>W = normalize (U cross V)</td></tr>
//! <tr><td>W.sizedCrossProduct (U,V, a)</td>    <td>W = a * normalize (U cross V)</td></tr>
//! <tr><td>W.geometricMeanCrossProduct (U,V)</td>    <td>W = sqrt (mag(U) * mag(V)) * normalize(U cross V)</td></tr>
//!
//! <tr><td>a = crossProductXY (U,V)</td>    <td>a = U cross V (xy only, scalar result)</td></tr>
//! <tr><td>a = tripleProduct (U,V,W)va = U cross V dot W</td></tr>
//! <tr><td>a = tripleProductToPoints (A,B,C,D)</td>    <td>a = (B-A) cross (C-A) dot (D-A)</td></tr>
//! <tr><td>a = dotProduct (U,V)</td>    <td>a = U dot V</td></tr>
//! <tr><td>a = dotProductXY (U,V)</td>    <td>a = U dot V, using only x and y parts</td></tr>
//!
//! <tr><td>U.getTriad (&Vx ,&Vy, &Vz)</td>    <td>return Vz=U, Vx,Vy perpenidcular, same length</td></tr>
//! <tr><td>U.getNormalizedTriad (&Vx, &Vy, &Vz)</td>    <td>return Vz=normalize(U), Vx,Vy perpenidcular, all unit</td></tr>
//!
//! <tr><td>U.normalizedDifference (A,B)</td>    <td>U = normalize (A-B)</td></tr>
//!
//! <tr><td>a = U.angleTo (V)</td>             <td>Spatial angle from U to V.  Range 0 to PI.</td></tr>
//! <tr><td>a = U.angleToXY (V)</td>             <td></td></tr>
//! <tr><td>a = U.smallerUnorientedAngleTo (V)</td>             <td></td></tr>
//! <tr><td>a = U.smallerUnorientedAngleToXY (V)</td>             <td></td></tr>
//! <tr><td>a = U.signedAngleTo (V, upVector)</td>             <td></td></tr>
//!
//! <tr><td>U.isVectorInSmallerSector(V,W)</td>             <td>Test if U appears in the smaller sector between V and W, when viewed with V cross W as up vector.</td></tr>
//! <tr><td>U.isVectorInCCWSector (V,W, upVector))</td>             <td>Test if U appears in the sector from V counterclockwise towards W when viewed with upVector</td></tr>
//! <tr><td>U.planarAngleTo (V,W)</td>             <td>Anlge after projection to plane with nomral W. </td></tr>
//! <tr><td>U.rotateXY (V, theta)</td>             <td>rotate around z axis</td></tr>
//! <tr><td>U.rotateXY (theta)</td>             <td>Inplace rotation around z</td></tr>
//!
//! <tr><td>U.unitPerpendicularXY (V)             <td></td></tr></tr>
//! <tr><td>U.scale (V,a)</td>    <td>U = V * a</tr>
//! <tr><td>U.scale (a)</td>    <td>U *= a</td></tr>
//! <tr><td>U.negate ()</td>    <td>U *= -1</td></tr>
//! <tr><td>U.negate(V)</td>    <td>U = - V</td></tr>
//! <tr><td>U.normalize (V)</td>    <td>U = V / V.magnitude ()</td></tr>
//! <tr><td>U.normalize ()</td>    <td>U /= U.magnitude ()</td></tr>
//! <tr><td>U.scaleToLength (V,a)</td>    <td>U = V * a / V.magnitude ()</td></tr>
//!
//! <tr><td>U.isParallelTo(V)</td>             <td></td></tr>
//! <tr><td>U.isPerpendicularTo(V)</td>             <td></td></tr>
//! <tr><td>if (U.safeDivide) ... (V,a)</td>             <td>Divde by a, but return false if a is too close to zero</td></tr>
//!
//! <tr><td>U.zero()</td>             <td>Sets all components to 0</td></tr>
//! <tr><td>U.one()</td>             <td>Sets all components to 1</td></tr>
//! <tr><td>U.init (x,y,z)</td>             <td></td></tr>
//! <tr><td>U.init (A)</td>             <td>copy from point</td></tr>
//! <tr><td>U.setComponent (a, i)</td>             <td>Set individual components</td></tr>
//! <tr><td>a = U.getComponent (i)</td>             <td>Get individual component</td></tr>
//! <tr><td>U.getComponents (U, &a, &b, &c)</td>             <td>Get all components as separate doubles</td></tr>
//!
//! <tr><td>U.sumOf (V,W)</td>    <td>U = V + W</td></tr>
//! <tr><td>U.add (V)</td>    <td>U += V</td></tr>
//! <tr><td>U.subtract (V)</td>    <td>U -= V</td></tr>
//! <tr><td>U.sumOf (V, W, a)</td>    <td>U = V + a W</td></tr>
//! <tr><td>U.interpolate (V, a, W)</td>    <td>U = (1-a)V * a W</td></tr>
//! <tr><td>W.sumOf (U, V1, a1, V2, a2)</td>    <td>W = U + a1 * V1 + a2 * V2 + a3 * V3</td></tr>
//! <tr><td>W.sumOf (U, V1, a1, V2, a2, V2, a3)</td>    <td>W = U + a1 * V1 + a2 * V2 + a3 * V3</td></tr>
//! <tr><td>W.differenceOf (U,V)</td>    <td>W = U - V</td></tr>
//! <tr><td>W.differenceOf (A,B)</td>    <td>W = A - B</td></tr>
//!
//! <tr><td>a = U.distance (V)</td>    <td>W = (V-U).magnitude ()</td></tr>
//! <tr><td>a = U.distanceSquared(V)</td>             <td></td></tr>
//! <tr><td>a = U.distanceXY (V)</td>             <td></td></tr>
//! <tr><td>a = U.distanceSquaredXY (V)</td>             <td></td></tr>
//!
//! <tr><td>a = U.magnitudeSquared ()</td>             <td></td></tr>
//! <tr><td>a = U.magnitudeSquaredXY ()</td>             <td></td></tr>
//! <tr><td>a = U.magnitude ()</td>             <td></td></tr>
//! <tr><td>a = U.magnitudeXY ()</td>             <td></td></tr>
//!
//! <tr><td>a = U.maxAbs()</td>             <td></td></tr>
//!
//! <tr><td>U.isEqual(V)</td>    <td>Bitwise equality</td>             <td></td></tr>
//! <tr><td>U.isEqual (V, tol)</td>    <td>componentwise absolute difference</td>             <td></td></tr>
//!
//!
//! <tr><td>U.multiply (M,V)</td>    <td>U = M * V<</td></tr>
//! <tr><td>mutliplyArray (U[], M, V[], count)</td></tr>
//! <tr><td>U.multiply (M, x, y, z)</td>    <td>U = M * <x,y,z></td></tr>
//! <tr><td>U.multiplyTranspose (M, V)</td>    <td>U = M^ * V</td></tr>
//! <tr><td>U.multiplyTranspose (M, x, y, z)</td>    <td>U = M' * <x,y,z></td></tr>
//! <tr><td>Dvec3d::multiplyTransposeArray (U[], M, V[], count)</td></tr>
//! </table>
//!
//! @ingroup BentleyGeom_PointsAndVectors
#ifdef __cplusplus
struct GEOMDLLIMPEXP DVec3d : public DPoint3d
{
//flex!! Construction and initialization
//flex|| computed matrix || result returned from static method || result placed in instance.  No use of prior instance contents. || modify instance in place ||

//flex|| by component  || vector = DVec3d::From (ax, ay, az)  || vector.Init (ax, ay, az) ||
//flex||               || vector = DVec3d::From (ax, ay)      || vector.Init (ax, ay) ||
//flex|| from array of doubles || vector = DVec3d::FromArray (xyz[])) || vector.InitFromArray (xyz[]) ||

//! @description Returns a DVec3d with 3 component array of double.
//! @param [in] pXyz x, y, z components
static DVec3d FromArray (const   double  *pXyz);

//! @description unit X vector
static DVec3d UnitX (){return DVec3d::From (1,0,0);}
//! @description unit Y vector
static DVec3d UnitY() { return DVec3d::From(0, 1, 0);}
//! @description unit Z vector
static DVec3d UnitZ() { return DVec3d::From (0, 0, 1);}

//! @description vector of zeros
static DVec3d FromZero() { return DVec3d::From (0, 0, 0);}

//! @description Copies doubles from a 3 component array to the x,y, and z components
//! of a DVec3d
//! @param [in] pXyz x, y, z components
void InitFromArray (double const* pXyz);

//! @description Returns a DVec3d with given  x,y, and z components of a vector
//! @param [in] ax The x component.
//! @param [in] ay The y component.
//! @param [in] az The z component.
static DVec3d From (double ax, double ay, double az = 0.0);

//! @description Sets the x,y, and z components of a vector
//! @param [in] ax The x component.
//! @param [in] ay The y component.
//! @param [in] az The z component.
void Init (double ax, double ay, double az);
//! @description Sets the x, and y components of a vector. Sets z to zero.
//! @param [in] ax The x component.
//! @param [in] ax The x component.
//! @param [in] ay The y component
void Init (double ax, double  ay);

//flex|| copy components from point   || vector = DVec3d::From (point)   || vector.InitFrom (point) ||

//! @description Returns a DVec3d from a point (treating the point as a vector from its origin).
//! @param [in] point the point.
static DVec3d From (DPoint3dCR point);

//! @description Returns a DVec3d from a 2d vector (with 0 z)
//! @param [in] vector the 2d vector
static DVec3d From (DVec2dCR vector);

//! @description Initialize a vector from a point (treating the point as a vector from its origin).
//! @param [in] point the point.
void Init (DPoint3dCR point);


//flex|| vector from start point to target point   || vector = DVec3d::FromStartEnd (startPoint, endPoint)    ||
//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static DVec3d FromStartEnd (DPoint3dCR start, DPoint3dCR end);

//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static DVec3d FromStartEnd (FPoint3dCR start, FPoint3dCR end);

//! promote FVec3d components to double
static DVec3d From (FVec3dCR);

//! @description Returns the unnormalized (full length) DVec3d from the frame origin to the target point.
//! @param [in] start start point
//! @param [in] target   end point
static DVec3d FromStartEnd (TransformCR start, DPoint3dCR target);

//flex||  normalized vector || vector = DVec3d::FromStartEndNormalize (startPoint, endPoint)    || outVector.Normalize (vector) || inoutVector.Normalize () ||


//! @description Returns a vector from start towards end, normalized if nonzero.
//! @param [in] start start point
//! @param [in] end   end point
static DVec3d FromStartEndNormalize (DPoint3dCR start, DPoint3dCR end);

//! @description return a vector same length as source but rotate 90 degrees CCW
static DVec3d FromCCWPerpendicularXY (DVec3d source);

//! @description return a vector same length as source but rotate 90 degrees towards target
static DVec3d FromRotate90Towards (DVec3dCR source, DVec3dCR target);

//! @description return a vector same length as source but rotate 90 degrees around axis
static DVec3d FromRotate90Around (DVec3dCR source, DVec3dCR axis);

//! @description return a vector same length as source but rotated by angle around axis. (Rodriguez formula)
static ValidatedDVec3d FromRotateVectorAroundVector (DVec3dCR source, DVec3dCR axis, Angle angle);


//!
//! @description Normalizes (scales) a vector to length 1.
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//! @param [in] vector The vector to be normalized.
//! @return The length prior to normalization
//!
double Normalize (DVec3dCR vector);

//!
//! @description initialize this vector to unit vector in the direction of source.
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//! @param [in] source The vector to be normalized.
//! @param [out] magnitude The orignal magnitude of the source vector;
//! @return true if the other vector length is large enough for DoubleOps::SafeDivide to compute 1/length.
bool TryNormalize (DVec3dCR source, double &magnitude);

//! @description return a (possibly invalid) unit vector in the direction of the calling instance.
ValidatedDVec3d ValidatedNormalize () const;
//!
//! @description Replaces a vector by a unit vector in the same direction, and returns
//! the original length.
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//! @return The length prior to normalization
//!
double Normalize ();

//flex|| Impose length ||     || outVector.ScaleToLength (length)  || inOutVector.ScaleToLength (length) ||

//! @description Scales a vector to specified length.
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//!
//! @param [in] vector The original vector.
//! @param [in] length The requested length.
//! @return The length prior to scaling.
//!
double ScaleToLength (DVec3dCR vector, double length);

//!
//! @description Scales a vector to a specified length, and returns
//! the prior length.
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//! @param [in] length The requested length
//! @return The length prior to scaling.
//!
double ScaleToLength (double length);


//flex|| target-origin || || vector.NormalizedDifference (target, origin) ||

//! @description Computes a unit vector  in the direction of the difference of the vectors
//! or vectors (Second parameter vector is subtracted from the first parameter vector,
//! exactly as in the subtract function.)
//! @remark In the 0-length error case, the vector is set to (1,0,0) in the legacy microstation style.
//! @param [in] target The target point.
//! @param [in] origin The origin point.
//! @return The length of original difference vector.
//!
double NormalizedDifference
(
DPoint3dCR      target,
DPoint3dCR      origin
);

//flex|| vectorB-vectorA || || vector.DifferenceOf (vectorB, vectorA) || inOutVectorB.Subtract (vectorA) ||
//flex|| pointB-pointA   || || vector.DifferenceOf (pointB, pointA) ||

//! @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second (subtracted) vector
void DifferenceOf  (DVec3dCR vector1, DVec3dCR vector2);

//! @description Subtract coordinates of two points. (Compute Point1 - Point2)
//! @param [in] target The target point
//! @param [in] base The base point
void DifferenceOf  (DPoint3dCR target, DPoint3dCR base);

//! @description Subtract coordinates of two points. (Compute Point1 - Point2)
//! @param [in] target The target point
//! @param [in] base The base point
void DifferenceOf  (FPoint3dCR target, FPoint3dCR base);

//! @description Subtract two vectors, and return the result in place of the first.
//! @param [in] vector2 The vector to subtract.
void Subtract (DVec3dCR vector2);


//flex|| polar construction || vector = DVec3d::FromXYAngleAndMagnitude (radians, magnitude) || vector.InitFromXYAngleAndMagnitude (radians, magntiude) ||
//! @description Returns a DVec3d from given angle and distance in xy plane.
//!       Z part is set to zero.
//! @param [in] theta Angle from X axis to the vector, in the xy plane.
//! @param [in] magnitude Vector magnitude
static DVec3d FromXYAngleAndMagnitude
(
double          theta,
double          magnitude
);


//! @description Sets a vector from given angle and distance in xy plane.
//!       Z part is set to zero.
//! @param [in] theta Angle from X axis to the vector, in the xy plane.
//! @param [in] magnitude Vector magnitude
void InitFromXYAngleAndMagnitude
(
double          theta,
double          magnitude
);








//flex|| Translation part of transform || vector = DVec3d::FromTranslation (transform) ||
//flex|| column of matrix part of transform || vector = DVec3d::FromMatrixColumn (transform, columnIndex) ||
//flex|| row of matrix part of transform || vector = DVec3d::FromMatrixRow (transform, columnIndex) ||
//flex|| column of matrix || vector = DVec3d::FromColumn (transform, columnIndex) || vector.InitFromColumn (matrix, columnIndex) ||
//flex|| row of matrix || vector = DVec3d::FromRow (transform, rowIndex) || vector.InitFromRow (matrix, columnIndex) ||

//! @description return the translation xyz as a DVec3d (which you can pass as a DPoint3d where needed.)
//! @param [in] source source transform
static DVec3d FromTranslation (TransformCR source);

//! @description return a (cyclically indexed) column of the matrix part of a transform.
//! @param [in] transform source transform
//! @param [in] i column index
static DVec3d FromMatrixColumn (TransformCR transform, int i);

//! @description return a (cyclically indexed) row of the matrix part of a transform.
//! @param [in] transform source transform
//! @param [in] i row index
static DVec3d FromMatrixRow (TransformCR transform, int i);


//! @description return a (cyclically indexed) column of the matrix.
//! @param [in] matrix source
//! @param [in] i column index
static DVec3d FromColumn (RotMatrixCR matrix, int i);
//! @description Extracts a column of a matrix.
//! @param [in] matrix The matrix.
//! @param [in] col column index. Columns are numbered 0,1,2.  Others
//!        indices are reduced cyclically.
void InitFromColumn
(
RotMatrixCR     matrix,
int             col
);

//! @description return a (cyclically indexed) row of the matrix.
//! @param [in] matrix source
//! @param [in] i row index
static DVec3d FromRow (RotMatrixCR matrix, int i);
//! @description Extracts a row of a matrix.
//! @param [in] matrix The matrix.
//! @param [in] row row index. Rows are numbered 0,1,2.  Others
//!        indices are reduced cyclically.
void InitFromRow  (RotMatrixCR matrix, int row);


//flex|| matrix^Transpose * vectorA   || || vector.Multiply (matrix, vectorA) ||
//flex|| ... component input   || || vector.Multiply (matrix, xA, yA, zA) ||
//flex|| matrix^Transpose * vectorA   || || vector.MultiplyTranpose (matrix, vectorA) ||
//flex|| ... component input   || || vector.MultiplyTranspose (matrix, xA, yA, zA) ||

//! @description Returns the product of a matrix times a vector.
//! @param [in] matrix The matrix.
//! @param [in] vector The known vector.
void Multiply  (RotMatrixCR matrix, DVec3dCR vector);

//! @description Returns the product of a matrix transpose times a vector.
//! @param [in] matrix The the matrix.
//! @param [in] vector The known vector.
void MultiplyTranspose  (RotMatrixCR matrix, DVec3dCR vector);

//! @description Returns the product of a matrix times a vector,
//!           with the vector given as separate components.
//! @param [in] matrix The matrix to apply
//! @param [in] x The x component of input vector
//! @param [in] y The y component of input vector
//! @param [in] z The z component of input vector
void Multiply  (RotMatrixCR matrix, double x, double y, double z);

//! @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
//!           the product vector.
//! @param [in] matrix The matrix to apply
//! @param [in] x The x component
//! @param [in] y The y component
//! @param [in] z The z component
void MultiplyTranspose  (RotMatrixCR matrix, double x, double y, double z);





//flex|| copy xyz parts from 4d (NO DEWEIGHTING) || || vector.XyzOf (point4d) ||
//! @description Sets the x,y, and z components of a DVec3d structure from the
//! corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
//! @param [in] hPoint The homogeneous point
void XyzOf (DPoint4dCR hPoint);

//! @description Returns DVec3d pseudo difference start.w*end - end.w*start
//! @param [in] start start point
//! @param [in] end   end point
static DVec3d FromStartEnd (DPoint4dCR start, DPoint4dCR end);


//flex|| cross product             || vector = DVec3d::FromCrossProduct (vectorA, vectorB) || vector.CrossProduct (vectorA, vectorB) ||
//flex|| ... normalized            || vector = DVec3d::FromNormalizedCrossProduct (vectorA, vectorB) || vector.NormalizedCrossProduct (vectorA, vectorB) ||
//flex|| ... scaled to size            || || vector.SizedCrossProduct (vectorA, vectorB) ||
//flex|| ... scaled to sqrt (vectorA.Magnitude() * vectorB.Magnitude())   || || vector.GeometricMeanCrossProduct (vectorA, vectorB) ||
//flex|| ... by component || vector = DVec3d::FromCrossProduct (xA, yA, zA, xB, yB, zB) ||
//flex|| (targetA-origin) cross (targetB - origin)   || || vector.CrossProductToPoints (origin, targetA, targetB) ||
//flex|| ... among columns of a matrix part of a transform  || vector = DVec3d::FromMatrixColumnCrossProduct (int i, int j) ||
//flex|| ... among rows of a matrix part of a transform  || vector = DVec3d::FromMatrixRowCrossProduct (int i, int j) ||


//! @description return the cross product of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
static DVec3d FromCrossProduct  (DVec3dCR vector0, DVec3dCR vector1);

//! @description return the cross product of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
static DVec3d FromCrossProduct  (FVec3dCR vector0, FVec3dCR vector1);

//! @description Returns the (vector) cross product of two vectors.
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second vector
//!
void CrossProduct  (DVec3dCR vector1, DVec3dCR vector2);

//! @description Returns the (vector) cross product of two vectors.
//! @param [in] vector1 The first vector
//! @param [in] point2 The second vector, given as a point.
//!        The point's xyz are understood to be a vector from the origin.
void CrossProduct  (DVec3dCR vector1, DPoint3dCR point2);

//! @description Returns the (vector) cross product of two vectors.
//! @param [in] point1 The first vector, givenn as a point.
//!        The point's xyz are understood to be a vector from the origin.
//! @param [in] vector2 The second vector.
void CrossProduct (DPoint3dCR point1, DVec3dCR vector2);


//! @description Sets the instance to the (vector) cross product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! @param [in] origin The base point for computing vectors.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
void CrossProductToPoints  (DPoint3dCR origin, DPoint3dCR target1, DPoint3dCR target2);

//! @description Returns the (vector) cross product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! @param [in] origin The base point for computing vectors.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
static DVec3d FromCrossProductToPoints  (DPoint3dCR origin, DPoint3dCR target1, DPoint3dCR target2);

//! @description Returns the (normalized vector) cross product of two vectors.
//!   The vectors are computed from the Origin to Target1 and Target2.
//! If the cross product is zero, a zero vector is returned.
//! @param [in] origin The base point for computing vectors.
//! @param [in] target1 The target point for the first vector.
//! @param [in] target2 The target point for the second vector.
static DVec3d FromNormalizedCrossProductToPoints  (DPoint3dCR origin, DPoint3dCR target1, DPoint3dCR target2);



//! @description return the normalized cross product of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
static DVec3d FromNormalizedCrossProduct  (DVec3dCR vector0, DVec3dCR vector1);


//! @description Compute the normalized cross product of two vectors
//! and return the length of the unnormalized cross product.
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second vector
//! @return The length of the original (prenormalization) cross product vector
double NormalizedCrossProduct (DVec3dCR  vector1, DVec3dCR vector2);

//!
//! @description Computes the cross product of the two parameter vectors and scales it to a given
//! length.  The scaled vector is stored as the product vector, and the length of the original
//! cross product vector is returned.
//!
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second vector
//! @param [in] productLength The Desired length
//! @return The The length of unscaled cross product.
//!
double SizedCrossProduct
(
DVec3dCR        vector1,
DVec3dCR        vector2,
double          productLength
);

//!
//! @description Computes the cross product of two vectors and scales it to the
//! geometric mean of the lengths of the two vectors.  This is useful
//! because it has the direction of the cross product (i.e. normal to the plane
//! of the two vectors) and a size in between the two vectors.
//!
//! @param [in] vector1 The first vector
//! @param [in] vector2 The second vector
//! @return The The length of unscaled cross product.
//!
double GeometricMeanCrossProduct
(
DVec3dCR        vector1,
DVec3dCR        vector2
);


//! @description return the cross product of vectors presented as explicit components
//! @param [in] x0 vector0 x component
//! @param [in] y0 vector0 y component
//! @param [in] z0 vector0 z component
//! @param [in] x1 vector1 x component
//! @param [in] y1 vector1 y component
//! @param [in] z1 vector1 z component
static DVec3d FromCrossProduct  (double x0, double y0, double z0, double x1, double y1, double z1);


//! @description return the cross product of vectors found in columns of (the matrix part of) a transform.
//! @param [in] transform source transform
//! @param [in] i first column index
//! @param [in] j second column index
static DVec3d FromMatrixColumnCrossProduct (TransformCR transform, int i, int j);

//! @description return the cross product of vectors found in columns a matrix
//! @param [in] matrix source transform
//! @param [in] i first column index
//! @param [in] j second column index
static DVec3d FromColumnCrossProduct (RotMatrixCR matrix, int i, int j);


//flex|| Roaated xy parts from vectorA || || vector.RotateXY (vectorA, radians) || inoutVector.RotateXY (radians) ||

//! @description Rotate a vector around the z axis.
//! @param [in] vector vector to rotate.
//! @param [in] theta The rotation angle.
void RotateXY  (DVec3dCR vector, double theta);

//! @description Rotate a vector around the z axis.
//! @param [in] theta The rotation angle.
void RotateXY (double theta);

//flex|| Perpendicular in XY plane || || vector.UnitPerpendicularXY (vectorA) ||
//! @description Compute a unit vector perpendicular to the xy parts of given vector.
//! @param [in] vector The source vector
//! @return true if the input vector has nonzero length
bool UnitPerpendicularXY(DVec3dCR vector);

//! @description Compute a unit vector perpendicular to the xy parts of given vector.
//! @param [in] vector The source vector
//! @return perpendicular vector, marked invalid if unable to divide.
static ValidatedDVec3d FromUnitPerpendicularXY (DVec3dCR vector);

//flex|| interpolation between || vector = DVec3d::FromInterpolate (vectorA, fraction, vectorB) || vector.Interpolate (vectorA, fraction, vectorB) ||
//flex|| bilinear combination || vector = DVec3d::FromInterpolateBilinear (vector00, vector10, vector01, vector11, u, v) ||

//! @description Returns an interpolated vector.
//! @param [in] vector0 vector at fraction 0
//! @param [in] fraction fraction from interpolation.
//! @param [in] vector1 vector at fraction 1
static DVec3d FromInterpolate (DVec3dCR vector0, double fraction, DVec3dCR vector1);
//! @description Computes a vector whose position is given by a fractional argument and two vectors.
//! @param [in] vector0 The vector corresponding to fractionParameter of 0.
//! @param [in] fractionParameter The fractional parametric coordinate.
//!               0.0 is the start of the segment, 1.0 is the end, 0.5 is middle
//! @param [in] vector1 The vector corresponding to fractionParameter of 1.
void Interpolate  (DVec3dCR vector0, double fractionParameter, DVec3dCR vector1);




//! @description Returns a bilinear interpolation from corners (00)(10)(01)(11)
//! @param [in] vector00 vector at (0,0)
//! @param [in] vector10 vector at (1,0)
//! @param [in] vector01 vector at (0,1)
//! @param [in] vector11 vector at (1,1)
//! @param [in] u interpolation fraction for edges (vector00,vector10) and (vector01,vector11)
//! @param [in] v interpolation fraction for edges (vector00,vector10) and (vector10,vector11)
static DVec3d FromInterpolateBilinear (DVec3dCR vector00, DVec3dCR vector10, DVec3dCR vector01, DVec3dCR vector11, double u, double v);

//flex|| vectorA + vectorB || || vector.SumOf (vectorA, vectorB) || inOutVectorA.Add (vectorB) ||
//flex|| vectorA *scaleA +  vectorB * scaleB || vector = DVec3d::FromSumOf (vectorA, scaleA, vectorB, scaleB) || vector.FromSumOf (vectorA, scaleA, vectorB, scaleB) ||
//flex|| vectorA *scaleA +  vectorB * scaleB + vectorC * scaleC || vector = DVec3d::FromSumOf (vectorA, scaleA, vectorB, scaleB, vectorC, scaleC) ||


//!
//! @description Compute the sum of two vectors.
//!
//! @param [in] vector1 The the first vector
//! @param [in] vector2 The second vector
//!
void SumOf  (DVec3dCR vector1, DVec3dCR vector2);

//!
//! @description Adds a vector to a pointer or vector, returns the result in place.
//!
//! @param [in] vector The vector to add.
//!
void Add (DVec3dCR vector);


//! @description Returns a sum of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
static DVec3d FromSumOf (DVec3dCR vector0, DVec3dCR vector1);

//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] scale0  first scale
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
static DVec3d FromSumOf (DVec3dCR vector0, double scale0, DVec3dCR vector1, double scale1);
//! @description Compute the sum of two vectors with scale factors.
//! @param [in] vector1 The the first vector
//! @param [in] scale1   first scale
//! @param [in] vector2 The second vector
//! @param [in] scale2 second scale
//!
void SumOf  (DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2);




//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] scale0  first scale
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
//! @param [in] vector2 third vector
//! @param [in] scale2 third scale
static DVec3d FromSumOf (DVec3dCR vector0, double scale0, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2);
//! @description Compute the sum of three vectors with scale factors.
//! @param [in] vector1 The the first vector
//! @param [in] scale1   first scale
//! @param [in] vector2 The second vector
//! @param [in] scale2 second scale
//! @param [in] vector3 The third vector
//! @param [in] scale3 third scale
void SumOf  (DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2, DVec3dCR vector3, double scale3);

//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
static DVec3d FromSumOf (DVec3dCR vector0, DVec3dCR vector1, double scale1);
//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 second scale
//! @param [in] vector2 third vector
//! @param [in] scale2 third scale
static DVec3d FromSumOf (DVec3dCR vector0, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2);
//! @description Returns a linear combination of vectors
//! @param [in] vector0 first vector
//! @param [in] vector1 second vector
//! @param [in] scale1 scale for vector1
//! @param [in] vector2 third vector
//! @param [in] scale2 scale fro vector2
//! @param [in] vector3 fourth vector
//! @param [in] scale3 scale for vector3
static DVec3d FromSumOf (DVec3dCR vector0, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2, DVec3dCR vector3, double scale3);



//!
//! @description Adds an origin and a scaled vector.
//!
//! @param [in] origin Origin for the sum.
//! @param [in] vector The vector to be added.
//! @param [in] scale The scale factor.
//!
void SumOf  (DVec3dCR origin, DVec3dCR vector, double scale);

//!
//! @description Adds an origin and two scaled vectors.
//!
//! @param [in] origin The origin.
//! @param [in] vector1 The first direction vector
//! @param [in] scale1 The first scale factor
//! @param [in] vector2 The second direction vector
//! @param [in] scale2 The second scale factor
//!
void SumOf  (DVec3dCR origin, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2);

//!
//! @description Adds an origin and three scaled vectors.
//!
//! @param [in] origin The origin.
//! @param [in] vector1 The first direction vector
//! @param [in] scale1 The first scale factor
//! @param [in] vector2 The second direction vector
//! @param [in] scale2 The second scale factor
//! @param [in] vector3 The third direction vector
//! @param [in] scale3 The third scale factor
//!
void SumOf  (DVec3dCR origin, DVec3dCR vector1, double scale1, DVec3dCR vector2, double scale2, DVec3dCR vector3, double scale3);


//flex|| scaled vector  || vector = DVec3d::FromScale (vectorA, scale) || vector.Scale (vectorA, scale) || vector.Scale (scale) ||
//flex|| negated vector || || vector.Negated (vectorA) || vector.Negate () ||

//! @description Returns a scalar multiple of a DVec3d
//! @param [in] vector input vector
//! @param [in] scale scale factor
static DVec3d FromScale (DVec3dCR vector, double scale);

//! @description Multiplies a vector by a scale factor.
//! @param [in] vector The vector to be scaled.
//! @param [in] scale The scale factor.
void Scale (DVec3dCR vector, double scale);

//! @description Multiplies a vector (in place) by a scale factor.
//! @param [in] scale The scale
void Scale (double scale);


 
//flex|| divide by scale ||       || vector.SafeDivide (vectorA, denominator) ||
//! @description Try to divide each component of a vector by a scalar.  If the denominator
//! near zero compared to any numerator component, return the original
//! vector.
//! @param [in] vector The initial vector.
//! @param [in] denominator The divisor.
//! @return true if division is numerically safe.
bool SafeDivide (DVec3dCR vector, double denominator);

//! @description Computes a negated (opposite) vector.
//! @param [in] vector The vector to be negated.
void Negate (DVec3dCR vector);


//! @description Negate a vector in place.
void Negate ();

//flex|| vector "in direction" from pointA to pointB, accounting for weights || || vector.WeightedDifferenceOf (point4dA, point4dB) ||
//flex|| cross product of weighted differences to targets || || vector.WeightedDifferenceCrossProduct (basePoint, target1, target2) ||

//! @description Scale each (homogeneous) point by the other's weight and subtract, i.e. form
//! (point1 * point2.w - point2 * point1.w).  The weight term
//! vanishes.   Copy the xyz parts back as a vector.
//! @param [in] hPoint1 The first homogeneous point
//! @param [in] hPoint2 The second homogeneous point.
void WeightedDifferenceOf  (DPoint4dCR hPoint1, DPoint4dCR hPoint2);

//! @description Form the cross product of the weighted differences from base poitn
//!    to two targets
//! @param [in] basePoint The common base point (second point for differences)
//! @param [in] target1 The first target point.
//! @param [in] target2 The second target point.
void WeightedDifferenceCrossProduct  (DPoint4dCR basePoint, DPoint4dCR target1, DPoint4dCR target2);

//! @description Promote point2 to homogeneous and form  (point1 *point2.w - point2 * point1.w).  The weight term
//! vanishes.   Copy the xyz parts back as a vector.
//! @param [in] hPoint1 The first homogeneous point
//! @param [in] point2 The second point.
static DVec3d FromWeightedDifferenceOf (DPoint4dCR hPoint1, DPoint3dCR point2);
//flex!! Scalar extraction

//flex|| dot product || a = vectorA.DotProduct (vectorB) || a = vectorA.DotProduct (x, y, z) ||
//flex|| ... xy parts only || a = vectorA.DotProductXY (vectorB) ||
//flex|| ... with matrix row || a = vectorA.DotProductRow (matrix, rowIndex) || a = vectorA.DotPrdouctMatrixRow (transform, rowIndex) ||
//flex|| ... with matrix column || a = vectorA.DotProductColumn (matrix, columnIndex) || a = vectorA.DotProductMatrixColumn (transform, columnIndex) ||
//flex|| cross product of xy parts || a = vectorA.CrossProductXY (vectorB) ||
//flex|| triple product || a = vectorA.TripleProduct (vectorB, vectorC) ||

//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] vector2 The second vector
//! @return The dot product of the two vectors
double DotProduct (DVec3dCR vector2) const;

//! @description Returns the (scalar) dot product of xy parts of two vectors.
//! @param [in] vector2 The second vector
//! @return The dot product of the xy parts of the two vectors
double DotProductXY (DVec3dCR vector2) const;

//!
//! @description Computes the dot product of one vector given as a vector structure and another given as
//! xyz components.
//! @param [in] ax The x component of second vector.
//! @param [in] ay The y component of second vector.
//! @param [in] az The z component of second vector.
//! @return The dot product of the vector with a vector with the given components
//!
double DotProduct  (double ax, double ay, double az) const;


//! @description Returns the (scalar) dot product of two vectors.
//! @param [in] point2 The second vector, given as point.
//!        The point's xyz are understood to be a vector from the origin.
//! @return dot product of the two vectors.
double DotProduct (DPoint3dCR point2) const;


//! Returns the (scalar) dot product of this vector and a row of a matrix.
//! @param [in] matrix 
//! @param [in] index row index.
//! @return The dot product of the two vectors
double DotProductRow (RotMatrixCR matrix, int index) const;

//! @description Returns the (scalar) dot product of this vector and a column of a matrix.
//! @param [in] matrix 
//! @param [in] index column index.
//! @return The dot product of the two vectors
double DotProductColumn (RotMatrixCR matrix, int index) const;

//! Returns the (scalar) dot product of this vector and a row of the matrix part of the transform.
//! @param [in] matrix 
//! @param [in] index row index.
//! @return The dot product of the two vectors
double DotProductMatrixRow (TransformCR matrix, int index) const;

//! Returns the (scalar) dot product of this vector and a column of the matrix part of the transform.
//! @param [in] matrix 
//! @param [in] index column index.
//! @return The dot product of the two vectors
double DotProductMatrixColumn (TransformCR matrix, int index) const;

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
//! @param [in] vector2 The second vector.
//! @param [in] vector3 The third vector.
//! @return The triple product
double TripleProduct  (DVec3dCR vector2, DVec3dCR vector3) const;

//flex|| Magnitude     || a = vectorA.Magnitude ()        || a = vectorA.MagnitudeSqaured () ||
//flex|| ... xy parts only || a = vectorA.MagnitudeXY ()        || a = vectorA.MagnitudeSqauredXY () ||
//flex|| largest component || a = vectorA.MaxAbs () ||
//flex|| Cross product magnitude || a = vectorA.CrossProductMagnitude (vectorB) ||

//! @description Computes the magnitude of a vector.
//! @return The length of the vector
double Magnitude () const;

//! @description Computes the squared magnitude of a vector.
//! @return The squared magnitude of the vector.
double MagnitudeSquared() const;

//! @description Returns 1 over the squared magnitude, or caller supplied default if squared magnitude is too small.
double SafeOneOverMagnitudeSquared(double defaultValue) const;

//! @description Returns fraction at which other projects onto this, in ValidatedDouble which records if the division was safe.
ValidatedDouble ValidatedFractionOfProjection(DVec3dCR vectorToProject, double defaultValue = 0.0) const;


//! @description Computes the magnitude of the xy part of a vector.
//! @return The magnitude of the xy parts of the given vector.
double MagnitudeXY() const;


//! @description Computes the squared magnitude of the xy part of a vector.
//! @return The squared magnitude of the xy parts of the given vector.
double MagnitudeSquaredXY() const;

//! @description Finds the largest absolute value among the components of a vector.
//! @return The largest absolute value among vector coordinates.
double MaxAbs () const;


//flex|| distance between  || a = vectorA.Distance (vectorB) || a = vectorA.DistanceSquared (vectorB) ||
//flex|| ... xy parts only || a = vectorA.DistanceXY (vectorB) || a = vectorA.DistanceSquaredXY (vectorB) ||
//! @description Computes the (cartesian) distance between two vectors
//! @param [in] vector2 The second vector
//! @return The distance between vector.
double Distance (DVec3dCR vector2) const;

//! @description Computes the squared distance between two vectors.
//! @param [in] vector2 The second vector.
//! @return The squared distance between the vectors.
double DistanceSquared (DVec3dCR vector2) const;

//! @description Computes the squared distance between two vectors, using only the
//!       xy parts.
//! @param [in] vector2 The second vector
//! @return The squared distance between the XY projections of the two vectors.
//!               (i.e. any z difference is ignored)
double DistanceSquaredXY (DVec3dCR vector2) const;

//! @description Computes the distance between two vectors, using
//!   only x and y components.
//! @param [in] vector2 The second vector
//! @return The distance between the XY projections of the two vectors.
//!               (i.e. any z difference is ignored)
double DistanceXY (DVec3dCR vector2) const;






//! Compute the cross product with other and return is magnitude (but the cross product is not returned)
//! @param [in] other second vector of cross product
double CrossProductMagnitude (DVec3dCR other) const;

//! @description Return the (scalar) cross product of the xy parts of two vectors.
//! @param [in] vector2 The second vector
//! @return The 2d cross product.
double CrossProductXY (DVec3dCR vector2) const;

//flex|| Single component access       || vector.SetComopnent (value, index) || a = vector.GetComponent (index) ||
//flex||                               ||             || vector.GetComponents (outX, outY, outZ) ||

//! @description Set one of three components (x,y,z) of the vector.
//! @param [in] a The component value.
//! @param [in] index Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
void SetComponent
(
double          a,
int             index
);

//! @description Gets a single component of a vector.  If the index is out of
//! range 0,1,2, it is interpreted cyclically.
//! @param [in] index Indicates which component is accessed.  The values
//!                       are 0=x, 1=y, 2=z.  Other values are treated cyclically.
//! @return The specified component of the vector.
double GetComponent (int index) const;


//! @description Copies x,y,z components from a vector to individual variables.
//! @param [out] xCoord x component
//! @param [out] yCoord y component
//! @param [out] zCoord z component
void GetComponents
(
double          &xCoord,
double          &yCoord,
double          &zCoord
) const;



//flex!! Geometric computations

//flex|| perpendicular triads, input becomes Z, all get length of input || bool vector.GetTriad (outVectorX, outVectorY, outVectorZ) ||
//flex|| perpendicular triads, input becomes Z, all normalized || bool vector.GetNormalizedTriad (outVectorX, outVectorY, outVectorZ) ||
//flex|| parallel and perpendicular parts || bool vector.GetPerpendicularParts (inHypotenuseVector, outFraction, outParallelVector, outPerpendicularVector) ||

//! @description Sets three vectors so that they are mutually
//! perpendicular, the third (Z) vector is identical to the
//! given axis vector, and all have the same length.
//! If the given axis vector contains only zeros, a (0,0,1) vector
//!   is used instead.
//! @param [out] xAxis x direction of the coordinate system
//! @param [out] yAxis y direction of the coordinate system
//! @param [out] zAxis z direction of the coordinate system
//! @return true unless given vector is z zero vector.
bool GetTriad
(
DVec3dR         xAxis,
DVec3dR         yAxis,
DVec3dR         zAxis
) const;
//!
//! @description Sets three vectors so that they are mutually
//! perpendicular unit vectors with the  third (Z) vector in the
//! direction of the given axis vector.
//! If the given axis vector contains only zeros, a (0,0,1) vector
//!   is used instead.
//!
//! @param [out] xAxis unit x direction vector
//! @param [out] yAxis unit y direction vector
//! @param [out] zAxis unit z direction vector
//! @return true unless given vector has zero length.
//!
bool GetNormalizedTriad
(
DVec3dR         xAxis,
DVec3dR         yAxis,
DVec3dR         zAxis
) const;

//! Find the fraction at which the instance projects to a vector.
//! @param [in] targetVector target vector.
//! @param [out] fraction projection fraction
//! @return false if targetVector has zero length.  In this case the fraction is zero.
bool ProjectToVector (DVec3dCR targetVector, double &fraction) const;

//! Find the projection of the instance vector to a plane defined by two vectors.
//! @param [in] vectorU u-direction vector of the plane.
//! @param [in] vectorV v-direction vector of the plane.
//! @param [out] uv fractional coordinates on vectors.
//! @return false if plane vectors are not independent.  In this case the method attempts to project to the 
//!             longer plane vector, and sets the other fraction to 0.   If that fails uv is 00.
bool ProjectToPlane (DVec3dCR vectorU, DVec3dCR vectorV, DPoint2dR uv) const;


//! Decompose hypotenuse into a vector parallel to the instance and a vector
//! perpendicular to the instance.
//! @param [in] hypotenuse vector to decompose
//! @param [out] fraction position where hypotenuse projects onto the instance.
//! @param [out] parallelPart vector parallel to the instance
//! @param [out] perpendicularPart vector perpendicular to the instance
bool GetPerpendicularParts (DVec3dCR hypotenuse, double &fraction, DVec3dR parallelPart, DVec3dR perpendicularPart) const;
//flex!! Angles and inclusions

//flex|| angle from vectorA to vectorB || radians(0..pi) = vectorA.AngleTo (vectorB)  || radians(-pi..+pi) = vectorA.AngleToXY (vectorB) ||
//flex|| angle from vectorA to vectorB or its negative || radians(0..pi/2) = vectorA.SmallerUnorientedAngleTo (vectorB) || radians (0..pi/2) vectorA.SmallerUnorientedAngleToXY (vectorB) ||
//flex|| angle from vectorA to vectorB in their own plane; upVector points to eye || radians (-pi..+pi) = vectorA.SignedAngleTo (vectorB, upVector) ||
//flex|| angle from projection of vectorA to projection of vectorB || radians (-pi..+pi) = vectorA.PlanarAngleTo (vectorB, planeNormal) ||
//flex|| is instance in smaller of two possible sweeps from vectorA to vectorB? @qD bool vector.IsVectorInSmallerSector (vectorA, vectorB) ||

//! @description Returns the angle between two vectors.  This angle is between 0 and
//! pi.  Rotating the first vector by this angle around the cross product
//! between the vectors aligns it with the second vector.
//! @param [in] vector2 The second vector
//! @return The angle between the vectors.
double AngleTo (DVec3dCR vector2) const;

//! @description Returns the angle from Vector1 to Vector2 using only xy parts.
//!  This angle is between -pi and +pi.
//! @param [in] vector2 The second vector
//! @return The angle between vectors.
double AngleToXY (DVec3dCR vector2) const;

//! @description Returns the angle between two vectors, choosing the smaller
//!   of the two possible angles when both the vectors and their negations are considered.
//!    This angle is between 0 and pi/2.
//! @param [in] vector2 The second vector
//! @return The angle between the vectors.
double SmallerUnorientedAngleTo (DVec3dCR vector2) const;

//! @description Returns the angle between two vectors, considering both
//!   the vectors and their negations and choosing the smaller.
//!   This angle is between 0 and pi/2.
//! @param [in] vector2 The second vector
//! @return The angle between vectors.
double SmallerUnorientedAngleToXY (DVec3dCR vector2) const;

//! @description Returns the angle that two vectors deviate from being perpendicular.
double AngleFromPerpendicular (DVec3dCR vector2) const;

//!
//! @description Computes the signed angle from one vector to another, in the plane
//!       of the two vectors.   Initial computation using only the two vectors
//!       yields two possible angles depending on which side of the plane of the
//!       vectors is viewed.  To choose which side to view, go on the side whose
//!       normal has a positive dot product with the orientation vector.
//! This angle can be between -pi and +pi.
//!
//! @param [in] vector2 The second vector
//! @param [in] orientationVector The vector used to determine orientation.
//! @return The signed angle
//!
double SignedAngleTo
(
DVec3dCR        vector2,
DVec3dCR        orientationVector
) const;

//! @description Computes the signed angle between the projection of two vectors
//!       onto a plane with given normal.
//!
//! @param [in] vector2 The second vector
//! @param [in] planeNormal The plane normal vector
//! @return The angle in plane
//!
double PlanarAngleTo
(
DVec3dCR        vector2,
DVec3dCR        planeNormal
) const;


//! Compute an axis and angle to rotate from the instance vector to a target.
//! @param [out] target direction that the instance is to rotate towards.
//! @param [out] axis   returned axis of rotation
//! @param [out] radians returned rotation angle
//! @return true if both vectors are nonzero.
bool AngleAndAxisOfRotationFromVectorToVector
(
DVec3dCR target,
DVec3dR axis,
double &radians
) const;

//flex|| is instance in counterclockwise sector of sweep from vectorA to vectorB, with bias vector towards eye || bool vector.IsInCCWSector (vectorA, vectorB, upVector) ||
//flex|| ... using xy parts only (implied Z to eye)    @qD bool vector.IsVectorInCCWXYSector (vectorA, vectorB) ||

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
bool IsVectorInSmallerSector  (DVec3dCR vector0, DVec3dCR vector1) const;

//!
//! @description Test if the test vector vector is "between" vector0 and vector1, with CCW direction
//! resolved by an up vector.  The cross product of vector0 and vector1 is
//! considered the positive plane normal if its dot product with the up vector
//! is positive.
//!
//! @param [in] vector0 The boundary vector.
//! @param [in] vector1 The boundary vector.
//! @param [in] upVector The out of plane vector. 
//! @return true if test vector is within the angle.
//!
bool IsVectorInCCWSector  (DVec3dCR vector0, DVec3dCR vector1, DVec3dCR upVector) const;

//!
//! @description Test if the instance vector vector is "between" vector0 and vector1, with CCW direction using only xy parts.
//!
//! @param [in] vector0 The boundary vector.
//! @param [in] vector1 The boundary vector.
//! @return true if test vector is within the angle.
//!
bool IsVectorInCCWXYSector
(
DVec3dCR        vector0,
DVec3dCR        vector1
) const;


//flex|| parallel and perpendicular tests || bool vectorA.IsPositiveParallelTo (vectorB) || bool vectorA.IsParallelTo (vectorB)  || vectorA.IsPerpendicularTo (vectorB) ||

//! @description Tests if two vectors are parallel (opposites are considered parallel!)
//! @param [in] vector2 The second vector
//! @return true if the vectors are parallel within tolerance
bool IsParallelTo (DVec3dCR vector2) const;

//! @description Tests if two vectors are parallel (opposites are considered parallel!)
//! @param [in] vector2 The second vector
//! @param [in] radians tolerance in radians.
//! @return true if the vectors are parallel within tolerance
bool IsParallelTo (DVec3dCR vector2, double radians) const;


//! @description Tests if two vectors are parallel and have positive dot product (opposites are NOT considered parallel)
//! @param [in] vector2 The second vector
//! @return true if the vectors are parallel within tolerance
bool IsPositiveParallelTo (DVec3dCR vector2) const;


//! @description Tests if two vectors are perpendicular.
//! @param [in] vector2 The second vector
//! @return true if vectors are perpendicular within tolerance
bool IsPerpendicularTo (DVec3dCR vector2) const;

//flex|| equality || vectorA.IsEqual (vectorB)    || vectorA.IsEqual (vectorB, tolerance ) ||

//! @description Test for exact equality between all components of two vectors.
//! @param [in] vector2 The second vector
//! @return true if the vectors are identical.
//!
bool IsEqual (DVec3dCR vector2) const;

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
DVec3dCR        vector2,
double          tolerance
) const;


//flex|| equality with system tolerance || vectorA.AlmostEqual (vectorB) ||

//! test if two points are equal.
//! Uses library "small angle" as both absolute and relative tolerance.
//! points are equal if squared distance between is less than
//!   (squared abstol) plus (squared relTol) * sum of cmponent squares
//! @param [in] dataB second DVec3d
//! @return true if within tolerance.
bool AlmostEqual (DVec3d const & dataB) const;

//flex|| all zeros || vector.IsZero () ||
//! @description checks if the vector has all zeros
bool IsZero() const {return x == 0.0 && y == 0.0 && z == 0.0;}

#endif
};


END_BENTLEY_NAMESPACE

#endif // dvec3d_H_
