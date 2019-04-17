/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef __cplusplus
class TriDiagonalSolver;

//! @description Storage and computation for a tridiagonal matrix.
class GEOMDLLIMPEXP TriDiagonalSolver
{
private:
    double *mpLeft;
    double *mpDiag;;
    double *mpRight;

    int mNumAllocated;
    int mNumRow;
public:


//! @description Release all memory addressed via pointers within the object.
void ReleaseMem ();

//! @description zero out the matrix.
void Zero ();

//! @description zero out the matrix.
//! @param [in] numRow  number of rows to allocate.
bool Allocate (int numRow);

//! @description destructor.  Releases memory.
~TriDiagonalSolver ();

//! @description Constructor.   Allocate for specified rows.
//! @param [in] numRow  number of rows.
TriDiagonalSolver (int numRow);

//! @description query the number of rows.
//! @return number of rows.
int GetNumRows ();

//! @description set all three entries in a specified row.
//! @param [in] row  row index
//! @param [in] aLeft  left-of-diagonal value.
//! @param [in] aDiag  diagonal value.
//! @param [in] aRight  right-of-diagonal value.
//! @return true if row index is valid.
bool SetRow
    (
    int row,
    double aLeft,
    double aDiag,
    double aRight
    );

//! @description LU factorization.  Factorization is IN PLACE,
//!     with factorization terms retained for subsequent application to
//!     RHS values.
//! @return true if factorization completes (i.e. no division by zero)
bool FactorInPlace ();

//! @description Multipy the matrix (A) times another matrix (X).
//! The X matrix and product matrix RHS are both stored in "row major order"
//! @remark THIS IS NOT VALID AFTER FACTORING.
//! @remark X and RHS must be different matrices.
//! @param [in] pX  X matrix, stored in row major order.
//! @param [out]pRHS  computed product A * X, in row major order.
//! @param [in] numRow  number of rows.  Must match with matrix.
//! @param [in] numRHS  number of columns in X, RHS.
//! @return false if count mismatch or less than 2 rows.
bool MultiplyRowMajor
(
double const *pX,
double *pRHS,
int numRow,
int numRHS
);

//! @description Apply previous factorization to RHS ("Right Hand Side")
//!     Back substitute to replace RHS with solution of linear system.
//! The RHS and solutions matrix is stored in "row major order"
//! @remark THIS IS ONLY VALID AFTER FACTORING.
//! @param [in,out] pRHS  On input, this is the right hand side.
//!             On return, this is the solution.
//! @param [in] numRow  number of rows.  Must match with matrix.
//! @param [in] numRHS  number of columns in RHS
//! @return false if count mismatch or less than 2 rows.
bool SolveRowMajorInPlace
(
double *pRHS,
int numRow,
int numRHS
);

#ifdef COMPILE_WITH_PRINTF
//! @description print entire matrix.
void PrintCoffs
(
char *pTitle
);
#endif

};
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
