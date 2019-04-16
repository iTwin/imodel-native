/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-------------------------------------------------------------
@description Release all memory addressed via pointers within the object.
-------------------------------------------------------------*/
void TriDiagonalSolver::ReleaseMem ()
    {
    if (mpLeft)
        delete [] mpLeft;
    if (mpRight)
        delete [] mpRight;
    if (mpDiag)
        delete [] mpDiag;
    mpLeft = mpRight = mpDiag = (double*)0;
    mNumAllocated = mNumRow = 0;
    }

/*-------------------------------------------------------------
@description zero out the matrix.
-------------------------------------------------------------*/
void TriDiagonalSolver::Zero ()
    {
    if (mNumAllocated > 0)
        {
        memset (mpLeft,  0, mNumAllocated * sizeof (double));
        memset (mpRight, 0, mNumAllocated * sizeof (double));
        memset (mpDiag,  0, mNumAllocated * sizeof (double));
        }
    }

/*-------------------------------------------------------------
@description zero out the matrix.
@param numRow IN number of rows to allocate.
-------------------------------------------------------------*/
bool TriDiagonalSolver::Allocate (int numRow)
    {
    if (numRow > mNumAllocated)
        ReleaseMem ();
    if (numRow <= 0)
        return false;
    if (numRow > mNumAllocated)
        {
        mNumAllocated = numRow;
        mpLeft = new double [numRow];
        mpDiag = new double [numRow];
        mpRight = new double [numRow];
        }
    if (!(mpLeft && mpDiag && mpRight))
        {
        ReleaseMem ();
        return false;
        }

    mNumRow = numRow;
    Zero ();
    return true;
    }

/*-------------------------------------------------------------
@description destructor.  Releases memory.
-------------------------------------------------------------*/
TriDiagonalSolver::~TriDiagonalSolver ()
    {
    ReleaseMem ();
    }
/*-------------------------------------------------------------
@description Constructor.   Allocate for specified rows.
@param numRow IN number of rows.
-------------------------------------------------------------*/
TriDiagonalSolver::TriDiagonalSolver (int numRow)
    {
    mpLeft = mpRight = mpDiag = (double*)0;
    mNumRow = mNumAllocated = 0;
    Allocate (numRow);
    }

/*-------------------------------------------------------------
@description query the number of rows.
@return number of rows.
-------------------------------------------------------------*/
int TriDiagonalSolver::GetNumRows () { return mNumRow;}

/*-------------------------------------------------------------
@description set all three entries in a specified row.
@param row IN row index
@param aLeft IN left-of-diagonal value.
@param aDiag IN diagonal value.
@param aRight IN right-of-diagonal value.
@return true if row index is valid.
-------------------------------------------------------------*/
bool TriDiagonalSolver::SetRow
    (
    int row,
    double aLeft,
    double aDiag,
    double aRight
    )
    {
    if (row >= 0 && row < mNumRow)
        {
        mpLeft  [row] = aLeft;
        mpDiag  [row] = aDiag;
        mpRight [row] = aRight;
        return true;
        }
    return false;
    }

/*-------------------------------------------------------------
@description LU factorization.  Factorization is IN PLACE,
    with factorization terms retained for subsequent application to
    RHS values.
@return true if factorization completes (i.e. no division by zero)
-------------------------------------------------------------*/
bool TriDiagonalSolver::FactorInPlace
(
)
    {
    int i, j;
    for (j = 0, i = 1; i < mNumRow; j = i++)
        {
        if (mpDiag[j] == 0.0)
            return false;
        double qi = mpLeft[i] / mpDiag[j];
        mpLeft[i] = qi;
        mpDiag[i]   -= qi * mpRight[j];
        if (mpDiag[i] == 0.0)
            return false;
        }

    return true;
    }

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
bool TriDiagonalSolver::MultiplyRowMajor
(
// IN Multiple-column X matrix, in row-major order.
double const *pX,
// OUT Multiple-column result matrix, in row-major order.
double *pRHS,
// IN number of rows.  Must agree with matrix.
int numRow,
// number of columns.
int numRHS
)
    {
    if (numRow != mNumRow)
        return false;
    if (numRow < 2)
        return false;
    int lastRow  = numRow - 1;
    for (int j0 = 0; j0 < numRHS; j0++)
        {
        int k = j0;
        int i = 0;
        pRHS[k] = mpDiag[i]  * pX[k]
                + mpRight[i] * pX[k + numRHS];
        for (int i = 1; i < lastRow; i++)
            {
            k = j0 + i * numRHS;
            pRHS[k] = mpLeft[i]  * pX[k - numRHS]
                    + mpDiag[i]  * pX[k]
                    + mpRight[i] * pX[k + numRHS];
            }
        i = lastRow;
        k = j0 + lastRow * numRHS;
        pRHS[k] = mpLeft[i]  * pX[k - numRHS]
                + mpDiag[i]  * pX[k];
        }
    return true;
    }

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
bool TriDiagonalSolver::SolveRowMajorInPlace
(
// RHS vector becomes solutions !!!!
double *pRHS,
int numRow,
int numRHS
)
    {
    if (numRow != mNumRow)
        return false;
    if (numRow < 2)
        return false;
    int i;
    // apply factorization to RHS...
    double *pRow0 = pRHS;
    double *pRow1;
    for (i = 1; i < numRow; i++, pRow0 = pRow1)
        {
        pRow1 = pRow0 + numRHS;
        double q = mpLeft[i];
        for (int j = 0; j < numRHS; j++)
            pRow1[j] -= pRow0[j] * q;
        }

    i = numRow - 1;
    if (mpDiag[i] == 0.0)
        return false;
    double qf = 1.0 / mpDiag[i];

    pRow0 = pRHS + (numRow - 1) * numRHS;
    for (int j = 0; j < numRHS; j++)
        pRow0[j] *= qf;

    for (i = numRow - 2; i >= 0; i--)
        {
        pRow1 = pRow0;
        pRow0 -= numRHS;
        if (mpDiag[i] == 0.0)
            return false;
        double qj = mpRight[i];
        double qi = 1.0 / mpDiag[i];
        for (int j = 0; j < numRHS; j++)
            {
            pRow0[j] -= qj * pRow1[j];
            pRow0[j] *= qi;
            }
        }
    return true;
    }

#ifdef COMPILE_WITH_PRINTF
/*-------------------------------------------------------------
@description print entire matrix.
-------------------------------------------------------------*/
void TriDiagonalSolver::PrintCoffs
(
char *pTitle
)
    {
    printf ("\n%s\n", pTitle);
    for (int i = 0; i < mNumRow; i++)
        {
        printf (" %2d (%g %g %g)\n", i,
                    mpLeft[i],
                    mpDiag[i],
                    mpRight[i]
                    );
        }
    }
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
