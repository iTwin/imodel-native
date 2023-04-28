/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_NAMESPACE

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint4dArray
(
RotMatrixCP pMatrix,
DPoint4dP pResult,
DPoint4dCP pPoint,
int            numPoint
);

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint2dArray
(
RotMatrixCP pMatrix,
DPoint2dP pResult,
DPoint2dCP pPoint,
int            numPoint
);

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyFPoint3dArray
(
RotMatrixCP pMatrix,
FPoint3dP pResult,
FPoint3dCP pPoint,
int            numPoint
);

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyFPoint2dArray
(
RotMatrixCP pMatrix,
FPoint2dP pResult,
FPoint2dCP pPoint,
int            numPoint
);

//!
//! Matrix times matrix, using only the xy rows (but z clumn) of B.
//! @param pProduct OUT     product matrix
//! @param pA IN      The first factor
//! @param pB IN      The second factor.  z row treated as zero.
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixXY
(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
);

//!
//! @description Returns the product {A*B} of two matrices.
//! @param pProduct  OUT     product matrix
//! @param pA IN      The first factor
//! @param pB IN      The second factor
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrix
(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
);

//!
//! @description Returns the product {AT*B} of a transposed matrix and a matrix.
//! @param pProduct  OUT     product matrix
//! @param pA IN      The first factor, to be tranposed.
//! @param pB IN      The second factor
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransposeRotMatrix
(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
);

//!
//! @description Returns the product {A*BT} of a matrix and a transposed matrix.
//! @param pProduct  OUT     product matrix
//! @param pA IN      The first factor
//! @param pB IN      The second factor, to be transposed
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixTranspose
(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB
);

//!
//! @description Returns the product of three matrices.
//! @param pProduct  OUT     product matrix
//! @param pA IN      The first factor
//! @param pB IN      The second factor
//! @param pC IN      The third factor
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixRotMatrixRotMatrix
(
RotMatrixP pProduct,
RotMatrixCP pA,
RotMatrixCP pB,
RotMatrixCP pC
);

//!
//! Sets this instance to the product of matrix pA times the matrix part
//! of transform pB.
//! In this 'post multiplication' the matrix part of pB multiplies pA ON THE RIGHT
//! and the translation part of pB is ignored.
//! @param pAB OUT     product matrix
//! @param pA IN      The first factor (matrix)
//! @param pB IN      The second factor (transform)
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransform
(
RotMatrixP pAB,
RotMatrixCP pA,
TransformCP pB
);

//!
//! @description Returns to the product of the matrix part of transform pA
//! times matrix pB.
//! In this 'pre multiplication' the matrix part of pA multiplies pB ON THE LEFT
//! and the translation part of pA is ignored.
//! @param pAB OUT     product matrix
//! @param pA IN      The first factor (transform)
//! @param pB IN      The second factor (matrix)
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransformRotMatrix
(
RotMatrixP pAB,
TransformCP pA,
RotMatrixCP pB
);

//!
//! @description Returns the (scalar) LeftPoint*Matrix*RightPoint
//! @param pLeftPoint IN      The point applied on left of matrix
//! @param pA IN      The matrix
//! @param pRightPoint IN      The point applied on right of matrix
//! @return evaluated quadratic form
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_quadraticForm
(
DPoint3dCP pLeftPoint,
RotMatrixCP pA,
DPoint3dCP pRightPoint
);

//!
//! computes the 8 corner points of the range cube defined by the two points
//! of pRange, multiplies matrix times each point, and computes the
//! twopoint range limits (min max) of the resulting rotated box.
//! Note that the volume of the rotated box is generally larger than
//! that of the input because the postrotation minmax operation expands
//! the box.
//! Ranges that are null as defined by pInRange->isNull are left
//! unchanged.
//! @param pMatrix IN      The matrix to apply
//! @param pOutRange OUT     range of transformed cube
//! @param pInRange IN      The any two corners of original cube
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRange
(
RotMatrixCP pMatrix,
DRange3dP pOutRange,
DRange3dCP pInRange
);

//!
//! @description Returns an identity matrix.
//! @param pMatrix OUT     identity matrix (1 on diagonal, 0 off diagonal)
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initIdentity (RotMatrixP pMatrix);

//!
//! @description returns a zero matrix.
//! @param pMatrix OUT     a matrix of zeros.
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_zero (RotMatrixP pMatrix);

//!
//! @description Returns a scaling matrix with respective x, y, and z
//! scaling factors.
//! @param pMatrix OUT     scale matrix
//! @param xscale IN      The x direction scale factor
//! @param yscale IN      The y direction scale factor
//! @param zscale IN      The z direction scale factor
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScaleFactors
(
RotMatrixP pMatrix,
double         xscale,
double         yscale,
double         zscale
);

//!
//! @description Returns a uniform scaling matrix.
//! @param pMatrix OUT     scale matrix
//! @param scale IN      The scale factor.
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScale
(
RotMatrixP pMatrix,
double      scale
);

//!
//! @description Returns a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//! @param pMatrix OUT     constructed matrix
//! @param pVector1 IN      The column vector U
//! @param pVector2 IN      The row vector V
//! @param scale IN      The scale factor
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromScaledOuterProduct
(
RotMatrixP pMatrix,
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
);

//!
//! @description Returns a 'rank one' matrix defined as a scale factor times
//!        the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//! @param pMatrix OUT     constructed matrix
//! @param pVector1 IN      The column vector U
//! @param pVector2 IN      The row vector V
//! @param scale IN      The scale factor
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_addScaledOuterProductInPlace
(
RotMatrixP pMatrix,
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
);

//!
//! Copies double arrays into corresponding columns of a matrix.
//! @param pMatrix OUT     initialized matrix.
//! @param pXVector IN      The array to insert in column 0
//! @param pYVector IN      The array to insert in column 1
//! @param pZVector IN      The array to insert in column 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromColumnArrays
(
RotMatrixP pMatrix,
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
);

//!
//! @description Returns a matrix with the 9 specified coefficients given
//! in "row major" order.
//! @param pMatrix OUT     initialized matrix.
//! @param x00 IN      The 00 entry
//! @param x01 IN      The 01 entry
//! @param x02 IN      The 02 entry
//! @param x10 IN      The 10 entry
//! @param x11 IN      The 11 entry
//! @param x12 IN      The 12 entry
//! @param x20 IN      The 20 entry
//! @param x21 IN      The 21 entry
//! @param x22 IN      The 22 entry
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowValues
(
RotMatrixP pMatrix,
double        x00,
double        x01,
double        x02,
double        x10,
double        x11,
double        x12,
double        x20,
double        x21,
double        x22
);

//!
//! @description Returns a matrix with 3 points copied to respective columns.
//! @param pMatrix OUT     initialized matrix.
//! @param pXVector IN      The vector to insert in column 0
//! @param pYVector IN      The vector to insert in column 1
//! @param pZVector IN      The vector to insert in column 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromColumnVectors
(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
);

//!
//! Copies double arrays into corresponding rows of a matrix.
//! @param pMatrix OUT     initialized matrix.
//! @param pXVector IN      The array to insert in row 0
//! @param pYVector IN      The array to insert in row 1
//! @param pZVector IN      The array to insert in row 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowArrays
(
RotMatrixP pMatrix,
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
);

//!
//! @description Returns a matrix with 3 points copied to respective rows.
//! @param pMatrix OUT     initialized matrix.
//! @param pXVector IN      The vector to insert in row 0
//! @param pYVector IN      The vector to insert in row 1
//! @param pZVector IN      The vector to insert in row 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromRowVectors
(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
);

//!
//! @description Returns a matrix representing rotation around a vector.
//! @param pMatrix OUT     rotation matrix
//! @param pAxis IN      The axis of rotation
//! @param radians IN      The rotation angle
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromVectorAndRotationAngle
(
RotMatrixP pMatrix,
DVec3dCP pAxis,
double         radians
);

//!
//! @description Returns a matrix which scales along a vector direction.
//! @param pMatrix OUT     scale matrix.
//! @param pVector IN      The scaling direction
//! @param scale IN      The scale factor
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromDirectionAndScale
(
RotMatrixP pMatrix,
DVec3dCP pVector,
double         scale
);

//!
//! @description Returns a matrix of rotation  about the x,y, or z axis
//!
//! (indicated by axis = 0,1, or 2) by an angle in radians.
//! @param pMatrix OUT     rotation matrix
//! @param axis IN      The axis index 0=x, 1=y, 2=z
//! @param radians IN      The rotation angle in radians
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromAxisAndRotationAngle
(
RotMatrixP pMatrix,
int            axis,
double         radians
);

//!
//! @description Returns the transpose of a matrix.
//! @param pTranspose OUT     transposed matrix
//! @param pMatrix IN      The input matrix
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_transpose
(
RotMatrixP pTranspose,
RotMatrixCP pMatrix
);

//!
//! @description Transposes a matrix in place.
//! @param pMatrix IN OUT  The transposed matrix
//! @group "RotMatrix In Place Modification"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_transposeInPlace (RotMatrixP pMatrix);

//!
//! @description Compute the inverse of the matrix.
//! @param pInverse OUT     inverted matrix
//! @param pForward IN      The input matrix
//! @return true if the matrix is invertible.
//! @group "RotMatrix Queries"
//!
GEOMDLLIMPEXP bool    bsiRotMatrix_invertRotMatrix
(
RotMatrixP pInverse,
RotMatrixCP pForward
);

//!
//! @description Tests if a matrix can be inverted
//! @param pMatrix IN      The matrix to test.
//! @return true if the matrix is invertible.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_hasInverse (RotMatrixCP pMatrix);

//!
//! @description Inverts this instance matrix in place.
//! @param pMatrix IN OUT  The inverted matrix
//! @return true if the matrix is invertible.
//! @group "RotMatrix In Place Modification"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_invertInPlace (RotMatrixP pMatrix);

//!
//! @description Applies scale factors to corresponding rows of the input matrix, and places the result
//! in this instance matrix.
//! @param pScaledMatrix OUT     scaled matrix
//! @param pInMatrix IN      The initial matrix
//! @param xScale IN      The scale factor for row 0
//! @param yScale IN      The scale factor for row 1
//! @param zScale IN      The scale factor for row 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_scaleRows
(
RotMatrixP pScaledMatrix,
RotMatrixCP pInMatrix,
double         xScale,
double         yScale,
double         zScale
);

//!
//! @description Applies scale factors to corresponding columns of the input matrix, and places the result
//! in this instance matrix.
//! @param pScaledMatrix OUT     scaled matrix
//! @param pIn IN      The initial matrix
//! @param xs IN      The scale factor for column 0
//! @param ys IN      The scale factor for column 1
//! @param zs IN      The scale factor for column 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_scaleColumns
(
RotMatrixP pScaledMatrix,
RotMatrixCP pIn,
double         xs,
double         ys,
double         zs
);

//!
//! @description Returns a matrix formed from a scaling matrix which is
//! multiplied on the left and/or right with other matrices.
//! That is, form LeftMatrix * ScaleMatrix * RightMatrix
//! where the ScaleMatrix is constructed from the given scale factors.
//! If LeftMatrix is null, this has the effect of scaling the rows
//! of the right matrix.  If RightMatrix is null this has the effect
//! of scaling the columns of the left matrix.
//! @param pScaledMatrix OUT     scaled matrix
//! @param pLeftMatrix IN      The matrix on left of product
//! @param xs IN      The x scale factor
//! @param ys IN      The y scale factor
//! @param zs IN      The z scale factor
//! @param pRightMatrix IN      The matrix on right of product
//! @group "RotMatrix Multiplication"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixScaleRotMatrix
(
RotMatrixP pScaledMatrix,
RotMatrixCP pLeftMatrix,
double         xs,
double         ys,
double         zs,
RotMatrixCP pRightMatrix
);

//!
//! @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
//! around X, Y, and Z axes, and M is the input matrix.
//! @param pRotatedMatrix OUT     rotated matrix
//! @param pInMatrix IN      The optional prior matrix
//! @param xrot IN      The x axis rotation
//! @param yrot IN      The y axis rotation
//! @param zrot IN      The z axis rotation
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_rotateByPrincipleAngles
(
RotMatrixP pRotatedMatrix,
RotMatrixCP pInMatrix,
double         xrot,
double         yrot,
double         zrot
);

//!
//! @description Copies from columns of this instance matrix to corresponding points.
//! @param pMatrix IN      The input matrix
//! @param pX OUT     first column
//! @param pY OUT     second column
//! @param pZ OUT     third column
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getColumns
(
RotMatrixCP pMatrix,
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
);

//!
//! @description Copies from rows of this instance matrix to corresponding points.
//! @param pMatrix IN      The input matrix
//! @param pX OUT     first row
//! @param pY OUT     second row
//! @param pZ OUT     third row
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getRows
(
RotMatrixCP pMatrix,
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
);

//!
//! @description Set the components in a column.
//! @param pMatrix IN OUT  The matrix to change.
//! @param pPoint IN      new values
//! @param col IN      The index of column to change. Column indices are 0, 1, 2.
//! @group "RotMatrix Initialization"
//! @group "RotMatrix In Place Modification"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_setColumn
(
RotMatrixP pMatrix,
DVec3dCP pPoint,
int            col
);

//!
//! @description Set the components in a row.
//! @param pMatrix IN OUT  The matrix to change.
//! @param pPoint IN      new values
//! @param row IN      The index of row to change. Row indices are 0, 1, 2.
//! @group "RotMatrix In Place Modification"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_setRow
(
RotMatrixP pMatrix,
DVec3dCP pPoint,
int            row
);

//!
//! @description Returns a point taken from a column of a matrix.
//! @param pMatrix IN      The input matrix
//! @param pPoint OUT     filled point
//! @param col IN      The index of column to extract. Column indices are 0, 1, 2.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getColumn
(
RotMatrixCP pMatrix,
DVec3dP pPoint,
int            col
);

//!
//! @description Returns a value from a specified row and column of the matrix.
//! @param pMatrix IN      The input matrix.
//! @param row IN      The index of row to read. Row indices are 0, 1, 2.
//! @param col IN      The index of column to read.  Column indices are 0, 1, 2.
//! @return specified component of matrix
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_getComponentByRowAndColumn
(
RotMatrixCP pMatrix,
int            row,
int            col
);

//!
//! @description Sets a value at a specified row and column of the matrix.
//! @param pMatrix IN      The input matrix.
//! @param row IN      The index of row to read.  Row indices are 0, 1, and 2.
//! @param col IN      The index of column to read.  Column indices are 0, 1, and 2.
//! @param value  IN      the value to set.
//! @group "RotMatrix In Place Modification"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_setComponentByRowAndColumn
(
RotMatrixP pMatrix,
int            row,
int            col,
double         value
);

//!
//! @description Returns a vector taken from a column of a matrix.
//! @param pMatrix IN      The input matrix
//! @param pPoint OUT     filled point
//! @param row IN      The index of row to extract.  Row indices are 0, 1, and 2.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getRow
(
RotMatrixCP pMatrix,
DVec3dP pPoint,
int            row
);

//!
//! @description Returns a matrix whose rows are unit vectors in the same
//! drection as corresponding rows of the input matrix.
//! Also (optionally) stores the original row magnitudes as components of the point.
//! @param pMatrix OUT     matrix with normalized rows
//! @param pInMatrix IN      The input matrix
//! @param pScaleVector OUT     length of original rows
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getNormalizedRows
(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
DVec3dP pScaleVector
);

//!
//! @description Returns a matrix whose rows are unit vectors in the same
//! drection as corresponding columns of the input matrix.
//! Also (optionally) stores the original column magnitudes as components of the point.
//! @param pMatrix OUT     matrix with normalized columns
//! @param pInMatrix IN      The input matrix
//! @param pScaleVector OUT     length of original columns
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_getNormalizedColumns
(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
DVec3dP pScaleVector
);

//!
//! @description Returns the determinant of the matrix.
//! @param pMatrix IN      The matrix to query
//! @return determinant of the matrix.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_determinant (RotMatrixCP pMatrix);

//!
//! Computes an estimate of the condition of this instance matrix.  Values near 0
//! are bad.
//! @param pMatrix IN      The input matrix
//! @return estimated condition number.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_conditionNumber (RotMatrixCP pMatrix);

//!
//! @description Tests if a matrix is the identity matrix.
//! @param pMatrix IN      The matrix to test
//! @return true if matrix is approximately an identity.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isIdentity (RotMatrixCP pMatrix);

//!
//! @description Tests if a matrix has small offdiagonal entries compared to
//! diagonals.   The specific test condition is that the
//! largest off diagonal absolute value is less than a tight tolerance
//! fraction times the largest diagonal entry.
//! @param pMatrix IN      The matrix to test
//! @return true if matrix is approximately diagonal
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isDiagonal (RotMatrixCP pMatrix);

//!
//! @description Tests if a matrix has (nearly) equal diagaonal entries and
//!
//! (nearly) zero off diagonals.  Tests use a tight relative tolerance.
//! @param pMatrix IN      The matrix to test
//! @param pMaxScale OUT     the largest diagaonal entry
//! @return true if matrix is approximately diagonal
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isUniformScale
(
RotMatrixCP pMatrix,
double        *pMaxScale
);

//!
//! @description Return the (signed) range of entries on the diagonal.
//! @param pMatrix IN      The matrix to test
//! @param pMinValue OUT     smallest signed value
//! @param pMaxValue OUT     largest signed value
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_diagonalSignedRange
(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
);

//!
//! @description Return the (absolute value) range of entries on the diagonal.
//! @param pMatrix IN      The matrix to test
//! @param pMinValue OUT     smallest absolute value
//! @param pMaxValue OUT     largest absolute value
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_diagonalAbsRange
(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
);

//!
//! @description Return the (signed) range of entries off the diagonal.
//! @param pMatrix IN      The matrix to test
//! @param pMinValue OUT     smallest signed value
//! @param pMaxValue OUT     largest signed value
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_offDiagonalSignedRange
(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
);

//!
//! @description Return the (absolute value) range of entries off the diagonal.
//! @param pMatrix IN      The matrix to test
//! @param pMinValue OUT     smallest absolute value
//! @param pMaxValue OUT     largest absolute value
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_offDiagonalAbsRange
(
RotMatrixCP pMatrix,
double              *pMinValue,
double              *pMaxValue
);

//!
//! @description Test if this instance matrix does nothing more
//! than exchange and possibly negate principle axes.
//! @param pMatrix IN      The input matrix
//! @return true if the matrix is a permutation of the principle axes.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isSignedPermutation (RotMatrixCP pMatrix);

//!
//! @description Test if a matrix is a rigid body rotation,
//! i.e. its transpose is its inverse and it has a positive determinant.
//! @param pMatrix IN      The matrix to test
//! @return true if the matrix is a rigid body rotation.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRigid (RotMatrixCP pMatrix);

//!
//! @description Test if this instance matrix is orthogonal, i.e. its transpose is its inverse.
//! This class of matrices includes both rigid body rotations and reflections.
//! @param pMatrix IN      The matrix to test
//! @return true if the matrix is orthogonal.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isOrthogonal (RotMatrixCP pMatrix);

//!
//! @description Test if this instance matrix has orthonormal columns, i.e. its columns
//! are all perpendicular to one another.
//! @param pMatrix IN      The matrix to test
//! @param pColumns OUT     (optional) matrix containing the unit vectors along the columns.
//! @param pAxisScales OUT     (optional) point whose x, y, and z components are the magnitudes of the
//! original columns.
//! @param pAxisRatio OUT     (optional) smallest axis length divided by largest.
//! @return true if the matrix is orthonormal.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_areColumnsOrthonormal
(
RotMatrixCP pMatrix,
RotMatrixP pColumns,
DVec3dP pAxisScales,
double      *pAxisRatio
);

//!
//! @description Test if this instance matrix is composed of only rigid rotation and a uniform
//! positive scale.
//! @param pMatrix IN      The matrix to test
//! @param pColumns OUT     (optional) matrix containing the unit vectors along the columns.
//! @param pScale OUT     largest axis scale factor.  If function value is true,
//!    the min scale is the same.  Use areColumnsOrthonormal to get
//!    separate column scales.
//! @return true if the matrix is orthonormal with positive determinant and same length of all columns.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isRigidScale
(
RotMatrixCP pMatrix,
RotMatrixP pColumns,
double      *pScale
);

//!
//! @description Tests if this instance matrix has no effects perpendicular to any plane with the given normal.  This
//! will be true if the matrix represents a combination of (a) scaling perpencicular to the normal
//! and (b) rotation around the normal.
//!
//! @param pMatrix IN      The input matrix
//! @param pNormal IN      The plane normal
//! @return true if the matrix has no effects perpendicular to any plane
//! with the given normal.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isPlanar
(
RotMatrixCP pMatrix,
DVec3dCP pNormal
);

//!
//! @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
//!    is aligned with the vector pDir.   The normalize flag selects between
//!    normalized axes (all matrix columns of unit length) and unnormalized
//!    axes (all matrix columns of same length as the pDir vector).
//! @param pMatrix OUT     computed matrix
//! @param pDir IN      The fixed direction vector
//! @param axis IN      The axis column to be aligned with direction
//! @param normalize IN      true to have normalized columns
//! @return true if the direction vector is nonzero.
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initFrom1Vector
(
RotMatrixP pMatrix,
DVec3dCP pDir,
int            axis,
bool               normalize
);

//!
//! @description Copies vectors into the x and y columns of this instance matrix
//! and sets the z column to their cross product.
//! Use squareAndNormalizeColumns to force these (possibly
//! skewed and scaled) coordinate axes to be perpendicular
//! @param pMatrix OUT     computed matrix
//! @param pXVector IN      The vector to place in column 0
//! @param pYVector IN      The vector to place in column 1
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFrom2Vectors
(
RotMatrixP pMatrix,
DVec3dCP pXVector,
DVec3dCP pYVector
);

//!
//! @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
//! from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
//! on the positive side, and column 2 as their cross product.
//! @param pMatrix OUT     computed matrix
//! @param pOrigin IN      The reference point
//! @param pXPoint IN      The x axis target point
//! @param pYPoint IN      The 3rd point defining xy plane.
//! @return true if the 3 points are non-colinear
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_initRotationFromOriginXY
(
RotMatrixP pMatrix,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
);

//!
//! @description Adjust the direction and length of columns of the input matrix to
//! produce an instance matrix which has perpendicular, unit length columns.
//! The column whose index is primaryAxis (i.e. 0,1,2 for x,y,z axis of
//! coordinate frame) is normalized to unit length in its current
//! direction.
//! The column whose index is secondaryAxis is unit length and perpendicular
//! to the primaryAxis column, and lies in the same plane as that
//! defined by the original primary and secondary columns.
//! To preserve X axis and XY plane, call with axis id's 0 and 1.
//! To preserve Z axis and ZX plane, call with axis id's 2 and 0.
//! pInMatrix and pMatrix may be the same address.
//! @param pMatrix OUT     normalized matrix
//! @param pInMatrix IN      The input matrix
//! @param primaryAxis IN      The axis id (0, 1, 2) which is to be normalized but left
//! in its current direction
//! @param secondaryAxis IN      The axis id (0, 1, 2) which is to be kept within the
//! plane of the primary and secondary axis.
//! @return false if primaryAxis and secondaryAxis are the same, or either axis has zero length
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_squareAndNormalizeColumns
(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
int            primaryAxis,
int            secondaryAxis
);

//!
//! @description Returns an orthogonal matrix that preserves aligns with
//! the columns of pInMatrix.  This is done by
//! trying various combinations of primary and secondary axes until one
//! succeeds in squareAndNormalizeColumns.
//! @param pMatrix OUT     normalized matrix
//! @param pInMatrix IN      The input matrix
//! @return true if a valid pair of primary and secondary axes was found.
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_squareAndNormalizeColumnsAnyOrder
(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix
);

//!
//! @description Moves columns 0, 1, 2 of the input matrix into columns i0, i1, i2
//! of the instance.
//! @param pMatrix OUT     shuffled matrix
//! @param pInMatrix IN      The input matrix
//! @param i0 IN      The column to receive input column 0
//! @param i1 IN      The column to receive input column 1
//! @param i2 IN      The column to receive input column 2
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_shuffleColumns
(
RotMatrixP pMatrix,
RotMatrixCP pInMatrix,
int            i0,
int            i1,
int            i2
);

//!
//! @description Returns {Matrix0 + Matrix1*s1+Matrix2*s2}, i.e. the sum of matrix M0,
//! matrix M1 multiplied by scale s1, and matrix M2 multiplied by scale s2.
//! Any combination of the matrix pointers may have identical addresses.
//! Any of the input matrix pointers may be NULL.
//! @param pSum OUT     sum of matrices
//! @param pMatrix0 IN      The matrix0 of formula
//! @param pMatrix1 IN      The matrix1 of formula
//! @param scale1 IN      The scale factor to apply to Matrix1
//! @param pMatrix2 IN      The matrix2 of formula
//! @param scale2 IN      The scale factor to apply to Matrix2
//! @group "RotMatrix Addition"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_add2ScaledRotMatrix
(
RotMatrixP pSum,
RotMatrixCP pMatrix0,
RotMatrixCP pMatrix1,
double         scale1,
RotMatrixCP pMatrix2,
double         scale2
);

//!
//! @description Add a matrix (componentwise, in place).
//! @param pMatrix  IN OUT  The  pMatrix + pDelta
//! @param pDelta IN      The matrix to add
//! @group "RotMatrix Addition"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_addRotMatrixInPlace
(
RotMatrixP pMatrix,
RotMatrixCP pDelta
);

//!
//! @description Subtract a matrix (componentwise, in place).
//! @param pMatrix  OUT     pMatrix - pDelta
//! @param pDelta IN      The matrix to subtract
//! @group "RotMatrix Addition"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_subtract
(
RotMatrixP pMatrix,
RotMatrixCP pDelta
);

//!
//! @description Return the sum of squares of coefficients in a matrix.
//! @param pMatrix IN      The matrix to sum
//! @return Sum of squares of all entries in matrix
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_sumSquares (RotMatrixCP pMatrix);

//!
//! @description Find the largest absolute value of entries in the matrix.
//! @param pMatrix IN      The matrix to inspect
//! @return largest absolute value in matrix
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_maxAbs (RotMatrixCP pMatrix);

//!
//! @description Returns the largest absolute value difference between
//! corresponding coefficients in Matrix1 and Matrix2.
//! @param pMatrix1 IN      The matrix to inspect
//! @param pMatrix2 IN      The matrix to compare to
//! @return largest absolute difference between the two matrices.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_maxDiff
(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2
);

//!
//! @description Computes the sum of the squares of the diagonal entries of this instance matrix.
//! @param pMatrix IN      The matrix to inspect
//! @return Sum of squares of diagonal entries
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_sumDiagonalSquares (RotMatrixCP pMatrix);

//!
//! @description Computes the sum of the squares of the off-diagonal entries of this instance matrix.
//! @param pMatrix IN      The matrix to inspect
//! @return sum of square of off-diagonal entries of the matrix.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_sumOffDiagonalSquares (RotMatrixCP pMatrix);

//!
//! @description Computes (complex) eigenvalues of a matrix.
//! @param pLambda0 OUT     eigenvalue; may be complex
//! @param pLambda1 OUT     eigenvalue; may be complex
//! @param pLambda2 OUT     eigenvalue; may be complex
//! @param pMatrix IN      The matrix whose eigenvalues are computed.
//! @return int
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP int bsiRotMatrix_getEigenvalues
(
DPoint2d            *pLambda0,
DPoint2d            *pLambda1,
DPoint2d            *pLambda2,
RotMatrixCP pMatrix
);

//!
//! @description Compute the eigenvalues of a symmetric matrix.   The symmetry
//! assures that all eigenvalues are real.
//!
//! @param pLambda0 OUT     first eigenvalue
//! @param pLambda1 OUT     second eigenvalue
//! @param pLambda2 OUT     third eigenvalue
//! @param pMatrix IN      The matrix whose eigenvalues are computed
//! @return int
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP int bsiRotMatrix_getEigenvaluesFromSymmetric
(
double              *pLambda0,
double              *pLambda1,
double              *pLambda2,
RotMatrixCP pMatrix
);

//!
//! @param pMatrix       OUT     matrix to be computed from quaternion
//! @param pQuat     IN      The quaternion, stored as (xyzw)
//! @param   transpose          IN      true if matrix is stored transposed
//! @group Quaternions
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_fromQuaternion
(
RotMatrixP pMatrix,
DPoint4dCP pQuat
);

//!
//! @param pMatrix  IN      The matrix to be converted to quaternion
//! @param pQuat   OUT     quaternion, stored as xyzw
//! @param transpose   IN      true if matrix is stored transposed
//! @group Quaternions
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_toQuaternion
(
RotMatrixCP pMatrix,
DPoint4dP pQuat,
bool                transpose
);

//!
//! @param pMatrix  IN      The matrix to be converted to quaternion
//! @param pQuatAsDoubleArray OUT     quaternion, stored as (w,x,y,z) in an array of doubles.
//! @param   transpose IN      true if matrix is stored transposed
//! @group Quaternions
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_toQuaternionAsDoubleArray
(
RotMatrixCP pMatrix,
double              *pQuatAsDoubleArray,
bool                transpose
);

//!
//! @param pMatrix OUT     matrix computed from quaternion
//! @param pQuatAsDoubleArray IN      The quaternion, stored as (w,x,y,z) in an array of doubles.
//! @group Quaternions
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_fromQuaternionAsDoubleArray
(
RotMatrixP pMatrix,
const double        *pQuatAsDoubleArray
);

//!
//! @description Implementation of slerp: spherical linear interpolation of quaternions.
//! @remarks It is assumed that the input matrices are pure rotations.
//! @param pSum               OUT     The interpolated rotation
//! @param pMatrix0           IN      The rotation corresponding to fractionParameter of 0.0
//! @param fractionParameter  IN      The interpolation parameter in [0,1]
//! @param pMatrix1           IN      The rotation corresponding to fractionParameter of 1.0
//!
Public GEOMDLLIMPEXP void     bsiRotMatrix_interpolateRotations
(
RotMatrixP   pSum,
RotMatrixCP   pRotation0,
double      fractionParameter,
RotMatrixCP   pRotation1
);

//!
//! Returns the angle of rotation of this instance and sets pAxis to be the
//! normalized vector about which this instance rotates.
//! NOTE: this instance is assumed to be a (rigid body, i.e. orthogonal) rotation
//! matrix.
//! Since negating both angle and axis produces an identical rotation,
//! calculations are simplified by assuming (and returning) the angle in [0,Pi].
//! @param pM      IN      The input matrix (assumed to be orthogonal)
//! @param    pAxis   OUT     normalized axis of rotation
//! @return rotation angle (in radians) between 0 and Pi, inclusive
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP double bsiRotMatrix_getRotationAngleAndVector
(
RotMatrixCP pM,
DVec3dP pAxis
);

//!
//! Sets this instance matrix by copying the matrix part of the trasnform.
//! @param pMatrix OUT     matrix part of transformation
//! @param pTransform IN      The transformation whose matrix part is returned
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_initFromTransform
(
RotMatrixP pMatrix,
TransformCP pTransform
);

//!
//! @description Sets this instance matrix by copying from the matrix parameter.
//! @param pMatrix  OUT     copied data
//! @param pIn IN      The source matrix
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_copy
(
RotMatrixP pMatrix,
RotMatrixCP pIn
);

//!
//! @description Tests for equality between two matrices
//! "Equality" means relative error less than 1.0e-12, in the sense that each
//! component-wise difference is less than 1.0e-12 times the largest absolute
//! value of the components of one matrix.
//! @param pMatrix1 IN      The first matrix
//! @param    pMatrix2   IN      The second matrix
//! @return   true if the matrices are identical.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_matrixEqual
(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2
);

//!
//! @description Tests for equality between two matrices.
//! @param pMatrix1 IN      The first matrix
//! @param    pMatrix2   IN      The second matrix
//! @param    tolerance IN      The relative error tolerance.  Absolute tolerance is
//! this (relative) tolerance times the largest absolute value in Matrix1.
//! @return true if the matrices are identical within tolerance.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_matrixEqualTolerance
(
RotMatrixCP pMatrix1,
RotMatrixCP pMatrix2,
double              tolerance
);

//!
//! Apply a Givens "row operation", i.e. pre-multiply by a Givens rotation matrix.
//! The Givens matrix is an identity except for the 4 rotational entries, viz
//! R(i0,i0)=R(i1,i1)=c
//! R(i0,i1)=s
//! R(i1,i0)=-s
//! @param pProduct      OUT     product of givens rotation and input matrix.
//! @param    c   IN      The cosine of givens rotation.
//! @param    s   IN      The sine of givens rotation.
//! @param    i0  IN      The index of the first affected row.
//! @param    i1  IN      The index of the second affected row.
//! @param    pMatrix IN      The input matrix
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void        bsiRotMatrix_givensRowOp
(
RotMatrixP pProduct,
double        c,
double        s,
int           i0,
int           i1,
RotMatrixCP pMatrix
);

//!
//! Apply a Givens "column operation", i.e. post-multiply by a Givens rotation matrix.
//! The Givens matrix is an identity except for the 4 rotational entries, viz
//! R(i0,i0)=R(i1,i1)=c
//! R(i0,i1)=-s
//! R(i1,i0)=s
//! @param pProduct      OUT     product of matrix and a givens rotation.
//! @param    pMatrix IN      The input matrix
//! @param    c   IN      The cosine of givens rotation.
//! @param    s   IN      The sine of givens rotation.
//! @param    i0  IN      The index of the first affected row.
//! @param    i1  IN      The index of the second affected row.
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void        bsiRotMatrix_givensColumnOp
(
RotMatrixP pProduct,
RotMatrixCP pMatrix,
double        c,
double        s,
int           i0,
int           i1
);

//!
//! Apply a hyperbolic "row operation", i.e. pre-multiply by a hyperbolic reflection matrix
//! The matrix is an identity except for the 4 entries
//! R(i0,i0)=R(i1,i1)=secant
//! R(i0,i1)=R(i1,i0)=tangent
//! @param pProduct      OUT     product of givens rotation and input matrix.
//! @param    secant   IN      The cosine of reflection.
//! @param    tangent   IN      The sine of reflection.
//! @param    i0  IN      The index of the first affected row.
//! @param    i1  IN      The index of the second affected row.
//! @param    pMatrix IN      The input matrix
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void        bsiRotMatrix_hyperbolicRowOp
(
RotMatrixP pProduct,
double        secant,
double        tangent,
int           i0,
int           i1,
RotMatrixCP pMatrix
);

//!
//! Apply a hyperbolic "column operation", i.e. pre-multiply by a hyperbolic reflection matrix
//! The matrix is an identity except for the 4 entries
//! R(i0,i0)=R(i1,i1)=secant
//! R(i0,i1)=R(i1,i0)=tangent
//! @param pProduct      OUT     product of givens rotation and input matrix.
//! @param    secant   IN      The cosine of reflection.
//! @param    tangent   IN      The sine of reflection.
//! @param    i0  IN      The index of the first affected row.
//! @param    i1  IN      The index of the second affected row.
//! @param    pMatrix IN      The input matrix
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void        bsiRotMatrix_hyperbolicColumnOp
(
RotMatrixP pProduct,
RotMatrixCP pMatrix,
double        secant,
double        tangent,
int           i0,
int           i1
);

//!
//! @description Compute a full rank matrix close to the input matrix.  Specifically,
//! form a singular value decomposition
//! pMatrix = U * Sigma * VT
//! For discussion, assume the null singular values are last.
//! If pMatrix has rank 1, pMatrix = U0 * sigma0 * V0T.   Form 2 vector B1, B2 perpendicular
//! to U0*sigma0.  Add B1 * V1T + V2 * V2T.
//! If pMatrix has rank 2, pMatrix = U0 * sigma0 * V0T + U1 * sigma1 * V1T.
//! Form B2 = (U0 * sigma0) cross (U1 * sigma1).  Add back B2 * V2T.
//! @param pFullRankMatrix OUT     augmented matrix.
//! @param pMatrix IN      original matrix.
//! @return number of deficient directions added in.
//! @group "RotMatrix Initialization"
//!
Public GEOMDLLIMPEXP int         bsiRotMatrix_augmentRank
(
RotMatrixP pFullRankMatrix,
RotMatrixCP pMatrix
);

//!
//! @description Factor a matrix as a product of a rotation and a skew factor.
//!    The rotation is defined (via parameters) as a coordiante frame with its xy axes
//!        aligned with specified columns of the input matrix.
//! @param pMatrixA IN matrix to query
//! @param pRotationMatrix OUT     the rotation factor.
//! @param pSkewMatrix OUT     the skew factor.
//! @param primaryAxis IN      this axis direction is preserved exactly.  (usually 0, x axis)
//! @param secondaryAxis IN      this axis direction is kept in plane of original primary and secondary
//! axes.  (usually 1, for  y axis)
//! @return false if singular matrix.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_rotateAndSkewFactors
(
RotMatrixCP pMatrixA,
RotMatrixP pRotationMatrix,
RotMatrixP pSkewMatrix,
int         primaryAxis,
int         secondaryAxis
);

//!
//! Return a crude 5-case summary of the z-direction components of a matrix.
//! @param pMatrix IN      matrix to analyze.
//! @return
//! <ul>
//! <li>  1 if complete effect of z entries is identity.
//! <li>  -1 if complete effect of z entries is to negate z
//! <li>  2 if complete effect of z entries is non-unit positive scale on z (zeros off diagonal)
//! <li>  -2 if complete efect of z entries is non-unit negative scale on z (zeros off diagonal)
//! <li>  0 if there are any off-diagonal nonzeros, so the matrix has z effects other than simple scaling.
//! </ul>
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP int bsiRotMatrix_summaryZEffects (RotMatrixCP pMatrix);

//!
//! Test if a matrix is "just" a rotation around z (i.e. in the xy plane)
//! @param pMatrix IN      matrix to analyze.
//! @param pRadians OUT     angle in radians.  This angle is the direction of column 0
//! of the matrix.
//! @return false if there are any non-rotational effects, or rotation is around any other axis.
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXYRotation
(
RotMatrixCP pMatrix,
double  *pRadians
);

//!
//! @Description Constructs a matrix from xy rotation, skew, and scale factors.
//! In this matrix, the first two columns represent the X and Y axes of a local coordinate
//! system.  These differ from global X and Y axes in both direction and length.
//! @param pMatrix OUT result matrix
//! @param xAxisAngle  IN Angle (in radians) from global x axis to local x axis.
//! @param yAxisSkewAngle IN (in radians) from a hypothetical y axis 90 CCW from the local axis
//! to the actual y axis represented in the matrix.
//! @param xScale IN (signed) scale factor to apply to x direction
//! @param yScale IN (signed) scale factor to apply to y direction.
//! @param zScale IN (signed) scale factor to apply to z direction.
//! @return Constructed matrix.
//! @remarks Various reflection (mirroring) effects can be introduced into the coordinate system
//! via negative xScale, negative yScale, or by ySkewAngle between 90 and 270 degrees
//!
//! (or, between -90 and -270)., or by negative zScale.
//! @group "RotMatrix Rotations"
//!
Public GEOMDLLIMPEXP void     bsiRotMatrix_initFromXYRotationSkewAndScale
(
RotMatrixP pMatrix,
double xAxisAngle,
double yAxisSkewAngle,
double xScale,
double yScale,
double zScale
);

//!
//! @Description Reinterprets the matrix as a combination of XY rotation, skew and scaling
//!    and Z scaling.
//! @param pMatrix IN matrix being analyzed.
//! @param pXAxisAngle OUT The angle, in radians counterclockwise, from the global x axis to the x axis (first column)
//!    of this matrix.
//! @param pYAxisSkewAngle OUT The angle, in radians counterclockwise, by which the Y axis of the matrix is skewed away from
//!    the usual position of 90 degrees counterclockwise from X.
//! @param pXScale OUT The length of the x axis vector of this matrix.  This scale factor is always positive.
//! @param pYScale OUT The length of the y axis vector of this matrix.  This scale factor may be positive or negative.
//!    If negative, the matrix operation includes mirroring (reflection).
//! @param pZScale OUT The (signed) scale factor which the matrix applies to z data.
//!    This is typically 1 or -1.
//! @return true if the matrix can be decomposed in this manner.  false if there are out-of-plane effects
//!    indicated by values in the z parts of the matrix.
//! @remarks If the matrix is "just" rotation around z, the two rotation angles will be identical and both scale
//!    factors are 1.0.
//! @remarks If the matrix is "just" rotation around z and uniform scaling, the two rotation angles will be identical
//!    and the scale factors will be identical to each other but different from 1.
//! @remarks In the non-mirrored case, the x direction is given by the first column of the matrix; the nominal
//!    y direction is 90 degrees counter clockwise, and the returned skew angle is the angle from that
//!    nominal y axis direction to the second column of the matrix.  In the mirrored case, the mirroring
//!    is indicated by a negated y scale factor, and the skew angle is measured from the same nominal
//!    y axis (90 degrees counterclockwise from first column) to the NEGATED second column.  That is,
//!    the returned numbers always represent a skewed-but-righthanded system, with reflection rolled into
//!    the y scale factor.
//! @group "RotMatrix Queries"
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXYRotationSkewAndScale
(
RotMatrixP pMatrix,
double          *pXAxisAngle,
double          *pYAxisSkewAngle,
double          *pXScale,
double          *pYScale,
double          *pZScale
);

//!
//! @description Compute angles such that a matrix is decomposed into a product
//!    of simple X, Y, and Z rotations and a uniform scale, i.e.
//! <p>
//!    Rotate(X,thetaX) * Rotate(Y,thetaY) * Rotate(Z, thetaZ) * uniformScale
//!        </p>
//!        where Rotate(axis,angle) is rotation matrix for specified axis and angle.
//!        This decompositon is sometimes called "Euler angles".  The user should be aware that
//!        Euler angles are of very limited value for computation.
//! @param pMatrix IN matrix to analyze.
//! @param pThetaX OUT rotation around X.
//! @param pThetaY OUT rotation around Y.
//! @param pThetaZ OUT rotation around Z.
//! @param pScale OUT scale factor.
//! @group "RotMatrix Queries"
//! @return true if the matrix is constructed of rotation and uniform scale.
//!    false if non-uniform scale.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXRotationYRotationZRotationScale
(
RotMatrixCP pMatrix,
double *pThetaX,
double *pThetaY,
double *pThetaZ,
double *pScale
);

//!
//! @param pEigenvectors IN OUT  Eigenvectors of matrix
//! @param pEigenvalues IN OUT  corresponding Eigenvalues
//! @param pOrder IN      pOrder[i] = original position of final i'th eigenvalue
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_sortEigensystem
(
RotMatrixP pEigenvectors,
DPoint3dP pEigenvalues,
int       *pOrder
);

END_BENTLEY_NAMESPACE

