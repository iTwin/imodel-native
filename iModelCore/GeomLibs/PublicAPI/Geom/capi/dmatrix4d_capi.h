/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Copies the double values directly into the rows of this instance.
//!
//! @param pInstance OUT     constructed transform
//! @param x00 IN      (0,0) entry of matrix (row, column)
//! @param x01 IN      (0,1) entry
//! @param x02 IN      (0,2) entry
//! @param x03 IN      (0,3) entry
//! @param x10 IN      (1,0) entry of matrix (row, column)
//! @param x11 IN      (1,1) entry
//! @param x12 IN      (1,2) entry
//! @param x13 IN      (1,3) entry
//! @param x20 IN      (2,0) entry of matrix (row, column)
//! @param x21 IN      (2,1) entry
//! @param x22 IN      (2,2) entry
//! @param x23 IN      (2,3) entry
//! @param x30 IN      (3,0) entry of matrix (row, column)
//! @param x31 IN      (3,1) entry
//! @param x32 IN      (3,2) entry
//! @param x33 IN      (3,3) entry
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromRowValues
(
DMatrix4dP pInstance,
double        x00,
double        x01,
double        x02,
double        x03,
double        x10,
double        x11,
double        x12,
double        x13,
double        x20,
double        x21,
double        x22,
double        x23,
double        x30,
double        x31,
double        x32,
double        x33
);

//!
//!
//! Transpose a 4x4 matrix.
//!
//! @param pA OUT     transposed matrix
//! @param pB IN      original matrix
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_transpose
(
DMatrix4dP pA,
DMatrix4dCP pB
);

//!
//!
//! Matrix multiplication, using all components of both the matrix
//! and the points.
//!
//! @param pA IN      Matrix term of multiplication.
//! @param pOutPoint OUT     Array of homogeneous products A*pInPoint[i]
//! @param pInPoint IN      Array of homogeneous points
//! @param n IN      number of points
//!
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply4dPoints
(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int n
);

//!
//! Compute eigenvectors, assuming A is symmetric.
//! @param pQ OUT     orthogonal, unit eigenvectors.
//! @param pD OUT     corresponding eigenvalues.
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_symmetricEigenvectors
(
DMatrix4dCP pA,
DMatrix4dP pQ,
DPoint4dP pD
);

//!
//!
//! Multiply a matrix times points.
//!
//! @param pA IN      Matrix term of multiplication.
//! @param pOutPoint OUT     Array of graphics points A*pInPoint[i]
//! @param pInPoint IN      Array of graphics points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyGrapicsPointArray
(
DMatrix4dCP pA,
GraphicsPointP pOutPoint,
GraphicsPointCP pInPoint,
int n
);

//!
//! Evaluate pA*X for m*n points X arranged in a grid.
//! The homogeneous coordinates of the i,j point in the grid is
//!               (x0 + i, y0 + j, 0, 1)
//! The returned point pGrid[i * m + j] is the xy components of the image
//! of grid poitn ij AFTER normalization.
//!
//! @param pA IN      Viewing transformation
//! @param pGrid OUT     Array of mXn mapped, normalized points
//! @param x00 IN      grid origin x
//! @param y00 IN      grid origin y
//! @param m IN      number of grid points in x direction
//! @param n IN      number of grid points in y direction
//! @param tol IN      relative tolerance for 0-weight tests.
//!                                        If 0, 1.0e-10 is used *
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_evaluateImageGrid
(
DMatrix4dCP pA,
DPoint2dP pGrid,
double x00,
double y00,
int m,
int n,
double tol
);

//!
//! Multiply an array of points by a matrix, using all components of both the matrix
//! and the points.
//!
//! @param pA IN      matrix term of product.
//! @param pOutPoint OUT     Array of products A*pPoint[i] renormalized
//! @param pInPoint IN      Array of points points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray
(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int n
);

//!
//! Matrix*point multiplication, using full 4d points but assuming the
//! matrix is affine, i.e. assume 0001 4th row.
//!
//! @param pA IN      matrix  4th row to be ignored
//! @param pOutPoint OUT     Array of homogeneous products A*pPoint[i]
//! @param pInPoint IN      Array of homogeneous points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix4dPoints
(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int   n
);

//!
//! Matrix*point multiplication, using full 4d points but assuming the
//! matrix is has 3D only scaling and translation.
//!
//! @param pA IN      matrix.
//! @param pOutPoint OUT     Array of homogeneous products A*pPoint[i]
//! @param pInPoint IN      Array of homogeneous points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_scaleAndTranslate4dPoints
(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int   n
);

//!
//! Matrix times vector multiplication, assume 0001 4th row
//!
//! @param pA IN      matrix  4th row to be ignored
//! @param pOut OUT     Destination array
//! @param pIn IN      Source array
//! @param n IN      number of vectors
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix3dPoints
(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int   n
);

//!
//! Matrix times vector multiplication, assume 0001 4th row and padding
//! 3d data with 0 weight.
//!
//! @param pA IN      matrix  4th row to be ignored
//! @param pOut OUT     Destination array
//! @param pIn IN      Source array
//! @param n IN      number of vectors
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAffineMatrix3dVectors
(
DMatrix4dCP pA,
DPoint3dP pOut,
DPoint3dCP pIn,
int   n
);

//!
//! Matrix*point multiplication, using only scale and translate entries from the
//! matrix.
//!
//! @param pA IN      matrix.
//! @param pOutPoint OUT     Array of products A*pPoint[i]
//! @param pInPoint IN      Array of input points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_scaleAndTranslate3dPoints
(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int   n
);

//!
//! Matrix*point multiplication, with input points represented by
//! separate DPoint3d and weight arrays.
//!
//! @param pA IN      matrix
//! @param pHPoint OUT     Array of homogeneous products A*pPoint[i]
//! @param pPoint IN      Array of xyz coordinates
//! @param pWeight IN      weight array. If NULL, unit weight is used
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyWeightedDPoint3dArray
(
DMatrix4dCP pA,
DPoint4dP pHPoint,
DPoint3dCP pPoint,
const double *pWeight,
int n
);

//!
//!
//! Multiply a matrix times a single homogeneous point.
//!
//! @param pA IN      matrix
//! @param pOutPoint OUT     product A*P, where P is a column vector
//! @param pInPoint IN      column vector
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyMatrixPoint
(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint
);

//!
//! Multiply the transformed matrix times points. (Equivalent to
//! multiplying transposed points times the matrix.)
//!
//! @param pA IN      matrix
//! @param pOutPoint OUT     Array of homogeneous products A^T *pPoint[i]
//! @param pInPoint IN      Array of homogeneous points
//! @param n IN      number of points
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyTransposePoints
(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int n
);

//!
//! @description Transform an ellipse.
//! @param pMatrix IN      4x4 matrix
//! @param pOutEllipse OUT     transformed ellipse (can be same as pInEllipse)
//! @param pInEllipse IN      untransformed ellipse
//! @group "DMatrix4d Multiplication"
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyDEllipse4d
(
DMatrix4dCP pMatrix,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
);

//!
//! Install c0, c1, c2, c3 in an indicated row of an DMatrix4d.
//!
//! @param pA IN OUT  Matrix whose row is being set
//! @param i IN      index of row 0 OUT     i < 4 whose values are to be set
//! @param c0 IN      column 0 value
//! @param c1 IN      column 1 value
//! @param c2 IN      column 2 value
//! @param c3 IN      column 3 value
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_setRow
(
DMatrix4dP pA,
int i,
double c0,
double c1,
double c2,
double c3
);

//!
//! Install r0, r1, r2, r3 in an indicated column of an DMatrix4d.
//!
//! @param pA IN OUT  Matrix whose column is being set
//! @param i IN      index of column 0 OUT     i < 4  whose values are to be set
//! @param r0 IN      row 0 value
//! @param r1 IN      row 1 value
//! @param r2 IN      row 2 value
//! @param r3 IN      row 3 value
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_setColumn
(
DMatrix4dP pA,
int i,
double r0,
double r1,
double r2,
double r3
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param pA IN OUT  Matrix whose column is being set
//! @param i IN      index of column 0 OUT     i < 4  whose values are to be set
//! @param pPoint IN      column values
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_setColumnDPoint4d
(
DMatrix4dP pA,
int i,
DPoint4dCP pPoint
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param pA IN OUT  Matrix whose colums are swapped
//! @param i IN      first column index
//! @param j IN      second column index
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_swapColumns
(
DMatrix4dP pA,
int i,
int j
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param pA IN OUT  Matrix whose colums are swapped
//! @param i IN      first column index
//! @param j IN      second column index
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_swapRows
(
DMatrix4dP pA,
int i,
int j
);

//!
//! Copy data from a matrix column to a DPoint4d structure.
//!
//! @param pMatrix IN      matrix whose column is retrieved
//! @param pVec OUT     point copied from column
//! @param i IN      index of column 0 OUT     i < 4  whose values are to be set
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_getColumnDPoint4d
(
DMatrix4dCP pMatrix,
DPoint4dP pVec,
int i
);

//!
//! Copy data from a matrix rows to DPoint4d structures.
//!
//! @param pMatrix IN      matrix whose rows are retrieved
//! @param pRow0 OUT     row 0 data. May be NULL.
//! @param pRow1 OUT     row 1 data. May be NULL.
//! @param pRow2 OUT     row 2 data. May be NULL.
//! @param pRow3 OUT     row 3 data. May be NULL.
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_getRowsDPoint4d
(
DMatrix4dCP pMatrix,
DPoint4dP pRow0,
DPoint4dP pRow1,
DPoint4dP pRow2,
DPoint4dP pRow3
);

//!
//! initialize an identity matrix.
//!
//! @param pA OUT     matrix initialized as an identity
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initIdentity (DMatrix4dP pA);

//!
//! Fill the affine part using xyz vectors for each row of the basis
//! part and an xyz vector for the translation
//!
//! @param pA OUT     matrix initialized as an identity
//! @param pRow0 IN      data for row 0 of leading 3x3 submatrix
//! @param pRow1 IN      data for row 1 of leading 3x3 submatrix
//! @param pRow2 IN      data for row 2 of leading 3x3 submatrix
//! @param pTranslation IN      data for translation part of matrix
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineRows
(
DMatrix4dP pA,
DPoint3dCP pRow0,
DPoint3dCP pRow1,
DPoint3dCP pRow2,
DPoint3dCP pTranslation
);

//!
//! Fill the scale and translate entries in an otherwise identity matrix
//!
//! @param pA OUT     matrix initialized as an identity
//! @param pScale IN      scale factor for each diagonal of leading 3x3 submatrix
//! @param pTranslation IN      translation vector
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initScaleAndTranslate
(
DMatrix4dP pA,
DPoint3dCP pScale,
DPoint3dCP pTranslation
);

//!
//! Fill the affine part using xyz vectors for each column of the basis
//! part and an xyz vector for the translation
//!
//! @param pA OUT     matrix initialized as an identity
//! @param pCol0 IN      data for column 0 of leading 3x3 submatrix
//! @param pCol1 IN      data for column 1 of leading 3x3 submatrix
//! @param pCol2 IN      data for column 2 of leading 3x3 submatrix
//! @param pTranslation IN      data for translation part of matrix
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineColumns
(
DMatrix4dP pA,
DPoint3dCP pCol0,
DPoint3dCP pCol1,
DPoint3dCP pCol2,
DPoint3dCP pTranslation
);

//!
//!
//! Copy a DMatrix3d into corresponding parts of a 4x4 matrix with
//! 4th row and column both 0001.
//!
//! @param pA OUT     matrix initialized with 0 translate, given 3x3 part
//! @param pMatrix IN      3x3 part to fill
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromDMatrix3d
(
DMatrix4dP pA,
DMatrix3dCP pMatrix
);

//!
//!
//! Copy a RotMatrix into corresponding parts of a 4x4 matrix with
//! 4th row and column both 0001.
//!
//! @param pA OUT     matrix initialized with 0 translate, given 3x3 part
//! @param pB IN      3x3 part to fill
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromRotMatrix
(
DMatrix4dP pA,
RotMatrixCP pB
);

//!
//!
//! Copy a DTransform3d into corresponding parts of a 4x4 matrix with
//! 4th row 0001.
//!
//! @param pA OUT     matrix initialized with 0 translate, given 3x3 part
//! @param pTransfrom IN      transform to copy
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromDTransform3d
(
DMatrix4dP pA,
DTransform3dCP pTransfrom
);

//!
//!
//! Copy a Transform into corresponding parts of a 4x4 matrix with
//! 4th row 0001.
//!
//! @param pA OUT     matrix initialized with 0 translate, given 3x3 part
//! @param pTransfrom IN      transform to copy
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initFromTransform
(
DMatrix4dP pA,
TransformCP pTransform
);

//!
//! Fill a 4x4 matrix with a given translation vector and otherwise
//! an identity.
//!
//! @param pA OUT     matrix initialized as pure translation
//! @param tx IN      x component
//! @param ty IN      y component
//! @param tz IN      z component
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initTranslate
(
DMatrix4dP pA,
double tx,
double ty,
double tz
);

//!
//! Reutrn world-to-local and local-to-world transforms for translating to/from
//! a local origin.  The local origin is taken as the 3d (cartesian) normalized
//! image of a 4D point.
//! @param pWorldToLocal OUT transformation from world to local coordiantes
//! @param pLocalToWorld OUT transformation from local to world coordinates
//! @param pPoint IN Homogeneous origin.
//! @return false if homogeneous origin cannot be normalized to xyz.
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_initForDPoint4dOrigin
(
DMatrix4dP   pWorldToLocal,
DMatrix4dP   pLocalToWorld,
DPoint4dCP pPoint
);

//!
//! Fill a matrix with entries in the perspective row, otherwise an
//! identity matrix.
//!
//! @param pA OUT     matrix initialized as perspective
//! @param px IN      x component
//! @param py IN      y component
//! @param pz IN      z component
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_initPerspective
(
DMatrix4dP pA,
double px,
double py,
double pz
);

//!
//!
//! Apply perspective to a matrix.  This is a fast matrix product C = P * A
//! where A is an arbitrary matrix, P is identity except for
//! perspective effects Pzz=taper and Pwz = taper - 1
//! @param <= Matrix with z perspective applied.
//! @param    taper IN      taper factor in perspective
//! @param    pA    IN      pre-perpsective matrix.
//!
Public GEOMDLLIMPEXP void     bsiDMatrix4d_premultiplyByZTaper
(
DMatrix4dP pInstance,   /* OUT     matrix after applying perspective */
double  taper,                  /* IN      perspective taper. 1.0 flat projection (no effect)
                                    0.0 has eyepoint on z=1 plane. */
const   DMatrix4d *pA           /* IN      pre-persepctive matrix */
);

//!
//! Premultiply A by a matrix with sx,sy,sz,1 on diagonal, tx,ty,tz,1 in last column
//! @param tx IN      03 entry (x translate) of mutliplier.
//! @param ty IN      13 entry (y translate) of multiplier.
//! @param tz IN      23 entry (z translate) of multiplier.
//! @param sx IN      00 entry (x scale) of multiplier.
//! @param sy IN      11 entry (y scale) of multiplier.
//! @param sz IN      22 entry (z scale) of multiplier.
//!
Public GEOMDLLIMPEXP void     bsiDMatrix4d_premultiplyByScaleAndTranslate
(
DMatrix4dP pInstance,
double tx,
double ty,
double tz,
double sx,
double sy,
double sz,
DMatrix4dCP pA
);

//!
//! @param pA IN      matrix whose RMS is being computed
//! @return Root mean square of entries in the matrix, i.e. square root of
//!       S/16 where S is the sum of squares.
//!
Public GEOMDLLIMPEXP double bsiDMatrix4d_RMS (DMatrix4dCP pA);

//!
//! Form the product of two 4x4 matrices.
//! @param pA IN      first matrix of product A*B
//! @param pB IN      second matrix of product A*B
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply
(
DMatrix4dP pC,
DMatrix4dCP pA,
DMatrix4dCP pB
);

//!
//! Form the product of three 4x4 matrices.
//!
//! @param pABC OUT     Product of A and B
//! @param pA IN      first matrix of product A*B*C
//! @param pB IN      second matrix of product A*B*C
//! @param pC IN      third matrix of product A*B*C
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply3
(
DMatrix4dP pABC,
DMatrix4dCP pA,
DMatrix4dCP pB,
DMatrix4dCP pC
);

//!
//!
//! Subtract two 4x4 matrices.
//!
//! @param pC OUT     result A - B
//! @param pA IN      A matrix of A - B
//! @param pB IN      B matrix of A - B
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_subtractDMatrix4d
(
DMatrix4dP pC,
DMatrix4dCP pA,
DMatrix4dCP pB
);

//!
//! Add a matrix (componentwise) to the instance.
//!
//! @param <= pInstance + pDelta
//! @param pVector IN      matrix to add
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_addDMatrix4dInPlace
(
DMatrix4dP pInstance,
DMatrix4dCP pDelta
);

//!
//! Subtract a matrix (componentwise) from the instance
//!
//! @param <= pInstance - pDelta
//! @param pDelta IN      matrix to subtract
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_subtract
(
DMatrix4dP pInstance,
DMatrix4dCP pDelta
);

//!
//! @param pMatrixA IN      matrix containing first column
//! @param columnA IN      column index within matrix A
//! @param pMatrixB IN      matrix containing second column
//! @param columnB IN      column index within matrix B
//! @param firstRow IN      row where partial column data begins
//! @param lastRowPlus1 IN      1 more than the last row to use
//! @return dot product of the trailing part of the indicated columns
//!
Public GEOMDLLIMPEXP double bsiDMatrix4d_partialColumnDotProduct
(
DMatrix4dCP      pMatrixA,
int                 columnA,
DMatrix4dCP      pMatrixB,
int                 columnB,
int                 firstRow,
int                 lastRowPlus1
);

//!
//! Subtract a scaled multiple of a (partial) column of B to a (partial)
//!   column of A.
//! A[*] = factor * B[*]
//! where ** ranges over the respective column entries.
//!
//! @param pMatrixA IN OUT  matrix being updated
//! @param columnA IN      affected column of pMatrixA
//! @param factor IN      scale factor
//! @param pMatrixB IN      added matrix
//! @param columnB IN      source column in B
//! @param firstRow IN      first row of updated
//! @param lastRowPlus1 IN      1 more than the last row of update
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_partialColumnUpdate
(
DMatrix4dP    pMatrixA,
int         columnA,
double      factor,
DMatrix4dCP    pMatrixB,
int         columnB,
int         firstRow,
int         lastRowPlus1
);

//!
//! Construct a unitary reflection (Householder) transform defined
//! by the reflection vector.
//!
//! @param pMatrixA OUT     matrix where reflection vector is placed
//! @param columnA IN      column where vector is placed
//! @param pMatrixB IN OUT  matrix containing original vector.
//!                                        The original vector is replaced by
//!                                        the result of applying the transform,
//!                                        i.e. +-a 0 0 ... where a is the
//!                                        length of the original
//! @param columnB IN      column of B containing original vector
//! @param firstRow IN      first row of vectors
//! @param lastRowPlus1 IN      1 more than the last row of vectors
//! @return true if vector was computed
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_constructPartialColumnReflection
(
DMatrix4dP    pMatrixA,
int         columnA,
DMatrix4dP    pMatrixB,
int         columnB,
int         firstRow,
int         lastRowPlus1
);

//!
//! Factor a matrix by Householder reflections
//!
//! @param pMatrixQ OUT     Matrix of reflection vectors
//! @param pMatrixR OUT     Upper triangular factor
//! @param pMatrixA IN      Matrix to factor
//! @return true if invertible
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_factorQR
(
DMatrix4dP    pMatrixQ,
DMatrix4dP    pMatrixR,
DMatrix4dCP    pMatrixA
);

//!
//!
//! Expands a partial column reflection into its complete matrix.
//!
//! @param pMatrixM OUT     Full matrix form
//! @param pMatrixQ IN      Compresed form
//! @param colQ IN      column where reflection vector is found
//! @param firstRow IN      start of vector
//! @param lastRowPlus1 IN      1 more than last index of vector
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_expandPartialColumnReflection
(
DMatrix4dP    pMatrixM,
DMatrix4dCP    pMatrixQ,
int         colQ,
int         firstRow,
int         lastRowPlus1
);

//!
//! @param pMatrixB IN OUT  Matrix whose column colB is to be replaced
//! @param colB IN      column to replace
//! @param pMatrixR IN      triangular matrix
//! @return true if diagonals are are nonzero
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_backSubstitute
(
DMatrix4dP    pMatrixB,
int         colB,
DMatrix4dP    pMatrixR
);

//!
//!
//! Expand reflection vectors to full matrix form.
//!
//! @param pMatrixB OUT     Expanded matrix
//! @param pMatrixQ IN      Matrix of reflection vectors
//! @param order IN      0 or positive for Q0*Q1*Q2...
//!                                              negative for ..Q2*Q1*Q0
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_expandReflections
(
DMatrix4dP              pMatrixB,
DMatrix4dCP        pMatrixQ,
int                     order
);

//!
//! Invert a matrix using QR (Householder reflection) factors.
//!
//! @param pMatrixB OUT     Inverse matrix
//! @param pMatrixA IN      Original matrix
//! @return true if inversion completed normally.
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_invertQR
(
DMatrix4dP    pMatrixB,
DMatrix4dCP    pMatrixA
);

//!
//!
//! Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
//! the scalar location defined by a pivot index along the diagonal.
//!
//! @param pMatrix OUT     3x3 submatrix
//! @param pRow OUT     off-diagonals in row
//! @param pColumn OUT     off-diagonals in column
//! @param pPivot OUT     pivot entry
//! @param pHMat IN      Original matrix
//! @param pivot IN      pivot index
//! @return true if pivot index is valid.
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_extractAroundPivot
(
DMatrix4dCP pHMat,
DMatrix3dP pMatrix,
DPoint3dP pRow,
DPoint3dP pColumn,
double        *pPivot,
int           pivot
);

//!
//!
//! Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
//! the scalar location defined by a pivot index along the diagonal.
//!
//! @param pMatrix OUT     3x3 submatrix
//! @param pRow OUT     off-diagonals in row
//! @param pColumn OUT     off-diagonals in column
//! @param pPivot OUT     pivot entry
//! @param pHMat IN      Original matrix
//! @param pivot IN      pivot index
//! @return true if pivot index is valid.
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_extractAroundPivot
(
DMatrix4dCP pHMat,
RotMatrixP pMatrix,
DPoint3dP pRow,
DPoint3dP pColumn,
double        *pPivot,
int           pivot
);


//!
//! Compute A = Bt * D * B where B is a matrix and D is a diagonal matrix
//! given as a vector.
//! REMARK: This is a Very Bad Thing for numerical purposes.  Are you sure
//! you can't get your result without forming this product?
//!
//! @param pA OUT     Bt*D*B
//! @param pSigma IN      entries of diagonal matrix D
//! @param pB IN      matrix B
//!
Public GEOMDLLIMPEXP void bsiDMatrix4d_symmetricProduct
(
DMatrix4dP pA,
DPoint4dCP pSigma,
DMatrix4dCP pB
);

//!
//!
//! Search the matrix for the entry with the largest absolute value.
//!
//! @param pMatrix IN      matrix to examine
//! @return max absolute value entry in the matrix.
//!
Public GEOMDLLIMPEXP double bsiDMatrix4d_maxAbs (DMatrix4dCP pMatrix);

//!
//! Tests if pInstance is an identity transform and returns the BoolInt
//! indicator.
//! The matrix is considered an identity if the sum of squared components
//! after subtracting 1 from the diagonals is less than a small tolerance
//!
//! @param IN      matrix to test
//! @return true if matrix is approximately an identity.
//!
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_isIdentity (DMatrix4dCP pInstance);

//!
//! Approximate solution of A*X=0, i.e. find an approximate null vector;
//! A is assumed upper triangular.
//! Method: Find the smallest diagonal.
//!         Set that entry in X to 1.
//!         Backsolve the linear system with RHS 0 and the chosen X component fixed.
//! @param pA IN      upper triangular matrix.
//! @param pX OUT     approximate null space vector.
//! @param pResidual OUT     estimate of error in A*X. (Squared residual.)
//!
Public GEOMDLLIMPEXP bool     bsiDMatrix4d_approximateNullVectorFromUpperTriangle
(
DMatrix4dCP pA,
DPoint4dP pX,
double        *pResidual
);

END_BENTLEY_GEOMETRY_NAMESPACE

