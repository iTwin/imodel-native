/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/bsibasegeom/test/test15/TriDiagonalSolver.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*-------------------------------------------------------------
@description Storage and computation for a tridiagonal matrix.
-------------------------------------------------------------*/
class TriDiagonalSolver
{
private:
    double *mpLeft;
    double *mpDiag;;
    double *mpRight;

    int mNumAllocated;
    int mNumRow;
public:

/*-------------------------------------------------------------
@description Release all memory addressed via pointers within the object.
-------------------------------------------------------------*/
void ReleaseMem ();

/*-------------------------------------------------------------
@description zero out the matrix.
-------------------------------------------------------------*/
void Zero ();

/*-------------------------------------------------------------
@description zero out the matrix.
@param numRow IN number of rows to allocate.
-------------------------------------------------------------*/
bool Allocate (int numRow);

/*-------------------------------------------------------------
@description destructor.  Releases memory.
-------------------------------------------------------------*/
~TriDiagonalSolver ();

/*-------------------------------------------------------------
@description Constructor.   Allocate for specified rows.
@param numRow IN number of rows.
-------------------------------------------------------------*/
TriDiagonalSolver (int numRow);

/*-------------------------------------------------------------
@description query the number of rows.
@return number of rows.
-------------------------------------------------------------*/
int GetNumRows ();

/*-------------------------------------------------------------
@description set all three entries in a specified row.
@param row IN row index
@param aLeft IN left-of-diagonal value.
@param aDiag IN diagonal value.
@param aRight IN right-of-diagonal value.
@return true if row index is valid.
-------------------------------------------------------------*/
bool SetRow
    (
    int row,
    double aLeft,
    double aDiag,
    double aRight
    );

/*-------------------------------------------------------------
@description LU factorization.  Factorization is IN PLACE,
    with factorization terms retained for subsequent application to
    RHS values.
@return true if factorization completes (i.e. no division by zero)
-------------------------------------------------------------*/
bool FactorInPlace
(
);

/*-------------------------------------------------------------
@description Multipy the matrix (A) times another matrix (X).
The X matrix and product matrix RHS are both stored in "row major order"
@remark THIS IS NOT VALID AFTER FACTORING.
@remark X and RHS must be different matrices.
@param pX IN X matrix, stored in row major order.
@param pRHS OUT computed product A * X, in row major order.
@param numRow IN number of rows.  Must match with matrix.
@param numRHS IN number of columns in X, RHS.
@return false if count mismatch or less than 2 rows.
-------------------------------------------------------------*/
bool MultiplyRowMajor
(
// IN Multiple-column X matrix, in row-major order.
double const *pX,
// OUT Multiple-column result matrix, in row-major order.
double *pRHS,
// IN number of rows.  Must agree with matrix.
int numRow,
// number of columns.
int numRHS
);

/*-------------------------------------------------------------
@description Apply previous factorization to RHS ("Right Hand Side")
    Back substitute to replace RHS with solution of linear system.
The RHS and solutions matrix is stored in "row major order"
@remark THIS IS ONLY VALID AFTER FACTORING.
@param pRHS INOUT On input, this is the right hand side.
            On return, this is the solution.
@param numRow IN number of rows.  Must match with matrix.
@param numRHS IN number of columns in RHS
@return false if count mismatch or less than 2 rows.
-------------------------------------------------------------*/
bool SolveRowMajorInPlace
(
// RHS vector becomes solutions !!!!
double *pRHS,
int numRow,
int numRHS
);

/*-------------------------------------------------------------
@description print entire matrix.
-------------------------------------------------------------*/
void TriDiagonalSolver::PrintCoffs
(
char *pTitle
);

};
