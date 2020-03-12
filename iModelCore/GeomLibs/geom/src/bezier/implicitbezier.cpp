/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* Compute the polynomial coefficients determinant of a matrix in which each entry
* is itself a polynomial with given bezier coefficients.
* @param pD <= determinant polynomial
* @param pRatio <= largest det coefficient divided by largest coefficient of summed terms.
* @param maxDOrder => maximum order of output polynomial
* @param pM => matrix m0 * m0 polynomials of order n.  Coefficients for each polynomial are packed.
* @param i0 => first row selector
* @param i1 => second row selector
* @param j0 => first column selector
* @param j1 => second column selector
* @param m0 => number of rows, columns in matrix-of-polynomials
* @param n  => degree of polynomials in the matrix.
* @return true if determinant computed.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlBezier_determinant22
(
double *pD,
double *pRatio,
int     maxDOrder,
double *pM,
int     i0,
int     i1,
int     j0,
int     j1,
int     m0,
int     n
)
    {
    int productOrder = 2 * n - 1;
    int iStep = m0 * n;
    double *p00, *p01, *p10, *p11;
    double term0[2 * MAX_BEZIER_ORDER];
    double term1[2 * MAX_BEZIER_ORDER];
    double max0, max1, maxD, max01;
    if (productOrder > maxDOrder)
        return false;

    p00 = pM + i0 * iStep + j0 * n;
    p01 = pM + i0 * iStep + j1 * n;
    p10 = pM + i1 * iStep + j0 * n;
    p11 = pM + i1 * iStep + j1 * n;

    bsiBezier_univariateProduct (term0, 0, 1, p00, n, 0, 1, p11, n, 0, 1);

    bsiBezier_univariateProduct (term1, 0, 1, p01, n, 0, 1, p10, n, 0, 1);
    bsiBezier_subtractPoles (pD, term0, term1, productOrder, 1);
    max0 = DoubleOps::MaxAbs (term0, productOrder);
    max1 = DoubleOps::MaxAbs (term1, productOrder);
    maxD = DoubleOps::MaxAbs (pD, productOrder);
    max01 = max0 > max1 ? max0 : max1;
    DoubleOps::SafeDivide (*pRatio, maxD, max01, 0.0);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the polynomial coefficients determinant of a matrix in which each entry
* is itself a polynomial with given bezier coefficients.
* @param pD <= determinant polynomial
* @param pRatio <= largest det coefficient divided by largest coefficient of summed terms.
* @param maxDOrder => maximum order of output polynomial
* @param pM => matrix m0 * m0 polynomials of order n.  Coefficients for each polynomial are packed.
* @param i0 => first row selector
* @param i1 => second row selector
* @param i2 => third row selector
* @param j0 => first column selector
* @param j1 => second column selector
* @param j2 => third column selector
* @param m0 => number of rows, columns in matrix-of-polynomials
* @param n  => degree of polynomials in the matrix.
* @return true if determinant computed.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlBezier_determinant33
(
double *pD,
double *pRatio,
int     maxDOrder,
double *pM,
int     i0,
int     i1,
int     i2,
int     j0,
int     j1,
int     j2,
int     m0,
int     n
)
    {
    int minorOrder   = 2 * n - 1;
    int productOrder = 3 * n - 2;
    int iStep = m0 * n;
    double *p00, *p01, *p02;
    double minor0[MAX_BEZIER_ORDER];
    double minor1[MAX_BEZIER_ORDER];
    double minor2[MAX_BEZIER_ORDER];
    double term[MAX_BEZIER_ORDER];
    double termMaxAbs[3];
    double dMax, tMax;
    double ratio2;
    if (productOrder > maxDOrder || productOrder > MAX_BEZIER_ORDER)
        return false;

    p00 = pM + i0 * iStep + j0 * n;
    p01 = pM + i1 * iStep + j0 * n;
    p02 = pM + i2 * iStep + j0 * n;

    jmdlBezier_determinant22 (minor0, &ratio2, maxDOrder, pM, i1, i2, j1, j2, m0, n);
    jmdlBezier_determinant22 (minor1, &ratio2, maxDOrder, pM, i2, i0, j1, j2, m0, n);
    jmdlBezier_determinant22 (minor2, &ratio2, maxDOrder, pM, i0, i1, j1, j2, m0, n);

    bsiBezier_univariateProduct    (pD,   0, 1, p00, n, 0, 1, minor0, minorOrder, 0, 1);
    termMaxAbs[0] = DoubleOps::MaxAbs (pD, productOrder);
    bsiBezier_univariateProduct    (term, 0, 1, p01, n, 0, 1, minor1, minorOrder, 0, 1);
    bsiBezier_addPolesInPlace      (pD, term, productOrder, 1);
    termMaxAbs[1] = DoubleOps::MaxAbs (term, productOrder);

    bsiBezier_univariateProduct    (term, 0, 1, p02, n, 0, 1, minor2, minorOrder, 0, 1);
    bsiBezier_addPolesInPlace      (pD, term, productOrder, 1);
    termMaxAbs[2] = DoubleOps::MaxAbs (term, productOrder);
    tMax = DoubleOps::MaxAbs (termMaxAbs, 3);
    dMax = DoubleOps::MaxAbs (pD, productOrder);
    DoubleOps::SafeDivide (*pRatio, dMax, tMax, 0.0);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @return true if determinant was expanded.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlBezier_expandDeterminant
(
double *pD,
int     *pOrderD,
double  *pRatioD,
int     maxOrderD,
double *pM,
int     *pI,
int     *pJ,
int     m0,
int     m,
int     n
)
    {
    int i, j, k;
    int ij;
    double dMax, dMaxCurr;
    *pOrderD = m * (n - 1) + 1;

    *pRatioD = 1.0;
    if (m == 1)
        {
        i = pI[0];
        j = pJ[0];
        ij = n * (i * m0 + j);
        for (k = 0; k < n; k++)
            pD[k] = pM[ij + k];
        return true;
        }
    else if (m == 2)
        {
        return jmdlBezier_determinant22 (pD, pRatioD, maxOrderD, pM,
                            pI[0], pI[1],
                            pJ[0], pJ[1],
                            m0, n);
        }
    else if (m == 3)
        {
        return jmdlBezier_determinant33 (pD, pRatioD, maxOrderD, pM,
                            pI[0], pI[1], pI[2],
                            pJ[0], pJ[1], pJ[2],
                            m0, n);
        }
    else
        {
        double minorCoffs [MAX_BEZIER_ORDER];
        int minorIndices[MAX_BEZIER_ORDER]; /* Well, this could be a lot smaller. */
        int minorOrder;
        int productOrder;
        double *pPivotCoffs;

        productOrder = m * n - m + 1;
        memset (pD, 0, productOrder * sizeof (double));
        if (productOrder > MAX_BEZIER_ORDER)
            return false;
        dMax = 0.0;
        /* Expand around column 0 */
        for (i = 0; i < m; i++)
            {
            double ratio;
            /* Explicitly copy subsets of indices from the pI index set into
                    minorIndices.   Take pJ[1], pJ[2]... implicitly by
                    jumping over pJ[0] on the recursive call. */
            for (j = k = 0; j < m; j++)
                {
                if (j != i)
                    minorIndices[k++] = pI[j];
                }
            if (!jmdlBezier_expandDeterminant
                            (minorCoffs, &minorOrder, &ratio, MAX_BEZIER_ORDER,
                                pM, minorIndices, pJ + 1, m0, m - 1, n))
                return false;
            pPivotCoffs = pM + pI[i] * m0 * n + pJ[0] * n;
            bsiBezier_accumulateUnivariateProduct
                    (
                    pD, 0, 1,
                    (i & 0x01) ? -1.0 : 1.0,
                    pPivotCoffs, n, 0, 1,
                    minorCoffs, minorOrder, 0, 1);
            dMaxCurr = DoubleOps::MaxAbs (pD, *pOrderD);
            if (dMaxCurr > dMax)
                dMax = dMaxCurr;
            }
        dMaxCurr = DoubleOps::MaxAbs (pD, *pOrderD);
        DoubleOps::SafeDivide (*pRatioD, dMaxCurr, dMax, 0.0);
#ifdef debugRecursiveDeterminant
        /* To enable this, set the define and disable branches into 2,3 cases above. */
        if (m == 3 || m == 2)
            {
            double d;
            if (m == 3)
                jmdlBezier_determinant33 (minorCoffs, maxOrderD, pM,
                            pI[0], pI[1], pI[2],
                            pJ[0], pJ[1], pJ[2],
                            m0, n);
            else if (m == 2)
                jmdlBezier_determinant22 (minorCoffs, maxOrderD, pM,
                            pI[0], pI[1],
                            pJ[0], pJ[1],
                            m0, n);

            d = 0.0;
            for (i = 0; i < productOrder; i++)
                {
                d += fabs (minorCoffs[i] - pD[i]);
                }
            printf(" %dx%d determinant coffs, order %d\n", m, m, productOrder);
            if (d > 1.0e-10)
                {
                for (i = 0; i < m; i++)
                    printf("pI.%d=%d  ", i, pI[i]);
                printf("\n");
                for (i = 0; i < m; i++)
                    printf("pJ.%d=%d  ", i, pJ[i]);
                printf("\n");
                for (i = 0; i < productOrder; i++)
                    printf (" %lf %lf\n", pD[i], minorCoffs[i]);
                }
            }
#endif
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Insert coefficients of each (H dot pPointB) to a specified position and its
* symmetric partner.
* matrix.
* @param
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void insertLineCoffs
(
      double        *pCoffs,
      int           i,
      int           j,
      int           n,
const DPoint4d      *pH,
const DPoint4d      *pPointB,
      int           orderB
)
    {
    double *pIJ = pCoffs + orderB * (j + i * n);
    double *pJI = pCoffs + orderB * (i + j * n);
    int k;
    for (k = 0; k < orderB; k++)
        {
        pIJ[k] = pJI[k] = pH->DotProduct (pPointB[k]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute bezier coefficients for the determinant of
*   (1-u) * ascale * A + u * bscale * B
* where A and B are 3x3 matrices.
* @param pCoffs <= coefficients of polynomial determinant.
* @param pOrder <= order of polynomial.
* @param maxOrder <= max order permitted.
* @param pA => first matrix.
* @param pB => second matrix.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezier_expandDMatrix3dPencilDeterminant
(
double  *pCoffs,
int     *pOrder,
int     maxOrder,
      double      ascale,
const DMatrix3d   *pA,
      double      bscale,
const DMatrix3d   *pB
)
    {
#define DIM33WORKSPACE 18
    double coffC[DIM33WORKSPACE];
    static int indices[3] = {0, 1, 2};
    const double *pAA = (double const*)pA, *pBB = (double const*)pB;
    double ratio;
    int i = 0;
    for (i = 0; i < 9; i++)
        {
        coffC[2 * i] = ascale * pAA[i];
        coffC[2 * i + 1] = bscale * pBB[i];
        }

    return jmdlBezier_expandDeterminant (pCoffs, pOrder, &ratio, DIM33WORKSPACE, coffC, indices, indices, 3, 3, 2);
    }

/*---------------------------------------------------------------------------------**//**
* Compute bezier coefficients for the determinant of
*   (1-u) * ascale * A + u * bscale * B
* where A and B are 4x4 matrices.
* @param pCoffs <= coefficients of polynomial determinant.
* @param pOrder <= order of polynomial.
* @param maxOrder <= max order permitted.
* @param pA => first matrix.
* @param pB => second matrix.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezier_expandDMatrix4dPencilDeterminant
(
double  *pCoffs,
int     *pOrder,
int     maxOrder,
      double      ascale,
const DMatrix4d   *pA,
      double      bscale,
const DMatrix4d   *pB
)
    {
#define DIM44WORKSPACE 32
    double coffC[DIM44WORKSPACE];
    static int indices[4] = {0, 1, 2, 3};
    const double *pAA = (double const*)pA, *pBB = (double const*)pB;
    double ratio;
    int i = 0;
    for (i = 0; i < 16; i++)
        {
        coffC[2 * i] = pAA[i];
        coffC[2 * i + 1] = pBB[i];
        }

    return jmdlBezier_expandDeterminant (pCoffs, pOrder, &ratio, DIM44WORKSPACE, coffC, indices, indices, 3, 3, 2);
    }

/*---------------------------------------------------------------------------------**//**
*
* @param
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlBezierDPoint4d_intersectXYThisOrder_go
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      double    *pConditionRatio,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB,
      double    conditionLimit
)
    {
#define MAX_PRODUCT_MATRIX_ENTRY 1000
    /* Array of orderA*orderA blocks of orderB entries.
       Each block of orderB entries contains the bezier coeffcients
       for one entry of the resultant matrix. */
    double Mijk[MAX_PRODUCT_MATRIX_ENTRY];
    double *pMijk;
    DPoint4d lineCoffs;     /* Has z==0 */
    DPoint4d tempCoffs;
    double detCoff[MAX_BEZIER_ORDER];
    double detRoot[MAX_BEZIER_ORDER];
    DPoint4d intersectB[MAX_BEZIER_ORDER];
    int indexI[MAX_BEZIER_CURVE_ORDER];
    int indexJ[MAX_BEZIER_CURVE_ORDER];
    int numI = orderA - 1;
    int numJ = orderA - 1;
    int numK = orderB;
    int numIJ = numI * numJ;
    int numIJK = numIJ * numK;
    int numRoot;
    int i, j, k, m;
    int detOrder;
    bool    boolstat = false;

    double conditionRatio = 1.0;

    *pNumIntersection = 0;
    if (pConditionRatio)
        *pConditionRatio = 0.0;

    if (numIJK > MAX_PRODUCT_MATRIX_ENTRY)
        return false;

    if (orderA * orderB > MAX_BEZIER_ORDER)
        return false;

    if ((orderA - 1) * (orderB - 1) > maxIntersection)
        return false;

    if (orderA <= 1 || orderB <= 1)
        return false;


    pMijk = Mijk;
    memset (pMijk, 0, numIJK * sizeof(double));
    /* Sederberg's method of lines, CAGD 1994 225-228.
       A lifetime in 4 pages. */


    if (orderA == 2)
        {
        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[1], 0, 1, orderA);
        insertLineCoffs (Mijk, 0, 0, numI, &lineCoffs, pB, orderB);
        }
    else if (orderA == 3)
        {
        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[1], 0, 1, orderA);
        insertLineCoffs (Mijk, 0, 0, numI, &lineCoffs, pB, orderB);

        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[2], 0, 2, orderA);
        insertLineCoffs (Mijk, 0, 1, numI, &lineCoffs, pB, orderB);

        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[1], &pA[2], 1, 2, orderA);
        insertLineCoffs (Mijk, 1, 1, numI, &lineCoffs, pB, orderB);
        }
    else if (orderA == 4)
        {
        /* Row 0: l01 l02 l03 */
        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[1], 0, 1, orderA);
        insertLineCoffs (Mijk, 0, 0, numI, &lineCoffs, pB, orderB);

        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[2], 0, 2, orderA);
        insertLineCoffs (Mijk, 0, 1, numI, &lineCoffs, pB, orderB);

        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[3], 0, 3, orderA);
        insertLineCoffs (Mijk, 0, 2, numI, &lineCoffs, pB, orderB);

        /* Row 1: xxx l03 + l12 l13 */
        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[0], &pA[3], 0, 3, orderA);
        bsiBezierDPoint4d_lineProductXYW (&tempCoffs, &pA[1], &pA[2], 1, 2, orderA);

        lineCoffs.SumOf (lineCoffs, tempCoffs);

        insertLineCoffs (Mijk, 1, 1, numI, &lineCoffs, pB, orderB);

        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[1], &pA[3], 1, 3, orderA);
        insertLineCoffs (Mijk, 1, 2, numI, &lineCoffs, pB, orderB);

        /* Row 2: xxx   xxx  l23 */
        bsiBezierDPoint4d_lineProductXYW (&lineCoffs, &pA[2], &pA[3], 2, 3, orderA);
        insertLineCoffs (Mijk, 2, 2, numI, &lineCoffs, pB, orderB);
        }
   else
        {
        /* If you really want to, you can find some repeated cross products and
                pB expansions here.  Compliments and smiles to you
                if you're that good with the indices. */
        DPoint4d sum, term;
        for (i = 0; i < orderA - 1; i++)
            {
            for (j = i; j < orderA - 1; j++)
                {
                sum.Init( 0.0, 0.0, 0.0, 0.0);
                k = i + j + 1 - orderA - 1;
                if (k < 0)
                    k = 0;
                for (; k <= i && k <= j; k++)
                    {
                    m = i + j + 1 - k;
                    if (0 <= m && m < orderA)
                        {
                        bsiBezierDPoint4d_lineProductXYW (&term, &pA[k], &pA[m], k, m, orderA);
                        sum.SumOf (sum, term);
                        }
                    }
                insertLineCoffs (Mijk, i, j, numI, &sum, pB, orderB);
                }
            }
        }

    for (i = 0; i < numI; i++)
        indexI[i] = indexJ[i] = i;

    if (jmdlBezier_expandDeterminant (detCoff, &detOrder, &conditionRatio, MAX_BEZIER_ORDER, Mijk,
                            indexI,
                            indexJ,
                            numI,
                            numI,
                            orderB)
        && conditionRatio > conditionLimit
        )
        {
        bsiBezier_univariateRoots (detRoot, &numRoot, detCoff, detOrder);
        /* Watch out for 0-everywhere case -- happens with inflated order */
        if (numRoot > 0 && numRoot < detOrder)
            {
            DPoint3d normalizedIntersection;
            DPoint4d closePoint;
            double   closeParam;
            double   closeDist2;
            double   scaleCoordinate = bsiBezierDPoint4d_getLargestCoordinateXY (pA, orderA);
            static double s_relTol = 1.0e-10;
            double   tolerance = s_relTol * scaleCoordinate;
            double   tol2      = tolerance * tolerance;
            int      numKeep = 0;

            bsiBezierDPoint4d_evaluateArray (intersectB, NULL,
                                pB, orderB, detRoot, numRoot);
            /* NEEDS WORK: We know the roots as both parameters and coordinates
                on curve B.  Need to get paramter and coordiante (possibly differing
                in z) on curve A.   Immediate solution: call for closest XYPoint on curve A.
                Problem: If the intersection is at a self-intersection point of A, we ony
                            get one parameter back.
                Solution 1: Make a variant of closestXYPoint which will return multiple
                            nearby points.  (e.g. all within tolerance, rather than just
                            the closest)
                Solution 2: Go over Sederberg's method of lines and see if the matrix inside
                            the determinant carries any interesting information.  For instance, are
                            the null vectors of the matrix related to the intersection parameter?
            */
            for (i = 0; i < numRoot; i++)
                {
                if (   intersectB[i].GetProjectedXYZ (normalizedIntersection)
                    && bsiBezierDPoint4d_closestXYPoint (&closePoint, &closeParam, &closeDist2,
                                        pA, orderA,
                                        normalizedIntersection.x, normalizedIntersection.y,
                                        0.0, 1.0)
                    )
                    {
                    /* REMARK: Assume closestXYPoint never returns a parameter outside 0..1 */
                    if (closeDist2 <= tol2)
                        {
                        pPointA[numKeep] = closePoint;
                        pParamA[numKeep] = closeParam;
                        pPointB[numKeep] = intersectB[i];
                        pParamB[numKeep] = detRoot[i];
                        /* NEEDS WORK: run a Newton-Raphson step to improve the root accuracy.
                                2 equations to improve:
                                            fx: xA(paramA) wB(paramB) - xB(paramB) wA(paramA)=0
                                            fy: yA(paramA) wB(paramB) - yB(paramB) wA(paramA)=0
                           Based on Spencer's thesis, we expect to see parameter adjustments of
                                order 10e-10 on first step, then nearly zero on 2nd or 3rd.
                            (But beware of tangency conditions!!!)
                            (Use SVD 2x2 solver -- ratio of singular values is a good hint about
                                    tangency)
                         */
                         numKeep++;
                        }
                    else
                        {
                        *pConditionRatio = 1.0e-12;
                        }
                    }
                 }
            *pNumIntersection = numKeep;
            }
        boolstat = true;
        }
    return boolstat;
    }

static bool     jmdlBezierDPoint4d_intersectXYThisOrder
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      double    *pConditionRatio,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB,
      double    conditionLimit
)
    {
    DPoint4d localA[MAX_BEZIER_ORDER];
    DPoint4d localB[MAX_BEZIER_ORDER];
    DMatrix4d worldToLocal, localToWorld;
    int numIntersection;
    bool    stat;
    *pConditionRatio = 0.0;

    if (orderA > MAX_BEZIER_ORDER || orderB > MAX_BEZIER_ORDER)
        {
        if (pNumIntersection)
            *pNumIntersection = 0;
        return false;
        }

    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, &localToWorld, &pA[0]);

    bsiDMatrix4d_multiply4dPoints (&worldToLocal, localA, pA, orderA);
    bsiDMatrix4d_multiply4dPoints (&worldToLocal, localB, pB, orderB);

    stat = jmdlBezierDPoint4d_intersectXYThisOrder_go
                    (
                    pPointA,
                    pParamA,
                    pPointB,
                    pParamB,
                    &numIntersection,
                    pConditionRatio,
                    maxIntersection,
                    localA,
                    orderA,
                    localB,
                    orderB,
                    conditionLimit
                    );

    if (pNumIntersection)
        *pNumIntersection = numIntersection;

    if (pPointA)
        bsiDMatrix4d_multiply4dPoints (&localToWorld, pPointA, pPointA, numIntersection);

    if (pPointB)
        bsiDMatrix4d_multiply4dPoints (&localToWorld, pPointB, pPointB, numIntersection);


    return stat;
    }

/*---------------------------------------------------------------------------------**//**
*
* @param
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlBezierDPoint4d_intersectXY_chooseOrder
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      double    *pConditionRatio,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB,
      double    conditionLimit
)
    {
    bool    status;
    static bool    s_preserverOrder = false;
    if (s_preserverOrder || orderA <= orderB)
        {
        status = jmdlBezierDPoint4d_intersectXYThisOrder
                        (
                        pPointA, pParamA,
                        pPointB, pParamB,
                        pNumIntersection,
                        pConditionRatio,
                        maxIntersection,
                        pA,
                        orderA,
                        pB,
                        orderB,
                        conditionLimit
                        );
        }
    else
        {
        status = jmdlBezierDPoint4d_intersectXYThisOrder
                        (
                        pPointB, pParamB,
                        pPointA, pParamA,
                        pNumIntersection,
                        pConditionRatio,
                        maxIntersection,
                        pB,
                        orderB,
                        pA,
                        orderA,
                        conditionLimit
                        );
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Get a tolerance for order reduction.
* xyz parts are based only on max diagonal x,y of poles.
* w part is based only on w of poles.
*
* @param pSource IN original curve
* @param pDest  OUT reduced degree curve.  Must be at least as large as input buffer.
* @param order IN original order
* @return reduced order.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void getReductionTolerance
(
DPoint4d *pTolPoint,
const DPoint4d *pPoles,
int order,
double abstol,
double reltol
)
    {
    DPoint4d minPoint, maxPoint;
    int i;
    double aa;

    if (order < 1)
        return;
    minPoint = maxPoint = pPoles[1];
    for (i = 1; i < order; i++)
        {
        if (pPoles[i].x < minPoint.x)
            minPoint.x = pPoles[i].x;
        if (pPoles[i].y < minPoint.y)
            minPoint.y = pPoles[i].y;
        if (pPoles[i].z < minPoint.z)
            minPoint.z = pPoles[i].z;
        if (pPoles[i].w < minPoint.w)
            minPoint.w = pPoles[i].w;

        if (pPoles[i].x > maxPoint.x)
            maxPoint.x = pPoles[i].x;
        if (pPoles[i].y > maxPoint.y)
            maxPoint.y = pPoles[i].y;
        if (pPoles[i].z > maxPoint.z)
            maxPoint.z = pPoles[i].z;
        if (pPoles[i].w > maxPoint.w)
            maxPoint.w = pPoles[i].w;
        }
    // Look (only) at xy ranges for xyz part of tol.  These differences are all non-negative.
    aa = (maxPoint.x - minPoint.x) + (maxPoint.y - minPoint.y);
    pTolPoint->x = pTolPoint->y = pTolPoint->z = abstol + reltol * aa;
    aa = maxPoint.w - minPoint.w;
    pTolPoint->w = abstol + reltol * aa;
    }
/*---------------------------------------------------------------------------------**//**
* @description Reduce degree of curve as far as possible.
*
* @param pSource IN original curve
* @param pDest  OUT reduced degree curve.  Must be at least as large as input buffer.
* @param order IN original order
* @return reduced order.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static int reduceDegree
(
        DPoint4d *pDest,
const   DPoint4d *pSource,
int order,
double absTol,
double relTol
)
    {
    if (order < 1)
        return 0;
    DPoint4d *pA1 = (DPoint4d*)_alloca (order * sizeof (DPoint4d));
    DPoint4d *pA2 = (DPoint4d*)_alloca (order * sizeof (DPoint4d));
    DPoint4d signedSum, absSum;
    DPoint4d tolerancePoint;
    double xyTol;
    double wTol;
    int i, order1, degree1;
    double scale;
    double a1, a2;

    getReductionTolerance (&tolerancePoint, pSource, order, absTol, relTol);
    if (NULL == pA1 || NULL == pA2)
        return 0;
    xyTol = tolerancePoint.x;
    if (tolerancePoint.y > tolerancePoint.x)
        xyTol = tolerancePoint.y;
    wTol = tolerancePoint.w;

    memcpy (pDest, pSource, order * sizeof (DPoint4d));
    while (order > 2)
        {
        // First look at leading coefficients in relative sense only.
        // In the usual (full degree) case this will give a quick exit.
        bsiBezier_sumScaledCoefficients ((double*)&signedSum, (double*)&absSum, (double*)pDest, order, 4);
        if (fabs (signedSum.x) > relTol * absSum.x)
            return order;
        if (fabs (signedSum.y) > relTol * absSum.y)
            return order;
        if (fabs (signedSum.w) > relTol * absSum.w)
            return order;


        // Form reduced degree polyonomials sweeping in both directions.
        // If we have excess degree these two will be the same.
        bsiBezier_reduceDegreeDirectional ((double*)pA1, (double*)pDest, order, 4, false);
        bsiBezier_reduceDegreeDirectional ((double*)pA2, (double*)pDest, order, 4, true );
        order1 = order - 1;
        // Jump out on any pole coordinate that violates tolerance ....
        for (i = 0; i < order1; i++)
            {
            if (fabs (pA1[i].x - pA2[i].x) > xyTol)
                return order;
            if (fabs (pA1[i].y - pA2[i].y) > xyTol)
                return order;
            // No, we don't care about z!!!
            if (fabs (pA1[i].w - pA2[i].w) > wTol)
                return order;
            }
        // Accept the reduced order curve ...
        // The reduced degree poles are interpolated between the two originals.
        // Interpolation favors curve1 at left, pA2 at right.
        degree1 = order1 - 1;
        scale = 1.0 / degree1;
        pDest[0] = pA1[0];
        a1 = degree1 - 1;
        a2 = 1.0;
        for (i = 1; i < degree1; i++, a1 -= 1.0, a2 += 1.0)
            {
            pDest[i].x = (a1 * pA1[i].x + a2 * pA2[i].x) * scale;
            pDest[i].y = (a1 * pA1[i].y + a2 * pA2[i].y) * scale;
            pDest[i].z = (a1 * pA1[i].z + a2 * pA2[i].z) * scale;
            pDest[i].w = (a1 * pA1[i].w + a2 * pA2[i].w) * scale;
            }
PUSH_STATIC_ANALYSIS_WARNING(6385)
        pDest[degree1] = pA2[degree1];
POP_STATIC_ANALYSIS_WARNING
        order = order1;
        }
    return order;
    }

/*---------------------------------------------------------------------------------**//**
*
* @param
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectXYExt
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      double    *pConditionRatio,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB,
      double    conditionLimit
)
    {
    bool    boolstat = false;
    int numIntersection;
    static double s_degreeReductionRelTol = 1.0e-10;
    static double s_degreeReductionAbsTol = 1.0e-15;

    DPoint4d *pA1 = (DPoint4d*)_alloca (orderA * sizeof (DPoint4d));
    DPoint4d *pB1 = (DPoint4d*)_alloca (orderB * sizeof (DPoint4d));

    int orderA1 = reduceDegree (pA1, pA, orderA, s_degreeReductionAbsTol, s_degreeReductionRelTol);
    int orderB1 = reduceDegree (pB1, pB, orderB, s_degreeReductionAbsTol, s_degreeReductionRelTol);

    if (pNumIntersection)
        *pNumIntersection = 0;
    if (orderA1 <= 1 || orderB1 <= 1)
        return false;

    boolstat = jmdlBezierDPoint4d_intersectXY_chooseOrder
                    (
                    pPointA, pParamA,
                    pPointB, pParamB,
                    &numIntersection,
                    pConditionRatio,
                    maxIntersection,
                    pA1, orderA1,
                    pB1, orderB1,
                    conditionLimit
                    );

    // If degree was reduced for xy parts, higher order z may have been
    // compressed out.   Reevaluate the original curves at solution parameters
    if (boolstat && orderA1 < orderA)
        bsiBezierDPoint4d_evaluateArray (pPointA, NULL, pA, orderA, pParamA, numIntersection);

    if (boolstat && orderB1 < orderB)
        bsiBezierDPoint4d_evaluateArray (pPointB, NULL, pB, orderB, pParamB, numIntersection);

    if (pNumIntersection)
        *pNumIntersection = numIntersection;
    return boolstat;
    }

static double sMinRatio = 1.0e-5;

/*---------------------------------------------------------------------------------**//**
*
* @param
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectXY
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB
)
    {
    double conditionRatio;
    bool    boolstat = bsiBezierDPoint4d_intersectXYExt (pPointA, pParamA, pPointB, pParamB, pNumIntersection,
                &conditionRatio, maxIntersection, pA, orderA, pB, orderB, sMinRatio);
    return boolstat;
    }

static int getConicZNullSpace
(
DConic4dCR conic,
DMatrix4dR nullVectors
)
    {
    double a = DoubleOps::MaxAbs (conic.vector0.MaxAbs (), conic.vector90.MaxAbs ());
    DMatrix4d matrix = DMatrix4d::FromRowValues
        (
        conic.center.x,   conic.center.y,   conic.center.z,   conic.center.w,
        conic.vector0.x,  conic.vector0.y,  conic.vector0.z,  conic.vector0.w,
        conic.vector90.x, conic.vector90.y, conic.vector90.z, conic.vector90.w,
        0.0, 0.0, a, 0.0
        );
    return bsiDMatrix4d_nullSpace (&nullVectors, &matrix);
    }

static bool GetConicIntersectionZAuxPlane
(
DConic4dCR conic,
DPoint4dCR auxPoint, 
DPoint4dCR perpPlane,
DPoint4dR  auxPlane
)
    {
    DVec3d conicPlaneNormal, auxPlaneNormal;
    DPoint3d origin;
    perpPlane.GetXYZ (conicPlaneNormal);
    auxPlaneNormal.NormalizedCrossProduct (conicPlaneNormal, DVec3d::From (0,0,1));
    if (!auxPoint.GetProjectedXYZ (origin))
        return false;
    return auxPlane.PlaneFromOriginAndNormal (origin, auxPlaneNormal);
    }

// [I sA][B]=[B+se*A]
// [0  w][e] [we]
void Translate (DPoint4dR result, double s, DPoint4dCR translation, DPoint4dCR pointB)
    {
    double se = s * pointB.w;
    result.x = pointB.x + se *translation.x;
    result.y = pointB.y + se *translation.y;
    result.z = pointB.z + se *translation.z;
    result.w = translation.w * pointB.w;
    }

void Translate (DPoint4dP result, int n, double s, DPoint4dCR origin, DPoint4dCP point)
    {
    for (int i = 0; i < n; i++)
        Translate (result[i], s, origin, point[i]);
    }

/*---------------------------------------------------------------------------------**//**
* Compute apparent intersection of a conic and bezier curve.
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     bsiBezierDPoint4d_intersectDConic4dXYExt_go
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointConic,
      double    *pAngleConic,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DConic4d  *pConic,
      bool      extendConic
)
    {
    RotMatrix B, Binv;
    double cond;
    //int baseDegree = orderA - 1;
    int orderFF = 2 * orderA - 1;
    RotMatrix originTransform, inverseOriginTransform;
    DPoint3d xywPole[MAX_BEZIER_CURVE_ORDER];
    DPoint3d xyw;
    double aa[MAX_BEZIER_ORDER];     /* temp polynomial for one term like xx, yy, ww */
    double ff[MAX_BEZIER_ORDER];     /* total polynomial */
    double roots[MAX_BEZIER_ORDER];  /* root parameters on the curve. */
    int i;
    int numRoots;
    double theta;
    int numOut = 0;
    bool    status = false;

    DMatrix4d nullSpaceMatrix;
    int nullCount = getConicZNullSpace (*pConic, nullSpaceMatrix);
    bsiDConic4d_getTranslatedRotMatrixXYW (pConic, &originTransform, &inverseOriginTransform, &B);
static bool s_favorB = false;
if (s_favorB)
    if (nullCount > 0 && bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&B, &Binv, &cond))
        {
        // huh? conic appears degenerate, but B is ok.
        nullCount = 0;
        }
    if (orderA > MAX_BEZIER_CURVE_ORDER)
        return false;
    if (nullCount == 1)
        {
        DPoint4d planeCurveIntersectionPoint[MAX_BEZIER_CURVE_ORDER];
        double   planeCurveIntersectionParameter[MAX_BEZIER_CURVE_ORDER];
        DPoint4d perpPlane, auxPlane;
        int numPlaneIntersection;
        // The one DPoint4d in the 
        nullSpaceMatrix.GetColumn (perpPlane, 0);
        bsiBezierDPoint4d_allDPlane4dIntersections (planeCurveIntersectionParameter, planeCurveIntersectionPoint,
                &numPlaneIntersection, MAX_BEZIER_CURVE_ORDER,
                pA, orderA, &perpPlane, 3, false);
        for (int i = 0; i < numPlaneIntersection; i++)
            {
            DPoint3d trigPoints[2];
            int numConicPoints;
            if (GetConicIntersectionZAuxPlane (*pConic, planeCurveIntersectionPoint[i], perpPlane, auxPlane)
                && 0 < (numConicPoints = bsiDConic4d_intersectPlane (pConic, trigPoints, &auxPlane)))
                {
                for (int j = 0; j < 2; j++)
                    {
                    double theta = trigPoints[j].z;
                    if (extendConic || bsiDConic4d_angleInSweep (pConic, theta))
                        {
                        DPoint4d xyzConic;
                        bsiDConic4d_trigParameterToDPoint4d (pConic, &xyzConic, trigPoints[j].x, trigPoints[j].y);
                        if (numOut < maxIntersection)
                            {
                            if (pPointA)
                                pPointA[numOut] = planeCurveIntersectionPoint[i];
                            if (pParamA)
                                pParamA[numOut] = planeCurveIntersectionParameter[i];
                            if (pPointConic)
                                pPointConic[numOut] = xyzConic;
                            if (pAngleConic)
                                pAngleConic[numOut] = theta;
                            }
                        numOut++;
                        }
                    }
                }
            }
        if (pNumIntersection)
            *pNumIntersection = numOut;
        status = true;
        }
    else if (bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&B, &Binv, &cond))
        {
        /* Form poles in the local coordinate system of the xyw projection of the conic: */
        for (i = 0; i < orderA; i++)
            {
            inverseOriginTransform.MultiplyComponents(xyw, pA[i].x, pA[i].y, pA[i].w);
            Binv.MultiplyComponents(xywPole[i], xyw.x, xyw.y, xyw.z);
            }
        /* In the local coordinates the equation of the conic is x^2 + y^2 - w^2 = 0. */
        bsiBezier_univariateProduct (ff, 0, 1,
                                        (double *)xywPole, orderA, 0, 3,
                                        (double *)xywPole, orderA, 0, 3);     /* Now ff = x * x */
        bsiBezier_univariateProduct (aa, 0, 1,
                                        (double *)xywPole, orderA, 1, 3,
                                        (double *)xywPole, orderA, 1, 3);     /* Now aa = y * y */
        bsiBezier_addPolesInPlace (ff, aa, orderFF, 1);
        bsiBezier_univariateProduct (aa, 0, 1,
                                        (double *)xywPole, orderA, 2, 3,
                                        (double *)xywPole, orderA, 2, 3);     /* Now aa = y * y */
        bsiBezier_subtractPolesInPlace (ff, aa, orderFF, 1);
        bsiBezier_univariateRoots (roots, &numRoots, ff, orderFF);
        bsiBezier_addRootNearEndpoint (roots, &numRoots, ff, orderFF, 0, 0.0, 0.0);
        bsiBezier_addRootNearEndpoint (roots, &numRoots, ff, orderFF, 1, 0.0, 0.0);

        for (i = 0; i < numRoots; i++)
            {
            /* Find the root back in the trignometric world */
            double curveParam = roots[i];
            bsiBezier_evaluate ((double*)&xyw, (double *)xywPole, orderA, 3, curveParam);
            theta = Angle::Atan2 (xyw.y, xyw.x);
            if (extendConic || bsiDConic4d_angleInSweep (pConic, theta))
                {
                if (pPointA)
                    bsiBezier_evaluate ((double *)&pPointA[numOut], const_cast<double*>((double const*)pA), orderA, 4, curveParam);
                if (pParamA)
                    pParamA[numOut] = curveParam;
                if (pPointConic)
                    bsiDConic4d_angleParameterToDPoint4d (pConic, &pPointConic[numOut], theta);
                if (pAngleConic)
                    pAngleConic[numOut] = theta;
                numOut++;
                }
            }
        if (pNumIntersection)
            *pNumIntersection = numOut;
        status = true;
        }
    else
        {
#ifdef NEEDS_WORK_DEGENERATE_CONIC
        /* NEEDS WORK: The conic is a straight line in this view. */
#endif
        }
    return status;
    }

Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectDConic4dXYExt
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointConic,
      double    *pAngleConic,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DConic4d  *pConic,
      bool      extendConic
)
    {
    DPoint4d localBezier[MAX_BEZIER_ORDER];
    DConic4d localConic = *pConic;
    DPoint4d origin = pA[0];
    Translate (localBezier, orderA, -1.0, origin, pA);
    Translate (localConic.center, -1.0, origin, pConic->center);
    Translate (localConic.vector0, -1.0, origin, pConic->vector0);
    Translate (localConic.vector90, -1.0, origin, pConic->vector90);
    bool stat = bsiBezierDPoint4d_intersectDConic4dXYExt_go (pPointA, pParamA, pPointConic, pAngleConic,
                    pNumIntersection, maxIntersection, localBezier, orderA, &localConic, extendConic);
    if (nullptr != pPointA)
        Translate (pPointA, *pNumIntersection, 1.0, origin, pPointA);
    if (nullptr != pPointConic)
        Translate (pPointConic, *pNumIntersection, 1.0, origin, pPointConic);
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* Compute apparent intersection of a conic and bezier curve.
* @param
* @return true if sufficient storage to solve.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectDConic4dXY
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointConic,
      double    *pAngleConic,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DConic4d  *pConic
)
    {
    return bsiBezierDPoint4d_intersectDConic4dXYExt
                (
                pPointA, pParamA, pPointConic, pAngleConic, pNumIntersection, maxIntersection,
                pA, orderA, pConic, false
                );
    }

END_BENTLEY_GEOMETRY_NAMESPACE
