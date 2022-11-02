/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifndef operators_H_
#define operators_H_

BEGIN_BENTLEY_NAMESPACE

//! operator overload for in-place addition of a point plus a vector
//    i.e. {point += vector}
//! @param [in] point point being updated.
//! @param [in] vector vector to be added to point.
GEOMDLLIMPEXP void operator+=(DPoint3d &point, DVec3d const &vector);

//! operator overload for in-place subtraction of a point minus a vector
//    i.e. {point -= vector}
//! @param [in] point point being updated.
//! @param [in] vector vector to be subtracted from point.
GEOMDLLIMPEXP void operator-=(DPoint3d &point, DVec3d const &vector);

//! operator overload for sum of a point plus a vector
//! @param [in] point left operand (point)
//! @param [in] vector right operand (vector)
//! @return point sum {point+vector}
GEOMDLLIMPEXP DPoint3d operator+(DPoint3d const &point, DVec3d const &vector);

//! operator overload for sum of a point minus a vector
//! @param [in] point left operand (point)
//! @param [in] vector right operand (vector)
//! @return point sum {point+vector}
GEOMDLLIMPEXP DPoint3d operator-(DPoint3d const &point, DVec3d const &vector);


//! operator overload for multiplication of a transform and a vector
//!<li>The vector appears on the left as a row, with implied 0 weigth that multiplies the transform's translation column.
//!<li>Compare to {vector*transform} which has the vector on the left as a row.
//!<li>Only the matrix part of the transform participates.
//!</ul>
//! @param [in] transform left operand (transform)
//! @param [in] vector right operand (vector)
//! @return vector product {transform*vector}
GEOMDLLIMPEXP DVec3d operator*( Transform const &transform, DVec3d const &vector);

//! operator overload for multiplication of a transform and a point
//! @param [in] transform left operand (transform)
//! @param [in] point right operand (point)
//! @return point product {transform*point}
GEOMDLLIMPEXP DPoint3d operator*( Transform const &transform, DPoint3d const &point);

//----------------------------------------------------------------------



//! operator overload for multiplication of a vector times the matrix part of a transform.
//!<ul> 
//!<li>The vector appears on the left as a row.
//!<li>Compare to {transform*vector} which has the vector on the right as a column.
//!<li>Only the matrix part of the transform participates.
//!</ul>
//! @param [in] vector left operand (vector)
//! @param [in] transform right operand
//! @return vector product {vector*transform}
GEOMDLLIMPEXP DVec3d operator*(DVec3d const &vector,Transform const &transform);


//! operator overload for multiplication of a vector times a rotmatrix
//!<ul> 
//!<li>The vector appears on the left as a row.
//!<li>Compare to {rotmatrix*vector} which has the vector on the right as a column.
//!</ul>
//! @param [in] vector vector to be multiplied
//! @param [in] rotmatrix rotmatrix multiplier
//! @return vector product {vector*rotmatrix}
GEOMDLLIMPEXP DVec3d operator*(DVec3d const &vector, RotMatrix const &rotmatrix);

//! operator overload for multiplication of a matrix and a vector
//!<ul> 
//!<li>Compare to {vector*matrix} which has the vector on the left as a row.
//!</ul>
//! @param [in] matrix left operand (matrix)
//! @param [in] vector right operand (vector)
//! @return vector product {matrix*vector}
GEOMDLLIMPEXP DVec3d operator*( RotMatrix const &matrix, DVec3d const &vector);

//----------------------------------------------------------------------

//! operator overload for in-place multiplication of a vector times a scalar,
//    i.e. {vector *= scalar}
//! @param [in] vector vector being updated.
//! @param [in] scalar scalar multiplier.
GEOMDLLIMPEXP void operator*=(DVec3d &vector, double const scalar);

//! operator overload for in-place addition of vectors
//    i.e. {vector += vector}
//! @param [in] left vector being updated
//! @param [in] right vector to be added to left
GEOMDLLIMPEXP void operator+=(DVec3d &left, DVec3d const &right);

//! operator overload for in-place subtraction of vectors
//    i.e. {vector -= vector}
//! @param [in] left vector being updated
//! @param [in] right vector to be subtracted from left
GEOMDLLIMPEXP void operator-=(DVec3d &left, DVec3d const &right);

//! operator overload for difference of vector
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector difference {left-right}
GEOMDLLIMPEXP DVec3d operator-(DVec3d const &left, DVec3d const &right);

//! operator overload for difference of points
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector difference {left-right}
GEOMDLLIMPEXP DVec3d operator-(DPoint3d const &left, DPoint3d const &right);


//! operator overload for sum of vectors
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector sum {left+right}
GEOMDLLIMPEXP DVec3d operator+(DVec3d const &left, DVec3d const &right);

//! operator overload for division of vector and scalar
//! @param [in] vector vector to be scaled (vector)
//! @param [in] scalar scalar divisor (scalar)
//! @return ValidatedVector quotient {vector/scalar}
GEOMDLLIMPEXP ValidatedDVec3d operator/(DVec3d const &vector, double const scalar);

//! operator overload for multiplication of a scalar times a vector
//! @param [in] scalar scalar multiplier (scalar)
//! @param [in] vector vector to be multiplied (vector)
//! @return vector product {scalar*vector}
GEOMDLLIMPEXP DVec3d operator*(double const scalar, DVec3d const &vector);

//! operator overload for multiplication of a vector times a scalar
//! @param [in] vector vector to be multiplied (vector)
//! @param [in] scalar scalar multiplier (scalar)
//! @return vector product {vector*scalar}
GEOMDLLIMPEXP DVec3d operator*(DVec3d const &vector, double const scalar);




//-----------------------------------------------------------------

//! operator overload for multiplication of a transform times a transform
//! @param [in] transformA left operand
//! @param [in] transformB right operand
//! @return transform product {transformA*transformB}
GEOMDLLIMPEXP Transform operator*(Transform const &transformA, Transform const &transformB);

//! operator overload for multiplication of a transform times a rotmatrix
//! @param [in] transform transform to be multiplied
//! @param [in] matrix rotmatrix multiplier
//! @return transform product {transform*matrix}
GEOMDLLIMPEXP Transform operator*(Transform const &transform, RotMatrix const &matrix);

//! operator overload for multiplication of a rotmatrix times a rotmatrix
//! @param [in] rotMatrixA left operand
//! @param [in] rotMatrixB right operand
//! @return rotmatrix product {rotMatrixA*rotMatrixB}
GEOMDLLIMPEXP RotMatrix operator*(RotMatrix const &rotMatrixA, RotMatrix const &rotMatrixB);

//! operator overload for multiplication of a rotmatrix times a transform
//! @param [in] matrix matrix multiplier
//! @param [in] transform transform to be multiplied
//! @return transform product {matrix*transform}
GEOMDLLIMPEXP Transform operator*(RotMatrix const &matrix, Transform const &transform);
//! operator overload, field by field bitwise comparison
GEOMDLLIMPEXP bool operator == (DPoint3dCR dataA, DPoint3dCR dataB);
//! operator overload, field by field bitwise comparison
GEOMDLLIMPEXP bool operator == (DPoint2dCR dataA, DPoint2dCR dataB);

GEOMDLLIMPEXP DVec2d operator* (DVec2d const &vector, double scale);
GEOMDLLIMPEXP DVec2d operator* (double scale, DVec2d const &vector);
GEOMDLLIMPEXP DPoint2d operator+ (DPoint2d const &point, DVec2d const &vector);
GEOMDLLIMPEXP DPoint2d operator- (DPoint2d const &point, DVec2d const &vector);

GEOMDLLIMPEXP DVec2d operator+ (DVec2d const &vectorA, DVec2d const &vectorB);

//! operator overload for difference of DMatrix4d
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return matrix difference {left-right}
GEOMDLLIMPEXP DMatrix4d operator-(DMatrix4d const &left, DMatrix4d const &right);

//! operator overload for sum of DMatrix4d
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return matrix sum {left+right}
GEOMDLLIMPEXP DMatrix4d operator+(DMatrix4d const &left, DMatrix4d const &right);

//! operator overload for product of DMatrix4d
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return matrix product {left*right}
GEOMDLLIMPEXP DMatrix4d operator*(DMatrix4d const &left, DMatrix4d const &right);

//! operator overload for product of DMatrix4d and DPoint4d
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return matrix point product {left*right}
GEOMDLLIMPEXP DPoint4d operator*(DMatrix4d const &left, DPoint4d const &right);

END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP Angle operator *(Angle const &theta, double factor);
GEOMDLLIMPEXP Angle operator *(double factor, Angle const &theta);
GEOMDLLIMPEXP Angle operator -(Angle const &theta);
GEOMDLLIMPEXP Angle operator +(Angle const &alpha, Angle const &beta);
GEOMDLLIMPEXP Angle operator -(Angle const &alpha, Angle const &beta);
END_BENTLEY_GEOMETRY_NAMESPACE

//==============================================================================
// FPoint3d and FVec3d family.
// These are entirely "float to float".
// Insert standard diatribe about the horrible precision of float.
BEGIN_BENTLEY_NAMESPACE
//! operator overload for in-place addition of a point plus a vector
//    i.e. {point += vector}
//! @param [in] point point being updated.
//! @param [in] vector vector to be added to point.
GEOMDLLIMPEXP void operator+=(FPoint3d &point, FVec3d const &vector);

//! operator overload for in-place subtraction of a point minus a vector
//    i.e. {point -= vector}
//! @param [in] point point being updated.
//! @param [in] vector vector to be subtracted from point.
GEOMDLLIMPEXP void operator-=(FPoint3d &point, FVec3d const &vector);

//! operator overload for sum of a point plus a vector
//! @param [in] point left operand (point)
//! @param [in] vector right operand (vector)
//! @return point sum {point+vector}
GEOMDLLIMPEXP FPoint3d operator+(FPoint3d const &point, FVec3d const &vector);

//! operator overload for sum of a point minus a vector
//! @param [in] point left operand (point)
//! @param [in] vector right operand (vector)
//! @return point sum {point+vector}
GEOMDLLIMPEXP FPoint3d operator-(FPoint3d const &point, FVec3d const &vector);


//! operator overload for multiplication of a transform and a vector
//!<li>The vector appears on the left as a row, with implied 0 weigth that multiplies the transform's translation column.
//!<li>Compare to {vector*transform} which has the vector on the left as a row.
//!<li>Only the matrix part of the transform participates.
//!</ul>
//! @param [in] transform left operand (transform)
//! @param [in] vector right operand (vector)
//! @return vector product {transform*vector}
GEOMDLLIMPEXP FVec3d operator*( Transform const &transform, FVec3d const &vector);

//! operator overload for multiplication of a transform and a point
//! @param [in] transform left operand (transform)
//! @param [in] point right operand (point)
//! @return point product {transform*point}
GEOMDLLIMPEXP FPoint3d operator*( Transform const &transform, FPoint3d const &point);

//----------------------------------------------------------------------



//! operator overload for multiplication of a vector times the matrix part of a transform.
//!<ul> 
//!<li>The vector appears on the left as a row.
//!<li>Compare to {transform*vector} which has the vector on the right as a column.
//!<li>Only the matrix part of the transform participates.
//!</ul>
//! @param [in] vector left operand (vector)
//! @param [in] transform right operand
//! @return vector product {vector*transform}
GEOMDLLIMPEXP FVec3d operator*(FVec3d const &vector,Transform const &transform);


//! operator overload for multiplication of a vector times a rotmatrix
//!<ul> 
//!<li>The vector appears on the left as a row.
//!<li>Compare to {rotmatrix*vector} which has the vector on the right as a column.
//!</ul>
//! @param [in] vector vector to be multiplied
//! @param [in] rotmatrix rotmatrix multiplier
//! @return vector product {vector*rotmatrix}
GEOMDLLIMPEXP FVec3d operator*(FVec3d const &vector, RotMatrix const &rotmatrix);

//! operator overload for multiplication of a matrix and a vector
//!<ul> 
//!<li>Compare to {vector*matrix} which has the vector on the left as a row.
//!</ul>
//! @param [in] matrix left operand (matrix)
//! @param [in] vector right operand (vector)
//! @return vector product {matrix*vector}
GEOMDLLIMPEXP FVec3d operator*( RotMatrix const &matrix, FVec3d const &vector);

//----------------------------------------------------------------------

//! operator overload for in-place multiplication of a vector times a scalar,
//    i.e. {vector *= scalar}
//! @param [in] vector vector being updated.
//! @param [in] scalar scalar multiplier.
GEOMDLLIMPEXP void operator*=(FVec3d &vector, double const scalar);

//! operator overload for in-place addition of vectors
//    i.e. {vector += vector}
//! @param [in] left vector being updated
//! @param [in] right vector to be added to left
GEOMDLLIMPEXP void operator+=(FVec3d &left, FVec3d const &right);

//! operator overload for in-place subtraction of vectors
//    i.e. {vector -= vector}
//! @param [in] left vector being updated
//! @param [in] right vector to be subtracted from left
GEOMDLLIMPEXP void operator-=(FVec3d &left, FVec3d const &right);

//! operator overload for difference of vector
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector difference {left-right}
GEOMDLLIMPEXP FVec3d operator-(FVec3d const &left, FVec3d const &right);

//! operator overload for difference of points
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector difference {left-right}
GEOMDLLIMPEXP FVec3d operator-(FPoint3d const &left, FPoint3d const &right);


//! operator overload for sum of vectors
//! @param [in] left left operand
//! @param [in] right right operand.
//! @return vector sum {left+right}
GEOMDLLIMPEXP FVec3d operator+(FVec3d const &left, FVec3d const &right);

//! operator overload for division of vector and scalar
//! @param [in] vector vector to be scaled (vector)
//! @param [in] scalar scalar divisor (scalar)
//! @return ValidatedVector quotient {vector/scalar}
GEOMDLLIMPEXP ValidatedFVec3d operator/(FVec3d const &vector, double const scalar);

//! operator overload for multiplication of a scalar times a vector
//! @param [in] scalar scalar multiplier (scalar)
//! @param [in] vector vector to be multiplied (vector)
//! @return vector product {scalar*vector}
GEOMDLLIMPEXP FVec3d operator*(double const scalar, FVec3d const &vector);

//! operator overload for multiplication of a vector times a scalar
//! @param [in] vector vector to be multiplied (vector)
//! @param [in] scalar scalar multiplier (scalar)
//! @return vector product {vector*scalar}
GEOMDLLIMPEXP FVec3d operator*(FVec3d const &vector, double const scalar);


END_BENTLEY_NAMESPACE

#endif