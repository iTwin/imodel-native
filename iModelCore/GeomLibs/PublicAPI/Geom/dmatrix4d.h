/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
4x4 matrix used for perspective (homogeneous coordinate) calculations.
@ingroup GROUP_Geometry
*/

struct GEOMDLLIMPEXP DMatrix4d
{
//! Coefficients of 4x4 matrix.
double          coff[4][4];
#ifdef __cplusplus

//BEGIN_FROM_METHODS


//!
//! Copies the double values directly into the rows of this instance.
//!
//! @param [in] x00 (0,0) entry of matrix (row, column)
//! @param [in] x01 (0,1) entry
//! @param [in] x02 (0,2) entry
//! @param [in] x03 (0,3) entry
//! @param [in] x10 (1,0) entry of matrix (row, column)
//! @param [in] x11 (1,1) entry
//! @param [in] x12 (1,2) entry
//! @param [in] x13 (1,3) entry
//! @param [in] x20 (2,0) entry of matrix (row, column)
//! @param [in] x21 (2,1) entry
//! @param [in] x22 (2,2) entry
//! @param [in] x23 (2,3) entry
//! @param [in] x30 (3,0) entry of matrix (row, column)
//! @param [in] x31 (3,1) entry
//! @param [in] x32 (3,2) entry
//! @param [in] x33 (3,3) entry
//!
static DMatrix4d FromRowValues
(
double          x00,
double          x01,
double          x02,
double          x03,
double          x10,
double          x11,
double          x12,
double          x13,
double          x20,
double          x21,
double          x22,
double          x23,
double          x30,
double          x31,
double          x32,
double          x33
);

//! Return a matrix with data by row.
static DMatrix4d FromRows
(
DPoint4dCR row0,
DPoint4dCR row1,
DPoint4dCR row2,
DPoint4dCR row3
);
//! Return a matrix with data by column.
static DMatrix4d FromColumns
(
DPoint4dCR column0,
DPoint4dCR column1,
DPoint4dCR column2,
DPoint4dCR column3
);

//! Return a matrix that permutes i and j.
//!<ul>
//!<li>If this multiplies another matrix from the left, it swaps rows.
//!<li>If this multiplies another matrix from the left, it swaps columns.
//!</ul>
static DMatrix4d FromSwap (int i, int j);
//!
//! Fill the scale and translate entries in an otherwise identity matrix
//!
//! @param [in] scale scale factor for each diagonal of leading 3x3 submatrix
//! @param [in] translation translation vector
//!
static DMatrix4d FromScaleAndTranslation
(
DPoint3dCR      scale,
DPoint3dCR      translation
);

//! Promote 3 inputs to 4x4 carrier matrices, with respective scale terms 1,0,1.
//! Return the product of the carriers.
//! (Huh?  What is this for?  If a transform is defined as A*F*C, and the derivative part of F is has nonzero derivative B only for its matrix part,
//!       A*B*C is the derivative 4x4 matrix.
//! @param [in] transformA First term.  Promoted to 4x4 with final row 0001.
//! @param [in] matrixB    Second term. Promoted to 4x4 with final row 0000.
//! @param [in] transformC Third term.  Promoted to 4x4 with final row 0001.
static DMatrix4d From101WeightedProduct
(
TransformCR transformA,
RotMatrixCR   matrixB,
TransformCR transformC
);


//!
//! Fill the affine part using xyz vectors for each column of the basis
//! part and an xyz vector for the translation
//!
//! @param [in] col0 data for column 0 of leading 3x3 submatrix
//! @param [in] col1 data for column 1 of leading 3x3 submatrix
//! @param [in] col2 data for column 2 of leading 3x3 submatrix
//! @param [in] translation data for translation part of matrix
//!
static DMatrix4d FromColumnVectors
(
DVec3dCR      col0,
DVec3dCR      col1,
DVec3dCR      col2,
DPoint3dCR      translation
);
//! Access the xyz parts of all columns.   (The w part is ignored.)
//! Return values are strongly typed as DVec3d (in columns 0,1,2) and DPoint3d (column3)
void GetColumnsXYZ
(
DVec3dR      col0,  //!< [out] xyz part of column 0
DVec3dR      col1,  //!< [out] xyz part of column 1
DVec3dR      col2,  //!< [out] xyz part of column 2
DPoint3dR      translation  //!< [out] translation xyz parts
) const;

//!
//!
//! Copy a RotMatrix into corresponding parts of a 4x4 matrix with
//! 4th row and column both 0001.
//!
//! @param [in] B 3x3 part to fill
//!
static DMatrix4d From (RotMatrixCR B);


//!
//!
//! Copy a Transform into corresponding parts of a 4x4 matrix with
//! 4th row 0001.
//!
//! @param [in] transfrom transform to copy
//!
static DMatrix4d From (TransformCR transfrom);


//!
//! Fill a 4x4 matrix with a given translation vector and otherwise
//! an identity.
//!
//! @param [in] tx x component
//! @param [in] ty y component
//! @param [in] tz z component
//!
static DMatrix4d FromTranslation
(
double          tx,
double          ty,
double          tz
);

//!
//! Fill a matrix with entries in the perspective row, otherwise an
//! identity matrix.
//!
//! @param [in] px x component
//! @param [in] py y component
//! @param [in] pz z component
//!
static DMatrix4d FromPerspective
(
double          px,
double          py,
double          pz
);

//!
//! Fill a matrix with zeros.
static DMatrix4d FromZero ();

//END_FROM_METHODS
/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/
//!
//! Copies the double values directly into the rows of this instance.
//!
//! @param [in] x00 (0,0) entry of matrix (row, column)
//! @param [in] x01 (0,1) entry
//! @param [in] x02 (0,2) entry
//! @param [in] x03 (0,3) entry
//! @param [in] x10 (1,0) entry of matrix (row, column)
//! @param [in] x11 (1,1) entry
//! @param [in] x12 (1,2) entry
//! @param [in] x13 (1,3) entry
//! @param [in] x20 (2,0) entry of matrix (row, column)
//! @param [in] x21 (2,1) entry
//! @param [in] x22 (2,2) entry
//! @param [in] x23 (2,3) entry
//! @param [in] x30 (3,0) entry of matrix (row, column)
//! @param [in] x31 (3,1) entry
//! @param [in] x32 (3,2) entry
//! @param [in] x33 (3,3) entry
//!
void InitFromRowValues
(
double          x00,
double          x01,
double          x02,
double          x03,
double          x10,
double          x11,
double          x12,
double          x13,
double          x20,
double          x21,
double          x22,
double          x23,
double          x30,
double          x31,
double          x32,
double          x33
);

//!
//!
//! Transpose a 4x4 matrix.
//!
//! @param [in] B original matrix
//!
void TransposeOf (DMatrix4dCR B);

//!
//!
//! Return the transpose a 4x4 matrix.
static DMatrix4d FromTranspose (DMatrix4dCR B);


//!
//!
//! Matrix multiplication, using all components of both the matrix
//! and the points.
//!
//! @param [out] outPoint Array of homogeneous products A*inPoint[i]
//! @param [in] inPoint Array of homogeneous points
//! @param [in] n number of points
//!
void Multiply
(
DPoint4dP       outPoint,
DPoint4dCP      inPoint,
int             n
) const;

//! Matrix times point multiplication
//! @param [in] xyz xyz parts of point.
//! @param [in] w weight.
DPoint4d Multiply (DPoint3dCR xyz, double w) const;


//! Compute eigenvectors, assuming A is symmetric.
//! @param [out] Q orthogonal, unit eigenvectors.
//! @param [out] D corresponding eigenvalues.
//!
void SymmetricEigenvectors
(
DMatrix4dR      Q,
DPoint4dR       D
) const;

//!
//!
//! Multiply a matrix times points.
//!
//! @param [out] outPoint Array of graphics points A*inPoint[i]
//! @param [in] inPoint Array of graphics points
//! @param [in] n number of points
//!
void Multiply
(
GraphicsPointP  outPoint,
GraphicsPointCP inPoint,
size_t n
) const;

//!
//! Evaluate pA*X for m*n points X arranged in a grid.
//! The homogeneous coordinates of the i,j point in the grid is
//!               (x0 + i, y0 + j, 0, 1)
//! The returned point grid[i * m + j] is the xy components of the image
//! of grid poitn ij AFTER normalization.
//!
//! @param [out] grid Array of mXn mapped, normalized points
//! @param [in] x00 grid origin x
//! @param [in] y00 grid origin y
//! @param [in] m number of grid points in x direction
//! @param [in] n number of grid points in y direction
//! @param [in] tol relative tolerance for 0-weight tests.
//!                                        If 0, 1.0e-10 is used *
//!
bool EvaluateImageGrid
(
DPoint2dP       grid,
double          x00,
double          y00,
int             m,
int             n,
double          tol
) const;

//!
//! Multiply an array of points by a matrix, using all components of both the matrix
//! and the points.
//!
//! @param [out] outPoint Array of products A*pPoint[i] renormalized
//! @param [in] inPoint Array of points points
//! @param [in] n number of points
//!
void MultiplyAndRenormalize
(
DPoint3dP       outPoint,
DPoint3dCP      inPoint,
int             n
) const;

//!
//! Multiply an array of points by a matrix, using all components of both the matrix
//! and the points.
//!
void MultiplyAndRenormalize (bvector<DPoint3d> &points) const;


//!
//! Multiply a single point and return renormalized.
//!
//! @param [out] outPoint  products A*inPoint normalized
//! @param [in] inPoint input point
//!
void MultiplyAndRenormalize
(
DPoint3dR       outPoint,
DPoint3dCR      inPoint
) const;

//!
//! Multiply an array of points by a matrix, using all components of both the matrix
//! and the points.
//!
//! @param [out] outPoint Array of products A*inPoint[i] normalized
//! @param [in] inPoint Array of points points
//! @param [in] n number of points
//!
void MultiplyAndNormalize
(
DPoint3dP       outPoint,
DPoint4dCP      inPoint,
size_t          n
) const;


//!
//! Matrix*point multiplication, using full 4d points but assuming the
//! matrix is affine, i.e. assume 0001 4th row.
//!
//! @param [out] outPoint Array of homogeneous products A*pPoint[i]
//! @param [in] inPoint Array of homogeneous points
//! @param [in] n number of points
//!
void MultiplyAffine
(
DPoint4dP       outPoint,
DPoint4dCP      inPoint,
int             n
) const;


//!
//! Matrix*transform multiplication, assuming the matrix is affine, i.e. assume 0001 4th row.
//!
//! @param [out] out product transform
//! @param [in] in input transform
//!
void MultiplyAffine
(
TransformR out,
TransformCR in
) const;

//!
//! Matrix times vector multiplication, assume 0001 4th row
//!
//! @param [out] outPoint Destination array
//! @param [in]  inPoint Source array
//! @param [in]  n number of vectors
//!
void MultiplyAffine
(
DPoint3dP       outPoint,
DPoint3dCP      inPoint,
int             n
) const;

//!
//! Matrix times vector multiplication, assume 0001 4th row and padding
//! 3d data with 0 weight.
//!
//! @param [out] out Destination array
//! @param [in] in Source array
//! @param [in] n number of vectors
//!
void MultiplyAffineVectors
(
DPoint3dP       out,
DPoint3dCP      in,
int             n
) const;

//!
//! Matrix*point multiplication, with input points represented by
//! separate DPoint3d and weight arrays.
//!
//! @param [out] hPoint Array of homogeneous products A*point[i]
//! @param [in] point Array of xyz coordinates
//! @param [in] pWeight weight array. If NULL, unit weight is used
//! @param [in] n number of points
//!
void Multiply
(
DPoint4dP       hPoint,
DPoint3dCP      point,
const double    *pWeight,
int             n
) const;




//!
//!
//! Multiply a matrix times a single homogeneous point.
//!
//! @param [out] outPoint product A*P, where P is a column vector
//! @param [in] inPoint column vector
//!
void Multiply
(
DPoint4dR       outPoint,
DPoint4dCR      inPoint
) const;

//!
//! Multiply the transformed matrix times points. (Equivalent to
//! multiplying transposed points times the matrix.)
//!
//! @param [out] outPoint Array of homogeneous products A^T *pPoint[i]
//! @param [in] inPoint Array of homogeneous points
//! @param [in] n number of points
//!
void MultiplyTranspose
(
DPoint4dP       outPoint,
DPoint4dCP      inPoint,
int             n
) const;

//!
//! Install c0, c1, c2, c3 in an indicated row of an DMatrix4d.
//!
//! @param [in] i index of row 0 <= i < 4 whose values are to be set
//! @param [in] c0 column 0 value
//! @param [in] c1 column 1 value
//! @param [in] c2 column 2 value
//! @param [in] c3 column 3 value
//!
void SetRow
(
int             i,
double          c0,
double          c1,
double          c2,
double          c3
);

//!
//! Install c0, c1, c2, c3 in an indicated row of an DMatrix4d.
//!
//! @param [in] i index of row 0 <= i < 4 whose values are to be set
//! @param [in] data data for row.
//!
void SetRow
(
int             i,
DPoint4dCR       data
);



//!
//! Install r0, r1, r2, r3 in an indicated column of an DMatrix4d.
//!
//! @param [in] i index of column 0 <= i < 4  whose values are to be set
//! @param [in] r0 row 0 value
//! @param [in] r1 row 1 value
//! @param [in] r2 row 2 value
//! @param [in] r3 row 3 value
//!
void SetColumn
(
int             i,
double          r0,
double          r1,
double          r2,
double          r3
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param [in] i index of column 0 <= i < 4  whose values are to be set
//! @param [in] point column values
//!
void SetColumn
(
int             i,
DPoint4dCR      point
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param [in] i first column index
//! @param [in] j second column index
//!
void SwapColumns
(
int             i,
int             j
);

//!
//! Install a DPoint4d in an indicated column of an DMatrix4d.
//!
//! @param [in] i first column index
//! @param [in] j second column index
//!
void SwapRows
(
int             i,
int             j
);

//!
//! Copy data from a matrix column to a DPoint4d structure.
//!
//! @param [out] vec point copied from column
//! @param [in] i index of column 0 <= i < 4  whose values are to be set
//!
void GetColumn
(
DPoint4dR       vec,
int             i
) const;

//!
//! Copy data from a matrix row to a DPoint4d structure.
//!
//! @param [out] vec point copied from row
//! @param [in] i index of row 0 <= i < 4  whose values are to be set
//!
void GetRow
(
DPoint4dR       vec,
int             i
) const;



//!
//! Copy data from a matrix rows to DPoint4d structures.
//!
//! @param [out] row0 row 0 data. 
//! @param [out] row1 row 1 data. 
//! @param [out] row2 row 2 data. 
//! @param [out] row3 row 3 data. 
//!
void GetRows
(
DPoint4dR       row0,
DPoint4dR       row1,
DPoint4dR       row2,
DPoint4dR       row3
) const;

//!
//! Copy data from a matrix columns to DPoint4d structures.
//!
//! @param [out] column0 column 0 data. 
//! @param [out] column1 column 1 data. 
//! @param [out] column2 column 2 data. 
//! @param [out] column3 column 3 data. 
//!
void GetColumns
(
DPoint4dR       column0,
DPoint4dR       column1,
DPoint4dR       column2,
DPoint4dR       column3
) const;

//!
//! initialize an identity matrix.
//!
//!
void InitIdentity ();

//!
//! Fill the scale and translate entries in an otherwise identity matrix
//!
//! @param [in] scale scale factor for each diagonal of leading 3x3 submatrix
//! @param [in] translation translation vector
//!
void InitFromScaleAndTranslation
(
DPoint3dCR      scale,
DPoint3dCR      translation
);

//!
//! Fill the affine part using xyz vectors for each column of the basis
//! part and an xyz vector for the translation
//!
//! @param [in] col0 data for column 0 of leading 3x3 submatrix
//! @param [in] col1 data for column 1 of leading 3x3 submatrix
//! @param [in] col2 data for column 2 of leading 3x3 submatrix
//! @param [in] translation data for translation part of matrix
//!
void InitFromColumnVectors
(
DVec3dCR      col0,
DVec3dCR      col1,
DVec3dCR      col2,
DPoint3dCR      translation
);


//!
//!
//! Copy a RotMatrix into corresponding parts of a 4x4 matrix with
//! 4th row and column both 0001.
//!
//! @param [in] B 3x3 part to fill
//!
void InitFrom (RotMatrixCR B);


//!
//!
//! Copy a DTransform3d into corresponding parts of a 4x4 matrix with
//! 4th row 0001.
//!
//! @param [in] transfrom transform to copy
//!
void InitFrom (TransformCR transfrom);


//!
//! Fill a 4x4 matrix with a given translation vector and otherwise
//! an identity.
//!
//! @param [in] tx x component
//! @param [in] ty y component
//! @param [in] tz z component
//!
void InitFromTranslation
(
double          tx,
double          ty,
double          tz
);

//!
//! Fill a matrix with entries in the perspective row, otherwise an
//! identity matrix.
//!
//! @param [in] px x component
//! @param [in] py y component
//! @param [in] pz z component
//!
void InitFromPerspective
(
double          px,
double          py,
double          pz
);

//!
//! Premultiply matirx A by a matrix with sx,sy,sz,1 on diagonal, tx,ty,tz,1 in last column
//! @param [in] tx 03 entry (x translate) of mutliplier.
//! @param [in] ty 13 entry (y translate) of multiplier.
//! @param [in] tz 23 entry (z translate) of multiplier.
//! @param [in] sx 00 entry (x scale) of multiplier.
//! @param [in] sy 11 entry (y scale) of multiplier.
//! @param [in] sz 22 entry (z scale) of multiplier.
//! @param [in] matrixA existing matrix
//!
void PreMultiplyByTranslateAndScale
(
double          tx,
double          ty,
double          tz,
double          sx,
double          sy,
double          sz,
DMatrix4dCR     matrixA
);

//!
//! Form the product of two 4x4 matrices.
//! @param [in] A first matrix of product A*B
//! @param [in] B second matrix of product A*B
//!
void InitProduct
(
DMatrix4dCR     A,
DMatrix4dCR     B
);

//!
//! Form the product of three 4x4 matrices.
//!
//! @param [in] A first matrix of product A*B*C
//! @param [in] B second matrix of product A*B*C
//! @param [in] C third matrix of product A*B*C
//!
void InitProduct
(
DMatrix4dCR     A,
DMatrix4dCR     B,
DMatrix4dCR     C
);

//!
//!
//! Subtract two 4x4 matrices.
//!
//! @param [in] A A matrix of A - B
//! @param [in] B B matrix of A - B
//!
void DifferenceOf
(
DMatrix4dCR     A,
DMatrix4dCR     B
);

//!
//! Add a matrix (componentwise) to the instance.
//!
//! @param [in] delta matrix to add
//!
void Add (DMatrix4dCR delta);

//!
//! Add a matrix (componentwise) to the instance.
//!
//! @param [in] delta matrix to add
//! @param [in] scaleFactor scale to apply to added matrix.
//!
void Add (DMatrix4dCR delta, double scaleFactor);

//!
//! Scale in place.
//! @param [in] scaleFactor scale to apply
//!
void Scale (double scaleFactor);


//! Add [xx xy xz x; xy yy yz y; xz yz zz 1; x y z 1] * scale
//! @param [in] xyz x,y,z for products.  Implicitly extened to DPoint4d [x,y,z,1] for products
//! @param [in] scale scale to apply to all terms.
void AddSymmetricScaledOuterProduct (DPoint3d xyz, double scale);

//! Add [xx xy xz wx; xy yy yz wy; xz yz zz wz; wx wy wz ww] * scale
//! @param [in] xyzw x,y,z,w for products.
//! @param [in] scale scale to apply to all terms.
void AddSymmetricScaledOuterProduct (DPoint4dCR xyzw, double scale);

//! Add scale * (U*V^ + V*U^) to the instance.
//! @param [in] U first operand
//! @param [in] V second operand
//! @param [in] scale scale factor to apply to all terms.
void AddSymmetricScaledOuterProduct (DPoint4dCR U, DPoint4dCR V, double scale);

//! Copy all 6 values from the upper triangle to mirrored positions in the lower triangle.
void CopyUpperTriangleToLower ();

//!
//! Subtract a matrix (componentwise) from the instance
//!
//! @param [in] delta matrix to subtract
//!
void Subtract (DMatrix4dCR delta);

/*__PUBLISH_SECTION_END__*/
//!
//! Factor a matrix by Householder reflections
//!
//! @param [out] pMatrixQ Matrix of reflection vectors
//! @param [out] matrixR Upper triangular factor
//! @param [in] matrixA Matrix to factor
//! @return true if invertible
//!
bool FactorAsQR
(
DMatrix4dR      matrixR,
DMatrix4dCR     matrixA
);

//!
//! Invert a matrix using QR (Householder reflection) factors.
//!
//! @param [out] pMatrixB Inverse matrix
//! @param [in] matrixA Original matrix
//! @return true if inversion completed normally.
//!
bool QrInverseOf (DMatrix4dCR matrixA);

//!
//!
//! Explode a 4x4 matrix into a 3x3 matrix, two 3D vectors, and a scalar, with
//! the scalar location defined by a pivot index along the diagonal.
//!
//! @param [out] matrix 3x3 submatrix
//! @param [out] row off-diagonals in row
//! @param [out] column off-diagonals in column
//! @param [out] pivotValue pivot entry
//! @param [in] pivot pivot index
//! @return true if pivot index is valid.
//!
bool ExtractAroundPivot
(
RotMatrixR      matrix,
DPoint3dR       row,
DPoint3dR       column,
double          &pivotValue,
int             pivot
) const;
/*__PUBLISH_SECTION_START__*/

//!
//! Compute A = Bt * D * B where B is a matrix and D is a diagonal matrix
//! given as a vector.
//! REMARK: This is a Very Bad Thing for numerical purposes.  Are you sure
//! you can't get your result without forming this product?
//!
//! @param [in] sigma entries of diagonal matrix D
//! @param [in] B matrix B
//!
void InitSymmetricProduct
(
DPoint4dCR      sigma,
DMatrix4dCR     B
);

//!
//! Return {A = T * B * Tt * s} where T is an affine transform (expanded to 4x4 with 0001 final row),
//!   B is a DMatrix4d, and s is a scale factor.
//!
//! This is used to do change of basis when entries in B are xx,xy,xz,x moments etc, T is the (possibly non-uniform) transformation,
//!  and s is a scale factor for changing the integration domain.  (If the integrations were volume integrals, s is the determinant of the
//!  orientation part of T.  If the integrations were area integrals in a plane, s is the jacobian of the plane-to-plane
//!  transformation.)
//!
//! @param [in] transform transform to apply
//! @param [in] momentMatrix matrix {B}
//! @param [in] scaleFactor
//!
static DMatrix4d FromSandwichProduct
(
TransformCR     transform,
DMatrix4dCR     momentMatrix,
double          scaleFactor
);

//!
//! Input matrix contains products of inertia {[xx,xy,xz,x; xy yy yz y; xz yz zz z; x y z v]}.
//! returned values are centroid, principal directions and 2nd moment tensor entries (yy+zz,xx+zz,xx+yy)
//! @param [out] volume volume
//! @param [out] centroid center of mass
//! @param [out] axes columns of this matrix are the principal x,y,z directions.
//! @param [out] momentTensorDiagonal tensor entries (inertias for rotation around each axis)
bool ConvertInertiaProductsToPrincipalMoments (double &volume, DVec3dR centroid, RotMatrixR axes, DVec3dR momentTensorDiagonal) const;

//!
//! Input matrix contains products of inertia {[xx,xy,xz,x; xy yy yz y; xz yz zz z; x y z v]}.
//! returned values are centroid, principal directions and 2nd moment tensor entries (yy+zz,xx+zz,xx+yy)
//! @param [in] localToWorld transformation from local coordinate system (where the products were computed) to world
//! @param [out] volume volume
//! @param [out] centroid center of mass
//! @param [out] axes columns of this matrix are the principal x,y,z directions.
//! @param [out] momentTensorDiagonal tensor entries (inertias for rotation around each axis)
bool ConvertInertiaProductsToPrincipalMoments (TransformCR localToWorld, double &volume, DVec3dR centroid, RotMatrixR axes, DVec3dR momentTensorDiagonal) const;

//!
//! Input matrix contains products of inertia {[xx,xy,xz,x; xy yy yz y; xz yz zz z; x y z v]}.
//! returned values are centroid, principal directions and 2nd moment tensor entries (yy+zz,xx+zz,xx+yy)
//! @param [in] localToWorld transformation from local coordinate system (where the products were computed) to world
//! @param [out] area area
//! @param [out] centroid center of mass
//! @param [out] axes columns of this matrix are the principal x,y,z directions.
//! @param [out] momentTensorDiagonal tensor entries (inertias for rotation around each axis)
bool ConvertInertiaProductsToPrincipalAreaMoments (TransformCR localToWorld, double &area, DVec3dR centroid, RotMatrixR axes, DVec3dR momentTensorDiagonal) const;

//!
//! Input matrix contains products of inertia {[xx,xy,xz,x; xy yy yz y; xz yz zz z; x y z v]}.
//! returned values are centroid, principal directions and 2nd moment tensor entries (yy+zz,xx+zz,xx+yy)
//! @param [in] localToWorld transformation from local coordinate system (where the products were computed) to world
//! @param [out] length length
//! @param [out] centroid center of mass
//! @param [out] axes columns of this matrix are the principal x,y,z directions.
//! @param [out] momentTensorDiagonal tensor entries (inertias for rotation around each axis)
bool ConvertInertiaProductsToPrincipalWireMoments (TransformCR localToWorld, double &length, DVec3dR centroid, RotMatrixR axes, DVec3dR momentTensorDiagonal) const;



//! Return the product matrix resulting from sweeping geometry with given baseProducts;
//! @param [in] baseProducts products integrated on base curve or area.
//! @param [in] sweepVector vector for sweep (extrusion)
//! @return products for integration over swept body.
static DMatrix4d SweepMomentProducts (DMatrix4dCR baseProducts, DVec3d sweepVector);

//! Return matrix, final row, final column, and scalar parts
//! @param [out] products upper 3x3 part
//! @param [out] row xyz parts of final row
//! @param [out] column xyz parts of final column
//! @param [out] scalar ww entry.
void GetBlocks (RotMatrixR products, DVec3dR row, DVec3dR column, double &scalar) const;

//!
//! Search the matrix for the entry with the largest absolute value.
//!
//! @return max absolute value entry in the matrix.
//!
double MaxAbs () const;

//!
//! Compute max absolute difference between various parts of matrices.
//! @param [in] other matrix to compare to.
//! @param [out] matrixDiff difference in matrix part.
//! @param [out] colDiff difference in final column XYZ part
//! @param [out] rowDiff difference in final row XYZ part
//! @param [out] weightDiff difference in final diagonal entry
//! @return max absolute difference
//!
double MaxAbsDiff (DMatrix4dCR other, double &matrixDiff, double &colDiff, double &rowDiff, double &weightDiff) const;

//!
//! Compute max absolute value in various parts of matrix
//! @param [out] matrixMax difference in matrix part.
//! @param [out] colMax difference in final column XYZ part
//! @param [out] rowMax difference in final row XYZ part
//! @param [out] weightAbs absolute value of WW entry
//! @return max absolute value
//!
double MaxAbs (double &matrixMax, double &colMax, double &rowMax, double &weightAbs) const;

//!
//! Tests if pInstance is an identity transform and returns the bool
//! indicator.
//! The matrix is considered an identity if the sum of squared components
//! after subtracting 1 from the diagonals is less than a small tolerance
//!
//! @return true if matrix is approximately an identity.
//!
bool IsIdentity () const;

//!
//! Tests if the instance has 0001 final row, within tolerance.
//!
//! @return true if matrix is approximately affine.
//!
bool IsAffine () const;

//!
//! Approximate solution of A*X=0, i.e. find an approximate null vector;
//! A is assumed upper triangular.
//! Method: Find the smallest diagonal.
//!         Set that entry in X to 1.
//!         Backsolve the linear system with RHS 0 and the chosen X component fixed.
//! @param [out] nullVector approximate null space vector.
//! @param [out] residual estimate of error in A*X. (Squared residual.)
//!
bool ApproximateNullVectorForUpperTriangle
(
DPoint4dR       nullVector,
double          &residual
) const;

#endif
};
END_BENTLEY_GEOMETRY_NAMESPACE
