/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


/*
#ifdef flexwiki
:Title: struct Bentley::RotMatrix

Summary: A RotMatrix is a 3x3 matrix.

#endif
*/

/*__PUBLISH_SECTION_START__*/

#ifndef rotmatrix_H_
#define rotmatrix_H_

BEGIN_BENTLEY_NAMESPACE
/**
3x3 matrix commonly used for pure rotations, but in general may also have scales and non-perpendicular rows and columns.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP RotMatrix
{
//! matrix coefficients.
double form3d[3][3];

#ifdef __cplusplus
//flex!! Construction and initialization

//flex|| computed matrix || result returned from static method || result placed in instance.  No use of prior instance contents. || modify instance in place ||

//flex|| Identity Matrix || outMatrix = RotMatrix.FromIdentity () || outMatrix.InitIdentity () ||

//! @description Returns an identity matrix.
static RotMatrix FromIdentity ();
//! @description Initialize an identity matrix.
void InitIdentity ();

//flex|| ZeroMatrix || || outMatrix.Zero () ||

//! @description returns a zero matrix.
void Zero ();



//flex|| uniform scale matrix || outMatrix = RotMatrix::FromScale (scale) || outMatrix.InitFromScale (scale) ||


//! @description Returns a uniform scaling matrix.
//! @param [in] scale The scale factor.
static RotMatrix FromScale (double scale);

//! @description Initializes a uniform scaling matrix.
//! @param [in] scale The scale factor.
//!
void InitFromScale (double scale);

//! @description Returns a (rotation) that is a right-handed signed permutation of axes.
//!<ul>
//!<li>The transform is described by directing the local u and v axes along positive or negative direction of any combination 
//!     of global axes.
//!<li>(0,1,0,1) creates an identity -- u along positive x, v along positive y.
//!<li>)0,1,2,1) points u axis along x, v axis along negative z.  {w=u cross v} is positive y.
//!<li>if the uAxisId and vAxisId are identical, the result is invalid (but will have u along that direction, v along the cyclic successor)
//!<ul>
//! @param [in] uAxisId the id (0,1,2) of the axis where the local u column points.
//! @param [in] uAxisSign Any positive number to point the u column in the forward direction of uAxisId, negative to point backward.
//! @param [in] vAxisId the id (0,1,2) of the axis where the local v column points.
//! @param [in] vAxisSign Any positive number to point the v column in the forward direction of xAxisId, negative to point backward.
static ValidatedRotMatrix FromPrimaryAxisDirections
(
int uAxisId,
int uAxisSign,
int vAxisId,
int vAxisSign
);



//flex|| Non-uniform scaling on principal axes || outMatrix = RotMatrix::FromScaleFactors (xScale, yScale, zScale) ||
//! @description Returns a scaling matrix with respective x, y, and z
//!   scaling factors.
//! @param [in] xscale The x direction scale factor (00 diagonal)
//! @param [in] yscale The y direction scale factor (11 diagonal)
//! @param [in] zscale The z direction scale factor (22 diagonal)
static RotMatrix FromScaleFactors
(
double          xscale,
double          yscale,
double          zscale
);

//! Returns a matrix which matches the effect of taking a cross product.
//!<ul>
//!<li>For vector U,   A*U = vector cross U = - (U cross vector)
//!<li>For vector U,  transpose (U) * A = tranpose (U cross vector) = -tranpose (vector cross U)
//!<li>Row X is [0,-Az,Ay]
//!<li>Row Y is [Ax,0,-Ax]
//!<li>Row Z is [-Ay,Ax,0]
//!</ul>
static RotMatrix FromCrossingVector (DVec3dCR vector);

//! @description Returns a scaling matrix with respective x, y, and z
//!   scaling factors.
//! @param [in] xscale The x direction scale factor
//! @param [in] yscale The y direction scale factor
//! @param [in] zscale The z direction scale factor
//!
void InitFromScaleFactors
(
double          xscale,
double          yscale,
double          zscale
);

//flex|| scale rows || || outMatrix.ScaleRows (matrixA, xScale, yScale, zScale) ||
//!
//! @description Applies scale factors to corresponding rows of the input matrix, and places the result
//! in this instance matrix.
//! @param [in] inMatrix The initial matrix
//! @param [in] xScale The scale factor for row 0
//! @param [in] yScale The scale factor for row 1
//! @param [in] zScale The scale factor for row 2
//!
void ScaleRows
(
RotMatrixCR     inMatrix,
double          xScale,
double          yScale,
double          zScale
);

//flex|| scale rows || || outMatrix.ScaleColumns (matrixA, xScale, yScale, zScale) || inoutmatrix.ScaleColumns (xScale, yScale, zScale) ||

//! @description Applies scale factors to corresponding columns of the input matrix, and places the result
//! in this instance matrix.
//! @param [in] in The initial matrix
//! @param [in] xs The scale factor for column 0
//! @param [in] ys The scale factor for column 1
//! @param [in] zs The scale factor for column 2
//!
void ScaleColumns
(
RotMatrixCR     in,
double          xs,
double          ys,
double          zs
);

//! @description Applies scale factors to corresponding columns
//! @param [in] xs The scale factor for column 0
//! @param [in] ys The scale factor for column 1
//! @param [in] zs The scale factor for column 2
void ScaleColumns
(
double xs,
double ys,
double zs
);


//flex|| normalize rows || || outMatrix.NormalizeRowsOf (matrixA, outVectorScales) ||
//!
//! @description Returns a matrix whose rows are unit vectors in the same
//!   drection as corresponding rows of the input matrix.
//!   Also (optionally) stores the original row magnitudes as components of the point.
//! @param [in] inMatrix The input matrix
//! @param [out] scaleVector length of original rows
//!
void NormalizeRowsOf
(
RotMatrixCR     inMatrix,
DVec3dR         scaleVector
);

//flex|| normalize columns || || outMatrix.NormalizeColumnsOf (matrixA, outVectorScales) ||

//! @description Returns a matrix whose rows are unit vectors in the same
//!   drection as corresponding columns of the input matrix.
//!   Also (optionally) stores the original column magnitudes as components of the point.
//! @param [in] inMatrix The input matrix
//! @param [out] scaleVector length of original columns
//!
void NormalizeColumnsOf
(
RotMatrixCR     inMatrix,
DVec3dR         scaleVector
);


//flex|| matrixA * xyzScales * matrixB || || outMatrix.Scale ||

//! @description Returns a matrix formed from a scaling matrix which is
//!       multiplied on the left and/or right with other matrices.
//!       That is, form LeftMatrix * ScaleMatrix * RightMatrix
//!       where the ScaleMatrix is constructed from the given scale factors.
//! @param [in] leftMatrix The matrix on left of product
//! @param [in] xs The x scale factor
//! @param [in] ys The y scale factor
//! @param [in] zs The z scale factor
//! @param [in] rightMatrix The matrix on right of product
//!
void Scale
(
RotMatrixCR     leftMatrix,
double          xs,
double          ys,
double          zs,
RotMatrixCR     rightMatrix
);



//flex|| outer product (column vector times row vector) || outMatrix = RotMatrix::FromScaledOuterProduct (vectorU, vectorV, scale) || outMatrix.InitFromScaledOuterProduct (vectorU, vectorV, scale) || inoutMatrix.AddScaledOuterProductInPlace(vectorU, vectorV, scale) ||

//! @description Returns a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//!
//! @param [in] vectorU The column vector U
//! @param [in] vectorV The row vector V
//! @param [in] scale The scale factor
//!
static RotMatrix FromScaledOuterProduct
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
double          scale
);


//! @description Returns a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//! @param [in] vectorU The column vector U
//! @param [in] vectorV The row vector V
//! @param [in] scale The scale factor
//!
void InitFromScaledOuterProduct
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
double          scale
);

//!
//!
//! @description Accumulates a 'rank one' matrix defined as a scale factor times
//! the 'vector outer product' of two vectors, i.e. as the matrix {s*U*V^T}
//!
//! @param [in] vectorU The column vector U
//! @param [in] vectorV The row vector V
//! @param [in] scale The scale factor
//!
void AddScaledOuterProductInPlace
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
double          scale
);

//flex|| Shuffle columns of source || || outMatrix.ShuffleColumnsOf (matrix, column0DestIndex, column1DestIndex, column2DestIndex) ||

//! @description Moves columns 0, 1, 2 of the input matrix into columns i0, i1, i2
//! of the instance.
//! @param [in] inMatrix The input matrix
//! @param [in] i0 The column to receive input column 0
//! @param [in] i1 The column to receive input column 1
//! @param [in] i2 The column to receive input column 2
//!
void ShuffleColumnsOf
(
RotMatrixCR     inMatrix,
int             i0,
int             i1,
int             i2
);


//flex|| complete list of values going across rows || outMatrix = RotMatrix ::FromRowValues (a00, a01, a02, a10, a11, a12, a20, a21, a22) || outMatrix.InitFromRowValues (....) ||

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
static RotMatrix FromRowValues
(
double          x00,
double          x01,
double          x02,
double          x10,
double          x11,
double          x12,
double          x20,
double          x21,
double          x22
);

//! @description Initializes a matrix with the 9 specified coefficients given
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
void InitFromRowValues
(
double          x00,
double          x01,
double          x02,
double          x10,
double          x11,
double          x12,
double          x20,
double          x21,
double          x22
);

//flex|| xy parts given, z parts from identity || || outMatrix.InitFromRowValuesXY (a00, a01, a10, a11) ||
//flex|| || || outMatrix.InitFromRowValuesXY (data[4]) ||


//!
//! @description Initializes a matrix with the 4 specified coefficients in xx,xy,yx,yy positions, and 1 in zz
//!   in "row major" order.
//! @param [in] x00 The 00 entry
//! @param [in] x01 The 01 entry
//! @param [in] x10 The 10 entry
//! @param [in] x11 The 11 entry
//!
void InitFromRowValuesXY
(
double          x00,
double          x01,
double          x10,
double          x11
);


//! @description Initializes a matrix with 4 coefficients in xx,xy,yx,yy positions, and 1 in zz
//!   in "row major" order.
//! @param [in] data matrix entries in order xx,xy,yx,yy
//!
void InitFromRowValuesXY
(
double const data[4]
);

//flex|| copy vectors to columns || outMatrix = RotMatrix::FromColumnVectors (vectorU, vectorV, vectorW) || outMatrix.InitFromColumVectors (vectorU, vectorV, vectorW) ||
//!
//! @description Returns a matrix with 3 vectors copied to respective columns.
//! @param [in] vectorU The vector to insert in column 0
//! @param [in] vectorV The vector to insert in column 1
//! @param [in] vectorW The vector to insert in column 2
//!
static RotMatrix FromColumnVectors
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
DVec3dCR        vectorW
);

//! @description Returns a matrix with 3 points copied to respective columns.
//! @param [in] vectorU The vector to insert in column 0
//! @param [in] vectorV The vector to insert in column 1
//! @param [in] vectorW The vector to insert in column 2
//!
void InitFromColumnVectors
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
DVec3dCR        vectorW
);

//flex|| copy vectors to rows || outMatrix = RotMatrix::FromRowVectors (vectorU, vectorV, vectorW) || outMatrix.InitFromRowVectors (vectorU, vectorV, vectorW) ||

//! @description Returns a matrix with 3 vectors copied to respective rows.
//! @param [in] vectorU The vector to insert in row 0
//! @param [in] vectorV The vector to insert in row 1
//! @param [in] vectorW The vector to insert in row 2
//!
static RotMatrix FromRowVectors
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
DVec3dCR        vectorW
);

//! @description Initializes a matrix with 3 points copied to respective rows.
//! @param [in] vectorU The vector to insert in row 0
//! @param [in] vectorV The vector to insert in row 1
//! @param [in] vectorW The vector to insert in row 2
//!
void InitFromRowVectors
(
DVec3dCR        vectorU,
DVec3dCR        vectorV,
DVec3dCR        vectorW
);

//flex|| rotation around a vector || outMatrix = RotMatrix::FromVectorAndRotationAngle (vectorAxis, radians) || outMatrix.InitFromVectorAndRotationAngle (vectorAxis, radians) ||

//! @description Returns a matrix representing rotation around a vector.
//! @param [in] axis The axis of rotation
//! @param [in] radians The rotation angle
//!
static RotMatrix FromVectorAndRotationAngle (DVec3dCR axis, double radians);

//!
//! @description Initializes a matrix representing rotation around a vector.
//! @param [in] axis The axis of rotation
//! @param [in] radians The rotation angle
//!

void InitFromVectorAndRotationAngle
(
DVec3dCR        axis,
double          radians
);


//flex|| rotation around a vector, with derivative matrix || outMatrix = RotMatrix::FromVectorAndRotationAngle (vectorAxis, radians, outDerivativeMatrix) ||

//!
//! @description Returns a matrix representing rotation around a vector.
//!   Also returns its derivatives with respect to the angle.
//! @param [in] axis The axis of rotation
//! @param [in] radians The rotation angle
//! @param [out] derivativeMatrix derivative of returned matrix.
static RotMatrix FromVectorAndRotationAngle
(
DVec3dCR axis,
double   radians,
RotMatrixR derivativeMatrix
);




//flex|| Rotation around an (indexed) principal axis || outMatrix = RotMatrix::FromAxisAndRotationAngle || outMatrix.InitFroMAxisAndRotationAngle ||

//! @description Returns a matrix of rotation  about the x,y, or z axis
//! (indicated by axis = 0,1, or 2) by an angle in radians.
//!
//! @param [in] axis The axis index 0=x, 1=y, 2=z
//! @param [in] radians The rotation angle in radians
static RotMatrix FromAxisAndRotationAngle
(
int             axis,
double          radians
);


//!
//! @description Returns a matrix of rotation  about the x,y, or z axis
//! (indicated by axis = 0,1, or 2) by an angle in radians.
//!
//! @param [in] axis The axis index 0=x, 1=y, 2=z
//! @param [in] radians The rotation angle in radians
//!
void InitFromAxisAndRotationAngle
(
int             axis,
double          radians
);

//! @description (conditionally) initialize the instance as the (smallest) rotation that
//! moves startVector so it is in the direction of endVector.  In the normal case where
//! the vectors are not parallel or antiparallel, this is a rotation around their cross
//! product.
//! @return false if one or both are zero vectors.
//! @remark if the vectors are direction opposite, the rotation is around an arbitrarily
//    chosen perpendicular vector.
bool InitRotationFromVectorToVector
(
DVec3dCR startVector,  //!< [in] starting vector
DVec3dCR endVector     //!< [in] ending vector.
);

//flex|| rotation around a vector, by 90 degrees || outMatrix outMatrix = RotMatrix::FromRotate90 (vectorAxis) ||


//! Return a matrix for retation of 90 degrees about a vector.
static RotMatrix FromRotate90 (DVec3dCR axis);


//flex|| scaling along a single direction || outMatrix outMatrix = RotMatrix::FromDirectionAndScale (vector, scale) || outMatrix.InitFromDirectionAndScale (vector, scale) ||

//! @description Returns a matrix which scales along a vector direction.
//! @param [in] vector The scaling direction
//! @param [in] scale The scale factor
static RotMatrix FromDirectionAndScale (DVec3dCR vector, double scale);

//!
//! @description Initializes a matrix which scales along a vector direction.
//! @param [in] vector The scaling direction
//! @param [in] scale The scale factor
//!
void InitFromDirectionAndScale
(
DVec3dCR        vector,
double          scale
);


//!
//! @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
//! around X, Y, and Z axes, and M is the input matrix.
//! @param [in] inMatrix The prior matrix
//! @param [in] xrot The x axis rotation
//! @param [in] yrot The y axis rotation
//! @param [in] zrot The z axis rotation
//!
static RotMatrix FromPrincipleAxisRotations
(
RotMatrixCR     inMatrix,
double          xrot,
double          yrot,
double          zrot
);


//!
//! @description Returns the product {RX*RY*RZ*M} where RX, RY, and RZ are rotations (in radians)
//! around X, Y, and Z axes, and M is the input matrix.
//! @param [in] inMatrix The prior matrix
//! @param [in] xrot The x axis rotation
//! @param [in] yrot The y axis rotation
//! @param [in] zrot The z axis rotation
//!
void InitFromPrincipleAxisRotations
(
RotMatrixCR     inMatrix,
double          xrot,
double          yrot,
double          zrot
);

//flex|| one column given, others constructed perpendicular || outMatrix = RotMatrix::From1Vector (vector, axisIndex, bool normalize) || outMatrix.InitFrom1Vector (vector, axisIndex, bool normalize) ||

//! @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
//! is aligned with the vector dir.   The normalize flag selects between
//! normalized axes (all matrix columns of unit length) and unnormalized
//! axes (all matrix columns of same length as the dir vector).
//!
//! @param [in] dir The fixed direction vector
//! @param [in] axis The axis column to be aligned with direction
//! @param [in] normalize true to have normalized columns
//! @return true if the direction vector is nonzero.
//!
static RotMatrix From1Vector
(
DVec3dCR        dir,
int             axis,
bool            normalize
);

//!
//! @description Initializes this instance matrix so that the indicated axis (axis = 0,1,or 2)
//! is aligned with the vector dir.   The normalize flag selects between
//! normalized axes (all matrix columns of unit length) and unnormalized
//! axes (all matrix columns of same length as the dir vector).
//!
//! @param [in] dir The fixed direction vector
//! @param [in] axis The axis column to be aligned with direction
//! @param [in] normalize true to have normalized columns
//! @return true if the direction vector is nonzero.
//!
bool InitFrom1Vector
(
DVec3dCR        dir,
int             axis,
bool            normalize
);

//flex|| x and y columns given. || outMatrix = RotMatrix::From2Vectors (vectorX, vectorY) || outMatrix.InitFrom2Vectors (vectorX, vectorY) ||

//! @description Returns a matrix copying vectors into the x and y columns of this instance matrix
//! and sets the z column to their cross product.
//! Use squareAndNormalizeColumns to force these (possibly
//! skewed and scaled) coordinate axes to be perpendicular
//!
//! @param [in] xVector The vector to place in column 0
//! @param [in] yVector The vector to place in column 1
//!
static RotMatrix From2Vectors
(
DVec3dCR        xVector,
DVec3dCR        yVector
);

//!
//! @description Copies vectors into the x and y columns of this instance matrix
//! and sets the z column to their cross product.
//! Use squareAndNormalizeColumns to force these (possibly
//! skewed and scaled) coordinate axes to be perpendicular
//!
//! @param [in] xVector The vector to place in column 0
//! @param [in] yVector The vector to place in column 1
//!
void InitFrom2Vectors
(
DVec3dCR        xVector,
DVec3dCR        yVector
);

//flex|| origin given, x direction given, y bias point given || outMatrix = RotMatrix::RotationFromOriginXY (origin, pointX, pointXY) || outMatrix.InitRotationFromOriginXY (origin, pointX, pointY) ||

//! @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
//! from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
//! on the positive side, and column 2 as their cross product.
//! @param [in] origin The reference point
//! @param [in] xPoint The x axis target point
//! @param [in] yPoint The 3rd point defining xy plane.
//!
static RotMatrix RotationFromOriginXY
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);

//!
//! @description Set this instance matrix to be an orthogonal (rotation) matrix with column 0 in the direction
//! from the origin to the x point, column 1 in the plane of the 3 points, directed so the Y point
//! on the positive side, and column 2 as their cross product.
//! @param [in] origin The reference point
//! @param [in] xPoint The x axis target point
//! @param [in] yPoint The 3rd point defining xy plane.
//!
bool InitRotationFromOriginXY
(
DPoint3dCR      origin,
DPoint3dCR      xPoint,
DPoint3dCR      yPoint
);

//flex|| Square and normalize columns || || bool outMatrix.SquareAndNormalizeColumns (matrix, primaryAxisIndex, secondaryAxisIndex) ||

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
//! inMatrix and pMatrix may be the same address.
//!
//! @param [in] inMatrix The input matrix
//! @param [in] primaryAxis The axis id (0, 1, 2) which is to be normalized but left
//!           in its current direction
//! @param [in] secondaryAxis The axis id (0, 1, 2) which is to be kept within the
//!           plane of the primary and secondary axis.
//! @return false if primaryAxis and secondaryAxis are the same, or either axis has zero length
//!
bool SquareAndNormalizeColumns
(
RotMatrixCR     inMatrix,
int             primaryAxis,
int             secondaryAxis
);

bool SquareAndNormalizeColumns
(
RotMatrixCR     inMatrix,
int             primaryAxis,
int             secondaryAxis,
int             preferredOrientation
);

//!
//! @description Returns an orthogonal matrix that preserves aligns with
//! the columns of inMatrix.  This is done by
//! trying various combinations of primary and secondary axes until one
//! succeeds in squareAndNormalizeColumns.
//!
//! @param [in] inMatrix The input matrix
//! @param [in] preferredOrientation 
//! <pre>
//! <ul>
//! <li>1 for right handed system
//! <li>-1 for left handed system
//! <li>0 to match orientation of input (but default to right handed if input is singular)
//! </ul>
//! </pre>
//!
bool SquareAndNormalizeColumnsAnyOrder (RotMatrixCR inMatrix, int preferredOrientation);

//!
//! @description Returns an orthogonal matrix that preserves aligns with
//! the columns of inMatrix.  This is done by
//! trying various combinations of primary and secondary axes until one
//! succeeds in squareAndNormalizeColumns.
//!
//! @param [in] inMatrix The input matrix
bool SquareAndNormalizeColumnsAnyOrder (RotMatrixCR inMatrix);


//flex|| matrix part of transform || outMatrix = RotMatrix::From (transform) || outMatrix.InitFrom (transform) ||

//! @description Returns a matrix copying the matrix part of the trasnform.
//! @param [in] transform The transformation whose matrix part is returned
static RotMatrix From (TransformCR transform);

//flex|| simple copy || || outMatrix.copy(matrix) ||

//! Sets this instance matrix by copying the matrix part of the trasnform.
//! @param [in] transform The transformation whose matrix part is returned
void InitFrom (TransformCR transform);

//!
//! @description Sets this instance matrix by copying from the matrix parameter.
//! @param [in] in The source matrix
//!
void Copy (RotMatrixCR in);




//flex|| matrixA * matrixB || || outMatrix.InitProduct (matrixA, matrixB) ||


//!
//! @description Returns the product {A*B} of two matrices.
//! @param [in] rotMatrixA The first factor
//! @param [in] rotMatrixB The second factor
//!
void InitProduct
(
RotMatrixCR     rotMatrixA,
RotMatrixCR     rotMatrixB
);

//flex|| matrixA * matrixB * matrixC || || outMatrix.InitProduct (matrixA, matrixB, matrixC) ||


//!
//! @description Returns the product of three matrices.
//! @param [in] rotMatrixA The first factor
//! @param [in] rotMatrixB The second factor
//! @param [in] rotMatrixC The third factor
//!
void InitProduct
(
RotMatrixCR     rotMatrixA,
RotMatrixCR     rotMatrixB,
RotMatrixCR     rotMatrixC
);

//flex|| matrixA * transforrmB || || outMatrix.InitProduct (matrixA, transformB) ||

//! Sets this instance to the product of matrix rotMatrixA times the matrix part
//! of transform transformB.
//! In this 'post multiplication' the matrix part of transformB multiplies rotMatrixA ON THE RIGHT
//! and the translation part of transformB is ignored.
//!
//! @param [in] rotMatrixA The first factor (matrix)
//! @param [in] transformB The second factor (transform)
//!
void InitProduct
(
RotMatrixCR     rotMatrixA,
TransformCR     transformB
);

//flex|| transformA * matrixB || || outMatrix.InitProduct (transformA, matrixB) ||


//!
//! @description Returns to the product of the matrix part of transform transformA
//! times matrix rotMatrixB.
//! In this 'pre multiplication' the matrix part of transformA multiplies rotMatrixB ON THE LEFT
//! and the translation part of transformA is ignored.
//!
//! @param [in] transformA The first factor (transform)
//! @param [in] rotMatrixB The second factor (matrix)
//!
void InitProduct
(
TransformCR     transformA,
RotMatrixCR     rotMatrixB
);

//flex|| matrixA^Transpose * matrixB || || outMatrix.InitProductRotMatrixRotMatrixTranspose (matrixA, matrixB) ||
//! @description Returns the product of the transpose of rotMatrixA times rotMatrixB
//! @param [in] rotMatrixA The first factor (to be transposed)
//! @param [in] rotMatrixB The second factor
void InitProductRotMatrixTransposeRotMatrix (RotMatrixCR rotMatrixA, RotMatrixCR rotMatrixB);

//flex|| matrixA * matrixB^Transpose || || outMatrix.InitProductRotMatrixRotMatrixTranspose (matrixA, matrixB) ||

//! @description Returns the product of rotMatrixA times the transpose of rotMatrixB
//! @param [in] rotMatrixA The first factor
//! @param [in] rotMatrixB The second factor (to be transposed)
void InitProductRotMatrixRotMatrixTranspose (RotMatrixCR rotMatrixA, RotMatrixCR rotMatrixB);

//flex|| matrix ^Transpose || || outMatrix.TransposeOf (matrixA) || inoutMatrix.Transpose () ||

//! @description Initializes this instance as the transpose of a matrix.
//! @param [in] matrix The input matrix
//!
void TransposeOf (RotMatrixCR matrix);

//! @description Transposes a matrix in place.
void Transpose ();

//! @description Return (as function value) the transpose of a matrix.
//! @param [in] matrix The input matrix
//!
static RotMatrix FromTransposeOf (RotMatrixCR matrix);

//flex|| Sum of matrices || || outMatrix.SumOf (matrixA, matrixB, scaleB, matrixC, scaleC) ||
//flex|| || || || inoutMatrix.Add (matrixB) ||
//flex|| || || || inoutMatrix.Subtract (matrixB) ||

//! @description Returns {Matrix0 + Matrix1*s1+Matrix2*s2}, i.e. the sum of matrix M0,
//! matrix M1 multiplied by scale s1, and matrix M2 multiplied by scale s2.
//! Any combination of the matrix pointers may have identical addresses.
//! @param [in] matrix0 The matrix0 of formula
//! @param [in] matrix1 The matrix1 of formula
//! @param [in] scale1 The scale factor to apply to Matrix1
//! @param [in] matrix2 The matrix2 of formula
//! @param [in] scale2 The scale factor to apply to Matrix2
//!
void SumOf
(
RotMatrixCR     matrix0,
RotMatrixCR     matrix1,
double          scale1,
RotMatrixCR     matrix2,
double          scale2
);

//! @description Add a matrix (componentwise, in place).
//! @param [in] delta The matrix to add
void Add (RotMatrixCR delta);

//! @description Subtract a matrix (componentwise, in place).
//! @param [in] delta The matrix to subtract
void Subtract (RotMatrixCR delta);



//flex|| Inverse || || bool outMatrix.InverseOf (matrixA) || bool inoutMatrix.Invert () ||

//! @description Returns the inverse of the a matrix.
//! @param [in] forward The input matrix
//! @return true if the matrix is invertible.
bool InverseOf (RotMatrixCR forward);

//!
//! @description Inverts this instance matrix in place.
//! @return true if the matrix is invertible.
//!
bool Invert ();


//flex!! Multiplying other types

//flex Warning: "point" inputs should technically be vectors, but are given as points for code compatibility

//flex|| operation || inplace form || separated input and output || other ||
//flex|| matrix * point || matrix.Multiply (inoutpoint) || matrix.multiply (outpoint, inpoint) || matrix.Multiply (outpoint, x, y, z) ||
//flex|| matrix^Transpose * point || matrix.MultiplyTranspose (inoutpoint) || matrix.multiplyTranpose (outpoint, inpoint) || matrix.MultiplyTranspose (outpoint, x, y, z) ||
//flex|| solve matrix*X = B || || matrix.Solve (X, B) ||
//flex|| solve A^Transpose *X = B || || matrix.SolveTranspose (X, B) ||
//flex|| matrix * [array] || || matrix.Multiply (bvector<DPoint3d> outpoints, bvector<DPoint3d>inpoints) || matrix.Multiply (outpoints[], inpoints[], n) ||
//flex|| matrix^Transpose * [array] || || matrix.MultiplyTranpose (bvector<DPoint3d> outpoints, bvector<DPoint3d>inpoints) || matrix.MultiplyTranspose (outpoints[], inpoints[], n) ||
//flex|| Solve matrix * pointX[] = pointB[] || || matrix.Solve (bvector<DPoint3d> outpoints, bvector<DPoint3d>inpoints) || matrix.Solve (outpoints[], inpoints[], n) ||

//flex|| array of DPoint4d || || matrix.Multiply (bvector<DPoint4d> outpoints, bvector<DPoint4d>inpoints) || matrix.Multiply (resultPoints4d, points4d, n) ||
//flex|| array of DPoint2d || || matrix.Multiply (bvector<DPoint2d> outpoints, bvector<DPoint2d>inpoints) || matrix.Multiply (resultPoints2d, points2d, n) ||
//flex|| corners of range box || || matrix.Multiply (outRange, range) ||

//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
//! Each result is placed in the corresponding entry in the output array result.   The same array
//! may be named for the input and output arrays.
//! @param [out] result output points
//! @param [in] point The input points
//! @param [in] numPoint The number of points
//!
void Multiply
(
DPoint4dP       result,
DPoint4dCP      point,
int             numPoint
) const;


//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
//! Each result is placed in the corresponding entry in the output array result.   The same array
//! may be named for the input and output arrays.
//! @param [out] result output points
//! @param [in] point The input points
//! @param [in] numPoint The number of points
//!
void Multiply
(
DPoint2dP       result,
DPoint2dCP      point,
int             numPoint
) const;



//! computes the 8 corner points of the range cube defined by the two points
//! of pRange, multiplies matrix times each point, and computes the
//! twopoint range limits (min max) of the resulting rotated box.
//! Note that the volume of the rotated box is generally larger than
//! that of the input because the postrotation minmax operation expands
//! the box.
//! Ranges that are null as defined by inRange->isNull are left
//! unchanged.
//! @param [out] outRange range of transformed cube
//! @param [in] inRange Any two corners of original cube
//!
void Multiply
(
DRange3dR       outRange,
DRange3dCR      inRange
) const;


//!
//! Computes the product {M*P} where M is this instance matrix and P the input point.
//! The result overwrites the previous coordinates in point.
//!
//! @param [in,out] point The point to be updated
//!
void Multiply (DPoint3dR point) const;

//!
//! @description Returns the product of a matrix times a point.
//! @param [out] result result of the multiplication.
//! @param [in] point The known point.
//!
void Multiply
(
DPoint3dR       result,
DPoint3dCR      point
) const;

//!
//! @description Returns the product of a matrix transpose times a point.
//! @param [out] result result of the multiplication.
//! @param [in] point The known point.
//!
void MultiplyTranspose
(
DPoint3dR       result,
DPoint3dCR      point
) const;

//!
//! @description Returns the product of a matrix times a point,
//!           with the point given as separate components.
//!
//! @param [out] result result of multiplication
//! @param [in] x The x component of input point
//! @param [in] y The y component of input point
//! @param [in] z The z component of input point
//!
void MultiplyComponents
(
DPoint3dR       result,
double          x,
double          y,
double          z
) const;

//!
//! @description Computes the product {P*M} where M is this instance matrix and P is the input point
//! The result overwrites the previous coordinates in the point.
//! @param [in,out] point The point to be multiplied
//!
void MultiplyTranspose (DPoint3dR point) const;

//!
//! @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
//!           the product point.
//! @param [out] result product point
//! @param [in] x The x component
//! @param [in] y The y component
//! @param [in] z The z component
//!
void MultiplyTransposeComponents
(
DPoint3dR       result,
double          x,
double          y,
double          z
) const;

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array point.
//! Each result is placed in the corresponding entry in the output array result.   The same array
//! may be named for the input and output arrays.
//! @param [out] result output points
//! @param [in] point The input points
//! @param [in] numPoint The number of points
//!
void Multiply
(
DPoint3dP       result,
DPoint3dCP      point,
int             numPoint
) const;

//!
//! Computes {P[i]*M} where M is this instance matrix and each P[i] is a point
//!   in the input array point, transposed into row form.  (Equivalently, multiply M^*p[i], where M^ indicates transpose.)
//! Each result is placed in the corresponding entry in the output array result.   The same array
//! may be named for the input and output arrays.
//! @param [out] result output points
//! @param [in] point The input points
//! @param [in] numPoint The number of points
//!
void MultiplyTranspose
(
DPoint3dP       result,
DPoint3dCP      point,
int             numPoint
) const;

//! Description Multiply points. (Matrix on left, point on right as column vector)
void Multiply
(
bvector<DPoint3d> &outXYZ,         //!< [in] transformed coordinates.
bvector<DPoint3d> const &inXYZ      //!< [in] input coordinates
) const;

//! @description Multiply the transposed matrix times points.  (Tranposed matrix on left, point on right as column vector.  Or (equivalent) point on left as row vector, matrix on right)
void MultiplyTranspose
(
bvector<DPoint3d> &outXYZ,        //!< [in] transformed coordinates.
bvector<DPoint3d> const &inXYZ      //!< [in] input coordinates
) const;

//! Description Multiply 2D points. (Matrix on left, point on right as column vector)
void Multiply
(
bvector<DPoint2d> &out,        //!< [in] transformed coordinates.  z row of matrix ignored.
bvector<DPoint2d> const &in     //!< [in] input coordinates.   z=0 implied for multiplication.
) const;

//! @description Multiply xyzw, with matrix w row and column 0001.
void Multiply
(
bvector<DPoint4d> &xyzwOut,         //!< [in] transformed coordinates.
bvector<DPoint4d> const &xyzwIn     //!< [in] input coordinates.
) const;

//! @description Solve M*xyzOut[i] = xyzIn[i] for array of points.
//! (Equivalent to multiplying by the matrix inverse)
bool SolveArray
(
bvector<DPoint3d> &xyzOut,             //!< [in] solution coordinates
bvector<DPoint3d> const &xyzIn         //!< [in] input coordinates.
) const;

//!
//! @description Return the product of a matrix inverse and a point.
//! @param [out] result the unknown point
//! @param [in] point The The known point
//! @return false if this instance is singular.
//!
bool Solve
(
DPoint3dR       result,
DPoint3dCR      point
) const;

//!
//! @description Return the product of a matrix inverse transpose and a point.
//! @param [out] result result of the multiplication
//! @param [in] point The known point multipled by the matrix inverse.
//! @return false if this instance is singular.
//!
bool SolveTranspose
(
DPoint3dR     result,
DPoint3dCR    point
) const;

//!
//! @description Solves the matrix equation AX=B, where A is this instance, B is the matrix
//! of numPoints input points and X is the matrix of numPoints output points.
//! point and result may have identical addresses.
//!
//! @param [out] result column points of solution matrix to system
//! @param [in] point The column points of constant matrix of system
//! @param [in] numPoints The number of input/output points
//! @return false if this instance is singular.
//!
bool SolveArray
(
DPoint3dP       result,
DPoint3dCP      point,
int             numPoints
) const;


//flex!! Queries

//flex|| || || by column || by row ||

//flex|| vectors || || matrix.GetColumns (vectorU, vectorV, vectorW) || matrix.GetRows (vectrorU, vectorV, vectorW) ||



//! @description Copies from columns of this instance matrix to corresponding points.
//! @param [out] vectorU first column
//! @param [out] vectorV second column
//! @param [out] vectorW third column
//!
void GetColumns
(
DVec3dR         vectorU,
DVec3dR         vectorV,
DVec3dR         vectorW
) const;

//!
//! @description Copies from rows of this instance matrix to corresponding points.
//! @param [out] vectorU first row
//! @param [out] vectorV second row
//! @param [out] vectorW third row
//!
void GetRows
(
DVec3dR         vectorU,
DVec3dR         vectorV,
DVec3dR         vectorW
) const;

//flex|| Single vector || || matrix.GetColumn (vector, index) || matrix.GetRow (vector, index) ||

//flex|| || || matrix.SetColumn (vector, index) || matrix.SetRow (vector, index) ||

//! @description Returns a point taken from a column of a matrix.
//! @param [out] vector filled vector
//! @param [in] col The index of column to extract. Column indices are 0, 1, 2.
void GetColumn
(
DVec3dR         vector,
int             col
) const;

//! @description Returns a vector taken from a column of a matrix.
//! @param [out] vector filled vector
//! @param [in] row The index of row to extract.  Row indices are 0, 1, and 2.
void GetRow
(
DVec3dR         vector,
int             row
) const;

//!
//! @description Set the components in a column.
//! @param [in] vector new values
//! @param [in] col The index of column to change. Column indices are 0, 1, 2.
//!
void SetColumn
(
DVec3dCR        vector,
int             col
);

//!
//! @description Set the components in a column.
//! @param [in] col The index of column to change. Column indices are 0, 1, 2.
//! @param [in] x x component.
//! @param [in] y y component.
//! @param [in] z z component.
void SetColumn (int col, double x, double y, double z);

//!
//! @description Set the components in a row.
//! @param [in] vector new values
//! @param [in] row The index of row to change. Row indices are 0, 1, 2.
//!
void SetRow
(
DVec3dCR        vector,
int             row
);


//flex|| components || a = matrix.GetComponentByRowAndColumn (row, col) || matrix.GetRowValues (a00, a01, a02, a10, a11, a12, a20, a21, a22) ||

//!
//! @description Returns a value from a specified row and column of the matrix.
//! @param [in] row The index of row to read. Row indices are 0, 1, 2.
//! @param [in] col The index of column to read.  Column indices are 0, 1, 2.
//!
double GetComponentByRowAndColumn
(
int             row,
int             col
) const;

//!
//! @description Sets a specified row and column of the matrix.
//! @param [in] row The index of row to set. Row indices are 0, 1, 2.
//! @param [in] col The index of column to set.  Column indices are 0, 1, 2.
//! @param [in] value the new value.
//!
void SetComponentByRowAndColumn
(
int             row,
int             col,
double          value
);

//! @description Get all contents as individual doubles, moving along rows
void GetRowValues
(
double          &x00,
double          &x01,
double          &x02,
double          &x10,
double          &x11,
double          &x12,
double          &x20,
double          &x21,
double          &x22
) const;

//flex|| || || matrix.GetRowValuesXY (outData[4]) ||

//! @description Copies 4 doubles from xx,xy,yx,yy positions into an array.
//! @param [out] data returned data -- first 2 entries in row 0, then first 2 in row 1.
void GetRowValuesXY
(
double data[4]
) const;
//flex!! Numeric Queries
//flex|| Determinant || a = matrix.Determinant () ||
//flex|| Condition number (0 bad, 1 good) || a = matrix.ConditionNumber () ||
//!
//! @description Returns the determinant of the matrix.
//! @return determinant of the matrix.
//!
double Determinant () const;

//!
//! Computes an estimate of the condition of this instance matrix.  Values near 0
//! are bad.
//!
//! @return estimated condition number.
//!
double ConditionNumber () const;

//flex|| Diagonal checks || bool matrix.IsIdentity () || bool matrix.IsDiagonal () || bool matrix.IsUniformScale () ||
//flex|| rotations || bool matrix.IsRigid () || bool matrix.IsRigidScale (outScale) || bool matrix.IsNearRigidScale (outMatrix, primaryAxisIndex, tolerance) ||
//flex|| Perpendicular columns || bool matrix.IsOrthogonal () || bool matrix.HasMutuallyPerpendicularColumns (outUnitColumns, outScales, outAxisLengthRatio) ||
//flex|| Permutation checks || bool matrix.IsSignedPermutation () || bool matrix.IsNearSignedPermutation (outMatrix, tolerance) ||
//flex|| Acts within a plane. || bool matrix.IsPlanar (outNormal) ||
//flex|| partial ranges || bool matrix.DiagonalSignedRange (outMinValue, outMaxValue) || bool matrix.DiagonalAbsRange (outMinValue, outMaxValue) ||
//flex|| || bool matrix.OffDiagonalSignedRange (outMinValue, outMaxValue) || bool matrix.OffDiagonalAbsRange (outMinValue, outMaxValue) ||
//flex|| || a = matrix.SumSquares () || a = matrix.SumDiagonalSquares () || a = matrix.SumOffDiagonalSquares () ||
//flex|| || a = matrix.MaxAbs () || a = matrix.MaxDiff () ||
//
//!
//! @description Tests if a matrix is the identity matrix.
//! @return true if matrix is approximately an identity.
//!
bool IsIdentity () const;

//!
//! @description Tests if a matrix has small offdiagonal entries compared to
//!                   diagonals.   The specific test condition is that the
//!                   largest off diagonal absolute value is less than a tight tolerance
//!                   fraction times the largest diagonal entry.
//! @return true if matrix is approximately diagonal
//!
bool IsDiagonal () const;

//!
//! @description Tests if a matrix has (nearly) equal diagaonal entries and
//!           (nearly) zero off diagonals.  Tests use a tight relative tolerance.
//! @param [out] maxScale the largest diagaonal entry
//! @return true if matrix is approximately diagonal
//!
bool IsUniformScale
(
double          &maxScale
) const;

//!
//! @description Return the (signed) range of entries on the diagonal.
//! @param [out] minValue smallest signed value
//! @param [out] maxValue largest signed value
//!
void DiagonalSignedRange
(
double          &minValue,
double          &maxValue
) const;

//!
//! @description Return the (absolute value) range of entries on the diagonal.
//! @param [out] minValue smallest absolute value
//! @param [out] maxValue largest absolute value
//!
void DiagonalAbsRange
(
double          &minValue,
double          &maxValue
) const;

//!
//! @description Return the (signed) range of entries off the diagonal.
//! @param [out] minValue smallest signed value
//! @param [out] maxValue largest signed value
//!
void OffDiagonalSignedRange
(
double          &minValue,
double          &maxValue
) const;

//!
//! @description Return the (absolute value) range of entries off the diagonal.
//! @param [out] minValue smallest absolute value
//! @param [out] maxValue largest absolute value
//!
void OffDiagonalAbsRange
(
double          &minValue,
double          &maxValue
) const;


//! @description return the range of absolute values strictly below the diagonal.
DRange1d LowerTriangleAbsRange ();
//! @description return the range of absolute values strictly above the diagonal.
DRange1d UpperTriangleAbsRange ();
//! @description return the range of absolute values on the diagonal.
DRange1d DiagonalAbsRange ();
//! @description return the largest absolute value in the lower triangle.
double LowerTriangleMaxAbs ();
//! @description return the largest absolute value in the upper triangle.
double UpperTriangleMaxAbs ();
//! @description return the largest absolute value on the diagonal
double DiagonalMaxAbs ();
//!
//! @description Return the sum of squares of coefficients in a matrix.
//! @return Sum of squares of all entries in matrix
//!
double SumSquares () const;

//!
//! @description Find the largest absolute value of entries in the matrix.
//! @return largest absolute value in matrix
//!
double MaxAbs () const;

//!
//! @description Returns the largest absolute value difference between
//!   corresponding coefficients in Matrix1 and Matrix2.
//! @param [in] matrix2 The matrix to compare to
//! @return largest absolute difference between the two matrices.
//!
double MaxDiff (RotMatrixCR matrix2) const;

//!
//! @description Computes the sum of the squares of the diagonal entries of this instance matrix.
//! @return Sum of squares of diagonal entries
//!
double SumDiagonalSquares () const;

//!
//! @description Computes the sum of the squares of the off-diagonal entries of this instance matrix.
//! @return sum of square of off-diagonal entries of the matrix.
//!
double SumOffDiagonalSquares () const;


//!
//! @description Test if this instance matrix does nothing more
//! than exchange and possibly negate principle axes.
//! @return true if the matrix is a permutation of the principle axes.
//!
bool IsSignedPermutation () const;


//!
//! @description Test if this instance matrix does nothing more
//! than exchange and possibly negate principle axes, within a tolerance.
//! @param [out] result the nearby permutation, or the orignal matrix if none near.
//! @param [in] tolerance tolerance for comparison to the permutation
//! @return true if the matrix is a near permutation of the principle axes.
//!
bool IsNearSignedPermutation (RotMatrixR result, double tolerance) const;
//!
//! @description Test if a matrix is a rigid body rotation,
//!   i.e. its transpose is its inverse and it has a positive determinant.
//! @return true if the matrix is a rigid body rotation.
//!
bool IsRigid () const;

//!
//! @description Test if this instance matrix is orthogonal, i.e. its transpose is its inverse.
//!   This class of matrices includes both rigid body rotations and reflections.
//! @return true if the matrix is orthogonal.
//!
bool IsOrthogonal () const;

//!
//! @description Test if this instance matrix has orthonormal columns, i.e. its columns
//! are all perpendicular to one another.
//! @param [out] columns matrix containing the unit vectors along the columns.
//! @param [out] axisScales  point whose x, y, and z components are the magnitudes of the
//!           original columns.
//! @param [out] axisRatio smallest axis length divided by largest.
//! @return true if the matrix is orthonormal.
//!
bool IsOrthonormal
(
RotMatrixR      columns,
DVec3dR         axisScales,
double          &axisRatio
) const;

//!
//! @description Test if this instance matrix is composed of only rigid rotation and scaling.
//! @param [out] columns  matrix containing the unit vectors along the columns.
//! @param [out] scale largest axis scale factor.  If function value is true,
//!       the min scale is the same.  Use areColumnsOrthonormal to get
//!       separate column scales.
//! @return true if the matrix is orthonormal.
//!
bool IsRigidScale
(
RotMatrixR      columns,
double          &scale
) const;

//!
//! @description Test if this instance matrix is composed of only rigid rotation and scaling, allowing negative (mirror) scale
//! @param [out] columns  descaled matrix.  (Specifically: The original matrix multiplied by the inverse of the scale)
//! @param [out] scale signed scale of largest magnitude. If function value is true,
//!       the min scale is the same.  Use areColumnsOrthonormal to get
//!       separate column scales.
//! @return true if the matrix is orthonormal (i.e. the return columns are a rotation)
//!
bool IsRigidSignedScale
(
RotMatrixR      columns,
double          &scale
) const;

//! Determine if a matrix is close to a pure rotate and scale.
//! If source is not near rigid, return false and copy to the output.
//! If near an identity return identity.
//! If nearly perpendicular with scales other than 1, clean preserving the length and direction of the primary axis.
//! This is intended to be used with a crude (e.g. 1.0e-6) reltol to identify old DGN file matrices that are "dirty" by modern standards but were 
//!   meant to be identity, rotation, or scaled rotations in the UOR era.
//! @param [in] dest result matrix
//! @param [in] primaryAxis axis whose orientation and direction is preserved.
//! @param [in] tolerance relative tolerance for recognizing near-perpendicular conditions.
bool IsNearRigidScale (RotMatrixR dest, int primaryAxis = 0, double tolerance = 1.0e-6) const;



//!
//! @description Tests if this instance matrix has no effects perpendicular to any plane with the given normal.  This
//! will be true if the matrix represents a combination of (a) scaling perpencicular to the normal
//! and (b) rotation around the normal.
//!
//! @param [in] normal The plane normal
//! @return true if the matrix has no effects perpendicular to any plane
//!   with the given normal.
//!
bool IsPlanar (DVec3dCR normal) const;

//flex|| Equality tests || bool matrixA.IsEqual (matrixB) || bool matrixA.IsEqual (matrixB, tolerance) ||

//!
//! @description Tests for equality between two matrices
//! "Equality" means relative error less than 1.0e-12, in the sense that each
//! component-wise difference is less than 1.0e-12 times the largest absolute
//! value of the components of one matrix.
//!
//! @param [in] matrix2 The second matrix
//! @return   true if the matrices are identical.
//!
bool IsEqual (RotMatrixCR matrix2) const;

//!
//! @description Tests for equality between two matrices.
//!
//! @param [in] matrix2 The second matrix
//! @param [in] tolerance The relative error tolerance.  Absolute tolerance is
//!           this (relative) tolerance times the largest absolute value in Matrix1.
//! @return true if the matrices are identical within tolerance.
//!
bool IsEqual
(
RotMatrixCR     matrix2,
double          tolerance
) const;


//flex!! Factorization

//flex|| Singular Value Decomposition || rank = matrix.FactorRotateScaleRotate (outRotationMatrix1, outScalePoint, outRotationMatrix2) ||

//! Factor as {rotation1 * scale * rotation2}
//! @return number of nonzero scales (independent columns).
//! @param [out] rotation1 pure rotation
//! @param [out] scalePoint scale factors, largest first.
//! @param [out] rotation2 pure rotation
int FactorRotateScaleRotate
(
RotMatrixR  rotation1,
DPoint3dR   scalePoint,
RotMatrixR  rotation2
) const;


//flex|| product of rotation and skew || bool RotateAndScaleFactors (outRotationMatrix, outSkewFactor, primaryAxisIndex, secondaryAxisIndex) ||

//! Factor as {rotation*skewFactor} where the rotation favors indicated primary and secondary axes.
//! @param [out] rotation the (orthogonal, right handed) rotation.
//! @param [out] skewFactor the scale and skew parts.
//! @param [in] primaryAxis selects column whose direction is preserved.
//! @param [in] secondaryAxis selects columns that defines plane (with primaryAxis)
//! @return true if primary and secondary are independent.
bool RotateAndSkewFactors
(
RotMatrixR rotation,
RotMatrixR skewFactor,
int         primaryAxis,
int         secondaryAxis
) const;

//! Factor the instance as a product B*V^ where B has mutually perpendicular columns and V is orthogonal.
//! @param [out] matrixB orthogonal columns
//! @param [out] matrixV transpose of right factor. (I.e. B = A*V)
//! 
bool    FactorOrthogonalColumns
(
RotMatrixR matrixB,
RotMatrixR matrixV
) const;

//flex!! Quaternions and Angle extraction
/*__PUBLISH_SECTION_END__*/

//flex
//flex A quaternion (http://en.wikipedia.org/wiki/Quaternion) is a way to encode a 3x3 rotation matrix into to 4 values (x,y,z,w).
//flex
/*__PUBLISH_SECTION_START__*/

//flex Warning: Due to inconsistent use of row/column structure of rotmatrix, there is often confusion about whether to construct a RotMatrix or its tranpose.  When returning to quaternion, the same issue shows up as negating the sign of the w term.

//flex Warning: The xyzw parts of the quaternion are sometimes stored with w first, i.e. wxyz.  Read carefully.

//flex|| quaternion as xyzw of DPoint4d || outMatrix = RotMatrix::FromQuaternion (point4d) || outMatrix.InitFromQuaternion (point4d) ||
//flex|| quaternion as wxyz in double array || outMatrix = RotMatrix::FromQuaternion (wxyz[]) || outMatrix.InitFromQuaternion (wxyz[]) || outMatrix.InitTransposedFromQuaternionWXYZ (wxyz[]) ||


//! @param [in] quat The quaternion, stored as (xyzw)
static RotMatrix FromQuaternion (DPoint4dCR quat);
//! @param [in] quat The quaternion, stored as (xyzw)
void InitFromQuaternion (DPoint4dCR quat);



//! @param [in] pQuatAsDoubleArray The quaternion, stored as (w,x,y,z) in an array of doubles.
static RotMatrix FromQuaternion (const double    *pQuatAsDoubleArray);

//! @param [in] pQuatAsDoubleArray The quaternion, stored as (w,x,y,z) in an array of doubles.
void InitFromQuaternion (const double    *pQuatAsDoubleArray);
//! Initialization, compatible with mdlRMatrix_fromQuat.
//! @param [in] pQuatAsDoubleArray The quaternion, stored as (w,x,y,z) in an array of doubles.
void InitTransposedFromQuaternionWXYZ (const double    *pQuatAsDoubleArray);

//flex Warning: Most microstation conversions seem to use transpose "true"

//flex|| Quaternion from rotmatrix || matrix.GetQuaternion (point4d, transpose) || matrix.GetQuaternion (wxyz[], transpose) ||

//! @param [out] quat quaternion, stored as xyzw
//! @param [in] transpose true if matrix is stored transposed
void GetQuaternion (DPoint4dR       quat, bool            transpose) const;

//! @param [out] pQuatAsDoubleArray quaternion, stored as (w,x,y,z) in an array of doubles.
//! @param [in] transpose true if matrix is stored transposed
void GetQuaternion (double *pQuatAsDoubleArray, bool transpose) const;

//flex|| Extracting rotation axis || radians = matrix.GetRotationAngleAndVector (outAxisVector) ||
//flex|| first column angle || radians = matrix.ColumnXAngleXY () ||

//! Returns the angle of rotation of this instance and sets axis to be the
//! normalized vector about which this instance rotates.
//! NOTE: this instance is assumed to be a (rigid body, i.e. orthogonal) rotation
//! matrix.
//! Since negating both angle and axis produces an identical rotation,
//! calculations are simplified by assuming (and returning) the angle in [0,Pi].
//!
//! @param [out] axis normalized axis of rotation
//! @return rotation angle (in radians) between 0 and Pi, inclusive
double GetRotationAngleAndVector (DVec3dR axis) const;

//!
//! Returns the (0 or positive) angle from (1,0) to the XY vector in the first column.
//! @return rotation angle (in radians) between 0 and 2Pi
//!
double ColumnXAngleXY () const;


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
//!
void GivensRowOp
(
double          c,
double          s,
int             i0,
int             i1
);

//!
//! Apply a Givens "column operation", i.e. post-multiply by a Givens rotation matrix.
//! The Givens matrix is an identity except for the 4 rotational entries, viz
//!       R(i0,i0)=R(i1,i1)=c
//!       R(i0,i1)=-s
//!       R(i1,i0)=s
//!
//! @param [in] c The cosine of givens rotation.
//! @param [in] s The sine of givens rotation.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//!
void GivensColumnOp
(
double          c,
double          s,
int             i0,
int             i1
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
//!
void HyperbolicRowOp
(
double          secant,
double          tangent,
int             i0,
int             i1
);

//!
//! Apply a hyperbolic "column operation", i.e. pre-multiply by a hyperbolic reflection matrix
//! The matrix is an identity except for the 4 entries
//!       R(i0,i0)=R(i1,i1)=secant
//!       R(i0,i1)=R(i1,i0)=tangent
//! @param [in] secant The cosine of reflection.
//! @param [in] tangent The sine of reflection.
//! @param [in] i0 The index of the first affected row.
//! @param [in] i1 The index of the second affected row.
//!
//!
void HyperbolicColumnOp
(
double          secant,
double          tangent,
int             i0,
int             i1
);


#endif // __cplusplus
};
END_BENTLEY_NAMESPACE

#endif // rotmatrix_H_
