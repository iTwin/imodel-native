/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/internal/dmatrix3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct GEOMDLLIMPEXP _dMatrix3d
{
DVec3d column[3];

#ifdef __cplusplus
//!
//! Copy all entries from a mdl-style matrix -
//!
void initFromRotMatrix (RotMatrixCP pRotMatrix);



//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint4dP pResult,
DPoint4dCP pPoint,
int            numPoint
) const;

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint2dP pResult,
DPoint2dCP pPoint,
int            numPoint
) const;

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
FPoint3dP pResult,
FPoint3dCP pPoint,
int            numPoint
) const;

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
FPoint2dP pResult,
FPoint2dCP pPoint,
int            numPoint
) const;

//!
//! @description Returns the product {A*B} of two matrices.
//! @param [in] pA The first factor
//! @param [in] pB The second factor
//!
void productOf
(
DMatrix3dCP pA,
DMatrix3dCP pB
);


//!
//! @description Returns the product of three matrices.
//! @param [in] pA The first factor
//! @param [in] pB The second factor
//! @param [in] pC The third factor
//!
void productOf
(
DMatrix3dCP pA,
DMatrix3dCP pB,
DMatrix3dCP pC
);


//!
//! Sets this instance to the product of matrix pA times the matrix part
//! of transform pB.
//! In this 'post multiplication' the matrix part of pB multiplies pA ON THE RIGHT
//! and the translation part of pB is ignored.
//!
//! @param [in] pA The first factor (matrix)
//! @param [in] pB The second factor (transform)
//!
void productOf
(
DMatrix3dCP pA,
DTransform3dCP pB
);


//!
//! @description Returns to the product of the matrix part of transform pA
//! times matrix pB.
//! In this 'pre multiplication' the matrix part of pA multiplies pB ON THE LEFT
//! and the translation part of pA is ignored.
//!
//! @param [in] pA The first factor (transform)
//! @param [in] pB The second factor (matrix)
//!
void productOf
(
DTransform3dCP pA,
DMatrix3dCP pB
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
//! @param [out] pOutRange range of transformed cube
//! @param [in] pInRange The any two corners of original cube
//!
void multiply
(
DRange3dP pOutRange,
DRange3dCP pInRange
) const;

//!
//! @description Returns an identity matrix.
//!
void initIdentity ();


//!
//! @description returns a zero matrix.
//!
void zero ();


//!
//! @description Returns a scaling matrix with respective x, y, and z
//!   scaling factors.
//! @param 'xscale => The x direction scale factor
//! @param [in] yscale The y direction scale factor
//! @param [in] zscale The z direction scale factor
//!
void initFromScaleFactors
(
double         xscale,
double         yscale,
double         zscale
);


//!
//! @description Returns a uniform scaling matrix.
//! @param [in] scale The scale factor.
//!
void initFromScale (double scale);


//!
//!
//! @description Returns a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//!
//! @param [out] pMatrix constructed matrix
//! @param [in] pVector1 The column vector U
//! @param [in] pVector2 The row vector V
//! @param [in] scale The scale factor
//!
void initFromScaledOuterProduct
(
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
);


//!
//!
//! @description Returns a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//!
//! @param [out] pMatrix constructed matrix
//! @param [in] pVector1 The column vector U
//! @param [in] pVector2 The row vector V
//! @param [in] scale The scale factor
//!
void addScaledOuterProductInPlace
(
DVec3dCP pVector1,
DVec3dCP pVector2,
double         scale
);


//!
//! Copies double arrays into corresponding columns of a matrix.
//! @param [in] pXVector The array to insert in column 0
//! @param [in] pYVector The array to insert in column 1
//! @param [in] pZVector The array to insert in column 2
//!
void initFromColumnArrays
(
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
);


//!
//! @description Returns a matrix with the 9 specified coefficients given
//!   in "row major" order.
//! @param [in] x00 The 00 entry
//! @param [in] x01 The 01 entry
//! @param [in] x02 The 02 entry
//! @param [in] x10 The 10 entry
//! @param [in] x11 The 11 entry
//! @param [in] x12 The 12 entry
//! @param [in] x20 The 20 entry
//! @param [in] x21 The 21 entry
//! @param [in] x22 The 22 entry
//!
void initFromRowValues
(
double      x00,
double      x01,
double      x02,
double      x10,
double      x11,
double      x12,
double      x20,
double      x21,
double      x22
);


//!
//! @description Returns a matrix with 3 points copied to respective columns.
//! @param [in] pXVector The vector to insert in column 0
//! @param [in] pYVector The vector to insert in column 1
//! @param [in] pZVector The vector to insert in column 2
//!
void initFromColumnVectors
(
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
);


//!
//! Copies double arrays into corresponding rows of a matrix.
//! @param [in] pXVector The array to insert in row 0
//! @param [in] pYVector The array to insert in row 1
//! @param [in] pZVector The array to insert in row 2
//!
void initFromRowArrays
(
const double        *pXVector,
const double        *pYVector,
const double        *pZVector
);


//!
//! @description Returns a matrix with 3 points copied to respective rows.
//! @param [in] pXVector The vector to insert in row 0
//! @param [in] pYVector The vector to insert in row 1
//! @param [in] pZVector The vector to insert in row 2
//!
void initFromRowVectors
(
DVec3dCP pXVector,
DVec3dCP pYVector,
DVec3dCP pZVector
);


//!
//! @description Returns a matrix representing rotation around a vector.
//! @param [in] pAxis The axis of rotation
//! @param [in] radians The rotation angle
//!
void initFromVectorAndRotationAngle
(
DVec3dCP pAxis,
double         radians
);


//!
//! @description Returns a matrix which scales along a vector direction.
//! @param [out] pMatrix scale matrix.
//! @param [in] pVector The scaling direction
//! @param [in] scale The scale factor
//!
void initFromDirectionAndScale
(
DVec3dCP pVector,
double         scale
);


//!
//! @description Returns a matrix of rotation  about the x,y, or z axis
//! (indicated by axis = 0,1, or 2) by an angle in radians.
//!
//! @param [in] axis The axis index 0=x, 1=y, 2=z
//! @param [in] radians The rotation angle in radians
//!
void initFromAxisAndRotationAngle
(
int            axis,
double         radians
);


//!
//! @description Returns the transpose of a matrix.
//! @param [out] pTranspose transposed matrix
//! @param [in] pMatrix The input matrix
//!
void transposeOf (DMatrix3dCP pMatrix);


//!
//! @description Transposes a matrix in place.
//!
void transpose ();


//!
//! @description Returns the inverse of the a matrix.
//! @param [in] pForward The input matrix
//! @return true if the matrix is invertible.
//!
bool    inverseOf (DMatrix3dCP pForward);


//!
//! @description Inverts this instance matrix in place.
//! @return true if the matrix is invertible.
//!
bool    invert ();


//!
//! @description Applies scale factors to corresponding rows of the input matrix, and places the result
//! in this instance matrix.
//! @param [out] pScaledMatrix scaled matrix
//! @param [in] pInMatrix The initial matrix
//! @param [in] xScale The scale factor for row 0
//! @param [in] yScale The scale factor for row 1
//! @param [in] zScale The scale factor for row 2
//!
void scaleRows
(
DMatrix3dCP pInMatrix,
double         xScale,
double         yScale,
double         zScale
);


//!
//! @description Applies scale factors to corresponding columns of the input matrix, and places the result
//! in this instance matrix.
//! @param [in] pIn The initial matrix
//! @param [in] xs The scale factor for column 0
//! @param [in] ys The scale factor for column 1
//! @param [in] zs The scale factor for column 2
//!
void scaleColumns
(
DMatrix3dCP pIn,
double         xs,
double         ys,
double         zs
);


//!
//! @description Returns a matrix formed from a scaling matrix which is
//!       multiplied on the left and/or right with other matrices.
//!       That is, form LeftMatrix * ScaleMatrix * RightMatrix
//!       where the ScaleMatrix is constructed from the given scale factors.
//!       If LeftMatrix is null, this has the effect of scaling the rows
//!       of the right matrix.  If RightMatrix is null this has the effect
//!       of scaling the columns of the left matrix.
//! @param [in] pLeftMatrix The matrix on left of product
//! @param [in] xs The x scale factor
//! @param [in] ys The y scale factor
//! @param [in] zs The z scale factor
//! @param [in] pLeftMatrix The matrix on right of product
//!
void scale
(
DMatrix3dCP pLeftMatrix,
double         xs,
double         ys,
double         zs,
DMatrix3dCP pRightMatrix
);


//!
//! @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
//! around X, Y, and Z axes, and M is the input matrix.
//! @param [in] pInMatrix The optional prior matrix
//! @param [in] xrot The x axis rotation
//! @param [in] yrot The y axis rotation
//! @param [in] zrot The z axis rotation
//!
void initFromPrincipleAxisRotations
(
DMatrix3dCP pInMatrix,
double         xrot,
double         yrot,
double         zrot
);


//!
//! @description Copies from columns of this instance matrix to corresponding points.
//! @param [out] pX first column
//! @param [out] pY second column
//! @param [out] pZ third column
//!
void getColumns
(
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
) const;

//!
//! @description Copies from rows of this instance matrix to corresponding points.
//! @param [out] pX first row
//! @param [out] pY second row
//! @param [out] pZ third row
//!
void getRows
(
DVec3dP pX,
DVec3dP pY,
DVec3dP pZ
) const;

//!
//! @description Set the components in a column.
//! @param [in] pPoint new values
//! @param [in] col The index of column to change. Column indices are 0, 1, 2.
//!
void setColumn
(
DVec3dCP pPoint,
int            col
);

//!
//! @description Set the components in a column.
//! @param [in] pPoint new values
//! @param [in] col The index of column to change. Column indices are 0, 1, 2.
//!
void SetColumn
(
DVec3dCR data,
int            col
)
    {
    col = Angle::Cyclic3dAxis (col);
    column[col] = data;
    }

//!
//! @description Set the components in a row.
//! @param [in] pPoint new values
//! @param [in] row The index of row to change. Row indices are 0, 1, 2.
//!
void setRow
(
DVec3dCP pPoint,
int            row
);


//!
//! @description Returns a point taken from a column of a matrix.
//! @param [out] pPoint filled point
//! @param [in] col The index of column to extract. Column indices are 0, 1, 2.
//!
void getColumn
(
DVec3dP pPoint,
int            col
) const;

//!
//! @description Returns a value from a specified row and column of the matrix.
//! @param [in] row The index of row to read. Row indices are 0, 1, 2.
//! @param [in] col The index of column to read.  Column indices are 0, 1, 2.
//!
double getComponentByRowAndColumn
(
int            row,
int          col
) const;

//!
//! @description Returns a vector taken from a column of a matrix.
//! @param [out] pPoint filled point
//! @param [in] row The index of row to extract.  Row indices are 0, 1, and 2.
//!
void getRow
(
DVec3dP pPoint,
int            row
) const;

//!
//! @description Returns a matrix whose rows are unit vectors in the same
//!   drection as corresponding rows of the input matrix.
//!   Also (optionally) stores the original row magnitudes as components of the point.
//! @param [in] pInMatrix The input matrix
//! @param [out] pScaleVector length of original rows
//!
void normalizeRowsOf
(
DMatrix3dCP pInMatrix,
DVec3dP pScaleVector
);


//!
//! @description Returns a matrix whose rows are unit vectors in the same
//!   drection as corresponding columns of the input matrix.
//!   Also (optionally) stores the original column magnitudes as components of the point.
//! @param [in] pInMatrix The input matrix
//! @param [out] pScaleVector length of original columns
//!
void normalizeColumnsOf
(
DMatrix3dCP pInMatrix,
DVec3dP pScaleVector
);


//!
//! @description Returns the determinant of the matrix.
//! @param [in] pMatrix The matrix to query
//! @return determinant of the matrix.
//!
double determinant () const;

//!
//! Computes an estimate of the condition of this instance matrix.  Values near 0
//! are bad.
//!
//! @param [in] pMatrix The input matrix
//! @return estimated condition number.
//!
double conditionNumber () const;

//!
//! @description Tests if a matrix is the identity matrix.
//! @param [in] pMatrix The matrix to test
//! @return true if matrix is approximately an identity.
//!
bool    isIdentity () const;

//!
//! @description Tests if a matrix has small offdiagonal entries compared to
//!                   diagonals.   The specific test condition is that the
//!                   largest off diagonal absolute value is less than a tight tolerance
//!                   fraction times the largest diagonal entry.
//! @param [in] pMatrix The matrix to test
//! @return true if matrix is approximately diagonal
//!
bool    isDiagonal () const;

//!
//! @description Tests if a matrix has (nearly) equal diagaonal entries and
//!           (nearly) zero off diagonals.  Tests use a tight relative tolerance.
//! @param [in] pMatrix The matrix to test
//! @param [out] pScale the largest diagaonal entry
//! @return true if matrix is approximately diagonal
//!
bool    isUniformScale
(
double      *pMaxScale
) const;

//!
//! @description Return the (signed) range of entries on the diagonal.
//! @param [in] pMatrix The matrix to test
//! @param [out] pMinValue smallest signed value
//! @param [out] pMaxValue largest signed value
//!
void diagonalSignedRange
(
double              *pMinValue,
double              *pMaxValue
) const;

//!
//! @description Return the (absolute value) range of entries on the diagonal.
//! @param [in] pMatrix The matrix to test
//! @param [out] pMinValue smallest absolute value
//! @param [out] pMaxValue largest absolute value
//!
void diagonalAbsRange
(
double              *pMinValue,
double              *pMaxValue
) const;

//!
//! @description Return the (signed) range of entries off the diagonal.
//! @param [in] pMatrix The matrix to test
//! @param [out] pMinValue smallest signed value
//! @param [out] pMaxValue largest signed value
//!
void offDiagonalSignedRange
(
double              *pMinValue,
double              *pMaxValue
) const;

//!
//! @description Return the (absolute value) range of entries off the diagonal.
//! @param [in] pMatrix The matrix to test
//! @param [out] pMinValue smallest absolute value
//! @param [out] pMaxValue largest absolute value
//!
void offDiagonalAbsRange
(
double              *pMinValue,
double              *pMaxValue
) const;

//!
//! @description Test if this instance matrix does nothing more
//! than exchange and possibly negate principle axes.
//! @param [in] pMatrix The input matrix
//! @return true if the matrix is a permutation of the principle axes.
//!
bool    isSignedPermutation () const;

//!
//! @description Test if a matrix is a rigid body rotation,
//!   i.e. its transpose is its inverse and it has a positive determinant.
//! @return true if the matrix is a rigid body rotation.
//!
bool    isRigid () const;

//!
//! @description Test if this instance matrix is orthogonal, i.e. its transpose is its inverse.
//!   This class of matrices includes both rigid body rotations and reflections.
//! @return true if the matrix is orthogonal.
//!
bool    isOrthogonal () const;

//!
//! @description Test if this instance matrix has orthonormal columns, i.e. its columns
//! are all perpendicular to one another.
//! @param [out] pColumns (optional) matrix containing the unit vectors along the columns.
//! @param [out] pAxisScales (optional) point whose x, y, and z components are the magnitudes of the
//!           original columns.
//! @param *pAxisRatio <= (optional) smallest axis length divided by largest.
//! @return true if the matrix is orthonormal.
//!
bool    isOrthonormal
(
DMatrix3dP pColumns,
DVec3dP pAxisScales,
double      *pAxisRatio
) const;

//!
//! @description Test if this instance matrix is composed of only rigid rotation and scaling.
//! @param [out] pColumns (optional) matrix containing the unit vectors along the columns.
//! @param *pMaxScale <= largest axis scale factor.  If function value is true,
//!       the min scale is the same.  Use areColumnsOrthonormal to get
//!       separate column scales.
//! @return true if the matrix is orthonormal.
//!
bool    isRigidScale
(
DMatrix3dP pColumns,
double      *pScale
) const;

//!
//! @description Tests if this instance matrix has no effects perpendicular to any plane with the given normal.  This
//! will be true if the matrix represents a combination of (a) scaling perpencicular to the normal
//! and (b) rotation around the normal.
//!
//! @param [in] pNormal The plane normal
//! @return true if the matrix has no effects perpendicular to any plane
//!   with the given normal.
//!
bool    isPlanar (DVec3dCP pNormal) const;

//!
//! @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
//! is aligned with the vector pDir.   The normalize flag selects between
//! normalized axes (all matrix columns of unit length) and unnormalized
//! axes (all matrix columns of same length as the pDir vector).
//!
//! @param [in] pDir The fixed direction vector
//! @param [in] axis The axis column to be aligned with direction
//! @param [in] normalize true to have normalized columns
//! @return true if the direction vector is nonzero.
//!
bool    initFrom1Vector
(
DVec3dCP pDir,
int            axis,
bool               normalize
);


//!
//! @description Copies vectors into the x and y columns of this instance matrix
//! and sets the z column to their cross product.
//! Use squareAndNormalizeColumns to force these (possibly
//! skewed and scaled) coordinate axes to be perpendicular
//!
//! @param [in] pXVector The vector to place in column 0
//! @param [in] pYVector The vector to place in column 1
//!
void initFrom2Vectors
(
DVec3dCP pXVector,
DVec3dCP pYVector
);


//!
//! @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
//! from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
//! on the positive side, and column 2 as their cross product.
//! @param [in] pOrigin The reference point
//! @param [in] pXPoint The x axis target point
//! @param [in] pYPoint The 3rd point defining xy plane.
//!
bool    initRotationFromOriginXY
(
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
//!
//! @param [in] pInMatrix The input matrix
//! @param [in] primaryAxis The axis id (0, 1, 2) which is to be normalized but left
//!           in its current direction
//! @param [in] secondaryAxis The axis id (0, 1, 2) which is to be kept within the
//!           plane of the primary and secondary axis.
//! @return false if primaryAxis and secondaryAxis are the same, or either axis has zero length
//!
bool    squareAndNormalizeColumns
(
DMatrix3dCP pInMatrix,
int            primaryAxis,
int            secondaryAxis
);


//!
//! @description Returns an orthogonal matrix that preserves aligns with
//! the columns of pInMatrix.  This is done by
//! trying various combinations of primary and secondary axes until one
//! succeeds in squareAndNormalizeColumns.
//!
//! @param [out] pMatrix normalized matrix
//! @param [in] pInMatrix The input matrix
//!
bool    squareAndNormalizeColumnsAnyOrder (DMatrix3dCP pInMatrix);


//!
//! @description Moves columns 0, 1, 2 of the input matrix into columns i0, i1, i2
//! of the instance.
//! @param [out] pMatrix shuffled matrix
//! @param [in] pInMatrix The input matrix
//! @param [in] i0 The column to receive input column 0
//! @param [in] i1 The column to receive input column 1
//! @param [in] i2 The column to receive input column 2
//!
void shuffleColumnsOf
(
DMatrix3dCP pInMatrix,
int            i0,
int            i1,
int            i2
);


//!
//! @description Returns {Matrix0 + Matrix1*s1+Matrix2*s2}, i.e. the sum of matrix M0,
//! matrix M1 multiplied by scale s1, and matrix M2 multiplied by scale s2.
//! Any combination of the matrix pointers may have identical addresses.
//! Any of the input matrix pointers may be NULL.
//! @param [in] pMatrix0 The matrix0 of formula
//! @param [in] pMatrix1 The matrix1 of formula
//! @param [in] scale1 The scale factor to apply to Matrix1
//! @param [in] pMatrix2 The matrix2 of formula
//! @param [in] scale2 The scale factor to apply to Matrix2
//!
void sumOf
(
DMatrix3dCP pMatrix0,
DMatrix3dCP pMatrix1,
double         scale1,
DMatrix3dCP pMatrix2,
double         scale2
);


//!
//! @description Add a matrix (componentwise, in place).
//!
//! @param [in] pDelta The matrix to add
//!
void add (DMatrix3dCP pDelta);


//!
//! @description Subtract a matrix (componentwise, in place).
//!
//! @param [in] pDelta The matrix to subtract
//!
void subtract (DMatrix3dCP pDelta);


//!
//! @description Return the sum of squares of coefficients in a matrix.
//! @return Sum of squares of all entries in matrix
//!
double sumSquares () const;

//!
//! @description Find the largest absolute value of entries in the matrix.
//! @return largest absolute value in matrix
//!
double maxAbs () const;

//!
//! @description Returns the largest absolute value difference between
//!   corresponding coefficients in Matrix1 and Matrix2.
//! @param [in] pMatrix2 The matrix to compare to
//! @return largest absolute difference between the two matrices.
//!
double maxDiff (DMatrix3dCP pMatrix2) const;

//!
//! @description Computes the sum of the squares of the diagonal entries of this instance matrix.
//! @return Sum of squares of diagonal entries
//!
double sumDiagonalSquares () const;

//!
//! @description Computes the sum of the squares of the off-diagonal entries of this instance matrix.
//! @return sum of square of off-diagonal entries of the matrix.
//!
double sumOffDiagonalSquares () const;

//!
//! @param [in] pQuatAsDoubles The quaternion, stored as (xyzw)
//! @param [in] transpose true if matrix is stored transposed
//!
void initFromQuaternion (DPoint4dCP pQuat);


//!
//! @param [out] pQuatAsDoubles quaternion, stored as xyzw
//! @param [in] transpose true if matrix is stored transposed
//!
void getQuaternion
(
DPoint4dP pQuat,
bool                transpose
) const;

//!
//! @param [out] pQuatAsDoubles quaternion, stored as (w,x,y,z) in an array of doubles.
//! @param [in] transpose true if matrix is stored transposed
//!
void getQuaternion
(
double              *pQuatAsDoubleArray,
bool                transpose
) const;

//!
//! @param [in] pQuatAsDoubles The quaternion, stored as (w,x,y,z) in an array of doubles.
//!
void initFromQuaternion
(
const double        *pQuatAsDoubleArray
);


//!
//! Returns the angle of rotation of this instance and sets pAxis to be the
//! normalized vector about which this instance rotates.
//! NOTE: this instance is assumed to be a (rigid body, i.e. orthogonal) rotation
//! matrix.
//! Since negating both angle and axis produces an identical rotation,
//! calculations are simplified by assuming (and returning) the angle in [0,Pi].
//!
//! @param [out] pAxis normalized axis of rotation
//! @return rotation angle (in radians) between 0 and Pi, inclusive
//!
double getRotationAngleAndVector (DVec3dP pAxis) const;

//!
//! Sets this instance matrix by copying the matrix part of the trasnform.
//! @param [out] pMatrix matrix part of transformation
//! @param [in] pTransform The transformation whose matrix part is returned
//!
void initFrom (DTransform3dCP pTransform);


//!
//! @description Sets this instance matrix by copying from the matrix parameter.
//! @param [in] pIn The source matrix
//!
void copy (DMatrix3dCP pIn);


//!
//! @description Tests for equality between two matrices
//! "Equality" means relative error less than 1.0e-12, in the sense that each
//! component-wise difference is less than 1.0e-12 times the largest absolute
//! value of the components of one matrix.
//!
//! @param [in] pMatrix2 The second matrix
//! @return   true if the matrices are identical.
//!
bool    isEqual (DMatrix3dCP pMatrix2) const;

//!
//! @description Tests for equality between two matrices.
//!
//! @param [in] pMatrix2 The second matrix
//! @param [in] tolerance The relative error tolerance.  Absolute tolerance is
//!           this (relative) tolerance times the largest absolute value in Matrix1.
//! @return true if the matrices are identical within tolerance.
//!
bool    isEqual
(
DMatrix3dCP pMatrix2,
double              tolerance
) const;

//!
//! Apply a Givens "row operation", i.e. pre-multiply by a Givens rotation matrix.
//! The Givens matrix is an identity except for the 4 rotational entries, viz
//!       R(i0,i0)=R(i1,i1)=c
//!       R(i0,i1)=s
//!       R(i1,i0)=-s
//!
//! @param [in] c The cosine of givens rotation.
//! @param [in] s The sine of givens rotation.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//! @param [in] pMatrix The input matrix
//!
void givensRowOp
(
double      c,
double      s,
int         i0,
int         i1,
DMatrix3dCP pMatrix
);


//!
//! Apply a Givens "column operation", i.e. post-multiply by a Givens rotation matrix.
//! The Givens matrix is an identity except for the 4 rotational entries, viz
//!       R(i0,i0)=R(i1,i1)=c
//!       R(i0,i1)=-s
//!       R(i1,i0)=s
//!
//! @param [in] pMatrix The input matrix
//! @param [in] c The cosine of givens rotation.
//! @param [in] s The sine of givens rotation.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//!
void givensColumnOp
(
DMatrix3dCP pMatrix,
double      c,
double      s,
int         i0,
int         i1
);


//!
//! Apply a hyperbolic "row operation", i.e. pre-multiply by a hyperbolic reflection matrix
//! The matrix is an identity except for the 4 entries
//!       R(i0,i0)=R(i1,i1)=secant
//!       R(i0,i1)=R(i1,i0)=tangent
//!
//! @param [in] secant The cosine of reflection.
//! @param [in] tangent The sine of reflection.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//! @param [in] pMatrix The input matrix
//!
void hyperbolicRowOp
(
double      secant,
double      tangent,
int         i0,
int         i1,
DMatrix3dCP pMatrix
);


//!
//! Apply a hyperbolic "column operation", i.e. pre-multiply by a hyperbolic reflection matrix
//! The matrix is an identity except for the 4 entries
//!       R(i0,i0)=R(i1,i1)=secant
//!       R(i0,i1)=R(i1,i0)=tangent
//!
//! @param [in] secant The cosine of reflection.
//! @param [in] tangent The sine of reflection.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//! @param [in] pMatrix The input matrix
//!
void hyperbolicColumnOp
(
DMatrix3dCP pMatrix,
double      secant,
double      tangent,
int         i0,
int         i1
);


#if !defined (DVEC3D_STRICT)

//!
//! Computes the product {M*P} where M is this instance matrix and P the input pPoint.
//! The result overwrites the previous coordinates in pPoint.
//!
//! @param [in,out] pPoint The point to be updated
//!
void multiply (DPoint3dP pPoint) const;

//!
//! @description Returns the product of a matrix times a point.
//! @param [out] pResult result of the multiplication.
//! @param [in] pPoint The known point.
//!
void multiply
(
DPoint3dP pResult,
DPoint3dCP pPoint
) const;

//!
//! @description Returns the product of a matrix transpose times a point.
//! @param [out] pResult result of the multiplication.
//! @param [in] pPoint The known point.
//!
void multiplyTranspose
(
DPoint3dP pResult,
DPoint3dCP pPoint
) const;

//!
//! @description Returns the product of a matrix times a point,
//!           with the point given as separate components.
//!
//! @param [out] pResult result of multiplication
//! @param [in] x The x component of input point
//! @param [in] y The y component of input point
//! @param [in] z The z component of input point
//!
void multiplyComponents
(
DPoint3dP pResult,
double         x,
double         y,
double         z
) const;

//!
//! @description Computes the product {P*M} where M is this instance matrix and P is the input point
//! The result overwrites the previous coordinates in the point.
//! @param [in,out] pPoint The point to be multiplied
//!
void multiplyTranspose (DPoint3dP pPoint) const;

//!
//! @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
//!           the product point.
//! @param [out] pPoint product point
//! @param [in] x The x component
//! @param [in] y The y component
//! @param [in] z The z component
//!
void multiplyTransposeComponents
(
DPoint3dP pResult,
double      x,
double      y,
double      z
) const;

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiply
(
DPoint3dP pResult,
DPoint3dCP pPoint,
int            numPoint
) const;

//!
//! Computes {P[i]*M} where M is this instance matrix and each P[i] is a point
//!   in the input array pPoint, transposed into row form.  (Equivalently, multiply M^*p[i], where M^ indicates transpose.)
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param [in] pMatrix The matrix to apply
//! @param [out] pResult output points
//! @param [in] pPoint The input points
//! @param [in] numPoint The number of points
//!
void multiplyTranspose
(
DPoint3dP pResult,
DPoint3dCP pPoint,
int            numPoint
) const;

//!
//! @description Return the product of a matrix inverse and a point.
//! @param [out] pResult the unknown point
//! @param [in] pPoint The The known point
//! @return false if this instance is singular.
//!
bool    solve
(
DPoint3dP pResult,
DPoint3dCP pPoint
) const;

//!
//! @description Return the product of a matrix inverse transpose and a point.
//! @param [out] pResult result of the multiplication
//! @param [in] pPoint The known point multipled by the matrix inverse.
//! @return false if this instance is singular.
//!
bool    solveTranspose
(
DPoint3dP pResult,
DPoint3dCP pPoint
) const;

//!
//! @description Solves the matrix equation AX=B, where A is this instance, B is the matrix
//! of numPoints input points and X is the matrix of numPoints output points.
//! pPoint and pResult may have identical addresses.
//!
//! @param [out] pResult column points of solution matrix to system
//! @param [in] pPoint The column points of constant matrix of system
//! @param [in] numPoints The number of input/output points
//! @return false if this instance is singular.
//!
bool    solveArray
(
DPoint3dP pResult,
DPoint3dCP pPoint,
int         numPoints
) const;
#endif
#endif
};

END_BENTLEY_GEOMETRY_NAMESPACE