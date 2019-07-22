/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void RowMajorMatrix::Print (RowMajorMatrix const &A)
    {
    size_t period = 5;
    GEOMAPI_PRINTF ("<matrix numRows=\"%d\" numColumns=\"%d\">\n", (int)A.NumRows (), (int)A.NumColumns());
    for (size_t i = 0; i < A.NumRows (); i++)
        {
        GEOMAPI_PRINTF ("<row i=\"%2d\">", (int)i);
        for (size_t j = 0; j < A.NumColumns (); j++)
            {
            GEOMAPI_PRINTF (" %.17g", A.At (i,j));
            if (j + 1 < A.NumColumns ())
                GEOMAPI_PRINTF (", ");
            if ((j % period) + 1 == period
                && j + 1 != A.NumColumns ()
                )
                GEOMAPI_PRINTF ("\n   ");
            }
        GEOMAPI_PRINTF ("</row>\n");
        }
    GEOMAPI_PRINTF ("</matrix>\n");
    }

RowMajorMatrix::RowMajorMatrix ()
    {
    m_numRows = m_numColumns = 0;
    }

RowMajorMatrix::RowMajorMatrix
(
size_t numRows,
size_t numColumns,
double *data,
bool transposeData
)
    {
    SetSizes (numRows, numColumns, data, transposeData);
    }

void RowMajorMatrix::SetSizes
(
size_t numRows,
size_t numColumns,
double *data,
bool transposeData
)
    {
    m_numRows = numRows;
    m_numColumns = numColumns;
    clear ();
    resize (numRows * numColumns, 0.0);
    if (data != NULL)
        {
        if (transposeData)
            {
            // sequential access to data goes down a column ==> i (row index) is inner loop.
            size_t k = 0;
            for (size_t j = 0; j < numColumns; j++)
                for (size_t i = 0; i < numRows; i++)
                    At (i, j) = data[k++];
            }
        else
            {
            size_t k = 0;
            // sequential access to data across a row ==> j (column index) is inner loop.
            // (This could be a memcpy)
            for (size_t i = 0; i < numRows; i++)
                for (size_t j = 0; j < numColumns; j++)
                    At (i, j) = data[k++];
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Compute the dot product of two vectors given start pointers and stride.
* @param pA     => start pointer for vector A
* @param stepA  => step within vector A
* @param pB     => start pointer for vector A
* @param stepB  => step within vector B
* @param n      => number of components
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiLinAlg_dot    /* dot product of vectors*/

(
double *pA,
int    stepA,
double *pB,
int    stepB,
int    n
)
    {
    double dot = 0.0;
    for (; n-- > 0; pA += stepA, pB += stepB)
        dot += (*pA) * (*pB);
    return dot;
    }


/*-----------------------------------------------------------------*//**
* Compute the dot product of two vectors given start pointers and stride, both
* restricted as const.
* @param pA     => start pointer for vector A
* @param stepA  => step within vector A
* @param pB     => start pointer for vector A
* @param stepB  => step within vector B
* @param n      => number of components
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiLinAlg_dotConst

(
const double *pA,
int    stepA,
const double *pB,
int    stepB,
int    n
)
    {
    double dot = 0.0;
    for (; n-- > 0; pA += stepA, pB += stepB)
        dot += (*pA) * (*pB);
    return dot;
    }

/*-----------------------------------------------------------------*//**
* Multiply matrices in stride format.
* @param pAB        <= start point for product.  Assumed distinct from factors
* @param rowStepAB  =>row-to-row step within AB
* @param colStepAB  => column-to-column step within AB
* @param numRowAB   => number of rows in AB and A
* @param numColAB   => number of columns in AB and B
* @param pA         => start pointer for matrix A
* @param rowStepA   => row-to-row step within A
* @param colStepA   => column-to-column step within A
* @param numColA    => number of columns of A, rows of B
* @param pB         => start pointer for matrix A
* @param rowStepB   => row-to-row step within B
* @param colStepB   => column-to-column step within B
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_multiplyMatrixMatrix

(
double *pAB,            /* => start pointer for product.  Assumed distinct from factors. */
int     rowStepAB,      /* => row-to-row step within AB */
int     colStepAB,      /* => column-to-column step within AB */
int     numRowAB,       /* => number of rows in AB and A */
int     numColAB,       /* => number of columns in AB and B */
const double *pA,       /* => start pointer for matrix A*/
int     rowStepA,       /* => row-to-row step within A */
int     colStepA,       /* => column-to-column step within A */
int     numColA,        /* => number of columns of A, rows of B */
const double *pB,       /* => start pointer for matrix A*/
int     rowStepB,       /* => row-to-row step within B */
int     colStepB        /* => column-to-column step within B */
)
    {
    int i, j;
    for (i = 0; i < numRowAB; i++)
        {
        for (j = 0; j < numRowAB; j++)
            {
            pAB[i * rowStepAB + j * colStepAB]
                = bsiLinAlg_dotConst
                            (
                            pA + i * rowStepA, colStepA,
                            pB + j * colStepB, rowStepB,
                            numColA
                            );
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Multiply matrices in dense, row-major format.
* @param pAB        <= start point for product.
*                       Assumed distinct from factors
* @param numRowAB   => number of rows in AB and A
* @param numColAB   => number of columns in AB and B
* @param pA         => start pointer for matrix A
* @param numColA    => number of columns of A, rows of B
* @param pB         => start pointer for matrix A
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_multiplyDenseRowMajorMatrixMatrix

(
double  *pAB,
int     numRowAB,
int     numColAB,
const   double  *pA,
int     numColA,
const   double  *pB
)
    {
    int i, j;
    for (i = 0; i < numRowAB; i++)
        {
        for (j = 0; j < numColAB; j++)
            {
            pAB[i * numColAB + j]
                = bsiLinAlg_dotConst
                            (
                            pA + i * numColA, 1,
                            pB + j * 1, numColAB,
                            numColA
                            );
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Fill a vector with zeros except for a specified nonzero.
* @param pA         => start pointer for vector A
* @param stepA      => step within vector A
* @param n          => number of entries in vector (not referenced. i0 and i1 really control the function.)
* @param i0         => first index to zero
* @param i1         => one after last index to zero
* @param iSpike     => index to receive spike value
* @param value      => spike value
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_initZerosWithSpike

(
double *pA,
int    stepA,
int    n,
int    i0,
int    i1,
int    iSpike,
double value
)

    {
    double *pZero = pA + i0 * stepA;
    int i = i0;
    for (i = i0; i < i1; i++, pZero += stepA)
        {
        *pZero = 0.0;
        }
    pA[iSpike * stepA] = value;
    }


/*-----------------------------------------------------------------*//**
* Swap corresponding entries in two vectors.
* @param pA     <=> start pointer for vector A
* @param pB     <=> start pointer for vector B
* @param stepA  => step within vector A
* @param stepB  => step within vector B
* @param n      => number of entries in vectors
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_swap

(
double *pA,
double *pB,
int     stepA,
int     stepB,
int     n
)

    {
    double temp;

    for (; n-- > 0; pA += stepA, pB += stepB)
        {
        temp = *pA;
        *pA = *pB;
        *pB = temp;
        }
    }


/*-----------------------------------------------------------------*//**
* Swap single entries within a vector.
* @param pIndexArray <=> array where swap is carried out.
* @param i0 => first index
* @param i1 => second index
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_swapIndices

(
int *pIndexArray,
int     i0,
int     i1
)

    {
    int temp = pIndexArray[i0];
    pIndexArray[i0] = pIndexArray[i1];
    pIndexArray[i1] = temp;
    }


/*-----------------------------------------------------------------*//**
* Apply a row swap sequence to an array.
* @param pMatrix <=> matrix of swappees.
* @param pIndex => array of succesive rows swapped to pivots.
* @param nSwap => number of swaps
* @param stepA => step between adjacent values in column.
* @param stepB => step between adjacent values in row.  (internal step during each swap)
* @param nB => number of items per swap (number per row)
* @param reverse => true to apply swaps in reverse order.
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_swapAll

(
double  *pMatrix,       /* <=> matrix to manipulate*/
int     *pIndex,        /* => array of nA row indices*/
int     nSwap,          /* => number of swaps*/
int     stepA,          /* => first direction step*/
int     stepB,          /* => second direction step (internal step during each swap)*/
int     nB,             /* => second direction count (number of items within each swap)*/
int     reverse         /* => true to apply in reverse order*/
)

    {
    int i;
    int iSwap;
    double *p0, *p1;

    if (!reverse)
        {
        for (i = 0; i < nSwap; i++)
            {
            iSwap = pIndex[i];
            if (iSwap >=0 && iSwap != i)
                {
                p0 = pMatrix + i * stepA;
                p1 = pMatrix + iSwap * stepA;
                bsiLinAlg_swap (p0, p1, stepB, stepB, nB);
                }
            }
        }
    else
        {
        for (i = nSwap; i-- > 0;)
            {
            iSwap = pIndex[i];
            if (iSwap >=0 && iSwap != i)
                {
                p0 = pMatrix + i * stepA;
                p1 = pMatrix + iSwap * stepA;
                bsiLinAlg_swap (p0, p1, stepB, stepB, nB);
                }
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Copy a rectangular block.
* @param pDest <=> destination matrix
* @param destRowStep => row-to-row step in destination
* @param destColStep => column-to-column step in destination
* @param pSource => source matrix
* @param sourceRowStep => row-to-row step in destination
* @param sourceColStep => column-to-column step in destination
* @param nRow => number of rows
* @param nCol => number of columns
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
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
)

    {
    double *pCurrSource, *pCurrDest;
    int row, col;
    if (   pSource == pDest
        && sourceRowStep == destRowStep
        && sourceColStep == destColStep
        )
        return;

    for (row = 0; row < nRow; row++)
        {
        pCurrSource = pSource + row * sourceRowStep;
        pCurrDest   = pDest   + row * destRowStep;
        for (col = 0;
             col < nCol;
             col++, pCurrSource += sourceColStep, pCurrDest += destColStep)
            {
            *pCurrDest = *pCurrSource;
            }
        }

    }

/* MAP bsiLinAlg_updateBlock=Geom.updateBlock ENDMAP */

/*-----------------------------------------------------------------*//**
* Apply a rank one block update to a rectangular matrix,
*    i.e. pMatrix[iA][iB] += scale * pA[iA] * pB[iB]
* @param pMatrix        <=> matrix to be updated
* @param stepAMatrix    => step within matrix corresponding to step in A
* @param stepBMatrix    => step within matrix corresponding to step in B
* @param scale          => scale factor to apply
* @param pA             => start pointer for vector A
* @param stepA          => step within vector A and between rows of matrix
* @param nA             => size of A
* @param pB             => start pointer for vector B
* @param stepB          => step within vector B and between columns of matrix
* @param nB             => size of B
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
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
)

    {
    double temp;
    int iA, iB;
    double *pBCurr, *pMCurr, *pMStart, *pACurr;

    pBCurr = pB;
    pMStart = pMatrix;

    for (iB = 0; iB < nB; iB++, pBCurr += stepB, pMStart += stepBMatrix)
        {
        pACurr = pA;
        pMCurr = pMStart;
        temp = scale *  *pBCurr;
        for (iA = 0; iA < nA; iA++, pACurr += stepA, pMCurr += stepAMatrix)
            {
            *pMCurr += temp * *pACurr;
            }
        }
    }
/* MAP bsiLinAlg_SearchMax=Geom.SearchMax ENDMAP */

/*-----------------------------------------------------------------*//**
* Find the indices of the maximum absolute entry in a matrix.
* @param pIndexA        => first index (e.g. row)
* @param pIndexB        => second index (e.g. column)
* @param pMatrix        <=> matrix to be search
* @param stepA,         => first step size
* @param nA,            => number of A steps
* @param stepB,         => second step size
* @param nB             => number of B steps
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiLinAlg_SearchMax

(
int     *pIndexA,
int     *pIndexB,
const double  *pMatrix,
int     stepA,
int     nA,
int     stepB,
int     nB
)

    {
    int maxA, maxB;
    int currA, currB;
    double currValue;
    const double *pStart = pMatrix;
    const double *pCurr;
    double maxValue = *pMatrix;
    maxA = maxB = 0;
    for (currA = 0; currA < nA; currA++, pStart += stepA)
        {
        pCurr = pStart;
        for (currB = 0; currB < nB; currB++, pCurr += stepB)
            {
            if ((currValue = fabs (*pCurr)) > maxValue)
                {
                maxA = currA;
                maxB = currB;
                maxValue = currValue;
                }
            }
        }
    *pIndexA = maxA;
    *pIndexB = maxB;
    return maxValue;
    }


/*---------------------------------------------------------------------------------**//**
* @param        pIndexMin <= index of minimum, in steps
* @param        pIndexmax <= index of maximum, in steps
* @param        pMin      => min value
* @param        pMax      => max value
* @param        pVector   => vector to search
* @param        step      => step size in vector
* @param        n         => number of items to search
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_searchMinMax

(
int     *pIndexMin,
int     *pIndexMax,
double  *pMin,
double  *pMax,
const double  *pVector,
int     step,
int     n
)

    {
    int indexMin, indexMax;
    double aMin, aMax;
    const double *pCurr;
    int i;

    if (n <= 0)
        {
        indexMin = indexMax = 0;
        aMin = aMax = 0.0;
        }
    else
        {
        indexMin = indexMax = 0;
        aMin = aMax = pVector[0];
        for (i = 0, pCurr = pVector; i < n; i++, pCurr += step)
            {
            if (*pCurr > aMax)
                {
                indexMax = i;
                aMax = *pCurr;
                }
            else if (*pCurr < aMin)
                {
                indexMin = i;
                aMin = *pCurr;
                }
            }
        }

    if (pIndexMin)
        *pIndexMin = indexMin;
    if (pIndexMax)
        *pIndexMax = indexMax;
    if (pMin)
        *pMin = aMin;
    if (pMax)
        *pMax = aMax;

    }

/* MAP bsiLinAlg_initIndices=Geom.initIndices ENDMAP */

/*---------------------------------------------------------------------------------**//**
* set pIndexArray[i] == i for 0 <= i < n
* @param        pIndexArea <= initialized array
* @param        n          => number of indices
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_initIndices

(
int     *pIndexArray,   /* <= array to be initialized */
int     n               /* => number of indices*/
)

    {
    int i;
    for (i = 0; i < n; i++)
        {
        pIndexArray[i] = i;
        }
    }


/*---------------------------------------------------------------------------------**//**
* In-place LU factorization with full pivot.
* @param    pMatrix     <=> matrix to factor.
* @param    pRowSwap    <= array of row swap indices.
* @param    pColSwap    <= array of column swap indices.
* @param    rowStep     => address step when row is incremented.
* @param    nRow        => number of rows.
* @param    colStep     => address step when column is incremented.
* @param    nCol        => number of columns.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiLinAlg_LUFullPivot       /* <= rank of matrix*/

(
double  *pMatrix,
int     *pRowSwap,
int     *pColSwap,
int     rowStep,
int     nRow,
int     colStep,
int     nCol
)
    {
    /* pivot is the index of a diagonal entry in the (swapped) matrix.  Its address is pPivot.*/
    int pivot, nPivot;
    double *pPivot = pMatrix;
    int  pivotStep = rowStep + colStep;
    /* The 'block' is the unreduced lower right part of the (swapped) matrix; its upper*/
    /* left is the diagonal immediately following the pivot.*/
    double *pBlock;
    int blockRows, blockCols;
    /* the pivot value is selected by search for the max absolute value anywhere in the*/
    /* block.*/
    int pivotRow, pivotCol;
    double globalMax, currMax, pivotValue, epsilon;
    static double relTol = 1.0e-12;
    int rank = 0;
    bsiLinAlg_initIndices (pRowSwap, nRow);
    bsiLinAlg_initIndices (pColSwap, nCol);

    currMax = globalMax = bsiLinAlg_SearchMax (&pivotRow, &pivotCol,
                        pMatrix, rowStep, nRow, colStep, nCol);
    epsilon = relTol * globalMax;
    if (nRow > nCol)
        {
        nPivot = nCol - 1;
        }
    else
        {
        nPivot = nRow - 1;
        }

   blockRows = nRow - 1;
   blockCols = nCol - 1;

    for (pivot = 0, pPivot = pMatrix, pBlock = pPivot + pivotStep;
         pivot < nPivot && currMax > epsilon;
         pivot++, pPivot = pBlock, pBlock += pivotStep, blockRows--, blockCols--)
         {
         rank++;
         /* Swap (trailing part of) rows*/
         if (pivotRow > pivot)
            bsiLinAlg_swap (pPivot,
                            pPivot + rowStep * (pivotRow - pivot),
                            colStep, colStep, nCol - pivot);
         pRowSwap[pivot] = pivotRow;
         /* swap (complete) columns*/
         if (pivotCol > pivot)
            bsiLinAlg_swap (pMatrix + colStep * pivot,
                             pMatrix + colStep * pivotCol,
                             rowStep, rowStep, nRow);
         pColSwap[pivot] = pivotCol;
         /* Row operations in unreduced block*/
         pivotValue = *pPivot;
         bsiLinAlg_updateBlock (pBlock, rowStep, colStep,
                                -1.0 / pivotValue,
                                 pPivot + rowStep, rowStep, blockRows,
                                 pPivot + colStep, colStep, blockCols);
         /* set up the next pivot*/
         currMax = bsiLinAlg_SearchMax (&pivotRow, &pivotCol,
                        pBlock, rowStep, blockRows, colStep, blockCols);
         pivotRow += pivot + 1; /* max search gave only an offset from the block top*/
         pivotCol += pivot + 1;
         }
    /* Check for final nonzero pivot:*/
    if (pivot == nPivot && currMax > epsilon)
        rank++;
    return rank;
    }


/*------------------------------------------------------------------*//**
* Back substitution to solve an upper triangular linear system.
*
* The structure of the matrices is:
*<pre>
* [ P U U U ][ S  S  S  S ] = [ B B B B]
* [ o P U U ][ S  S  S  S ]   [ B B B B]
* [ o o P U ][ S  S  S  S ]   [ B B B B]
* [ o o o P ][ S  S  S  S ]   [ B B B B]
*
* o = a zero introduced during factorization. (But the factorization
*       process probably filled these spots with other things...
*       null vector computation never reads or writes them.)
* P = a nonzero pivot from factorization.
* U = an entry in the upper triangle of the factored matrix, within
*       the pivot column portion.
* B = a right hand side value (overwritten by S)
* S = a value computed by this routine via back substitution.
*</pre>
*
* @param pB         <=> On input, the right side (Y) values
*                       On output, the solution (S) values.
* @param bRowStep   => address step when row of B is incremented.
* @param bColStep   => address step when column of B is incremented.
* @param nColB      => number of columns in B
* @param pU         => upper triangular matrix
* @param uRowStep   => address step when row of U is incremented.
* @param uColStep   => address step when column of U is incremented.
* @param nRowU      => number of rows (and columns) of U.
+---------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiLinAlg_backSub

(
double  *pB,
int     bRowStep,       /* => row step in solution*/
int     bColStep,       /* => column step in solution*/
int     nColB,          /* => number of columns in B*/
double  *pU,            /* => upper triangular matrix*/
int     uRowStep,       /* => address step when row is incremented*/
int     uColStep,       /* => address step when column is incremented*/
int     nRowU           /* => number of rows (and columns) of U*/
)
    {
    int pivot;
    int bCol;
    int pivotStep = uRowStep + uColStep;
    double *pLastRowB = pB + bRowStep * (nRowU - 1);
    double *pLastColU = pU + uColStep * (nRowU - 1);
    double *pPivot = pLastColU + uRowStep * (nRowU - 1);
    double  pivotValue, factor;
    double *pBVal;
    for (pivot = nRowU - 1;
        pivot >= 0;
        pivot--, pPivot -= pivotStep, pLastRowB -= bRowStep, pLastColU -= uColStep )
        {
        pivotValue = *pPivot;
        if (pivotValue == 0.0)
            return false;
        factor = 1.0 / pivotValue;
        for (bCol = 0, pBVal = pLastRowB;
            bCol < nColB;
            bCol++, pBVal += bColStep)
            {
            *pBVal *= factor;
            }
        if (pivot > 0)
            bsiLinAlg_updateBlock (
                                pB, bRowStep, bColStep,
                                -1.0,
                                pLastColU, uRowStep, pivot,
                                pLastRowB, bColStep, nColB
                                );
        }
    return true;
    }


/*------------------------------------------------------------------*//**
* Apply the L part of an LU factorization to a RHS matrix.
*
* The structure of the LU matrix is:
*<pre>
* [ P U U U ]
* [ L P U U ]
* [ L L P U ]
* [ L L L P ]
*
* L = lower triangle of factorization
* P = a nonzero pivot from factorization.
* U = upper triangle of factorization
*</pre>
* @param pB         <=> On input, the right side (Y) values
*                       On output, the solution (S) values.
* @param rowStepB   => address step when row of B is incremented.
* @param colStepB   => address step when column of B is incremented.
* @param nColB      => number of columns in B
* @param pLU        => upper triangular matrix
* @param pRowSwap   => array of row swap indices.
* @param pColSwap   => array of column swap indices.
* @param rowStep    => address step when row of pLU is incremented.
* @param nRow       => number of rows of pLU
* @param columnStep => address step when column of pLU is incremented.
* @param nCol       => number of columns of pLU
* nPivot            => number of pivots to apply (e.g. rank - 1)
+---------------------------------------------------------------------*/
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
)
    {
    /* pivot is the index of a diagonal entry in the (swapped) matrix.  Its address is pPivot.*/
    int pivot;
    double *pPivot, *pBlockB;
    double pivotValue;
    //int  pivotStep = rowStep + colStep;
    int blockRows, blockCols;
    int pivotRow;

    blockRows = nRow  - 1;
    blockCols = nColB;

    for (pivot = 0, pPivot = pLU, pBlockB = pB;
         pivot < nPivot;
         pivot++, pPivot += rowStep + colStep, pBlockB += rowStepB, blockRows--)
         {
         /* Swap (complete) rows in B*/
         pivotRow = pRowSwap[pivot];
         bsiLinAlg_swap (pB + rowStep * pivot,     /* Yes, that should be the same as pBlockB*/
                          pB + rowStep * pivotRow, colStepB, colStepB, nCol);
         pivotValue = *pPivot;
         bsiLinAlg_updateBlock (pBlockB + rowStepB, rowStepB, colStepB,
                                -1.0 / pivotValue,
                                 pPivot + rowStep, rowStep, blockRows,
                                 pBlockB, colStepB, blockCols);
         }
    }






/*------------------------------------------------------------------*//**
* Overwrite columns of pNullVectors with columns of the null space
* of a factored matrix.
* The structure of the matrices is:
*<pre>
* [ P U U U Y Y Y Y ][ S  S  S  S ] = [ 0 ]
* [ o P U U Y Y Y Y ][ S  S  S  S ]   [ 0 ]
* [ o o P U Y Y Y Y ][ S  S  S  S ]   [ 0 ]
* [ o o o P Y Y Y Y ][ S  S  S  S ]   [ 0 ]
*                    [-1  0  0  0 ]   [ 0 ]
*                    [ 0 -1  0  0 ]   [ 0 ]
*                    [ 0  0 -1  0 ]   [ 0 ]
*                    [ 0  0  0 -1 ]   [ 0 ]
*
* o = a zero introduced during factorization. (But the factorization
*       process probably filled these spots with other things...
*       null vector computation never reads or writes them.)
* P = a nonzero pivot from factorization.
* U = an entry in the upper triangle of the factored matrix, within
*       the pivot column portion.
* Y = an entry in the upper triangle of the factored matrix, in the
*       non-pivot portion.
* S = a value computed by this routine via back substitution.
*       Each column is the solution of [U] * [S] = [Y]
*       where [U] is the pivot portion of the factored matrix, and [Y]
*       is a column from the non-pivot portion.
*       Corresponding Y and S columns may overwrite in a square matrix.
*</pre>
* The -1 and 0 entries below the S values are supplied by this
* function.
*
* @param pNullVectors   <= matrix to receive the nCol-rank null vectors
* For a square matrix, this may be the address of column rank+1 of pMatrix.
* Computations are ordered so that all necessary values
* from those columns are used before being overwritten.
* @param nullVectorRowStep => row step in null vectors
* @param nullVectorColStep => column step in null vectors
* @param pMatrix    => reduced and swapped matrix from LU factorization
* @param pRowSwap   => array of nA row indices
* @param pColSwap   => array of nB column indices
* @param rowStep    => address step when row is incremented
* @param nRow       => number of rows
* @param colStep    => address step when column is incremented
* @param nCol       => number of colums
* @param rank       => matrix rank as determined by factorization.
* @return the number of null vectors computed.
+----------------------------------------------------------------------*/
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
)
    {
    int nNullVector = nCol - rank;
    int col = rank + 1;
    double *pNullCol;

    for (col = rank, pNullCol = pNullVectors;
        col < nCol;
        col++, pNullCol += nullVectorColStep)
        {
        bsiLinAlg_initZerosWithSpike (pNullCol, nullVectorRowStep, nCol, col, nCol, col, -1.0);
        }
    bsiLinAlg_copyBlock (pNullVectors, nullVectorRowStep, nullVectorColStep,
                          pMatrix + colStep * rank,
                          rowStep, colStep,
                          rank, nNullVector);
    bsiLinAlg_backSub (pNullVectors, nullVectorRowStep, nullVectorColStep, nNullVector,
                        pMatrix, rowStep, colStep, rank);
    return nNullVector;
    }


/*------------------------------------------------------------------*//**
* @param pNullVectors <= array of null vectors.
* @param pMatrix => matrix analyzed.
* @returnn the number of vector (columns) placed into pNullVectors.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsiDMatrix4d_nullSpace
(
DMatrix4dP pNullVectors,
DMatrix4dP pMatrix
)
    {
    DMatrix4d   workMatrix = *pMatrix;
    DMatrix4d   nullVectors;
    int         rowSwap[4];
    int         colSwap[4];
    int         rank;
    rank = bsiLinAlg_LUFullPivot ((double*)&workMatrix, rowSwap, colSwap,
                        4, 4, 1, 4);
    memset (&nullVectors, 0, sizeof(DMatrix4d));
    if (rank < 4)
        {
        bsiLinAlg_expandNullSpace ((double*)&nullVectors, 4, 1,
                        (double*)&workMatrix, NULL, NULL,
                        4, 4, 1, 4, rank);
        bsiLinAlg_swapAll ((double*)&nullVectors, colSwap, rank,
                                        4, 1, 4, true);
        }
    *pNullVectors = nullVectors;
    return 4 - rank;
    }


/*---------------------------------------------------------------------------------**//**
* Invert a 4x4 matrix using full swapping (rows and columns)
*
* @param pInverse <= inverse matrix.
* @param pCond    <= estimate of condition number.   Near zero is bad, near 1 is good.
* @param pMatrix  => matrix to invert.
* @return false if unable to invert
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDMatrix4d_invertFullSwapCond

(
DMatrix4dP pInverse,
double          *pCond,
DMatrix4dCP pMatrix
)
    {
    DMatrix4d   workMatrix;
    DMatrix4d   BMatrix;
    double      *pB = (double *)&BMatrix;
    double      *pA = (double *)&workMatrix;
    int         rowSwap[4];
    int         colSwap[4];
    int         rank;
    workMatrix = *pMatrix;
    rank = bsiLinAlg_LUFullPivot ((double*)&workMatrix, rowSwap, colSwap,
                        1, 4, 4, 4);

    *pCond = 0.0;

    if (rank < 4)
        return false;

    memset (&BMatrix, 0, sizeof(DMatrix4d));

    *pCond = fabs (workMatrix.coff[3][3] / workMatrix.coff[0][0]);

    BMatrix.coff[0][0]
        = BMatrix.coff[1][1]
        = BMatrix.coff[2][2]
        = BMatrix.coff[3][3]
        = 1.0;

    bsiLinAlg_applyLFactor (pB, 1, 4, 4,
                             pA, rowSwap, colSwap,
                                        1, 4, 4, 4, 4);
    bsiLinAlg_backSub (pB, 1, 4, 4,
                               pA, 1, 4, 4);

    bsiLinAlg_swapAll (pB, colSwap, 4, 1, 4, 4, true);
    *pInverse = BMatrix;
    return true;
    }
#define CompileAll
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Invert a dense matrix using full pivoting.
*
* @param pAinv <= inverse matrix.
* @param pRowOrder => array of n ints for row order.
* @param pColOrder => array of n ints for column order.
* @param cond <= estimate of condition number.
* @param pA    <=> original matrix. On return, replaced by LU factors.
* @param n  => number of rows and columns of data.
* @param rowStep => step from row to row
* @param colStep => step from column to column
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlLinAlg_invertFullSwapCond

(
double          *pAinv,
int             *pRowOrder,
int             *pColOrder,
double          *pCond,
double          *pA,
int             rowStep,
int             colStep,
int             n
)
    {
    int         rank;
    int         i, k;
    int         iLast = n - 1;
    rank = bsiLinAlg_LUFullPivot (pA, pRowOrder, pColOrder,
                        rowStep, n, colStep, n);
    *pCond = 0.0;

    if (rank < n)
        return false;

    memset (pAinv, 0, n * rowStep * colStep * sizeof(double));
    for (i = k = 0; i < n; i++, k += rowStep + colStep)
        pAinv[k] = 1.0;

    *pCond =  fabs (pA[iLast * colStep + iLast * rowStep])
            / fabs (pA[0]);


    bsiLinAlg_applyLFactor
                    (
                    pAinv, rowStep, colStep, n,
                    pA, pRowOrder, pColOrder,
                    rowStep, n, colStep, n, rank - 1);

    bsiLinAlg_backSub (pAinv, rowStep, colStep, n,
                               pA, rowStep, colStep, n);

    bsiLinAlg_swapAll (pAinv, pColOrder,
                            rank, rowStep, colStep, n, true);

    return true;
    }
#endif
#define ARRAYENTRY(pMatrix,i,j,rowStep,colStep) pMatrix[(i)*(rowStep)+(j)*(colStep)]

/*---------------------------------------------------------------------------------**//**
* Init an identity matrix
* @param pA <= buffer initialized to identity.
* @param n  => number of rows and columns
* @param rowStep => row-to-row step (in doubles)
* @param colStep => column-to-column step (in doubles)
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_initIdentity

(
double *pA,
int     n,
int     rowStep,
int     colStep
)
    {
    int i, j;
    for (i = 0; i < n; i++)
        {
        for (j = 0; j < n; j++ )
            {
            ARRAYENTRY(pA, i, j, rowStep, colStep) = 0.0;
            }
        ARRAYENTRY(pA, i, i, rowStep, colStep) = 1.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @param pQ     <=Full orthogonal factor. Must be different from pA.
* @param pA     <=> On input, the full matrix.  Replaced by triangular factor.
* @param nARow  => rows in A, hence rows, columns of Q.
* @param nACol  => columns in A, hence rows, columns of Q.
* @param rowStepQ => row-to-row step in Q
* @param colStepQ => column-to-column step in Q
* @param rowStepA => row-to-row step in A
* @param colStepA => column-to-column step in A
* @bsimethod                                                    EarlinLutz      05/98
+---------------+---------------+---------------+---------------+---------------+------*/
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
)
    {
    int i, k;
    int numPivot = nARow < nACol ? nARow : nACol;
    double cc, ss;
    double akk, aik;

    bsiLinAlg_initIdentity (pQ, nARow, rowStepQ, colStepQ);
    /* For each pivot .. */
    for (k = 0; k < numPivot; k++)
        {
        /* Zero out under the pivot */
        for (i = k + 1; i < nARow; i++)
            {
            akk = ARRAYENTRY(pA, k, k, rowStepA, colStepA);
            aik = ARRAYENTRY(pA, i, k, rowStepA, colStepA);

            bsiTrig_constructGivensWeights (&cc, &ss, akk, aik);
            bsiDMatrix3d_rotate (pA + k * colStepA,
                            rowStepA, colStepA,
                            nACol - k, k, i,
                            cc, ss);
            bsiDMatrix3d_rotate (pQ,
                            rowStepQ, colStepQ,
                            nARow, k, i,
                            cc, ss);
            }
        }
    }

/*---------------------------------------------------------------------*//**
* Invert a general matrix using givens rotations for an initial
* triangular factorization.
*
* The diagonals of the factorization are inspected to determine if
* the matrix is singular.
*
* If a diagonal entry is zero, or very small relative to its row and
* column, the matrix is deemed singular.   On return from this case,
* pWork contains the triangular factor R, and pAinv is the orthogonal
* matrix Q^T.  Q^T*A = R, and A = Q * R.
* pAinv contains the orthgonal matrix Q used to triangularize.
*
* If the diagonals are all healthy, the Q matrix is overwritten by
* the inverse.  This is done by backsolving R*B=Q^T for B, which is
* the inverse of A.
* @param pAinv <= inverse.  Same memory layout as A. Must be different from pWork and pA.
* @param pWork <= work area, same layout as pA and pAinv.  May be same as pA, but
*       in this case contents of pA will be destroyed.
* @param pCond <= estimate of condition.  0 is singular, near-zero is bad, and 1 is best.
* @param pA => original square matrix.
* @param n => number of rows (hence columns!!)
* @param nACol => number of columns -- not used.
* @param rowStep => row-to-row increment within A
* @param colStep => col-to-col increment within A
+----------------------------------------------------------------------*/
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
)
    {
    int i, pivot;
    static double relTol = 1.0e-12;

    double colSum, rowSum, a;

    if  (pA != pWork)
        {
        bsiLinAlg_copyBlock (pWork, rowStep, colStep, pA, rowStep, colStep, n, n);
        }

    bsiLinAlg_givensQR (pAinv, pWork, n, n, rowStep, colStep, rowStep, colStep);

    /* It is hereby declared that the matrix is singular if
        (a) any absolute zero appears on the diagonal of the triangular factor
        (b) any diagonal of the triangular factor is
                "small compared to the rest of its row and column"
    */

    *pCond = 1.0;
    for (pivot = 0; pivot < n; pivot++)
        {
        a = fabs (ARRAYENTRY(pWork, pivot, pivot, rowStep, colStep));
        if  (a == 0.0)
            {
            *pCond = 0.0;
            return  false;
            }

        colSum = rowSum = 0.0;

        if  (pivot > 0)
            {
            for (i = 0; i < pivot; i++)
                {
                colSum += fabs (ARRAYENTRY (pWork, i, pivot, rowStep, colStep));
                }

            if  (a <= relTol * colSum)
                {
                *pCond = 0.0;
                return  false;
                }
            }

        if  (pivot < n - 1)
            {
            for (i = pivot + 1; i < n; i++)
                {
                rowSum += fabs (ARRAYENTRY (pWork, pivot, i, rowStep, colStep));
                }

            if  (a <= relTol * rowSum)
                {
                *pCond = 0.0;
                return  false;
                }
            }

        colSum += a;
        rowSum += a;

        if  (a < *pCond * colSum)
            *pCond = a / colSum;

        if  (a < *pCond * rowSum)
            *pCond = a / rowSum;

        }

    /* OK, we got past that part.  It's safe to backsolve */
    bsiLinAlg_backSub (pAinv, rowStep, colStep, n, pWork, rowStep, colStep, n);

    return true;
    }


/*--------------------------------------------------------------------*//**
* Compute QT*A*Q, where Q is a rotation matrix and A is 2x2 symmetric
* given by its three distinct values.
* @param pAii <=> ii entry. Updated in place.
* @param pAij <=> ij entry. Updated in place.
* @param pAjj <=> jj entry. Updated in place.
* @param cc => cosine of rotation angle.
* @param ss => sine of rotation angle.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix3d_symmetric2x2Rotation

(
double      *pAii,
double      *pAij,
double      *pAjj,
double      cc,
double      ss
)

    {
    double a = *pAii;
    double b = *pAij;
    double d = *pAjj;

    double cb = cc * b;
    double sb = ss * b;

    /* B00, B01, B10, B11 = entries in Q*A*/
    double B00 = cc * a + sb;
    double B01 = cb     + ss * d;
    double B10 = cb     - ss * a;
    double B11 = cc * d - sb;

    *pAii = B00 * cc + B01 * ss;
    *pAij = B01 * cc - B00 * ss;
    *pAjj = B11 * cc - B10 * ss;
    }

#define INDEX(AA,ii,jj,nn) AA[(ii)*(nn) + (jj)]


/*----------------------------------------------------------------------+
* Given SYMMETRIC (but full storage) matrix A, form Q*A*Q'
* where Q is a givens rotation, i.e. an identity matrix except for
* the four positions (i,i)=c, (i,j) = s, (j,i)=-s, (j,j)=c
* @param pMatrix <=> dense, row-major symmetric matrix.
* @param n => size of matrix.
* @param i => rotation index.
* @param j => rotation index.
* @param cc => cosine of rotation angle.
* @param ss => sine of rotation angle.
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
)
    {
    double aa,bb;
    int k;
    if (j < i)
        {
        bsiDMatrix3d_symmetricRotation (pMatrix, n, j, i, cc, -ss);
        }
    else if (0 <= i && i < j && j < n)
        {
        /* All inputs come from upper right triangle. Store to both triangles.*/

        /* k before i.*/
        for (k = 0; k < i; k++)
            {
            aa = INDEX(pMatrix, k, i, n);
            bb = INDEX(pMatrix, k, j, n);
            INDEX(pMatrix, i, k, n) = INDEX(pMatrix, k, i, n) = cc * aa + ss * bb;
            INDEX(pMatrix, j, k, n) = INDEX(pMatrix, k, j, n) = cc * bb - ss * aa;
            }

        /* i,j submatrix*/
        bsiDMatrix3d_symmetric2x2Rotation (
                    &INDEX(pMatrix, i, i, n),
                    &INDEX(pMatrix, i, j, n),
                    &INDEX(pMatrix, j, j, n),
                    cc, ss);
        INDEX(pMatrix, j, i, n) = INDEX(pMatrix, i, j, n);

        /* internal segment*/
        for (k = i + 1; k < j; k++)
            {
            aa = INDEX(pMatrix, i, k, n);
            bb = INDEX(pMatrix, k, j, n);
            INDEX(pMatrix, i, k, n) = INDEX(pMatrix, k, i, n) = cc * aa + ss * bb;
            INDEX(pMatrix, j, k, n) = INDEX(pMatrix, k, j, n) = cc * bb - ss * aa;
            }

        /* k after j.*/
        for (k = j+1; k < n; k++)
            {
            aa = INDEX(pMatrix, i, k, n);
            bb = INDEX(pMatrix, j, k, n);
            INDEX(pMatrix, i, k, n) = INDEX(pMatrix, k, i, n) = cc * aa + ss * bb;
            INDEX(pMatrix, j, k, n) = INDEX(pMatrix, k, j, n) = cc * bb - ss * aa;
            }
        }
    }


/*--------------------------------------------------------------------*//**
* @param pMatrix <=> matrix where rows is to be rotated.  (Or column,
*       by choice of steps.)
* @param rowStep => row-to-row step.
* @param colStep => col-to-col step.
* @param nCol => number of columns.
* @param i => row corresponding to (cc, ss) row of rotation.
* @param j => row corresponding to (-ss,cc) row of rotation.
* @param cc => cosine of rotation angle.
* @param ss => sine of rotation angle.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix3d_rotate

(
double      *pMatrix,       /* <=> matrix*/
int         rowStep,        /* => row step in A*/
int         colStep,        /* => column step in A*/
int         nCol,           /* => columns of A*/
int         i,              /* => index of cc diagonal*/
int         j,              /* => index of ss diagonal*/
double      cc,             /* => c coefficient*/
double      ss              /* => s coefficient*/
)

    {
    double *pAi = pMatrix + i * rowStep;
    double *pAj = pMatrix + j * rowStep;

    int k;
    double aa, bb;
    for (k = 0; k < nCol; k++, pAi += colStep, pAj += colStep)
        {
        aa = *pAi;
        bb = *pAj;
        *pAi = cc * aa + ss * bb;
        *pAj = cc * bb - ss * aa;
        }
    }


/*-------------------------------------------------------------------*//**
* Compute pMatrix such that pMatrix * pVector 'rotates' selective
* parts of the vector into a single location.   pIndex[0] is the
* target index of the rotation.   pIndex[1], pIndex[2] ... are zeroed
* by the multiplication.  (Note that pVector itself is not actually
* modified.   The zeroing 'would occur' if pMatrix * pVector were to
* be carried out)
* @param pMatrix <= orthogonal matrix.
* @param pMag <= magnitude of rotated vector.
* @param n => size of rotation matrix.
* @param pVector => dense vector to be zeroed.
* @param pIndex => indices of vector entries to be affected.
*                       pIndex[0] is the pivot entry which receives rotated content.
*                       pIndex[1..n-1]
* @param nRot => number of rotations, i.e. n-1.  (Hmm??)
* @return false if nRot < 1 or nRot >= n, or any other index out of bounds.  No check for
*               repeated indices.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiDMatrix3d_initSelectiveRotation

(
double          *pMatrix,
double          *pMag,
int             n,
const double    *pVector,
const int       *pIndex,
int       nRot
)
    {
    int i, j;
    int row, row0;
    int step, diagIndex;

    double a0, ai, mag2, mag;
    double cosine, sine;
    double *pRow0, *pRow;

    if (nRot < 1 || nRot >= n)
        return false;

    row0 = pIndex[0];
    if (row0 < 0 || row0 > n)
        return false;


    memset (pMatrix, 0, n * n * sizeof( double));

    step = n + 1;

    for (row = diagIndex = 0; row < n; row++, diagIndex += step)
        {
        pMatrix[diagIndex] = 1.0;
        }

    /* First step is explicit fill of simple rotation.*/
    pRow0 = pMatrix + n * row0;
    row   = pIndex[1];
    if (row < 0 || row >= n)
        return false;
    pRow = pMatrix + n * row;

    a0 = pVector[row0];
    ai = pVector[row];
    if ((fabs (a0) + fabs(ai)) != 0.0)
        {
        bsiTrig_constructGivensWeights (&cosine, &sine, a0, ai);
        pRow0[row0] = cosine;
        pRow0[row]  = sine;
        pRow[row0]  = -sine;
        pRow[row]   = cosine;
        }
    mag2        = a0 * a0 + ai * ai;
    mag         = sqrt (mag2);

    /* Remaining rows fill with recurrence on prior rows.*/

    for (i = 2; i < nRot; i++)
        {
        row = pIndex[i];
        if (row < 0 || row >= nRot)
            return false;
        pRow    = pMatrix + n * row;
        ai      = pVector[row];
        mag2    += ai * ai;
        /* The old mag is currently at the row0 entry.*/
        if (ai != 0.0)
            {
            int col;
            bsiTrig_constructGivensWeights (&cosine, &sine, mag, ai);
            mag     = sqrt (mag2);
            for (j = 0; j < i; j++)
                {
                col = pIndex[j];
                pRow [col]  = -sine * pRow0[col];
                pRow0[col] *= cosine;
                }
            pRow0[row] = sine;
            pRow [row] = cosine;
            }
        }
    *pMag = mag;
    return true;
    }

/*------------------------------------------------------------------*//**
* Initialize a  matrix of size numEq * numEq, and right side of size
* numEq, to be used for subseqent incremental least squares entry.
* @param pA <=> coefficient matrix.
* @param pB <=> right side vector
* @param pEE <=> accumulating squared error.
* @param numEq => number of rows and columns.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_initIncrementalLeastSquares

(
double          *pA,
double          *pB,
double          *pEE,
int             numEq
)
    {
    memset (pA, 0, numEq * numEq * sizeof (double));
    if (pB)
        memset (pB, 0, numEq * sizeof (double));
    if (pEE)
        *pEE = 0.0;
    }

/*------------------------------------------------------------------*//**
* Add one row (equation) to an incremental least squares problem.
* The evolving matrix is maintained as a (full) upper triangular system
* with lower zeros left untouched.
* @param pA <=> coefficient matrix.
* @param pB <=> right side vector
* @param pEE <=> accumulating squared error.
* @param pC <=> work vector of size numEq doubles.
* @param pRowCoffs => coefficients for the new row.
* @param b => right side entry for new row.
* @param numEq => number of rows and columns.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsiLinAlg_extendIncrementalLeastSquares

(
double          *pA,            /* <= Left side matrix */
double          *pB,            /* <= Right side vector */
double          *pEE,           /* <= accumulating squared error */
double          *pC,            /* <=> work vector of size numEq doubles */
double          *pRowCoffs,     /* => matrix entries for new row */
double          b,              /* => right side entry for new row */
int             numEq           /* => number of unknowns */
)
    {
    int i, j;
    double *pPivotRow = pA;
    double cosine, sine;

    for (i = 0; i < numEq; i++)
        pC[i] = pRowCoffs[i];

    for (i = 0; i < numEq; i++, pPivotRow += numEq)
        {
        bsiTrig_constructGivensWeights (&cosine, &sine, pPivotRow[i], pC[i]);

        /* Apply givens weight to remainder of matrix (we know there are
            zeros prior to column i) */
        for (j = i; j < numEq; j++)
            {
            bsiTrig_applyGivensWeights
                        (
                        pPivotRow + j, pC + j,
                        pPivotRow[j], pC[j],
                        cosine, sine);
            }

        /* and apply to right side vector */
        if (pB)
            bsiTrig_applyGivensWeights (pB + i, &b, pB[i], b, cosine, sine);
        }
    if (pEE)
        *pEE += b * b;
    }

/*------------------------------------------------------------------*//**
* Final solution step for evolved incremental least squares.
* (back substitution in triangular system -- not uniqe to least squares)
* @param pX <= solution vector.
* @param pA => factored (triangular, dense row-order) matrix.
* @param pB => factored RHS (i.e. Q vector applied)
* @param numEq => number of equations.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        bsiLinAlg_solveIncrementalLeastSquares

(
double          *pX,
double          *pA,
double          *pB,
int             numEq
)
    {
    int i, j;
    double bb;
    double *pPivotRow;


    pPivotRow = pA + numEq * (numEq - 1);

    for (i = numEq - 1; i >= 0; i--, pPivotRow -= numEq)
        {
        bb = pB[i];
        for (j = i + 1; j < numEq; j++)
            bb -= pX[j] * pPivotRow[j];
        if (!bsiTrig_safeDivide (pX + i, bb, pPivotRow[i], 0.0))
            return false;
        }
    return true;
    }
#define ROWORDERARRAYENTRY(pMatrix,i,j,rowStep) pMatrix[(i)*(rowStep)+(j)]


/*------------------------------------------------------------------*//**
* In a square, triangular matrix, select a pivot to be forced to unit
*   value so as to solve a homogeneous system.
* @param pA => triangular factor.  Assumed in dense, row major order.
* @param numEq => number of equations
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsiLinAlg_selectHomogeneousLeastSquaresPivot

(
double          *pA,            /* => Left side matrix */
int             numEq           /* => number of unknowns */
)
    {
    int k;
    int kMin;
    double a;
    double aMin;
    aMin = fabs (pA[0]);
    kMin = 0;
    for (k = 0; k < numEq; k++)
        {
        a = fabs (ROWORDERARRAYENTRY (pA, k, k, numEq));
        if (a < aMin)
            {
            aMin = a;
            kMin = k;
            }
        }
    return kMin;
    }

/*------------------------------------------------------------------*//**
* Given an upper triangular A, solve A*X = 0 with a designated pivot
* fixed at 1, using givens rotations to reorthogonalize after removal
* of the pivot row.  (Hint: use bsiLinAlg_selectHomogeneousLeastSquaresPivot
*   to pick the pivot index.)
* @param pX <= solution vector.
* @param pResidual <= residual of solution.
* @param pA <=> coefficient matrix. Destroyed during factorization.  Assumed
*                   dense, row major order.
* @param kPivot => index of unit solution component.
* @param numEq => number of equations.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        bsiLinAlg_solveHomogeneousLeastSquares

(
double          *pX,            /* <= solution vector */
double          *pResidual,     /* <= residual of solution */
double          *pA,            /* => coefficient matrix matrix */
int             kPivot,         /* => pivot index. */
int             numEq           /* => number of unknowns */
)
    {
    int j, k;
    double bb;
    double *pRowA, *pRowB;
    double *pPivotRow;
    double cosine, sine;
    if (kPivot < 0 || kPivot > numEq)
        return false;

    for (k = kPivot + 1; k < numEq; k++)
        {
        /* Point to first element in (row major) pivot row */
        pRowA = pA + (k - 1) * numEq;
        pRowB = pA + k * numEq;
        bsiTrig_constructGivensWeights (&cosine, &sine, pRowA[k], pRowB[k]);

        /* Apply givens weight to retriangularize the matrix ignoring the pivot.
            We know there are zeros prior to column i).
            Note: In column k, this should recompute a 0!!! */
        for (j = k - 1; j < numEq; j++)
            {
            bsiTrig_applyGivensWeights
                        (
                        pRowA + j,
                        pRowB + j,
                        pRowA[j],
                        pRowB[j],
                        cosine, sine);
            }
        }

    /* Negated pivot column is the right hand side for the elimination step */
    for (k = 0; k < numEq; k++)
        pX[k] = - ROWORDERARRAYENTRY(pA, k, kPivot, numEq);

    /* back substitution for remaining terms, overwriting the nominal RHS values
       in pX[] and forcing the pivot solution to 1 */

    pPivotRow = pA + numEq * (numEq - 1);
    for (k = numEq - 1; k >= 0; k--, pPivotRow -= numEq)
        {
        if (k == kPivot)
            pX[k] = 1.0;
        else
            {
            bb = pX[k];
            for (j = k + 1; j < numEq; j++)
                {
                if (j != kPivot)
                    bb -= pX[j] * pPivotRow[j];
                }
            if (!bsiTrig_safeDivide (pX + k, bb, pPivotRow[k], 0.0))
                return false;
            }
        }

    if (pResidual)
        {
        double r = 0.0;
        for (j = kPivot; j < numEq; j++)
            r += ROWORDERARRAYENTRY (pA, kPivot, j, numEq) * pX[j];
        *pResidual = r;
        }
    return true;
    }

#define SIGNIFICANT12(v)        (fabs (v) > 1.0e-12)

bool LinearAlgebra::SolveInplaceGaussPartialPivot (RowMajorMatrix &matrix, RowMajorMatrix &rhs)
    {
    size_t n = matrix.NumRows ();
    if (n == 0)
        return false;
    assert (matrix.IsSquare ());
    assert (rhs.NumRows () == matrix.NumRows ());
    bvector<double>pivots;
    double aMax = 0.0;
    double aMin = DBL_MAX;
    for (size_t k = 0; k < n; k++)
        {
        size_t iMax;
        double a = matrix.MaxAbsBelowPivot (k, iMax);
        matrix.SwapRows (k, iMax);
        rhs.SwapRows (k, iMax);
        aMin = DoubleOps::Min (aMin, a);
        aMax = DoubleOps::Max (aMax, a);
        if (a == 0.0)
            return false;
        pivots.push_back (a);
        matrix.BlockElimination (k, rhs);
        }
    //DoubleOps::SafeDivide (condition, aMin, aMax);
    matrix.BackSubstitute (rhs);
    return true;
    }

bool LinearAlgebra::SolveInplaceGaussFullPivot (RowMajorMatrix &matrix, RowMajorMatrix &rhs, double &condition)
    {
    size_t n = matrix.NumRows ();
    if (n == 0)
        return false;
    assert (matrix.IsSquare ());
    assert (rhs.NumRows () == matrix.NumRows ());
    bvector<size_t> index (n);
    bvector<double>pivots;
    for (size_t i = 0; i < n; i++)
        index[i] = i;
    double a0 = 0.0;
    double aMax = 0.0;
    for (size_t k = 0; k < n; k++)
        {
        size_t iMax, jMax;
        aMax = matrix.MaxAbsBeyondPivot (k, iMax, jMax);
        matrix.SwapRows (k, iMax);
        rhs.SwapRows (k, iMax);
        matrix.SwapColumns (k, jMax, index);
        if (k == 0)
            a0 = aMax;
        if (aMax == 0.0)
            return false;
        pivots.push_back (aMax);
        matrix.BlockElimination (k, rhs);
        }
    // (final aMax falls through)
    condition = aMax / a0;
    matrix.BackSubstitute (rhs);
    for (size_t i = 0; i < n;)
        {
        size_t j = index[i];
        if (i != j)
            {
            std::swap (index[i], index[j]);
            rhs.SwapRows (i, j);
            }
        else
            {
            i++;
            }
        }
    return true;
    }

bool LinearAlgebra::InvertFullSwap (RowMajorMatrix const &matrix, RowMajorMatrix &inverse)
    {
    assert (matrix.IsSquare ());
    size_t n = matrix.NumRows ();
    bvector<int> rowOrder (n), columnOrder(n);
    double condition;
    RowMajorMatrix B = matrix;
    inverse = matrix;
    return jmdlLinAlg_invertFullSwapCond (&inverse[0], &rowOrder[0], &columnOrder[0], &condition, &B[0],
          (int)n, 1, (int)n);
    }

void RowMajorMatrix::SwapRows (size_t i, size_t j)
    {
    for (size_t k = 0; k < m_numColumns; k++)
        std::swap (At(i,k), At(j,k));
    }

void RowMajorMatrix::SwapColumns (size_t i, size_t j)
    {
    for (size_t k = 0; k < m_numRows; k++)
        {
        std::swap (At(k,i), At(k,j));
        }
    }

void RowMajorMatrix::SwapColumns (size_t i, size_t j, bvector<size_t> &index)
    {
    SwapColumns (i, j);
    std::swap (index[i], index[j]);
    }

void RowMajorMatrix::SetColumn (size_t j, bvector<double> const &data)
    {
    assert (j < m_numColumns);
    for (size_t i = 0; i < m_numRows; i++)
        At(i,j) = data[i];
    }

void RowMajorMatrix::GetColumn (size_t j, bvector<double> &data) const
    {
    assert (j < m_numColumns);
    data.resize (m_numRows);
    for (size_t i = 0; i < m_numRows; i++)
        data[i] = At(i,j);
    }

size_t RowMajorMatrix::GetAll (double * data, size_t maxCount, bool transpose) const
    {
    size_t myCount = m_numRows * m_numColumns;
    if (myCount > maxCount)
        return 0;
    if (myCount == 0)
        return 0;
    if (transpose)
        {
        size_t k = 0;
        for (size_t j = 0; j < m_numColumns; j++)
            for (size_t i = 0; i < m_numRows; i++)
                data[k++] = at (i * m_numColumns + j);
        }
    else
        {
        memcpy (data, &at(0), sizeof (double) * myCount);
        }
    return myCount;        
    }


void RowMajorMatrix::SetRow (size_t i, bvector<double> const &data)
    {
    assert (i < m_numRows);
    for (size_t j = 0; j < m_numColumns; j++)
        At(i,j) = data[j];
    }

/// <summary>Store into a checked position.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::Set (ptrdiff_t i, ptrdiff_t j, double value)
    {
    if (i >= 0 && i < (ptrdiff_t)m_numRows && j >= 0 && j < (ptrdiff_t)m_numColumns)
        {
        At(i,j) = value;
        return 1;
        }
    return 0;
    }

/// <summary>Read from a checked position.  Any out-of-bounds index is 0.</summary>
size_t RowMajorMatrix::Get (ptrdiff_t i, ptrdiff_t j, double &value)
    {
    if (i >= 0 && i < (ptrdiff_t)m_numRows && j >= 0 && j < (ptrdiff_t)m_numColumns)
        {
        value = At(i,j);
        return 1;
        }
    return 0;
    }

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::SetInRow (ptrdiff_t i, ptrdiff_t j, double value0, double value1)
    {
    return Set (i, j, value0) + Set (i, j+1, value1);
    }

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::SetInRow (ptrdiff_t i, ptrdiff_t j, DPoint3dCR xyz)
    {
    return Set (i, j, xyz.x) + Set (i, j+1, xyz.y) + Set (i, j+2, xyz.z);
    }

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::SetInRow (ptrdiff_t i, ptrdiff_t j, double const *data, size_t n)
    {
    size_t numOut = 0;
    for (size_t k = 0; k < n; k++)
      numOut += Set (i, j+k, data[k]);
    return numOut;
    }


/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::GetFromRow (ptrdiff_t i, ptrdiff_t j, double *data, size_t n)
    {
    size_t numOut = 0;
    for (size_t k = 0; k < n; k++)
      numOut += Get (i, j+k, data[k]);
    return numOut;
    }

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::GetFromRow (ptrdiff_t i, ptrdiff_t j, DPoint3dR xyz)
    {
    return Get (i, j, xyz.x) + Get (i, j+1, xyz.y) + Get (i, j+2, xyz.z);
    }

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
size_t RowMajorMatrix::SetInRow (ptrdiff_t i, ptrdiff_t j, double value0, double value1, double value2)
    {
    return Set (i, j, value0) + Set (i, j+1, value1) + Set (i, j+2, value2);
    }


bool RowMajorMatrix::BlockElimination (size_t pivot, RowMajorMatrix &rhs)
    {
    double q = At(pivot, pivot);
    size_t numColumnsRHS = rhs.NumColumns ();
    if (q == 0.0)
        return false;
#ifdef abc
    for (size_t i = pivot + 1; i < m_numRows; i++)
        {
        double a = At (i, pivot) / q;
        for (size_t j = pivot + 1; j < m_numColumns; j++)
            At (i,j) -= a * At(pivot, j);
        rhs[i] -= a * rhs[pivot];
        }
#endif
    /// in pivot row (beyond pivot) divide by the pivot...
    double divQ = 1.0 / q;
    for (size_t j = pivot + 1; j < m_numColumns; j++)
        At(pivot, j) *= divQ;
      for (size_t j = 0; j < numColumnsRHS; j++)
          rhs.At (pivot, j) *= divQ;
    // eliminate in rows below pivot
    for (size_t i = pivot + 1; i < m_numRows; i++)
        {
        double a = At (i, pivot);
        for (size_t j = pivot + 1; j < m_numColumns; j++)
            At (i,j) -= a * At(pivot, j);
        for (size_t j = 0; j < numColumnsRHS; j++)
            rhs.At (i, j) -= a * rhs.At (pivot, j);
        }

    return true;
    }

void RowMajorMatrix::BackSubstitute(RowMajorMatrix &rhs)
    {
    // rhs[last,] is already correct from elimination steps !!!
    if (m_numRows < 2)
        return;
    size_t numJ = rhs.NumColumns ();
    for (size_t j = 0; j < numJ; j++)
        {
        for (size_t i = m_numRows - 1; i > 0; i--)
            {
            double a = rhs.At (i, j);
            for (size_t k = 0; k < i; k++)
                rhs.At(k,j) -= At(k, i) * a;
            }
        }
    }


void RowMajorMatrix::SetZero ()
    {
    for (size_t i = 0, n = size (); i < n; i++)
        at(i) = 0.0;
    }

void RowMajorMatrix::Multiply (bvector<double> const &x, bvector<double> &y) const
    {
    assert (x.size () == m_numColumns);
    y.resize (m_numRows);
    for (size_t k = 0; k < m_numRows; k++)
        y[k] = 0.0;
    for (size_t j = 0; j < m_numColumns; j++)
        {
        double xj = x[j];
        for (size_t i = 0; i < m_numRows; i++)
            y[i] += At(i,j) * xj;
        }
    }
void RowMajorMatrix::SetIdentity ()
    {
    SetZero ();
    for (size_t i = 0, n = m_numRows; i < n; i++)
        At (i,i) = 1.0;
    }

double RowMajorMatrix::MaxAbsBelowPivot (size_t pivot, size_t &i) const
    {
    i = pivot;
    double a = At(pivot, pivot);
    for (size_t i1 = pivot + 1; i1 < m_numRows; i1++)
        {
        double b = fabs (At(i1, pivot));
        if (b > a)
            {
            a = b;
            i = i1;
            }
        }
    return a;
    }

double RowMajorMatrix::MaxAbsBeyondPivot (size_t pivot, size_t &i, size_t &j) const
    {
    i = pivot;
    j = pivot;
    double a = At(pivot, pivot);
    for (size_t i1 = pivot; i1 < m_numRows; i1++)
        {
        for (size_t j1 = pivot; j1 < m_numColumns; j1++)
            {
            double b = fabs (At(i1, j1));
            if (b > a)
                {
                a = b;
                i = i1;
                j = j1;
                }
            }
        }
    return a;
    }

void RowMajorMatrix::AddScaledOuterProductInPlace (double s, bvector<double> const &x, bvector<double> const&y)
    {
    for (size_t i = 0; i < m_numRows; i++)
        {
        for (size_t j = 0; j < m_numColumns; j++)
            At(i,j) += s * x[i] * y[j];
        }
    }

void DoubleOps::SetZeros (bvector<double> &dest)
    {
    for (size_t i = 0, n = dest.size (); i < n; i++)
        dest[i] = 0.0;
    }

void DoubleOps::SetSequential (bvector<double> &dest, double a0, double delta)
    {
    double a = a0;
    for (size_t i = 0, n = dest.size (); i < n; i++, a += delta)
        dest[i] = a;
    }

double DoubleOps::MaxAbsDiff (bvector<double> const &x, bvector<double> const &y)
    {
    double a = 0.0;
    assert (x.size () == y.size ());
    for (size_t i = 0, n = x.size (); i < n; i++)
        {
        double b = fabs (x[i] - y[i]);
        if (b > a)
            a = b;
        }
    return a;
    }



void DoubleOps::ApplyFunction (bvector<double> const &x, double (*f)(double), bvector <double> &fOfx)
    {
    if (fOfx.size () < x.size ())
        fOfx.resize (x.size ());
    for (size_t i = 0; i < x.size (); i++)
        fOfx[i] = f(x[i]);
    }
void DoubleOps::CopyToIndex (bvector<double> const &source, bvector<size_t> const &index, bvector<double> &dest)
    {
    size_t n = index.size ();
    for (size_t i = 0; i < n; i++)
        {
        dest[index[i]] = source[i];
        }
    }

void DoubleOps::CopyFromIndex (bvector<double> const &source, bvector<size_t> const &index, bvector<double> &dest)
    {
    size_t n = index.size ();
    for (size_t i = 0; i < n; i++)
        {
        dest[i] = source[index[i]];
        }
    }


/*---------------------------------------------------------------------------------**//**
* Solve a dense linear system using Gaussian elimination with partial pivoting.
* @remark In classic microstation, this code was solve_linear_system.
* @remark Should turn repeated division by same pivot into multiplications by inverse.
* @remark Absolute error tolerance for small pivots.
* @param amatrix => dense, square matrix stored in row-major order.
* @param dimensions => row and column dimension of matrix.
* @param rhsides    => array of right hand sides stored in column-major order.
* @param numrhs     => number of right hand sides.
* @return false if system appears singular.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiLinAlg_solveLinearGaussPartialPivot

(
double  *amatrix,       /* =>  matrix of coefficients */
int     dimensions,     /* =>  dimension of matrix */
double  *rhsides,       /* <=> input=right hand sides.  output=solutions */
int     numrhs          /* =>  number of right hand sides */
)
    {
    int         row, irow, column, i, j, pivotrow;
    double      *dp, *dp1, *dp2;
    double      temp, pivotel;

    for (row=0; row<dimensions; row++)
        {
        /* first find the largest element of column i and interchange
            that row with row row */
        for (j=pivotrow=row, dp=amatrix+(row*dimensions+row), temp = 0.0;
                j < dimensions;  j++, dp+=dimensions)
            {
            if (fabs(*dp) > temp)
                {
                temp = fabs(*dp);
                pivotrow = j;
                }
            }

        /* if we have an illconditioned matrix, quit */
        if (! SIGNIFICANT12 (temp))
            return false;
        else if (row != pivotrow)
            {
            /* interchange row and pivotrow of coefficient matrix & rhsides */
            for (j=row, dp=amatrix+(row*dimensions+j),
                      dp1=amatrix+(pivotrow*dimensions+j); j<dimensions;
                        j++, dp++, dp1++)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }

            for (j=0, dp=rhsides+row, dp1=rhsides+pivotrow; j<numrhs;
                    j++, dp+=dimensions, dp1+=dimensions)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }
            }

        /* now go through and do the elimination */
        for (irow=row+1, pivotel= *(amatrix+(row*dimensions+row));
                irow<dimensions; irow++)
            {
            for (column=row+1, dp=amatrix+(irow*dimensions+row),
                dp1=amatrix+(row*dimensions+column),
                temp= *dp++/pivotel; column<dimensions; column++, dp++, dp1++)
                {
                *dp -= temp * *dp1;
                }
            /* do the same operation on the right hand sides */
            for (column=0, dp=rhsides+irow, dp1=rhsides+row;
                 column<numrhs; column++, dp1+= dimensions, dp+=dimensions)
                {
                *dp -= temp * *dp1;
                }
            }
        }

    /* next go through and do the back substitution for all rh sides */
    for (i=0; i<numrhs; i++)
        {
        for (row=dimensions-1, dp=rhsides+i*dimensions+row; row>=0; row--, dp--)
            {
            for (j=dimensions-1, dp1=rhsides+i*dimensions+j,
                    dp2=amatrix+row*dimensions+j; j>row; j--, dp1--, dp2--)
                {
                *dp -= *dp1 * *dp2;
                }
            *dp /= *dp2;
            }
        }
    return true;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          ROTATE -- macro                                         |
|                                                                       |
                    this macro has the intentional side-effects
                    of setting h & g to new values.

| author        SamWilson                               3/93            |
|                                                                       |
+----------------------------------------------------------------------*/
#define ROTATE(a,ii,jj,kk,ll)\
    {\
    g = ROWORDERARRAYENTRY (a, ii, jj, n);\
    h = ROWORDERARRAYENTRY (a, kk, ll, n);\
    ROWORDERARRAYENTRY (a, ii, jj, n) = g - s * (h + g*tau);\
    ROWORDERARRAYENTRY (a, kk, ll, n) = h + s * (g - h*tau);\
    }

#define MAX_JACOBI_ITERS    50

/*---------------------------------------------------------------------------------**//**
* Diagonalize a symmetric matrix by Jacobi's method.
* The input matrix is destroyed!!!
* Return eigenvalues and normalized eigenvectors (column vectors).
* @param pA <=> symmetric matrix.  Stored in row-major order.
*               Contents are destroyed (diagonalized) by the iteration!
* @param pQ <= matrix of eigenvectors.
* @param pD <= array of n eigenvalues.
* @param pB <= scratch array, length n.
* @param pZ <= scratch array, length n.
*
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiLinAlg_symmetricJacobiEigensystem

(
double  *pA,
double  *pQ,
double  *pD,
double  *pB,
double  *pZ,
int     n
)
    {
    int     j, iq, ip, i;
    double  tresh, theta, tau, t, sum, s, c, g, h;
    //int nn = n * n;
    int numRotations = 0;
    //int diagonalStep = n + 1;

    /* Initialize Eigenvectors as identity matrix */
    bsiLinAlg_initIdentity (pQ, n, n, 1);

    for (ip=0; ip<n; ip++)
        {
        pD[ip] = pB[ip] = ROWORDERARRAYENTRY (pA, ip, ip, n);
        pZ[ip] = 0;
        }

    for (i=0; i<MAX_JACOBI_ITERS; i++)
        {
        sum = 0.0;
        for (ip=0; ip<n; ip++)
            for (iq=ip+1; iq<n; iq++)
               sum += fabs (ROWORDERARRAYENTRY (pA, ip, iq, n));

        if (sum == 0.0)
            return;

        /* During first sweeps, only attack off diagonals that are
            fairly large.   Later, do them all. */
        if (i < 3)
            tresh = 0.2 * sum / (n*n);
        else
            tresh = 0.0;

        /* Sweep through upper triangle, rotating each entry away */
        for (ip=0; ip<n-1; ip++)
            {
            for (iq=ip+1; iq<n; iq++)
                {
                /* Call off-diagonal entry a zero if adding 100a to
                    each of eigenvalues leaves the eigenvalues unchanged */
                g = 100.0 * fabs (ROWORDERARRAYENTRY (pA, ip, iq, n));
                if (i > 3 && fabs (pD[ip]) + g == fabs (pD[ip])
                          && fabs (pD[iq]) + g == fabs (pD[iq]))
                    {
                    ROWORDERARRAYENTRY(pA, ip, iq, n) = 0.0;
                    }
                else if (fabs(ROWORDERARRAYENTRY(pA, ip, iq, n)) > tresh)
                    {
                    h = pD[iq] - pD[ip];
                    if (fabs (h) + g == fabs (h))
                        {
                        t = ROWORDERARRAYENTRY(pA, ip, iq, n) / h;
                        }
                    else
                        {
                        theta = 0.5 * h / ROWORDERARRAYENTRY(pA, ip, iq, n);
                        t = 1.0 / (fabs (theta) + sqrt (1.0 + theta * theta));
                        if (theta < 0.0) t = -t;
                        }
                    c = 1.0 / sqrt (1.0 + t*t);
                    s = t * c;
                    tau = s/(1.0 + c);
                    h = t * ROWORDERARRAYENTRY(pA, ip, iq, n);
                    pZ[ip] -= h;
                    pZ[iq] += h;
                    pD[ip] -= h;
                    pD[iq] += h;
                    ROWORDERARRAYENTRY(pA, ip, iq, n) = 0.0;
                    for (j=0; j<=ip-1; j++)
                        ROTATE (pA, j, ip, j, iq)
                    for (j=ip+1; j<=iq-1; j++)
                        ROTATE (pA, ip, j, j, iq)
                    for (j=iq+1; j<n; j++)
                        ROTATE (pA, ip, j, iq, j)
                    for (j=0; j<n; j++)
                        ROTATE (pQ, j, ip, j, iq)
                    ++numRotations;
                    }
                }
            }

        for (ip=0; ip<n; ip++)
            {
            pB[ip] += pZ[ip];
            pD[ip] = pB[ip];
            pZ[ip] = 0.0;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Solve a block tridiagonal linear system.
* The conceptual matrix structure is
*                                |X0|
*       LL|DD RR 00 00 00 00|    |X1|    |F0|
*         |LL DD RR 00 00 00|    |X2|    |F1|
*         |00 LL DD RR 00 00|    |X3|    |F2|
*         |00 00 LL DD RR 00|   *|X |  = |F3|
*         |00 00 00 LL DD RR|    |X |    |F|
*         |00 00 00 00 LL DD|RR  |XM|    |FM-1|
*                                |XM+1|
* Each LL, DD, RR denotes a square block.  The LL and RR outside the main matrix
*   are placeholders added so that all block rows can be stored with their diagonal DD
*   block at the same internal offsets.  Likewise X0 and XM are placeholders which are
*   assigned zero values as part of the solution process.
* If there are M block diagonals, each block being N by N, the matrix is supplied
*   as M * N rows, each row containing 3N numeric entries in row-major layout.
* That is, the first 3N coefficients are the llddrr sequence from (scalar) row 0.
* The second 3N coefficients llddrr from row 1, up to row N-1.
*
* The right hand side column vector consists of M*N doubles
* The solution vector consists of (M+2)*N doubles, with the first and last N as placeholders
*   which are zeroed as part of the solution process.
*
* Some explanation for the placeholders: The placeholders fit nicely with typical geometric
* setting in which each block row contains the equations for one interior point of a sequence
* of (M+2)points numbered 0,1,..M+1.  The right hand side values are constraints applied to
* each of the M interior points, and the soluton values X are corrections to be applied to
* each point's coordinates.  The endpoints 0 and (M+1) are fixed.  With the placeholder
* convention, the equation for each interior point can be constructed without consideration
* for whether its neighbor is an interior or endpoint.   Just fill the block row as if all points
* are interior. The endpoint blocks will be ignored and the endpoint solution values X0 and X(M+1)
* zeroed out.
*
* Solution process is straight Gaussian elimination.  (No pivoting.)
* @param pX OUT Solution values.
* @param pA IN OUT coefficient matrix.  This is destroyed by the factorization.
* @param pf IN OUT right hand side.  This is destroyed by the factorization.
* @param numBlock IN number of block rows.
* @param blockSize IN size of each (square) block.
* @return true if solution completed.
* @bsimethod                                                    EarlinLutz      06/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiLinalg_solveBlockTridiagonal

(
double *pX,
double *pA,
double *pf,
int     numBlock,
int     blockSize
)
    {
    double rPivot;
    double pivot;
    int block;
    int numThisRow;
    int i, j, k;
    int pivotRow;
    double aa;
    int rowStep = 3 * blockSize;
    int blockStep = rowStep * blockSize;
    int bandWidth = 2 * blockSize;
    int pivotStep = rowStep + 1;    /* Physical step only within a block */
    double *pB00;     /* 00 entry of current diagonal block */
    double *pC00;     /* 00 entry of subdiagonal blocks  */
    double *pf0;      /* RHS entry at row of pB00 */
    double *pf1;
    double *pX0;
    double *pPivot;
    double *pTarget;

    for (block = 0; block < numBlock; block++)
        {
        pB00 = pA + block * blockStep + blockSize;  /* 00 entry in current diagonal block. */
        pC00 = pA + (block + 1) * blockStep;        /* 00 entry in left block just below diagonal. (to right of pB00) */
        pf0  = pf + block * blockSize;              /* 0 entry in F to right of pB00 */
        pf1  = pf0 + blockSize;                     /* 0 entry in F to right of pC00 */

        /* Eliminate below diagonal in two blocks from pB00 and pC00 */
        for (k = 0; k < blockSize; k++)
            {
            numThisRow = 2 * blockSize - k;
            pPivot = pB00 + k * pivotStep;
            pivotRow = block * blockSize + k;
            pivot = pPivot[0];
            if (pivot == 0.0)
                return false;
            rPivot = 1.0 / pivot;
            pTarget = pPivot;
            /* Elimination WITHIN pB00 block. */
            for (i = k + 1; i < blockSize; i++)
                {
                pTarget += rowStep;
                aa = pTarget[0] * rPivot;
                for (j = 0; j < numThisRow; j++)
                    {
                    pTarget[j] -= aa * pPivot[j];
                    }
                pf0[i] -= aa * pf0[k];
                }

            /* Elimination of pC00 block using pivots in pB00 block. */
            if (block < numBlock - 1)
                {
                pTarget = pC00 + k;
                for (i = 0; i < blockSize; i++, pTarget += rowStep)
                    {
                    aa = pTarget[0] * rPivot;
                    for (j = 0; j < numThisRow; j++)
                        {
                        pTarget[j] -= aa * pPivot[j];
                        }
                    pf1[i] -= aa * pf0[k];
                    }
                }
            }
        }

    memset (pX, 0, (numBlock + 2) * blockSize * sizeof (double));
    /* Back substitution. */
    for (block = numBlock - 1; block >= 0; block--)
        {
        pB00 = pA + block * blockStep + blockSize;
        pX0  = pX + blockSize  + block * blockSize;
        for (k = blockSize - 1; k >= 0; k--)
            {
            pPivot = pB00 + k * rowStep;
            pivotRow = block * blockSize + k;
            aa = pf[pivotRow];
            for (j = k + 1; j < bandWidth; j++)
                aa -= pX0[j] * pPivot[j];
            pX0[k] = aa / pPivot[k];
            }
        }

    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
