/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
#include <memory.h>
#include <Geom/internal2/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

Public GEOMDLLIMPEXP void bsiCTrig_dotProduct

(
DPoint2dP pDot, /* <= complex dot product */
const double *pUr,      /* => real part of column U */
const double *pUi,      /* => imaginary part of column U */
const double *pVr,      /* => real part of column V */
const double *pVi,      /* => imaginary part of column V */
int         step,       /* => step between adjacent members of all vectors */
int         n           /* => vector lengths */
);
#define PENCIL_DEBUG_not

#ifdef PENCIL_DEBUG
void printRealMatrix

(
DMatrix4dCP pA,
const char *pName
)
    {
    int i, j;
    double smallNum = 1.0e-4;
    double tinyNum = 1.0e-12;
    printf( " %s = \n", pName);
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            double a = pA->coff[i][j];
            if (fabs (a) > smallNum)
                printf( "%20.11lf", a);
            else if (fabs (a) > tinyNum)
                printf( "%20.7le", a);
            else
                printf( "%20.3le", a);
            }
        printf( "\n");
        }
    }

void printShortNum

(
double x,
const char *pSuffix
)
    {
    if (fabs (x) < 1.0e20)
        {
        int ix = (int)x;

        if (ix == x)
            {
            printf("%d", ix);
            }
        else
            {
            printf("%lf", x);
            }
        }
    else
        {
        printf("%lf%", x);
        }

    if (pSuffix)
        printf("%s", pSuffix);

    }

void printSigma

(
DPoint4dCP pSigma,
const char *pName
)
    {
    printf (" %s = (", pName);
    printShortNum (pSigma->x, ",");
    printShortNum (pSigma->y, ",");
    printShortNum (pSigma->z, ",");
    printShortNum (pSigma->w, ")\n");
    }
#endif
#ifdef CompileAll
/*-----------------------------------------------------------------*//**
* @return ratio of off diagonal sum of squares divided by diagonal sum of
*       squares.
* @param pA <= matrix
*/
static double jmdlDMatrix4d_diagonalRatio

(
DMatrix4dP pA
)
    {
    int i, j;
    double s0 = 0.0;
    double s1 = 0.0;
    double ratio = 1.0;
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            if (i == j)
                s0 += pA->coff[i][j] * pA->coff[i][j];
            else
                s1 += pA->coff[i][j] * pA->coff[i][j];
            }
        }
    if (s0 == 0.0)
        ratio = 1000.0;
    else
        ratio = s1 / s0;
    return ratio;
    }
#endif
#ifdef PENCIL_DEBUG
void printComplexMatrix

(
DMatrix4dP pAr,
DMatrix4dP pAi,
const char *pName
)
    {
    int i, j;
    printf( " %s = \n", pName);
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            printf( "%8.4lf+%8.4lfi ",
                        pAr->coff[i][j],
                        pAi->coff[i][j]);
            }
        printf( "\n");
        }

    }
#endif
/*-----------------------------------------------------------------*//**
* Form the (complex) matrix
*       D = sqrt (sigma0) * B * invsqrt(sigma1)
* where
* B is a real matrix
* sigma0 and sigma1 are real diagonal matrices
* sqrt(sigma0) * sqrt(sigma0) = sigma0
* invsqrt(sigma1) * sigma1 * invsqrt(sigma1) = I
* @param pDr            <= real part of D matrix
* @param pDi            <= imaginary part of D matrix
* @param pSigma0        <= diagonal components
* @param pB             <= B matrix
* @param pSigma1        <= diagonal components
*/
static void expandDMatrix

(
DMatrix4dP pDr,
DMatrix4dP pDi,
DPoint4dCP pSigma0,
DMatrix4dP pB,
DPoint4dCP pSigma1
)
    {
    int i, j;
    double *pS0 = (double *)pSigma0;
    double *pS1= (double *)pSigma1;
    for (i = 0; i < 4; i++)
        {
        for(j = 0; j < 4; j++)
            {
            if (pS0[i] == 0.0 || pS1[i] == 0.0)
                {
                }
            else
                {
                DPoint2d zA, zB;
                DPoint2d szA, szB;
                DPoint2d z;
                bsiDPoint2d_setComponents (&zA, pS0[i], 0.0);
                bsiDPoint2d_setComponents (&zB, pS1[j], 0.0);
                bsiComplex_sqrt (&szA, &zA);
                bsiComplex_sqrt (&szB, &zB);
                bsiComplex_divide (&z, &szA, &szB);
                bsiComplex_realMultiplyComplex (&z, pB->coff[i][j], &z);
                pDr->coff[i][j] = z.x;
                pDi->coff[i][j] = z.y;
                }
            }
        }
    }

/*-----------------------------------------------------------------*//**
* @param pLambda[i] <= dot product of column i with itself.
* @param pRootLambda[i] <= sqare root of pLambda[i]
* pType[i]      <= 0 if pLambda[i] is zero within tolerance
*                  1 if pLambda[i] is a positive real eigenvalue
*                 -1 if pLambda[i] is a negative real eigenvalue
*                  2 if pLambda[i] is part of a complex conjugate pair
*                 -2 if pLambda[i] is part of a real conjutate pair
* pPartnerIndex <= index of pLambda[i]'s conjugate partner or duplicate real
* pZeroTol      <= tolerance used to distinguish zeros on eigenvalues
* pAr           => real part of matrix
* pAi           => complex part of matrix
*/
static int jmdlCHMatrix_eigenpairsFromOrthogonalFactors

(
DPoint2dP pLambda,
DPoint2dP pRootLambda,
int      *pPartnerIndex,
int      *pType,
double   *pZeroTol,
DMatrix4dCP pAr,
DMatrix4dCP pAi
)
    {
    int i, j;
    double maxCoff, a;
    double ar, ai;
    double epsilon;
    static double s_relTol = 1.0e-7;
    int status = SUCCESS;
    maxCoff = 0.0;


    /* Inner product of each column with itself is the raw eigenvalue */
    /* Square roots of eigenvalues are axis lengths */
    for (i = 0; i < 4; i++)
        {
        bsiCTrig_dotProduct (&pLambda[i], &pAr->coff[0][i], &pAi->coff[0][i],
                                           &pAr->coff[0][i], &pAi->coff[0][i], 4, 4);
        bsiComplex_sqrt (&pRootLambda[i], &pLambda[i]);
        if ((a = fabs (pRootLambda[i].x)) > maxCoff)
                maxCoff = a;
        if ((a = fabs (pRootLambda[i].y)) > maxCoff)
                maxCoff = a;
        pPartnerIndex[i] = i;
        }

    if (maxCoff == 0.0)
        {
        /* The eigenvalues are all zero. */
        for (i = 0; i < 4; i++)
            {
            pPartnerIndex[i] = i - 1;
            pType[i] = 0;
            }
        pPartnerIndex[0] = 3;
        }
    else
        {
        *pZeroTol = epsilon = s_relTol * maxCoff;

        for (i = 0; i < 4; i++)
            {
            ar = pRootLambda[i].x;
            ai = pRootLambda[i].y;
            if (fabs (ai) < epsilon)
                {
                if (fabs (ar) < epsilon)
                    {
                    /* It's just a zero eigenvalue.  Link to a prior one. */
                    pType[i] = 0;
                    for (j = i - 1; j >= 0; j--)
                        {
                        if (pType[j] == 0)
                            {
                            pPartnerIndex[i] = pPartnerIndex[j];
                            pPartnerIndex[j] = i;
                            break;
                            }
                        }
                    }
                else
                    {
                    /* It's a real eigenvalue.  Link to a prior one of the same value. */
                    pType[i] = 1;
                    for (j = i - 1; j >= 0; j--)
                        {
                        if (pType[j] == 1 && fabs (pRootLambda[j].x - pRootLambda[i].x) < epsilon)
                            {
                            pPartnerIndex[i] = pPartnerIndex[j];
                            pPartnerIndex[j] = i;
                            break;
                            }
                        }
                    }
                }
            else if (fabs (ar) < epsilon)
                {
                /* Pure imaginary root, i.e. negative real eigenvalue */
                pType[i] = -1;
                for (j = i - 1; j >= 0; j--)
                    {
                    if (pType[j] == -1 && fabs (pRootLambda[j].y - pRootLambda[i].y) < epsilon)
                        {
                        pPartnerIndex[i] = pPartnerIndex[j];
                        pPartnerIndex[j] = i;
                        break;
                        }
                    }
                }
            else
                {
                /* It's a complex eigenvalue.  Look for a prior unmatched conjugate. */
                pType[i] = 3;
                for (j = i - 1; j >= 0; j--)
                    {
                    if (   pType[j] == 3                /* It's complex */
                        && pPartnerIndex[j] == j        /* It doesn't have a conjugate yet */
                        && fabs (pRootLambda[j].x - pRootLambda[i].x) < epsilon         /* Real parts match */
                        && fabs (pRootLambda[j].y + pRootLambda[i].y) < epsilon         /* Complex parts are opposite */
                        )
                        {
                        pPartnerIndex[i] = pPartnerIndex[j];
                        pPartnerIndex[j] = i;
                        pType[i] = 2;
                        pType[j] = 2;
                        break;
                        }
                    }
                if (pType[i] == 3)
                    {
                    /* No complex conjugate found */
                    status = ERROR;
                    }
                }
            }
        }
    return status;
    }


#ifdef PENCIL_TRANSPOSE
/*-----------------------------------------------------------------*//**
* Transpose a 4x4 matrix.
* @param pA <= transposed matrix
* @param pB => original matrix
*/
static void bsiDMatrix4d_transpose

(
DMatrix4dP pA,
DMatrix4dCP pB
)
    {
    DMatrix4d work;
    int i, j;
    for (i = 0; i < 4 ; i++)
        {
        for (j = 0; j < 4 ; j++    )
            {
            work.coff[i][j] = pB->coff[j][i];
            }
        }
    *pA = work;
    }
#endif

/*-----------------------------------------------------------------*//**
* @param pA     <= real matrix
* @param col    => column to examine
* @return       maximum absolute value in the column.
*/
static double jmdlDMatrix4d_colMaxAbs

(
DMatrix4dP pA,
int     col
)
    {
    double maxAbs = fabs (pA->coff[0][col]);
    double a;
    int i;
    if (col >= 0 && col < 4)
        {
        for (i = 1; i < 4; i++)
            {
            a = fabs (pA->coff[i][col]);
            if (a > maxAbs)
                maxAbs = a;
            }
        }
    return maxAbs;
    }

/*-----------------------------------------------------------------*//**
* @param pA     <=> destination matrix
* @param colA   => destination column index
* @param pB     => source matrix
* @param colB   => source column index
* @param scale  => scale factor to apply
*/
static void jmdlDMatrix4d_copyScaledColumn

(
DMatrix4dP pA,
int     colA,
DMatrix4dCP pB,
int     colB,
double  scale
)
    {
    int i;
    if (0 <= colA && colA < 4 && 0 <= colB && colB < 4)
        {
        for (i = 0; i < 4; i++)
            {
            pA->coff[i][colA] = scale * pB->coff[i][colB];
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Copy a "real" eigenvector from the complex factorization to the
* real subspace matrix.
* @param pSigma         <= sign for diagonal matrix
* @param pE             <=> real destination matrix
* @param colE           => destination column index
* @param pDr            => real part of source matrix
* @param pDi            => imaginary part of source matrix
* @param colD           => source column index
* @param pScale         => scale factor to apply.
* @param zeroTol        => tolerance derived from eigenvalues.
*/
static int jmdlDMatrix4d_selectFromScaledColumn

(
double          *pSigma,        /* <= Diagonal sign */
DMatrix4dP pE,          /* <= destination matrix */
int             colE,           /* => destination column */
DMatrix4dP pDr,         /* => source real matrix */
DMatrix4dP pDi,         /* => source imaginary part */
int             colD,           /* => source column */
DPoint2dP pScale,       /* => scale factor to apply */
double          zeroTol         /* => tolerance for zero tests */
)
    {
    DPoint2d aa, bb;
    double max0, max1;
    DMatrix4d workMatrix;
    int i;
    int status = SUCCESS;

    /* Put the real and imaginary components of pScale times column in workMatrix */
    for (i = 0; i < 4; i++)
        {
        aa.x = pDr->coff[i][colD];
        aa.y = pDi->coff[i][colD];
        bsiComplex_multiply (&bb, &aa, pScale);
        workMatrix.coff[i][0] = bb.x;
        workMatrix.coff[i][1] = bb.y;
        }

    max0 = jmdlDMatrix4d_colMaxAbs (&workMatrix, 0);
    max1 = jmdlDMatrix4d_colMaxAbs (&workMatrix, 1);

    if (max0 <= zeroTol && max1 > zeroTol)
        {
        *pSigma = -1.0;
        jmdlDMatrix4d_copyScaledColumn (pE, colE, &workMatrix, 1, 1.0);
        }
    else if (max1 <= zeroTol && max0 > zeroTol)
        {
        *pSigma = 1.0;
        jmdlDMatrix4d_copyScaledColumn (pE, colE, &workMatrix, 0, 1.0);
        }
    else
        {
        status = ERROR;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Copy complex conjugate pair of eigenvectors from the complex factorization to the
* real subspace matrix.
* @param pSigma0        <= sign for diagonal matrix
* @param pSigma1        <= sign for diagonal matrix
* @param pE             <=> real destination matrix
* @param colE0          => destination column index
* @param colE1          => destination column index
* @param pDr            => real part of source matrix
* @param pDi            => imaginary part of source matrix
* @param colD0          => source column index
* @param colD0          => source column index
* @param pScale0        => scale factor to apply.
* @param pScale1        => scale factor to apply.
* @param zeroTol        => tolerance derived from eigenvalues.
*/
static int jmdlHmatrix_selectFromConjugateColumns

(
double          *pSigma0,       /* <= Diagonal sign */
double          *pSigma1,       /* <= Diagonal sign */
DMatrix4dP pE,          /* <= destination matrix */
int             colE0,          /* => destination column */
int             colE1,          /* => destination column */
DMatrix4dP pDr,         /* => source real matrix */
DMatrix4dP pDi,         /* => source imaginary part */
int             colD0,          /* => source column */
int             colD1,          /* => source column */
DPoint2dP pScale0,      /* => scale factor to apply to column 0 */
DPoint2dP pScale1,      /* => scale factor to apply to column 1 */
double          zeroTol         /* => tolerance for zero tests */
)
    {
    DPoint2d aa, bb, cc;
    double xSum, xDiff, ySum, yDiff;
    DMatrix4d workMatrix;
    int i;
    double sumTol = 4.0 * zeroTol;
    double colScale = sqrt (2.0);
    int status = SUCCESS;

    xSum = xDiff = ySum = yDiff = 0.0;
    /* Put the real and imaginary components of pScale times column in workMatrix */
    for (i = 0; i < 4; i++)
        {
        aa.x = pDr->coff[i][colD0];
        aa.y = pDi->coff[i][colD0];
        bsiComplex_multiply (&bb, &aa, pScale0);
        workMatrix.coff[i][0] = bb.x;
        workMatrix.coff[i][1] = bb.y;

        aa.x = pDr->coff[i][colD1];
        aa.y = pDi->coff[i][colD1];
        bsiComplex_multiply (&cc, &aa, pScale1);
        workMatrix.coff[i][2] = cc.x;
        workMatrix.coff[i][3] = cc.y;

        xSum += fabs(bb.x + cc.x);
        ySum += fabs(bb.y + cc.y);
        xDiff += fabs(bb.x - cc.x);
        yDiff += fabs(bb.y - cc.y);
        }


    if (   (xDiff < sumTol && ySum  < sumTol)
        || (xSum  < sumTol && yDiff < sumTol)
       )
        {
        *pSigma0 = 1.0;
        *pSigma1 = -1.0;
        jmdlDMatrix4d_copyScaledColumn (pE, colE0, &workMatrix, 0, colScale);
        jmdlDMatrix4d_copyScaledColumn (pE, colE1, &workMatrix, 1, colScale);
        }
     else
        {
        status = ERROR;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Return the real factorization of a shifted eigensystem.
* @param pE             <= characteristic real form
* @param pDr            => real part of factorization
* @param pDi            => imaginary part of factorization
* @param pLambda[i]     => eigenvalue from factorization
* @param pRootLambda[i] => square root of eigenvalue
* @param pPartnerIndex  => conjugate or multiple root successor
* @param pType          => eigenvalue type
* @param zeroTol        => zero tolerance for eigenvalues
* @param index          => index of eigenvalue to use as shift
*/
static int jmdlDMatrix4d_shiftedBasis

(
DMatrix4dP pE,          /* <= expanded basis matrix */
double  *pSigma,        /* <= characteristic signs */
DMatrix4dP pDr,         /* => real part of factor */
DMatrix4dP pDi,         /* => imaginary part of factor */
DPoint2dP pLambda,      /* => raw eigenvalues */
DPoint2dP pRootLambda,  /* => square roots of eigenvalues */
int      *pPartnerIndex,/* => indices of partner eigenvalues */
int      *pType,        /* => eigenvalue types */
double   zeroTol,       /* => tolerance for zero values */
int      index          /* => index of eigenvalue to use as shift */
)
    {
    int i;
    int status = ERROR;
    DPoint2d shiftedLambda[4];
    double dummySigma;
    DPoint2d shiftedRoot[4];
    DPoint2d scale;
    double shift;

    if (pType[index] == 1 || pType[index] == -1 || pType[index] == 0)
        {
        shift = pLambda[index].x;
        for (i = 0; i < 4; i++)
            {
            shiftedLambda[i] = pLambda[i];
            shiftedLambda[i].x -= shift;
            bsiComplex_sqrt (&shiftedRoot[i], &shiftedLambda[i]);
            }

        for (i = 0; i < 4; i++)
            {
            double maxReal = jmdlDMatrix4d_colMaxAbs (pDr, i);
            double maxImag = jmdlDMatrix4d_colMaxAbs (pDi, i);

            if (pType[i] == 1 || pType[i] == -1 || pType[i] == 0)
                {
                /* It's a real eigenvalue. */
                double absRoot = sqrt (fabs (shiftedLambda[i].x));
                scale.x = 1.0;
                scale.y = 0.0;

                if (absRoot <= zeroTol)
                    {
                    if (   (maxReal > zeroTol && maxImag <= zeroTol)
                        || (maxReal <= zeroTol && maxImag > zeroTol)
                       )
                        {
                        /* Pure real or pure imaginary eigenvector */
                        pSigma[i] = 0.0;
                        status = jmdlDMatrix4d_selectFromScaledColumn
                                                (&dummySigma, pE, i, pDr, pDi, i, &scale, zeroTol);
                        }
                    else
                        {
                        /* Double root?? */
                        int k = pPartnerIndex[i];
                        if (k < i)
                            {
                            /* It should already have been done from k */
                            }
                        else if (k > i && pPartnerIndex[k] == i)
                            {
                            status = jmdlHmatrix_selectFromConjugateColumns
                                        (
                                        &pSigma[i],
                                        &pSigma[k],
                                        pE,
                                        i,
                                        k,
                                        pDr,
                                        pDi,
                                        i,
                                        k,
                                        &shiftedRoot[i],
                                        &shiftedRoot[k],
                                        zeroTol
                                        );
                            pSigma[i] = pSigma[k] = 0.0;

                            }
                        else
                            {
                            /* Ummm ... quadruple root? */
                            status = ERROR;
                            break;
                            }
                        }
                    }
                else
                    {
                    scale = shiftedRoot[i];
                    jmdlDMatrix4d_selectFromScaledColumn (&pSigma[i], pE, i, pDr, pDi, i, &scale, zeroTol);
                    }
                }
            else if (pType[i] == 2)
                {
                int k = pPartnerIndex[i];
                if (k > i)
                    {
                    jmdlHmatrix_selectFromConjugateColumns
                                        (
                                        &pSigma[i],
                                        &pSigma[k],
                                        pE,
                                        i,
                                        k,
                                        pDr,
                                        pDi,
                                        i,
                                        k,
                                        &shiftedRoot[i],
                                        &shiftedRoot[k],
                                        zeroTol
                                        );
                    }
                }
            else
                {
                status = ERROR;
                }
            }
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Matrix product C = A^T * B
* @param pC <= product matrix
* @param pA => first term, to be transposed
* @param pB => second term
*/
static void bsiDMatrix4d_multiplyAtB

(
DMatrix4dP pC,
DMatrix4dCP pA,
DMatrix4dCP pB
)
    {
    int i, j, k;
    DMatrix4d product = DMatrix4d::FromZero ();
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            product.coff[i][j] = 0.0;
            for (k = 0; k < 4; k++)
                product.coff[i][j] += pA->coff[k][i] * pB->coff[k][j];
            }
        }
    *pC = product;
    }

/*-----------------------------------------------------------------*//**
* Matrix product C = A * sigma * B^T
* @param pC     <= product matrix
* @param pA     => first factor
* @param pSigma => diagonal entries of sigma factor
* @param pB     => last factor, to be transposed
*/
static void bsiDMatrix4d_multiplyASigmaBt

(
DMatrix4dP pC,
DMatrix4dCP pA,
DPoint4dCP pSigma,
DMatrix4dCP pB
)
    {
    double *pDiag =  (double *)pSigma;
    int i, j, k;
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            pC->coff[i][j] = 0.0;
            for (k = 0; k < 4; k++)
                pC->coff[i][j] += pA->coff[i][k] * pDiag[k] * pB->coff[j][k];
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Matrix product C = A * sigma * B^T
* @param pC     <= product matrix
* @param pA     => first factor
* @param pSigma => diagonal entries of sigma factor
* @param pB     => last factor, to be transposed
*/
static void bsiDMatrix4d_multiplyAtSigmaB

(
DMatrix4dP pC,
DMatrix4dCP pA,
DPoint4dCP pSigma,
DMatrix4dCP pB
)
    {
    double *pDiag =  (double *)pSigma;
    int i, j, k;
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            pC->coff[i][j] = 0.0;
            for (k = 0; k < 4; k++)
                pC->coff[i][j] += pA->coff[k][i] * pDiag[k] * pB->coff[k][j];
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Matrix product A = K * B
* @param pAr    <= real part of product
* @param pAi    <= imaginary part of product
* @param pSigma => diagonal entries of sigma factor: K^T * K = sigma
* @param pBr    => real part of B
* @param pBi    => imaginary part of B
*/
static void jmdlCHMatrix_multiplyKB

(
DMatrix4dP pAr,
DMatrix4dP pAi,
DPoint4dCP pSigma,
DMatrix4dCP pBr,
DMatrix4dCP pBi
)
    {
    int i, j;
    double sigma;
    DPoint2d rootSigma;
    DPoint2d bVal, aVal;
    double *pSigmaDouble = (double*)pSigma;
    for (i = 0; i < 4; i++)
        {
        sigma = pSigmaDouble[i];
        if (sigma >= 0.0)
            {
            bsiDPoint2d_setComponents (&rootSigma, sqrt (sigma), 0.0);
            }
        else
            {
            bsiDPoint2d_setComponents (&rootSigma, 0.0,  sqrt (fabs(sigma)));
            }
        for (j = 0; j < 4; j++)
            {
            bsiDPoint2d_setComponents (&bVal, pBr->coff[i][j], pBi->coff[i][j]);
            bsiComplex_multiply (&aVal, &rootSigma, &bVal);
            pAr->coff[i][j] = aVal.x;
            pAi->coff[i][j] = aVal.y;
            }
        }
    }
#ifdef CompileAll
/*-----------------------------------------------------------------*//**
* Matrix product A = B * sigma
* @param pAr    <= real part of product
* @param pAi    <= imaginary part of product
* @param pBr    => real part of B
* @param pBi    => imaginary part of B
* @param pSigma => diagonal components
*/
static void jmdlCHMatrix_multiplyBColumns

(
DMatrix4dP pAr,
DMatrix4dP pAi,
DMatrix4dCP pBr,
DMatrix4dCP pBi,
DPoint2dCP pSigma       /* => diagonal entries */
)
    {
    int i, j;
    DPoint2d bVal, aVal, cVal;
    for (j = 0; j < 4; j++)
        {
        cVal = pSigma[j];
        for (i = 0; i < 4; i++)
            {
            bsiDPoint2d_setComponents (&bVal, pBr->coff[i][j], pBi->coff[i][j]);
            bsiComplex_multiply (&aVal, &bVal, &cVal);
            pAr->coff[i][j] = aVal.x;
            pAi->coff[i][j] = aVal.y;
            }
        }
    }
#endif

/*-----------------------------------------------------------------*//**
* Complex Matrix product C = A^T * B
* @param pCr    <= real part of product
* @param pCi    <= imaginary part of product
* @param pAr    => real part of A
* @param pAi    => imaginary part of A
* @param pBr    => real part of B
* @param pBi    => imaginary part of B
*/
static void jmdlCHMatrix_multiplyAtB

(
DMatrix4dP pCr,
DMatrix4dP pCi,
DMatrix4dCP pAr,
DMatrix4dCP pAi,
DMatrix4dCP pBr,
DMatrix4dCP pBi
)
    {
    DMatrix4d ArBr;
    DMatrix4d ArBi;
    DMatrix4d AiBr;
    DMatrix4d AiBi;
    int i, j;
    bsiDMatrix4d_multiplyAtB (&ArBr, pAr, pBr);
    bsiDMatrix4d_multiplyAtB (&ArBi, pAr, pBi);
    bsiDMatrix4d_multiplyAtB (&AiBr, pAi, pBr);
    bsiDMatrix4d_multiplyAtB (&AiBi, pAi, pBi);
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            pCr->coff[i][j] = ArBr.coff[i][j] - AiBi.coff[i][j];
            pCi->coff[i][j] = AiBr.coff[i][j] + ArBi.coff[i][j];
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Compute the hypotenuse of a right triangle with complex side lengths.
* @param pR <= sqrt (X^2 + Y^2)
* @param pX => leg of triangle
* @param pY => leg of triangle
*/
static Public void jmdlVector_complexHypotenuse

(
DPoint2dP pR,     /* <= (complex) hypotenuse of (complex) right triangle,
                                        R = sqrt (X^2 + Y^2) */
const DPoint2d  *pX,     /* => (complex) leg of triangle */
const DPoint2d  *pY      /* => (complex) leg of triangle */
)
    {
    DPoint2d XX, YY, RR;
    bsiComplex_multiply (&XX, pX, pX);
    bsiComplex_multiply (&YY, pY, pY);
    bsiComplex_add (&RR, &XX, &YY);
    bsiComplex_sqrt (pR, &RR);
    }

#ifdef PENCIL_HALFANGLE
/*-----------------------------------------------------------------*//**
* Compute the sine and cosine of the angle which is half of the
* angle whose sine and cosine are given.
* @param pCosA <= cosine of half angle
* @param pSinA <= sine of half angle
* @param rCos2A => x component of a vector
* @param rSin2A => y component of a vector
*/
Publicx void bsiTrig_halfAngleFunctions

(
double      *pCosA,     /* <= cosine(a) cosine of angle*/
double      *pSinA,     /* <= sine (a)  sine of angle*/
double      rCos2A,      /* => cosine(2a) cosine of double angle, possibly scaled by some r*/
double      rSin2A       /* => sine(2a) sine of double angle, possibly scaled by same r.*/
)
    {
    double r = sqrt (rCos2A * rCos2A + rSin2A * rSin2A);
    /* If the caller really gave you sine and cosine values, r should be 1.  However,*/
    /* to allow scaled values -- e.g. the x and y components of any vector -- we normalize*/
    /* right here.  This adds an extra sqrt and 2 divides to the whole process, but improves*/
    /* both the usefulness and robustness of the computation.*/
    double cos2A, sin2A;
    if (r == 0.0)
        {
        *pCosA = 1.0;
        *pSinA = 0.0;
        }
    else
        {
        cos2A = rCos2A / r;
        sin2A = rSin2A / r;
        if (cos2A >= 0.0)
            {
            /* Original angle in NE and SE quadrants.  Half angle in same quadrant */
            *pCosA = sqrt (0.5 * ( 1.0 + cos2A ));
            *pSinA = sin2A / (2.0 * (*pCosA));
            }
        else
            {
            if (sin2A > 0.0)
                {
                /* Original angle in NW quadrant. Half angle in NE quadrant */
                *pSinA = sqrt (0.5 * ( 1.0 - cos2A));
                }
            else
                {
                /* Original angle in SW quadrant. Half angle in SE quadrant*/
                /* cosA comes out positive because both sines are negative. */
                *pSinA = - sqrt (0.5 * ( 1.0 - cos2A));
                }
            *pCosA = sin2A / (2.0 * (*pSinA));
            }
        }
    }
#endif
/*-----------------------------------------------------------------*//**
* Compute the sine and cosine of the angle which is half of the
* angle whose sine and cosine are given; all values are complex.
* @param pCosA <= cosine of half angle
* @param pSinA <= sine of half angle
* @param rCos2A => x component of a vector
* @param rSin2A => y component of a vector
*/
Public GEOMDLLIMPEXP void bsiCTrig_halfAngleFunctions

(
DPoint2dP pCosA,     /* <= cosine(a) cosine of angle*/
      DPoint2d  *pSinA,     /* <= sine (a)  sine of angle*/
const DPoint2d  *pCos2A,      /* => cosine(2a) cosine of double angle, possibly scaled by some r*/
const DPoint2d  *pSin2A       /* => sine(2a) sine of double angle, possibly scaled by same r.*/
)
    {
#if defined REAL_PART_ONLY
    double c, s;
    bsiTrig_halfAngleFunctions (&c, &s, pCos2A->x, pSin2A->x);
    pCosA->x = c;
    pCosA->y = 0.0;
    pSinA->x = s;
    pSinA->y = 0.0;
#else
    DPoint2d R;
    DPoint2d cos2A, sin2A;
    DPoint2d temp0, temp1;
    jmdlVector_complexHypotenuse (&R, pCos2A, pSin2A);
    /* If the caller really gave you sine and cosine values, r should be 1.  However,*/
    /* to allow scaled values -- e.g. the x and y components of any vector -- we normalize*/
    /* right here.  This adds an extra sqrt and 2 divides to the whole process, but improves*/
    /* both the usefulness and robustness of the computation.*/
    if (R.x == 0.0 && R.y == 0.0)
        {
        bsiDPoint2d_setComponents (pCosA, 1.0, 0.0);
        bsiDPoint2d_setComponents (pSinA, 0.0, 0.0);
        }
    else
        {
        /* Normalize the inputs */
        bsiComplex_divide (&cos2A, pCos2A, &R);
        bsiComplex_divide (&sin2A, pSin2A, &R);
        if (cos2A.x >= 0.0)
            {
            /* Original angle in NE and SE quadrants.  Half angle in same quadrant */
            bsiDPoint2d_setComponents (&temp0, 0.5 * (1.0 + cos2A.x), 0.5 * cos2A.y);
            bsiComplex_sqrt (pCosA, &temp0);
            bsiComplex_realMultiplyComplex (&temp1, 2.0, pCosA);
            bsiComplex_divide (pSinA, &sin2A, &temp1);
            }
        else
            {
            bsiDPoint2d_setComponents (&temp0, 0.5 * (1.0 - cos2A.x), -0.5 * cos2A.y);
            bsiComplex_sqrt (pSinA, &temp0);

            if (sin2A.x < 0.0)
                bsiComplex_realMultiplyComplex (pSinA, -1.0, pSinA);

            bsiComplex_realMultiplyComplex (&temp1, 2.0, pSinA);
            bsiComplex_divide (pCosA, &sin2A, &temp1);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Dot product of vectors with real and complex parts stored as independent arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiCTrig_dotProduct

(
DPoint2dP pDot, /* <= complex dot product */
const double *pUr,      /* => real part of column U */
const double *pUi,      /* => imaginary part of column U */
const double *pVr,      /* => real part of column V */
const double *pVi,      /* => imaginary part of column V */
int         step,       /* => step between adjacent members of all vectors */
int         n           /* => vector lengths */
)
    {
    int i;
    pDot->x = pDot->y = 0.0;

    for (i = 0; i < n;
                i++,
                pUr += step,
                pUi += step,
                pVr += step,
                pVi += step)
        {
        pDot->x += *pUr * *pVr - *pUi * *pVi;
        pDot->y += *pUr * *pVi + *pUi * *pVr;
        }
    }

/*-----------------------------------------------------------------*//**
* Apply a complex rotation to a pair of complex vectors.
* @param pUr <=> real part of vector U
* @param pUi <=> complex part of vector V
* @param pVr <=> real part of vector V
* @param pVi <=> imaginary part of vector V
* @param step   => step size in vectors
* @param n      => number of components in vectors
* @param pCosTheta => complex cosine of complex rotation angle
* @param pSinTheta => complex sine of complex rotation angle
*/
Public GEOMDLLIMPEXP void bsiCTrig_rotate

(
double      *pUr,
double      *pUi,
double      *pVr,
double      *pVi,
int         step,
int         n,
DPoint2dCP pCosTheta,
DPoint2dCP pSinTheta
)
    {
    int i;
    double ur, ui, vr, vi;

    for (i = 0; i < n;
                i++,
                pUr += step,
                pUi += step,
                pVr += step,
                pVi += step)
        {
        ur =   pCosTheta->x * *pUr - pCosTheta->y * *pUi
             + pSinTheta->x * *pVr - pSinTheta->y * *pVi;

        ui =   pCosTheta->y * *pUr + pCosTheta->x * *pUi
             + pSinTheta->y * *pVr + pSinTheta->x * *pVi;

        vr = - pSinTheta->x * *pUr + pSinTheta->y * *pUi
             + pCosTheta->x * *pVr - pCosTheta->y * *pVi;

        vi = - pSinTheta->y * *pUr - pSinTheta->x * *pUi
             + pCosTheta->y * *pVr + pCosTheta->x * *pVi;

        *pUr = ur;
        *pUi = ui;
        *pVr = vr;
        *pVi = vi;
        }
    }

/*-----------------------------------------------------------------*//**
* Compute and apply a complex jacobi rotation to orthogonalize a pair of complex columns
* @param pUr    <=> real part of column U
* @param pUi    <=> imaginary part of column U
* @param pVr    <=> real part of column V
* @param pVi    <=> imaginary part of column V
* @param pQjr   <=> real part of column of Qj
* @param pQji   <=> real part of column of Qj
* @param pQkr   <=> real part of column of Qk
* @param pQki   <=> real part of column of Qk
* @param step   => step between adjacent members of U and V vectors
* @param stepQ  => step between adjacent members of Q vectors
* @param n
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiStep

(
double      *pUr,       /* <=> real part of column U */
double      *pUi,       /* <=> imaginary part of column U */
double      *pVr,       /* <=> real part of column V */
double      *pVi,       /* <=> imaginary part of column V */
double      *pQjr,      /* <=> real part of column of Qj */
double      *pQji,      /* <=> real part of column of Qj */
double      *pQkr,      /* <=> real part of column of Qk */
double      *pQki,      /* <=> real part of column of Qk */
int         step,       /* => step between adjacent members of U and V vectors */
int         stepQ,      /* => step between adjacent members of Q vectors */
int         n           /* => vector lengths */
)
    {
    DPoint2d UdotU, VdotV, UdotV, cos2Theta, sin2Theta;
    DPoint2d cosTheta, sinTheta;
    bsiCTrig_dotProduct (&UdotU, pUr, pUi, pUr, pUi, step, n);
    bsiCTrig_dotProduct (&VdotV, pVr, pVi, pVr, pVi, step, n);
    bsiCTrig_dotProduct (&UdotV, pUr, pUi, pVr, pVi, step, n);
    cos2Theta.x = UdotU.x - VdotV.x;
    cos2Theta.y = UdotU.y - VdotV.y;
    sin2Theta.x = 2.0 * UdotV.x;
    sin2Theta.y = 2.0 * UdotV.y;
    bsiCTrig_halfAngleFunctions (&cosTheta, &sinTheta, &cos2Theta, &sin2Theta);
    bsiCTrig_rotate (pUr, pUi, pVr, pVi, step, n, &cosTheta, &sinTheta);
    bsiCTrig_rotate (pQjr, pQji, pQkr, pQki, stepQ, n, &cosTheta, &sinTheta);
#ifdef PENCIL_DEBUG
    {
    DPoint2d testDot;
    double mag;
    bsiCTrig_dotProduct (&testDot, pUr, pUi, pVr, pVi, step, n);
    mag = bsiComplex_abs (&testDot);
    if (mag > 1.0e-10)
        {
        mag = mag;
        }
    }
#endif
    }

/*-----------------------------------------------------------------*//**
* Perform a jacobi sweep to successively zero out an entire
*
* @param pAr            <=> real part of matrix
* @param pAi            <=> imaginary part of matrix
* @param pQr            <=> real part of eigenvectors
* @param pQi            <=> imaginary part of eigenvectors
* @param rowStepA       => row step for A matrices
* @param colStepA       => column step for A matrices
* @param rowStepQ       => row step for Q matrices
* @param colStepQ       => column step for Q matrices
* @param numRow         => number of rows in A
* @param numCol         => number of columns in A (hence rows, columns in Q)
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiSweep

(
double  *pAr,       /* <=> real part of matrix */
double  *pAi,       /* <=> imaginary part of matrix */
double  *pQr,       /* <=> real part of eigenvectors */
double  *pQi,       /* <=> imaginary part of eigenvectors */
int     rowStepA,   /* => row step for A matrices */
int     colStepA,   /* => column step for A matrices */
int     rowStepQ,   /* => row step for Q matrices */
int     colStepQ,   /* => column step for Q matrices */
int     numRow,     /* => number of rows in A */
int     numCol      /* => number of columns in A (hence rows, columns in Q) */
)
    {
    int j, k;
    int numColMinus1 = numCol - 1;
    double *pUr, *pUi, *pVr, *pVi;
    double *pQjr, *pQji, *pQkr, *pQki;
    for (j = 0; j < numColMinus1; j++)
        {
        pUr = pAr + j * colStepA;
        pUi = pAi + j * colStepA;
        pQjr = pQr + j * colStepQ;
        pQji = pQi + j * colStepQ;
        for (k = j + 1; k < numCol; k++)
            {
            pVr = pAr + k * colStepA;
            pVi = pAi + k * colStepA;
            pQkr = pQr + k * colStepQ;
            pQki = pQi + k * colStepQ;
            bsiCTrig_jacobiStep
                        (
                        pUr,  pUi,  pVr,  pVi,
                        pQjr, pQji, pQkr, pQki,
                        rowStepA, rowStepQ, numRow
                        );
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Use one-sided complex orthogonal transformations to
* orthogonalize a complex symmetric matrix given in factored form
* @param pAr <=> real part of symmetric factor
* @param pAi <=> imaginary part of factor
* @param pQr <=> real part of orthogonal basis
* @param pQi <=> imaginary part of orthogonal basis
* @param rowStepA => row step in A
* @param colStepA => column step in A
* @param rowStepQ => row step in Q
* @param numRow   => number of rows in A
* @param numCol   => number of columns in A (hence rows in Q)
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiColumnOrthogonalization

(
double  *pAr,       /* <=> real part of matrix */
double  *pAi,       /* <=> imaginary part of matrix */
double  *pQr,       /* <=> real part of eigenvectors */
double  *pQi,       /* <=> imaginary part of eigenvectors */
int     rowStepA,   /* => row step for A matrices */
int     colStepA,   /* => column step for A matrices */
int     rowStepQ,   /* => row step for Q matrices */
int     colStepQ,   /* => column step for Q matrices */
int     numRow,     /* => number of rows in A */
int     numCol      /* => number of columns in A (hence rows, columns in Q) */
)
    {
    int maxSweep = 12;
    int i;
    int numQDouble = numCol * numCol;
    memset (pQr, 0, sizeof(double) * numQDouble);
    memset (pQi, 0, sizeof(double) * numQDouble);
    for (i = 0; i < numCol; i++)
        {
        pQr[i * (rowStepQ + colStepQ)] = 1.0;
        }

    for (i = 0; i < maxSweep; i++)
        {
        bsiCTrig_jacobiSweep
                        (
                        pAr, pAi,
                        pQr, pQi,
                        rowStepA, colStepA,
                        rowStepQ, colStepQ,
                        numRow, numCol
                        );
        }
    }


Public GEOMDLLIMPEXP StatusInt bsiDMatrix4d_searchSingularPencil

(
DPoint4dP pSigma2,        /* <= array of 0 to 4 characteristics for singular surfaces in the pencil */
DMatrix4dP pB2,      /* <= array of 0 to 4 coordinate frames for singular surfaces in the pencil */
DMatrix4dP pB2inv,           /* <= array of 0 to 4 coordinate frame inverses for singular surfaces in the pencil */
int         *pNumSurface,    /* <= number of singular surfaces */
DPoint4dCP pSigma0,
DMatrix4dCP pB0,
DMatrix4dCP pB0inv,
DPoint4dCP pSigma1,
DMatrix4dCP pB1,
DMatrix4dCP pB1inv
)
    {
    int partnerIndex[4];
    int rootType[4];
    DPoint2d lambda[4];
    DPoint2d gamma[4];
    int i, j, pivot;
    double zeroTol;
    DMatrix4d B2, B2inv;
    DMatrix4d Dr, Di, B, Qr, Qi, Er, Ei, Jr, Ji, KQr, KQi;
    DMatrix4d A0, A1, A2, A20;
    DMatrix4d work1;

    *pNumSurface = 0;

    bsiDMatrix4d_multiplyAtSigmaB (&A0, pB0inv, pSigma0, pB0inv);
    bsiDMatrix4d_multiplyAtSigmaB (&A1, pB1inv, pSigma1, pB1inv);

#ifdef PENCIL_DEBUG
    printRealMatrix (pB0, "B0");
    printRealMatrix (pB0inv, "B0inv");
    printRealMatrix (&A0, "A0");

    printRealMatrix (pB1, "B1");
    printRealMatrix (pB1inv, "B1inv");
    printRealMatrix (&A1, "A1");
#endif

    bsiDMatrix4d_multiply (&B, pB0inv, pB1);
    expandDMatrix (&Dr, &Di, pSigma0, &B, pSigma1);

    bsiCTrig_jacobiColumnOrthogonalization (
                                &Dr.coff[0][0],
                                &Di.coff[0][0],
                                &Qr.coff[0][0],
                                &Qi.coff[0][0],
                                4, 1, 4, 1, 4, 4);
    jmdlCHMatrix_multiplyAtB (&Er, &Ei, &Dr, &Di, &Dr, &Di);
#ifdef PENCIL_DEBUG
    printComplexMatrix (&Dr, &Di, "Normalized D");
    printComplexMatrix (&Er, &Ei, "DtD should be diagonal");
#endif
    jmdlCHMatrix_eigenpairsFromOrthogonalFactors
                                (
                                lambda,
                                gamma,
                                partnerIndex,
                                rootType,
                                &zeroTol,
                                &Dr,
                                &Di
                                );
#ifdef PENCIL_DEBUG
    printf(" type next lambda     root\n");
    for (i = 0; i < 4; i++)
        {
        printf(" %4d %4d (%10lf %10lf) (%10lf, %10lf)\n",
                        rootType[i], partnerIndex[i],
                        lambda[i].x, lambda[i].y,
                        gamma[i].x,  gamma[i].y
                        );
        }
#endif
    jmdlCHMatrix_multiplyAtB (&Jr, &Ji, &Qr, &Qi, &Qr, &Qi);
#ifdef PENCIL_DEBUG
    printComplexMatrix (&Qr, &Qi, "Q");
    printComplexMatrix (&Jr, &Ji, "QTQ should be identity");
#endif
    jmdlCHMatrix_multiplyKB (&KQr, &KQi, pSigma1, &Qr, &Qi);
#ifdef PENCIL_DEBUG
    printComplexMatrix (&KQr, &KQi, "KQ");
#endif

    for (pivot = 0; pivot < 4; pivot++)
        {
        DPoint2d gamma[4];
        DMatrix4d H2;
        DMatrix4d H2T;
        double error;
        DPoint4d sigma2;
        double s;
        int status;
#ifdef PENCIL_DEBUG
        printf(" shift eigenvalue %d is %lf %lf\n",
                                pivot,
                                lambda[pivot].x,
                                lambda[pivot].y);
#endif

        if (rootType[pivot] == 2 || rootType[pivot] == -2)
            {
#ifdef PENCIL_DEBUG
            printf (" \n\n ******** SKIPPING SHIFT FOR COMPLEX EIGENVALUE ******** \n\n");
#endif
            continue;
            }

        status = jmdlDMatrix4d_shiftedBasis
                                (
                                &H2,
                                (double*)&sigma2,
                                &KQr,
                                &KQi,
                                lambda,
                                gamma,
                                partnerIndex,
                                rootType,
                                zeroTol,
                                pivot
                                );

        if (SUCCESS == status)
            {
            bsiDMatrix4d_transpose (&H2T, &H2);
            bsiDMatrix4d_multiply (&B2inv, &H2T, pB1inv);

#ifdef PENCIL_DEBUG
            printRealMatrix (&H2, "H2");
            printRealMatrix (&B2inv, "B2inv");
#endif

            if (!bsiDMatrix4d_invertQR(&B2, &B2inv))
                {
#ifdef PENCIL_DEBUG
                printf(" ******* ERROR -- B2 not invertible ******\n");
#endif
                }
            else
                {
                int i = *pNumSurface;
#ifdef PENCIL_DEBUG
            printRealMatrix (&B2, "B2 from Symmetric reduction ");
#endif
                pB2[i] = B2;
                pB2inv[i] = B2inv;
                pSigma2[i] = sigma2;
                *pNumSurface += 1;
                }

#ifdef PENCIL_DEBUG
            printSigma (&sigma2, "sigma2");
#endif

            bsiDMatrix4d_multiplyASigmaBt (&work1, &H2, &sigma2, &H2);
            bsiDMatrix4d_multiplyAtSigmaB (&A2, &B2inv, &sigma2, &B2inv);
            s = lambda[pivot].x;

#ifdef PENCIL_DEBUG
            printRealMatrix (&work1, "H2 sigma2 H2T");
            printRealMatrix (&A2, "A2 from B2inv");

            printf(" shift eigenvalue %lf %lf\n",
                                    lambda[pivot].x,
                                    lambda[pivot].y);
#endif
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    {
                    A20.coff[i][j] = A0.coff[i][j] - s * A1.coff[i][j];
                    }

            error = 0.0;
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    error += fabs (A20.coff[i][j] - A2.coff[i][j]);
#ifdef PENCIL_DEBUG
            printRealMatrix (&A20, "A2 from explicit formula (A0 - s*A1)");
            printf("\t\t\t Error = %10.2le\n",error);
#endif
            }
        }
    return SUCCESS;
    }

#ifdef PENCIL_TESTPROGRAM
#define STACK_LIMIT 10
static DPoint2d stack[STACK_LIMIT];
static int stackDepth = 0;
void cs_error (const char *pDescr)
    {
    printf(" complex stack error: %s\n");
    exit (0);
    }

DPoint2d *cs_top ()
    {
    return stack + stackDepth - 1;
    }

void cs_push (DPoint2d *pZ)
    {
    if (stackDepth >= STACK_LIMIT)
        cs_error(" Stack overflow");

    stack[stackDepth] = *pZ;
    stackDepth++;
    }

void cs_pushProduct (DPoint2d *pZ0, DPoint2d *pZ1)
    {
    DPoint2d product;
    bsiComplex_multiply (&product, pZ0, pZ1);
    cs_push (&product);
    }

void cs_pop (DPoint2d *pZ)
    {
    if (stackDepth <= 0)
        cs_error(" Stack underflow");
    *pZ = stack[stackDepth-1];
    stackDepth--;
    }

void cs_multiply (DPoint2d *pZ)
    {
    DPoint2d *pTop = cs_top ();
    bsiComplex_multiply (pTop, pTop, pZ);
    }

void cs_add (DPoint2d *pZ)
    {
    DPoint2d *pTop = cs_top ();
    bsiComplex_add (pTop, pTop, pZ);
    }

void cs_subtract (DPoint2d *pZ)
    {
    DPoint2d *pTop = cs_top ();
    bsiComplex_subtract (pTop, pTop, pZ);
    }

void cs_addTop ()
    {
    DPoint2d arg;
    cs_pop (&arg);
    cs_add (&arg);
    }

void cs_subtractTop ()
    {
    DPoint2d arg;
    cs_pop (&arg);
    cs_subtract (&arg);
    }

void cs_show (const char *pDescr)
    {
    int i;
    printf(" Stack contents (%ls)\n", pDescr);
    for (i = stackDepth - 1; i >= 0; i--)
        {
        printf(" %d: %lf + %lf i\n", i, stack[i].x, stack[i].y);
        }
    }
void putComplex (DPoint2d *pZ, const char *pString)
    {
    printf("%12s = %lf + %lf i\n", pString, pZ->x, pZ->y);
    }
void rootTest ()
    {
    DPoint2d C2, S2, C, S;
    DPoint2d R2, R;
    DPoint2d C2a, S2a;
    DPoint2d C2b, S2b;
    DPoint2d Cratio, Sratio;

    for (;;)
        {
        if (4 != scanf ("%lf %lf %lf %lf", &C2.x, &C2.y, &S2.x, &S2.y))
            return;
        bsiCTrig_halfAngleFunctions (&C, &S, &C2, &S2);

        jmdlVector_complexHypotenuse (&R2, &C2, &S2);
        jmdlVector_complexHypotenuse (&R, &C, &S);
        bsiComplex_divide (&C2b, &C2, &R2);
        bsiComplex_divide (&S2b, &S2, &R2);
        putComplex (&C2b, "C2/r");
        putComplex (&S2b, "S2/r");
        putComplex (&C, "C");
        putComplex (&S, "S");
        cs_pushProduct (&C, &C);
        cs_pushProduct (&S, &S);
        cs_subtractTop ();
        cs_pop (&C2a);
        cs_pushProduct (&C, &S);
        cs_pop (&S2a);
        bsiComplex_realMultiplyComplex (&S2a, 2.0, &S2a);
        putComplex (&C2a, "C2*");
        putComplex (&S2a, "S2*");
        bsiComplex_divide (&Cratio, &C2b, &C2a);
        bsiComplex_divide (&Sratio, &S2b, &S2a);
        putComplex (&Cratio, "C/C*");
        putComplex (&Sratio, "S/S*");
        }
    }

double randomDouble (double a, double b)
    {
    double rand0 = (double)rand () / RAND_MAX;
    double rand1 = (double)rand () / RAND_MAX;
    double fraction;
    fraction = rand0 + rand1 / RAND_MAX;
    return a + fraction * (b - a);
    }

void scanSigma

(
DPoint4dP pSigma
)
    {
    if (4 != scanf ("%lf %lf %lf %lf", &pSigma->x, &pSigma->y, &pSigma->z, &pSigma->w))
        {
        exit(10);
        }
    }

void scanHMatrix

(
DMatrix4dP pA
)
    {
    int i, j;
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            if (1 != scanf ("%lf", &pA->coff[i][j]))
                exit (10);
            }
        }
    }


void fullReductionInput

(
)
    {
    DMatrix4d B0, B1, B0inv, B1inv;
    DMatrix4d A0, A1;
    DPoint4d sigma0 = { 1.0, 1.0, 1.0, -1.0};
    DPoint4d sigma1 = { 1.0, 1.0, 1.0, -1.0};
    printf(" Enter B0:\n");
    scanHMatrix (&B0);

#ifdef enterSigma
    printf(" Enter sigma0:\n");
    scanSigma (&sigma0);
#endif

    printf(" Enter B1\n");
    scanHMatrix (&B1);

#ifdef enterSigma
    printf(" Enter sigma1:\n");
    scanSigma (&sigma1);
#endif

    if (   bsiDMatrix4d_invertQR(&B0inv, &B0)
        && bsiDMatrix4d_invertQR(&B1inv, &B1))
        {
        printRealMatrix (&B0, "B0");
        printRealMatrix (&B0inv, "B0inv");

        printRealMatrix (&B1, "B1");
        printRealMatrix (&B1inv, "B1inv");

        bsiDMatrix4d_symmetricProduct (&A0, &sigma0, &B0inv);
        printRealMatrix (&A0, "A0");

        bsiDMatrix4d_symmetricProduct (&A1, &sigma1, &B1inv);
        printRealMatrix (&A1, "A1");

        fullReduction (&sigma0, &B0, &sigma1, &B1);

        }
    }

void main ()
    {
    DMatrix4d B;
    DMatrix4d Dr, Di;
    DMatrix4d Er, Ei;
    DMatrix4d Qr, Qi;
    DMatrix4d KQr, KQi;
    DMatrix4d Jr, Ji;
    int i, j;
    DPoint4d sigma0, sigma1;
    int opcode;

    sigma0.x = sigma0.y = sigma0.z = 1.0;
    sigma0.w = -1.0;
    sigma1 = sigma0;

    memset (&B, 0, sizeof (B));
    for (i = 0; i < 4; i++)
        B.coff[i][i] = 1.0;


    for (;;)
        {
        printf(" Select Test: (1) half angles    (2) enter B (3) Random B\n");
        printf("              (4) sigma0         (5) sigma1  (6) solve \n");
        printf("              (8) B0,B1 entry    (9) non-planar test  (+-10) planar test \n");
        if (1 != scanf ("%d", &opcode))
                break;

        expandDMatrix (&Dr, &Di, &sigma0, &B, &sigma1);
        if (opcode == 0)
            {
            printRealMatrix (&B, "B");
            printSigma (&sigma0, "sigma0");
            printSigma (&sigma1, "sigma1");
            printComplexMatrix (&Dr, &Di, "D");
            }

        if (opcode == 1)
            {
            rootTest ();
            }
        else if (opcode == 2)
            {
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    if ( 1 != fscanf(stdin, "%lf", &B.coff[i][j]))
                        {
                        printf( " **** input failure -- need 16 entries of B ****\n");
                        exit (1);
                        }
            }
        else if (opcode == 3)
            {

            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    B.coff[i][j] = randomDouble (-1.0, 1.0);
            }
        else if (opcode == 4)
            {
            scanSigma (&sigma0);
            }
        else if (opcode == 5)
            {
            scanSigma (&sigma1);
            }
        else if (opcode == 6 || (opcode >= 60 && opcode <= 63))
            {
            int partnerIndex[4];
            int rootType[4];
            DPoint2d lambda[4];
            DPoint2d gamma[4];
            double zeroTol;

            bsiCTrig_jacobiColumnOrthogonalization (
                                        &Dr.coff[0][0],
                                        &Di.coff[0][0],
                                        &Qr.coff[0][0],
                                        &Qi.coff[0][0],
                                        4, 1, 4, 1, 4, 4);
            jmdlCHMatrix_multiplyAtB (&Er, &Ei, &Dr, &Di, &Dr, &Di);
            printComplexMatrix (&Dr, &Di, "Normalized D");
            jmdlCHMatrix_eigenpairsFromOrthogonalFactors
                                        (
                                        lambda,
                                        gamma,
                                        partnerIndex,
                                        rootType,
                                        &zeroTol,
                                        &Dr,
                                        &Di
                                        );

            printComplexMatrix (&Er, &Ei, "DtD");
            printf(" type next lambda     root\n");
            for (i = 0; i < 4; i++)
                {
                printf(" %4d %4d (%10lf %10lf) (%10lf, %10lf)\n",
                                rootType[i], partnerIndex[i],
                                lambda[i].x, lambda[i].y,
                                gamma[i].x,  gamma[i].y
                                );
                }

            jmdlCHMatrix_multiplyAtB (&Jr, &Ji, &Qr, &Qi, &Qr, &Qi);
            printComplexMatrix (&Qr, &Qi, "Q");
            jmdlCHMatrix_multiplyKB (&KQr, &KQi, &sigma1, &Qr, &Qi);
            printComplexMatrix (&KQr, &KQi, "KQ");
            /* printComplexMatrix (&Jr, &Ji, "QTQ");*/
            if (opcode >= 60 && opcode <= 63)
                {
                int pivot = opcode - 60;
                DPoint2d gamma[4];
                DMatrix4d H2, A2, A20;
                double error;
                DPoint4d sigma2;
                double s;
                double *pSigma0 = (double *)&sigma0;
                int status = jmdlDMatrix4d_shiftedBasis
                                        (
                                        &H2,
                                        (double*)&sigma2,
                                        &KQr,
                                        &KQi,
                                        lambda,
                                        gamma,
                                        partnerIndex,
                                        rootType,
                                        zeroTol,
                                        pivot
                                        );
                bsiDMatrix4d_multiplyASigmaBt (&A2, &H2, &sigma2, &H2);
                printRealMatrix (&H2, "H2");
                printSigma (&sigma2, "sigma2");
                printRealMatrix (&A2, "A2");
                bsiDMatrix4d_multiplyAtSigmaB (&A20, &B, &sigma0, &B);
                s = lambda[pivot].x;
                printf(" shift eigenvalue %lf %lf\n",
                                        lambda[pivot].x,
                                        lambda[pivot].y);
                for (i = 0; i < 4; i++)
                    {
                    A20.coff[i][i] -= s * pSigma0[i];
                    }

                error = 0.0;
                for (i = 0; i < 4; i++)
                    for (j = 0; j < 4; j++)
                        error += fabs (A20.coff[i][i] - A2.coff[i][i]);
                printRealMatrix (&A20, "A20");
                printf("\t\t\t Error = %10.2le\n",error);
                }
            }
#ifdef SVD
        else if (opcode == 7)
            {
            DMatrix4d svdB, U, VT;
            svdB = B;
            printRealMatrix (&svdB, "B");
            jmdlDMatrix4d_svd (&svdB, &U, &VT);
            printRealMatrix (&svdB, "B");
            printRealMatrix (&U, "U");
            printRealMatrix (&VT, "VT");
            }
#endif
        else if (opcode == 8)
            {
            fullReductionInput ();
            }
        else if (abs(opcode) == 9)
            {
            static DPoint4d sigma0 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B0 = {{
                {1.0, 0.0, 0.0, 0.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 2.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};

            static DPoint4d sigma1 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B1 = {{
                {2.0, 0.0, 0.0, 2.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};
            if (opcode > 0)
                fullReduction (&sigma0, &B0, &sigma1, &B1);
            else
                fullReduction (&sigma1, &B1, &sigma0, &B0);
            }
        else if (abs(opcode) == 10)
            {
            static DPoint4d sigma0 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B0 = {{
                {1.0, 0.0, 0.0, 1.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};

            static DPoint4d sigma1 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B1 = {{
                {1.0, 0.0, 0.0, 0.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};
            if (opcode > 0)
                fullReduction (&sigma0, &B0, &sigma1, &B1);
            else
                fullReduction (&sigma1, &B1, &sigma0, &B0);

            }
        else if (opcode == 11)
            {
            static DPoint4d sigma0 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B0 = {{
                {1.0, 0.0, 0.0, 0.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};

            static DPoint4d sigma1 = {1.0, 1.0, 1.0, -1.0};
            static DMatrix4d B1 = {{
                {1.0, 0.0, 0.0, 1.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
                }};
            fullReduction (&sigma0, &B0, &sigma1, &B1);
            }
        }
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
