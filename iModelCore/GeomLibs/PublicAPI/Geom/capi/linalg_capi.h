/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//!
//! Compute the dot product of two vectors given start pointers and stride.
//! @param pA     IN      start pointer for vector A
//! @param stepA  IN      step within vector A
//! @param pB     IN      start pointer for vector A
//! @param stepB  IN      step within vector B
//! @param n      IN      number of components
//!
Public GEOMDLLIMPEXP double bsiLinAlg_dot    /* dot product of vectors*/
(
double *pA,
int    stepA,
double *pB,
int    stepB,
int    n
);

//!
//! Compute the dot product of two vectors given start pointers and stride, both
//! restricted as const.
//! @param pA     IN      start pointer for vector A
//! @param stepA  IN      step within vector A
//! @param pB     IN      start pointer for vector A
//! @param stepB  IN      step within vector B
//! @param n      IN      number of components
//!
Public GEOMDLLIMPEXP double bsiLinAlg_dotConst
(
const double *pA,
int    stepA,
const double *pB,
int    stepB,
int    n
);

//!
//! Multiply matrices in stride format.
//! @param pAB        OUT     start point for product.  Assumed distinct from factors
//! @param rowStepAB  =>row-to-row step within AB
//! @param colStepAB  IN      column-to-column step within AB
//! @param numRowAB   IN      number of rows in AB and A
//! @param numColAB   IN      number of columns in AB and B
//! @param pA         IN      start pointer for matrix A
//! @param rowStepA   IN      row-to-row step within A
//! @param colStepA   IN      column-to-column step within A
//! @param numColA    IN      number of columns of A, rows of B
//! @param pB         IN      start pointer for matrix A
//! @param rowStepB   IN      row-to-row step within B
//! @param colStepB   IN      column-to-column step within B
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_multiplyMatrixMatrix
(
double *pAB,            /* IN      start pointer for product.  Assumed distinct from factors. */
int     rowStepAB,      /* IN      row-to-row step within AB */
int     colStepAB,      /* IN      column-to-column step within AB */
int     numRowAB,       /* IN      number of rows in AB and A */
int     numColAB,       /* IN      number of columns in AB and B */
const double *pA,       /* IN      start pointer for matrix A*/
int     rowStepA,       /* IN      row-to-row step within A */
int     colStepA,       /* IN      column-to-column step within A */
int     numColA,        /* IN      number of columns of A, rows of B */
const double *pB,       /* IN      start pointer for matrix A*/
int     rowStepB,       /* IN      row-to-row step within B */
int     colStepB        /* IN      column-to-column step within B */
);

//!
//! Multiply matrices in dense, row-major format.
//! @param pAB        OUT     start point for product.
//!                       Assumed distinct from factors
//! @param numRowAB   IN      number of rows in AB and A
//! @param numColAB   IN      number of columns in AB and B
//! @param pA         IN      start pointer for matrix A
//! @param numColA    IN      number of columns of A, rows of B
//! @param pB         IN      start pointer for matrix A
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_multiplyDenseRowMajorMatrixMatrix
(
double  *pAB,
int     numRowAB,
int     numColAB,
const   double  *pA,
int     numColA,
const   double  *pB
);

//!
//! Fill a vector with zeros except for a specified nonzero.
//! @param pA         IN      start pointer for vector A
//! @param stepA      IN      step within vector A
//! @param n          IN      number of entries in vector (not referenced. i0 and i1 really control the function.)
//! @param i0         IN      first index to zero
//! @param i1         IN      one after last index to zero
//! @param iSpike     IN      index to receive spike value
//! @param value      IN      spike value
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_initZerosWithSpike
(
double *pA,
int    stepA,
int    n,
int    i0,
int    i1,
int    iSpike,
double value
);

//!
//! Swap corresponding entries in two vectors.
//! @param pA     IN OUT  start pointer for vector A
//! @param pB     IN OUT  start pointer for vector B
//! @param stepA  IN      step within vector A
//! @param stepB  IN      step within vector B
//! @param n      IN      number of entries in vectors
//!
Public GEOMDLLIMPEXP void bsiLinAlg_swap
(
double *pA,
double *pB,
int     stepA,
int     stepB,
int     n
);

//!
//! Swap single entries within a vector.
//! @param pIndexArray IN OUT  array where swap is carried out.
//! @param i0 IN      first index
//! @param i1 IN      second index
//!
Public GEOMDLLIMPEXP void bsiLinAlg_swapIndices
(
int *pIndexArray,
int     i0,
int     i1
);

//!
//! Apply a row swap sequence to an array.
//! @param pMatrix IN OUT  matrix of swappees.
//! @param pIndex IN      array of succesive rows swapped to pivots.
//! @param nSwap IN      number of swaps
//! @param stepA IN      step between adjacent values in column.
//! @param stepB IN      step between adjacent values in row.  (internal step during each swap)
//! @param nB IN      number of items per swap (number per row)
//! @param reverse IN      true to apply swaps in reverse order.
//!
Public GEOMDLLIMPEXP void bsiLinAlg_swapAll
(
double  *pMatrix,       /* IN OUT  matrix to manipulate*/
int     *pIndex,        /* IN      array of nA row indices*/
int     nSwap,          /* IN      number of swaps*/
int     stepA,          /* IN      first direction step*/
int     stepB,          /* IN      second direction step (internal step during each swap)*/
int     nB,             /* IN      second direction count (number of items within each swap)*/
int     reverse         /* IN      true to apply in reverse order*/
);

//!
//! Copy a rectangular block.
//! @param pDest IN OUT  destination matrix
//! @param destRowStep IN      row-to-row step in destination
//! @param destColStep IN      column-to-column step in destination
//! @param pSource IN      source matrix
//! @param sourceRowStep IN      row-to-row step in destination
//! @param sourceColStep IN      column-to-column step in destination
//! @param nRow IN      number of rows
//! @param nCol IN      number of columns
//!
Public GEOMDLLIMPEXP void bsiLinAlg_copyBlock
(
double *pDest,
int    destRowStep,
int    destColStep,
double *pSource,
int    sourceRowStep,
int    sourceColStep,
int    nRow,
int    nCol
);

//!
//! Apply a rank one block update to a rectangular matrix,
//!    i.e. pMatrix[iA][iB] += scale * pA[iA] * pB[iB]
//! @param pMatrix        IN OUT  matrix to be updated
//! @param stepAMatrix    IN      step within matrix corresponding to step in A
//! @param stepBMatrix    IN      step within matrix corresponding to step in B
//! @param scale          IN      scale factor to apply
//! @param pA             IN      start pointer for vector A
//! @param stepA          IN      step within vector A and between rows of matrix
//! @param nA             IN      size of A
//! @param pB             IN      start pointer for vector B
//! @param stepB          IN      step within vector B and between columns of matrix
//! @param nB             IN      size of B
//!
Public GEOMDLLIMPEXP void bsiLinAlg_updateBlock
(
double *pMatrix,
int    stepAMatrix,
int    stepBMatrix,
double scale,
double *pA,
int    stepA,
int    nA,
double *pB,
int    stepB,
int    nB
);

//!
//! Find the indices of the maximum absolute entry in a matrix.
//! @param pIndexA        IN      first index (e.g. row)
//! @param pIndexB        IN      second index (e.g. column)
//! @param pMatrix        IN OUT  matrix to be search
//! @param stepA,         IN      first step size
//! @param nA,            IN      number of A steps
//! @param stepB,         IN      second step size
//! @param nB             IN      number of B steps
//!
Public GEOMDLLIMPEXP double bsiLinAlg_SearchMax
(
int     *pIndexA,
int     *pIndexB,
const double  *pMatrix,
int     stepA,
int     nA,
int     stepB,
int     nB
);

//!
//! @param        pIndexMin OUT     index of minimum, in steps
//! @param        pIndexmax OUT     index of maximum, in steps
//! @param        pMin      IN      min value
//! @param        pMax      IN      max value
//! @param        pVector   IN      vector to search
//! @param        step      IN      step size in vector
//! @param        n         IN      number of items to search
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_searchMinMax
(
int     *pIndexMin,
int     *pIndexMax,
double  *pMin,
double  *pMax,
const double  *pVector,
int     step,
int     n
);

//!
//! set pIndexArray[i] == i for 0 OUT     i < n
//! @param        pIndexArea OUT     initialized array
//! @param        n          IN      number of indices
//!
Public GEOMDLLIMPEXP void bsiLinAlg_initIndices
(
int     *pIndexArray,   /* OUT     array to be initialized */
int     n               /* IN      number of indices*/
);

//!
//! In-place LU factorization with full pivot.
//! @param    pMatrix     IN OUT  matrix to factor.
//! @param    pRowSwap    OUT     array of row swap indices.
//! @param    pColSwap    OUT     array of column swap indices.
//! @param    rowStep     IN      address step when row is incremented.
//! @param    nRow        IN      number of rows.
//! @param    colStep     IN      address step when column is incremented.
//! @param    nCol        IN      number of columns.
//!
Public GEOMDLLIMPEXP int bsiLinAlg_LUFullPivot       /* OUT     rank of matrix*/
(
double  *pMatrix,
int     *pRowSwap,
int     *pColSwap,
int     rowStep,
int     nRow,
int     colStep,
int     nCol
);

//!
//! Back substitution to solve an upper triangular linear system.
//!
//! The structure of the matrices is:
//! <pre>
//! [ P U U U ][ S  S  S  S ] = [ B B B B]
//! [ o P U U ][ S  S  S  S ]   [ B B B B]
//! [ o o P U ][ S  S  S  S ]   [ B B B B]
//! [ o o o P ][ S  S  S  S ]   [ B B B B]
//!
//! o = a zero introduced during factorization. (But the factorization
//!       process probably filled these spots with other things...
//!       null vector computation never reads or writes them.)
//! P = a nonzero pivot from factorization.
//! U = an entry in the upper triangle of the factored matrix, within
//!       the pivot column portion.
//! B = a right hand side value (overwritten by S)
//! S = a value computed by this routine via back substitution.
//! </pre>
//!
//! @param pB         IN OUT  On input, the right side (Y) values
//!                       On output, the solution (S) values.
//! @param bRowStep   IN      address step when row of B is incremented.
//! @param bColStep   IN      address step when column of B is incremented.
//! @param nColB      IN      number of columns in B
//! @param pU         IN      upper triangular matrix
//! @param uRowStep   IN      address step when row of U is incremented.
//! @param uColStep   IN      address step when column of U is incremented.
//! @param nRowU      IN      number of rows (and columns) of U.
//!
Public GEOMDLLIMPEXP bool    bsiLinAlg_backSub
(
double  *pB,
int     bRowStep,       /* IN      row step in solution*/
int     bColStep,       /* IN      column step in solution*/
int     nColB,          /* IN      number of columns in B*/
double  *pU,            /* IN      upper triangular matrix*/
int     uRowStep,       /* IN      address step when row is incremented*/
int     uColStep,       /* IN      address step when column is incremented*/
int     nRowU           /* IN      number of rows (and columns) of U*/
);

//!
//! Apply the L part of an LU factorization to a RHS matrix.
//!
//! The structure of the LU matrix is:
//! <pre>
//! [ P U U U ]
//! [ L P U U ]
//! [ L L P U ]
//! [ L L L P ]
//!
//! L = lower triangle of factorization
//! P = a nonzero pivot from factorization.
//! U = upper triangle of factorization
//! </pre>
//! @param pB         IN OUT  On input, the right side (Y) values
//!                       On output, the solution (S) values.
//! @param rowStepB   IN      address step when row of B is incremented.
//! @param colStepB   IN      address step when column of B is incremented.
//! @param nColB      IN      number of columns in B
//! @param pLU        IN      upper triangular matrix
//! @param pRowSwap   IN      array of row swap indices.
//! @param pColSwap   IN      array of column swap indices.
//! @param rowStep    IN      address step when row of pLU is incremented.
//! @param nRow       IN      number of rows of pLU
//! @param columnStep IN      address step when column of pLU is incremented.
//! @param nCol       IN      number of columns of pLU
//! nPivot            IN      number of pivots to apply (e.g. rank - 1)
//!
Public GEOMDLLIMPEXP void bsiLinAlg_applyLFactor
(
double  *pB,
int     rowStepB,
int     colStepB,
int     nColB,
double  *pLU,
int     *pRowSwap,
int     *pColSwap,
int     rowStep,
int     nRow,
int     colStep,
int     nCol,
int     nPivot
);

//!
//! Overwrite columns of pNullVectors with columns of the null space
//! of a factored matrix.
//! The structure of the matrices is:
//! <pre>
//! [ P U U U Y Y Y Y ][ S  S  S  S ] = [ 0 ]
//! [ o P U U Y Y Y Y ][ S  S  S  S ]   [ 0 ]
//! [ o o P U Y Y Y Y ][ S  S  S  S ]   [ 0 ]
//! [ o o o P Y Y Y Y ][ S  S  S  S ]   [ 0 ]
//!                    [-1  0  0  0 ]   [ 0 ]
//!                    [ 0 -1  0  0 ]   [ 0 ]
//!                    [ 0  0 -1  0 ]   [ 0 ]
//!                    [ 0  0  0 -1 ]   [ 0 ]
//!
//! o = a zero introduced during factorization. (But the factorization
//!       process probably filled these spots with other things...
//!       null vector computation never reads or writes them.)
//! P = a nonzero pivot from factorization.
//! U = an entry in the upper triangle of the factored matrix, within
//!       the pivot column portion.
//! Y = an entry in the upper triangle of the factored matrix, in the
//!       non-pivot portion.
//! S = a value computed by this routine via back substitution.
//!       Each column is the solution of [U] * [S] = [Y]
//!       where [U] is the pivot portion of the factored matrix, and [Y]
//!       is a column from the non-pivot portion.
//!       Corresponding Y and S columns may overwrite in a square matrix.
//! </pre>
//! The -1 and 0 entries below the S values are supplied by this
//! function.
//!
//! @param pNullVectors   OUT     matrix to receive the nCol-rank null vectors
//! For a square matrix, this may be the address of column rank+1 of pMatrix.
//! Computations are ordered so that all necessary values
//! from those columns are used before being overwritten.
//! @param nullVectorRowStep IN      row step in null vectors
//! @param nullVectorColStep IN      column step in null vectors
//! @param pMatrix    IN      reduced and swapped matrix from LU factorization
//! @param pRowSwap   IN      array of nA row indices
//! @param pColSwap   IN      array of nB column indices
//! @param rowStep    IN      address step when row is incremented
//! @param nRow       IN      number of rows
//! @param colStep    IN      address step when column is incremented
//! @param nCol       IN      number of colums
//! @param rank       IN      matrix rank as determined by factorization.
//! @return the number of null vectors computed.
//!
Public GEOMDLLIMPEXP int bsiLinAlg_expandNullSpace
(
double  *pNullVectors,
int     nullVectorRowStep,
int     nullVectorColStep,
double  *pMatrix,
int     *pRowSwap,
int     *pColSwap,
int     rowStep,
int     nRow,
int     colStep,
int     nCol,
int     rank
);

//!
//! @param pNullVectors OUT  null vectors (as columns)
//! @param pMatrix IN      matrix analyzed.
//! @returnn the number of vector (columns) placed into pNullVectors.
//!
Public GEOMDLLIMPEXP int bsiDMatrix4d_nullSpace
(
DMatrix4dP pNullVectors,
DMatrix4dP pMatrix
);

//!
//! Invert a 4x4 matrix using full swapping (rows and columns)
//!
//! @param pInverse OUT     inverse matrix.
//! @param pCond    OUT     estimate of condition number.   Near zero is bad, near 1 is good.
//! @param pMatrix  IN      matrix to invert.
//! @return false if unable to invert
//!
Public GEOMDLLIMPEXP bool     bsiDMatrix4d_invertFullSwapCond
(
DMatrix4dP pInverse,
double          *pCond,
DMatrix4dCP pMatrix
);

//!
//! Init an identity matrix
//! @param pA OUT     buffer initialized to identity.
//! @param n  IN      number of rows and columns
//! @param rowStep IN      row-to-row step (in doubles)
//! @param colStep IN      column-to-column step (in doubles)
//!
Public GEOMDLLIMPEXP void bsiLinAlg_initIdentity
(
double *pA,
int     n,
int     rowStep,
int     colStep
);

//!
//! @param pQ     <=Full orthogonal factor. Must be different from pA.
//! @param pA     IN OUT  On input, the full matrix.  Replaced by triangular factor.
//! @param nARow  IN      rows in A, hence rows, columns of Q.
//! @param nACol  IN      columns in A, hence rows, columns of Q.
//! @param rowStepQ IN      row-to-row step in Q
//! @param colStepQ IN      column-to-column step in Q
//! @param rowStepA IN      row-to-row step in A
//! @param colStepA IN      column-to-column step in A
//!
Public GEOMDLLIMPEXP void bsiLinAlg_givensQR
(
double      *pQ,
double      *pA,
int         nARow,
int         nACol,
int         rowStepQ,
int         colStepQ,
int         rowStepA,
int         colStepA
);

//!
//! Invert a general matrix using givens rotations for an initial
//! triangular factorization.
//!
//! The diagonals of the factorization are inspected to determine if
//! the matrix is singular.
//!
//! If a diagonal entry is zero, or very small relative to its row and
//! column, the matrix is deemed singular.   On return from this case,
//! pWork contains the triangular factor R, and pAinv is the orthogonal
//! matrix Q^T.  Q^T*A = R, and A = Q * R.
//! pAinv contains the orthgonal matrix Q used to triangularize.
//!
//! If the diagonals are all healthy, the Q matrix is overwritten by
//! the inverse.  This is done by backsolving R*B=Q^T for B, which is
//! the inverse of A.
//! @param pAinv OUT     inverse.  Same memory layout as A. Must be different from pWork and pA.
//! @param pWork OUT     work area, same layout as pA and pAinv.  May be same as pA, but
//!       in this case contents of pA will be destroyed.
//! @param pCond OUT     estimate of condition.  0 is singular, near-zero is bad, and 1 is best.
//! @param pA IN      original square matrix.
//! @param n IN      number of rows (hence columns!!)
//! @param nACol IN      number of columns -- not used.
//! @param rowStep IN      row-to-row increment within A
//! @param colStep IN      col-to-col increment within A
//!
Public GEOMDLLIMPEXP bool    bsiLinAlg_givensInverse
(
double      *pAinv,
double      *pWork,
double      *pCond,
double      *pA,
int         n,
int         nACol,
int         rowStep,
int         colStep
);

//!
//! Compute QT*A*Q, where Q is a rotation matrix and A is 2x2 symmetric
//! given by its three distinct values.
//! @param pAii IN OUT  ii entry. Updated in place.
//! @param pAij IN OUT  ij entry. Updated in place.
//! @param pAjj IN OUT  jj entry. Updated in place.
//! @param cc IN      cosine of rotation angle.
//! @param ss IN      sine of rotation angle.
//!
Public GEOMDLLIMPEXP void bsiDMatrix3d_symmetric2x2Rotation
(
double      *pAii,
double      *pAij,
double      *pAjj,
double      cc,
double      ss
);

/*----------------------------------------------------------------------+
* Given SYMMETRIC (but full storage) matrix A, form Q*A*Q'
* where Q is a givens rotation, i.e. an identity matrix except for
* the four positions (i,i)=c, (i,j) = s, (j,i)=-s, (j,j)=c
* @param pMatrix IN OUT  dense, row-major symmetric matrix.
* @param n IN      size of matrix.
* @param i IN      rotation index.
* @param j IN      rotation index.
* @param cc IN      cosine of rotation angle.
* @param ss IN      sine of rotation angle.
* @return false if i==j or either is out of range.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix3d_symmetricRotation
(
double      *pMatrix,
int         n,
int         i,
int         j,
double      cc,
double      ss
);

//!
//! @param pMatrix IN OUT  matrix where rows is to be rotated.  (Or column,
//!       by choice of steps.)
//! @param rowStep IN      row-to-row step.
//! @param colStep IN      col-to-col step.
//! @param nCol IN      number of columns.
//! @param i IN      row corresponding to (cc, ss) row of rotation.
//! @param j IN      row corresponding to (-ss,cc) row of rotation.
//! @param cc IN      cosine of rotation angle.
//! @param ss IN      sine of rotation angle.
//!
Public GEOMDLLIMPEXP void bsiDMatrix3d_rotate
(
double      *pMatrix,       /* IN OUT  matrix*/
int         rowStep,        /* IN      row step in A*/
int         colStep,        /* IN      column step in A*/
int         nCol,           /* IN      columns of A*/
int         i,              /* IN      index of cc diagonal*/
int         j,              /* IN      index of ss diagonal*/
double      cc,             /* IN      c coefficient*/
double      ss              /* IN      s coefficient*/
);

//!
//! Compute pMatrix such that pMatrix * pVector 'rotates' selective
//! parts of the vector into a single location.   pIndex[0] is the
//! target index of the rotation.   pIndex[1], pIndex[2] ... are zeroed
//! by the multiplication.  (Note that pVector itself is not actually
//! modified.   The zeroing 'would occur' if pMatrix * pVector were to
//! be carried out)
//! @param pMatrix OUT     orthogonal matrix.
//! @param pMag OUT     magnitude of rotated vector.
//! @param n IN      size of rotation matrix.
//! @param pVector IN      dense vector to be zeroed.
//! @param pIndex IN      indices of vector entries to be affected.
//!                       pIndex[0] is the pivot entry which receives rotated content.
//!                       pIndex[1..n-1]
//! @param nRot IN      number of rotations, i.e. n-1.  (Hmm??)
//! @return false if nRot < 1 or nRot >= n, or any other index out of bounds.  No check for
//!               repeated indices.
//!
Public GEOMDLLIMPEXP bool     bsiDMatrix3d_initSelectiveRotation
(
double          *pMatrix,
double          *pMag,
int             n,
const double    *pVector,
const int       *pIndex,
int       nRot
);

//!
//! Initialize a  matrix of size numEq * numEq, and right side of size
//! numEq, to be used for subseqent incremental least squares entry.
//! @param pA IN OUT  coefficient matrix.
//! @param pB IN OUT  right side vector
//! @param pEE IN OUT  accumulating squared error.
//! @param numEq IN      number of rows and columns.
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_initIncrementalLeastSquares
(
double          *pA,
double          *pB,
double          *pEE,
int             numEq
);

//!
//! Add one row (equation) to an incremental least squares problem.
//! The evolving matrix is maintained as a (full) upper triangular system
//! with lower zeros left untouched.
//! @param pA IN OUT  coefficient matrix.
//! @param pB IN OUT  right side vector
//! @param pEE IN OUT  accumulating squared error.
//! @param pC IN OUT  work vector of size numEq doubles.
//! @param pRowCoffs IN      coefficients for the new row.
//! @param b IN      right side entry for new row.
//! @param numEq IN      number of rows and columns.
//!
Public GEOMDLLIMPEXP void     bsiLinAlg_extendIncrementalLeastSquares
(
double          *pA,            /* OUT     Left side matrix */
double          *pB,            /* OUT     Right side vector */
double          *pEE,           /* OUT     accumulating squared error */
double          *pC,            /* IN OUT  work vector of size numEq doubles */
double          *pRowCoffs,     /* IN      matrix entries for new row */
double          b,              /* IN      right side entry for new row */
int             numEq           /* IN      number of unknowns */
);

//!
//! Final solution step for evolved incremental least squares.
//! (back substitution in triangular system -- not uniqe to least squares)
//! @param pX OUT     solution vector.
//! @param pA IN      factored (triangular, dense row-order) matrix.
//! @param pB IN      factored RHS (i.e. Q vector applied)
//! @param numEq IN      number of equations.
//!
Public GEOMDLLIMPEXP bool        bsiLinAlg_solveIncrementalLeastSquares
(
double          *pX,
double          *pA,
double          *pB,
int             numEq
);

//!
//! In a square, triangular matrix, select a pivot to be forced to unit
//!   value so as to solve a homogeneous system.
//! @param pA IN      triangular factor.  Assumed in dense, row major order.
//! @param numEq IN      number of equations
//!
Public GEOMDLLIMPEXP int      bsiLinAlg_selectHomogeneousLeastSquaresPivot
(
double          *pA,            /* IN      Left side matrix */
int             numEq           /* IN      number of unknowns */
);

//!
//! Given an upper triangular A, solve A*X = 0 with a designated pivot
//! fixed at 1, using givens rotations to reorthogonalize after removal
//! of the pivot row.  (Hint: use bsiLinAlg_selectHomogeneousLeastSquaresPivot
//!   to pick the pivot index.)
//! @param pX OUT     solution vector.
//! @param pResidual OUT     residual of solution.
//! @param pA IN OUT  coefficient matrix. Destroyed during factorization.  Assumed
//!                   dense, row major order.
//! @param kPivot IN      index of unit solution component.
//! @param numEq IN      number of equations.
//!
Public GEOMDLLIMPEXP bool        bsiLinAlg_solveHomogeneousLeastSquares
(
double          *pX,            /* OUT     solution vector */
double          *pResidual,     /* OUT     residual of solution */
double          *pA,            /* IN      coefficient matrix matrix */
int             kPivot,         /* IN      pivot index. */
int             numEq           /* IN      number of unknowns */
);

//!
//! Solve a dense linear system using Gaussian elimination with partial pivoting.
//! @remark Should turn repeated division by same pivot into multiplications by inverse.
//! @remark Absolute error tolerance for small pivots.
//! @param amatrix IN      dense, square matrix stored in row-major order.
//! @param dimensions IN      row and column dimension of matrix.
//! @param rhsides    IN      array of right hand sides stored in column-major order.
//! @param numrhs     IN      number of right hand sides.
//! @return false if system appears singular.
//!
Public GEOMDLLIMPEXP int      bsiLinAlg_solveLinearGaussPartialPivot
(
double  *amatrix,       /* IN      matrix of coefficients */
int     dimensions,     /* IN      dimension of matrix */
double  *rhsides,       /* IN OUT  input=right hand sides.  output=solutions */
int     numrhs          /* IN      number of right hand sides */
);

//!
//! Diagonalize a symmetric matrix by Jacobi's method.
//! The input matrix is destroyed!!!
//! Return eigenvalues and normalized eigenvectors (column vectors).
//! @param pA IN OUT  symmetric matrix.  Stored in row-major order.
//!               Contents are destroyed (diagonalized) by the iteration!
//! @param pQ OUT     matrix of eigenvectors.
//! @param pD OUT     array of n eigenvalues.
//! @param pB OUT     scratch array, length n.
//! @param pZ OUT     scratch array, length n.
//!
//!
Public GEOMDLLIMPEXP void bsiLinAlg_symmetricJacobiEigensystem
(
double  *pA,
double  *pQ,
double  *pD,
double  *pB,
double  *pZ,
int     n
);

//!
//! Solve a block tridiagonal linear system.
//! The conceptual matrix structure is
//!                                |X0|
//!       LL|DD RR 00 00 00 00|    |X1|    |F0|
//!         |LL DD RR 00 00 00|    |X2|    |F1|
//!         |00 LL DD RR 00 00|    |X3|    |F2|
//!         |00 00 LL DD RR 00|   *|X |  = |F3|
//!         |00 00 00 LL DD RR|    |X |    |F|
//!         |00 00 00 00 LL DD|RR  |XM|    |FM-1|
//!                                |XM+1|
//! Each LL, DD, RR denotes a square block.  The LL and RR outside the main matrix
//!   are placeholders added so that all block rows can be stored with their diagonal DD
//!   block at the same internal offsets.  Likewise X0 and XM are placeholders which are
//!   assigned zero values as part of the solution process.
//! If there are M block diagonals, each block being N by N, the matrix is supplied
//!   as M * N rows, each row containing 3N numeric entries in row-major layout.
//! That is, the first 3N coefficients are the llddrr sequence from (scalar) row 0.
//! The second 3N coefficients llddrr from row 1, up to row N-1.
//!
//! The right hand side column vector consists of M*N doubles
//! The solution vector consists of (M+2)*N doubles, with the first and last N as placeholders
//!   which are zeroed as part of the solution process.
//!
//! Some explanation for the placeholders: The placeholders fit nicely with typical geometric
//! setting in which each block row contains the equations for one interior point of a sequence
//! of (M+2)points numbered 0,1,..M+1.  The right hand side values are constraints applied to
//! each of the M interior points, and the soluton values X are corrections to be applied to
//! each point's coordinates.  The endpoints 0 and (M+1) are fixed.  With the placeholder
//! convention, the equation for each interior point can be constructed without consideration
//! for whether its neighbor is an interior or endpoint.   Just fill the block row as if all points
//! are interior. The endpoint blocks will be ignored and the endpoint solution values X0 and X(M+1)
//! zeroed out.
//!
//! Solution process is straight Gaussian elimination.  (No pivoting.)
//! @param pX OUT Solution values.
//! @param pA IN OUT coefficient matrix.  This is destroyed by the factorization.
//! @param pf IN OUT right hand side.  This is destroyed by the factorization.
//! @param numBlock IN number of block rows.
//! @param blockSize IN size of each (square) block.
//! @return true if solution completed.
//!
Public GEOMDLLIMPEXP bool    bsiLinalg_solveBlockTridiagonal
(
double *pX,
double *pA,
double *pf,
int     numBlock,
int     blockSize
);

END_BENTLEY_GEOMETRY_NAMESPACE

