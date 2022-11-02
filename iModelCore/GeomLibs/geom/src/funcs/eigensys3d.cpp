/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* macro
* this macro has the intentional side-effects of setting h & g to new values.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#define ROTATE(a,i,j,k,l)\
    {\
    g = a[i][j];\
    h = a[k][l];\
    a[i][j] = g - s * (h + g*tau);\
    a[k][l] = h + s * (g - h*tau);\
    }

#define MAX_JACOBI_ITERS    50

/*---------------------------------------------------------------------------------**//**
* Diagonalize a 3X3 symmetric matrix by Jacobi's method. Return eigenvalues and normalized eigenvectors (column vectors).
* The method is iterative, but it achieves machine precision -- it's NOT approximate.
* Pre-requisites & errors: This algorithm only applies to symmetric matrices -- it will signal an assertion failure if input is not symmetric.
* (Given that the matrix is symmetric, we can be sure that there are 3 mutually orthogonal eigenvectors and, therefore, that all the
* eigenvalues are distinct. This algorithm is guaranteed to find them ... unless the numbers involved are completely degenerate from the
* start. No explicit error returned in that case.)
* Notes: Taken directly from Numerical Recipes in C (adapted to index from 0). The method is iterative, but NRCEE claims it usually finishes
* in 5 or 6 iterations. ROTATE macro is TRICKY! It deliberatlely changes the values of g and h as a side effect each time it is used.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_jacobi3X3

(
double  d[],
double  v[3][3],
int     *nRot,
double  a[3][3]
)
    {
    int     j, iq, ip, i;
    double  tresh, theta, tau, t, sum, s, c, b[4], z[4], g, h;
    int     n = 3;              /*  3! */
    for (ip=0; ip<n; ip++)
        {
        for (iq=0; iq<n; iq++)
            v[ip][iq] = 0.0;

        v[ip][ip] = 1.0;
        }
    for (ip=0; ip<n; ip++)
        {
        b[ip] = d[ip] = a[ip][ip];
        z[ip] = 0;
        }
    *nRot = 0;

    for (i=0; i<MAX_JACOBI_ITERS; i++)
        {
        sum = 0.0;
        for (ip=0; ip<n; ip++)
            for (iq=ip+1; iq<n; iq++)
               sum += fabs (a[ip][iq]);

        if (sum == 0.0)
            return;

        if (i < 3)
            tresh = 0.2 * sum / (n*n);
        else
            tresh = 0.0;

        for (ip=0; ip<n-1; ip++)
            {
            for (iq=ip+1; iq<n; iq++)
                {
                g = 100.0 * fabs (a[ip][iq]);
                if (i > 3 && fabs (d[ip]) + g == fabs (d[ip])
                          && fabs (d[iq]) + g == fabs (d[iq]))
                    {
                    a[ip][iq] = 0.0;
                    }
                else if (fabs(a[ip][iq]) > tresh)
                    {
                    h = d[iq] - d[ip];
                    if (fabs (h) + g == fabs (h))
                        {
                        t = a[ip][iq] / h;
                        }
                    else
                        {
                        theta = 0.5 * h / a[ip][iq];
                        t = 1.0 / (fabs (theta) + sqrt (1.0 + theta * theta));
                        if (theta < 0.0) t = -t;
                        }
                    c = 1.0 / sqrt (1.0 + t*t);
                    s = t * c;
                    tau = s/(1.0 + c);
                    h = t * a[ip][iq];
                    z[ip] -= h;
                    z[iq] += h;
                    d[ip] -= h;
                    d[iq] += h;
                    a[ip][iq] = 0.0;
                    for (j=0; j<=ip-1; j++)
                        ROTATE (a, j, ip, j, iq)
                    for (j=ip+1; j<=iq-1; j++)
                        ROTATE (a, ip, j, j, iq)
                    for (j=iq+1; j<n; j++)
                        ROTATE (a, ip, j, iq, j)
                    for (j=0; j<n; j++)
                        ROTATE (v, j, ip, j, iq)
                    ++(*nRot);
                    }
                }
            }
        for (ip=0; ip<n; ip++)
            {
            b[ip] += z[ip];
            d[ip] = b[ip];
            z[ip] = 0.0;
            }
        }
    }




#ifdef CompileAll
/*-----------------------------------------------------------------*//**
* @description Insertion sort
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void sortDPoint2dX

(
DPoint2dP pBuffer,
int numPoint
)
    {
    int base, k;
    DPoint2d temp;
    for (base = 0; base < numPoint - 1; base++)
        {
        for (k = base + 1; k < numPoint; k++)
            {
            if (   pBuffer[k].x < pBuffer[base].x
                ||  (  pBuffer[k].x == pBuffer[base].x
                    && pBuffer[k].y >  pBuffer[base].y
                    )
                )
                {
                temp = pBuffer[base];
                pBuffer[base] = pBuffer[k];
                pBuffer[k] = temp;
                }
            }
        }
    }
 #endif

static double rotate
(
DVec3dR U,
DVec3dR V,
DVec3dR QU,
DVec3dR QV
)
    {
    double c, s;
    DVec3d Q, R;
    bsiTrig_constructOneSided3DJacobiRotation (&c, &s, &U, &V);
    Q.SumOf (U, c, V, s);
    R.SumOf (U, -s, V, c);
    U = Q;
    V = R;

    Q.SumOf (QU, c, QV, s);
    R.SumOf (QU, -s, QV, c);
    QU = Q;
    QV = R;
#ifdef CheckOrthogonalization
    double uu = U.DotProduct (U);
    double uv = U.DotProduct (V);
    double vv = V.DotProduct (V);
#endif
    return fabs (s);
    }

Public GEOMDLLIMPEXP bool    bsiRotMatrix_orthogonalizeColumns
(
RotMatrixCP pMatrixA,
RotMatrixP pB,
RotMatrixP pV
)
    {
    DVec3d U, V, W;
    pMatrixA->GetColumns (U, V, W);
    DVec3d QU, QV, QW;
    QU = DVec3d::From (1.0, 0.0, 0.0);
    QV = DVec3d::From (0.0, 1.0, 0.0);
    QW = DVec3d::From (0.0, 0.0, 1.0);
    bool    boolstat = false;
    int i;
    static int maxIteration = 10;
    static double s_sineTol = 1.0e-14;;
    double s;
    for (i = 0; i < maxIteration; i++)
        {
        s  = rotate (U, V, QU, QV);
        s += rotate (V, W, QV, QW);
        s += rotate (U, W, QU, QW);
        if (s <= s_sineTol)
            {
#ifdef CheckOrthogonalization
            CheckBV (U, V, W, QU, QV, QW, pMatrixA);
#endif
            boolstat = true;
            break;
            }
        }

    if (pB)
        pB->InitFromColumnVectors (U, V, W);
    if (pV)
        pV->InitFromColumnVectors (QU, QV, QW);
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
* @description Invert a matrix by constructing orthogonal factorization.
*   This is more expensive than Cramer's rule or elimination, but provides
*   a more accurate estimate of condition.   This function considers
*   any nonzero condition to be nonsingular; caller may wish to
*   apply a stricter test to the condition.
* @param pMatrixA => matrix to invert.
* @param pAInv <= the computed inverse.
* @param pCondition => the smallest singular value divided by the largest.
* @return true if the matrix is invertible.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_invertRotMatrixByOrthogonalFactors

(
RotMatrixCP pMatrixA,
RotMatrixP pMatrixAInv,
double      *pCondition
)
    {
    RotMatrix matrixB, matrixV, matrixUSigmaInverse;
    RotMatrix inverse;
    bool    boolstat = false;
    double sigma2[3];
    double sigma2Min, sigma2Max;
    double condition = 0.0;
    static double s_relTol = 1.0e-26;
    int i;
    inverse.InitIdentity ();
    if (bsiRotMatrix_orthogonalizeColumns (pMatrixA, &matrixB, &matrixV))
        {
        for (i = 0; i < 3; i++)
            sigma2[i] = DVec3d::FromColumn (matrixB, i).MagnitudeSquared ();
        sigma2Min = sigma2Max = sigma2[0];
        if (sigma2[1] > sigma2Max)
            sigma2Max = sigma2[1];
        if (sigma2[2] > sigma2Max)
            sigma2Max = sigma2[2];

        if (sigma2[1] < sigma2Min)
            sigma2Min = sigma2[1];
        if (sigma2[2] < sigma2Min)
            sigma2Min = sigma2[2];

        if (sigma2Max > 0.0 && sigma2Min > s_relTol * sigma2Max)
            {
            /* 1/sigma is ok for each sigma. */
            condition = sqrt (sigma2Min / sigma2Max);
            matrixUSigmaInverse.ScaleColumns (matrixB,
                    1.0 / sigma2[0],
                    1.0 / sigma2[1],
                    1.0 / sigma2[2]);
            inverse.InitProductRotMatrixRotMatrixTranspose (matrixV, matrixUSigmaInverse);
            //            bsiRotMatrix_multiplyRotMatrixRotMatrixTranspose, (&inverse, &matrixV, &matrixUSigmaInverse);
            boolstat = true;
            }
        }
    if (pMatrixAInv)
        {
        if (boolstat)
            *pMatrixAInv = inverse;
        else
            pMatrixAInv->InitIdentity ();//bsiRotMatrix_initIdentity (pMatrixAInv);
        }
    if (pCondition)
        *pCondition = condition;
    return boolstat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
