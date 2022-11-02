/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* Solve a 2x2 linear system -- unconditional svd.
*
* @param pX0 <= first solution parameter
* @param pX1 <= second solution parameter
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @param b0 => 0 coefficient on RHS
* @param b1 => 1 coefficient on RHS
* @return true iff the system is invertible
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static bool    bsiSVD_solve2x2_go

(
double      *pX0,
double      *pX1,
double      a00,
double      a01,
double      a10,
double      a11,
double      b0,
double      b1
)
    {
    double Ebar = a00 + a11;
    double Fbar = a00 - a11;
    double Gbar = a01 + a10;
    double Hbar = a01 - a10;
    double rootEH = 0.5 * sqrt (Ebar * Ebar + Hbar * Hbar);
    double rootFG = 0.5 * sqrt (Fbar * Fbar + Gbar * Gbar);

    double wLarge = rootEH + rootFG;    /* SUM of two sqrts, cannot be negative */
    double wSmall = rootEH - rootFG;    /* DIFFERENCE of same, absolute value
                                            cannot exceed prior sum. */

    static double relTol = 1.0e-12;

    bool    result;

    if (fabs(wSmall) <= relTol * wLarge)
        {
        *pX0 = *pX1 = 0;
        result = false;
        }
    else
        {
        double determinant = wLarge * wSmall;   /* We know that neither of these
                                                    is zero */
        double det0 = b0 * a11 - b1 * a01;
        double det1 = a00 * b1 - a10 * b0;
        *pX0 = det0 / determinant;
        *pX1 = det1 / determinant;
        result = true;
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Solve a 2x2 linear system.
* @remarks Solution process uses the SVD (Singular Value Decomposition) to compute the matrix determinant.
*       This is pricey (2 square roots) compared to a simple Cramer's rule, but provides a clearer test for nearzero determinant.
* @remarks Explicit SVD formulas from Jim Blinn, "Consider the lowly 2x2 Matrix", <I>IEEE Computer Graphics and Applications</I>,
*       March 1996, p. 82-88.  (This is a beautiful paper. READ IT!!!!)
* @param pX0 <= first solution parameter
* @param pX1 <= second solution parameter
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @param b0 => 0 coefficient on RHS
* @param b1 => 1 coefficient on RHS
* @return true iff the system is invertible
* @group "Singular Value Decomposition"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiSVD_solve2x2

(
double      *pX0,
double      *pX1,
double      a00,
double      a01,
double      a10,
double      a11,
double      b0,
double      b1
)
    {
    /* We all believe (or do we?) that SVD is the most stable way to solve a system.
       Maybe this is true for badly conditioned systems.
       However, for a diagonal system with large scale difference between entries, we observe BAD THINGS.
           (Namely, the results differ from the obvious one-step division.  Just common sense
           says that the adding and subtracting of diagonals can't be a good thing.)
       We also observe that this is corrected by scaling each column of the matrix to its largest entry,
            which makes the diagonal system an identity.
       So here we go.
    */

    double scale0 = fabs (a00);
    double scale1 = fabs (a11);
#define USE_PRECOMPUTED_DIVISION
#ifdef USE_PRECOMPUTED_DIVISION
    double factor0, factor1;
#endif
    double a;
    bool    boolstat;


    a = fabs (a10);
    if (a > scale0)
        scale0 = a;

    a = fabs (a01);
    if (a > scale1)
        scale1 = a;


    if (scale0 == 0.0 || scale1 == 0.0)
        {
        *pX0 = 0.0;
        *pX1 = 0.0;
        return false;
        }


#ifdef USE_PRECOMPUTED_DIVISION
    factor0 = 1.0 / scale0;
    factor1 = 1.0 / scale1;
    a00 *= factor0;
    a10 *= factor0;
    a01 *= factor1;
    a11 *= factor1;
#else
    a00 /= scale0;
    a10 /= scale0;
    a01 /= scale1;
    a11 /= scale1;
#endif
    boolstat = bsiSVD_solve2x2_go (pX0, pX1, a00, a01, a10, a11, b0, b1);
    /* If fail, these are zeros */
#ifdef USE_PRECOMPUTED_DIVISION
    *pX0 *= factor0;
    *pX1 *= factor1;
#else
    *pX0 /= scale0;
    *pX1 /= scale1;
#endif
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
* @description Solve a 2x2 linear system.
* @remarks This function does same thing as ~mbsiSVD_solve2x2 except it:
*   <UL>
*   <LI>uses a caller-defined tolerance for singularity test, and
*   <LI>returns (both) singular values as output parameters.
*   </UL>
* @param pX0 <= first solution parameter
* @param pX1 <= second solution parameter
* @param pW0 <= larger singular value
* @param pW1 <= smaller singular value
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @param b0 => 0 coefficient on RHS
* @param b1 => 1 coefficient on RHS
* @param relTol => relative tolerance to apply to ratio of singular values
* @return true iff the system is invertible.
* @group "Singular Value Decomposition"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiSVD_solve2x2Ext

(
double      *pX0,
double      *pX1,
double      *pW0,
double      *pW1,
double      a00,
double      a01,
double      a10,
double      a11,
double      b0,
double      b1,
double      relTol
)
    {
    double Ebar = a00 + a11;
    double Fbar = a00 - a11;
    double Gbar = a01 + a10;
    double Hbar = a01 - a10;
    double rootEH = 0.5 * sqrt (Ebar * Ebar + Hbar * Hbar);
    double rootFG = 0.5 * sqrt (Fbar * Fbar + Gbar * Gbar);

    double wLarge = rootEH + rootFG;    /* SUM of two sqrts, cannot be negative */
    double wSmall = rootEH - rootFG;    /* DIFFERENCE of same, absolute value
                                            cannot exceed prior sum. */

    bool    result;

    if (fabs(wSmall) <= relTol * wLarge)
        {
        *pW0 = wLarge;
        *pW1 = wSmall;
        *pX0 = *pX1 = 0;
        result = false;
        }
    else
        {
        double determinant = wLarge * wSmall;   /* We know that neither of these is zero. */
        double det0 = b0 * a11 - b1 * a01;
        double det1 = a00 * b1 - a10 * b0;
        *pW0 = wLarge;
        *pW1 = wSmall;
        *pX0 = det0 / determinant;
        *pX1 = det1 / determinant;
        result = true;
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Invert a 2x2 matrix.
* @remarks Solution process uses the SVD (Singular Value Decomposition) to compute the matrix determinant.
*       This is pricey (2 square roots) compared to a simple Cramer's rule, but provides a clearer test for nearzero determinant.
* @remarks Explicit SVD formulas from Jim Blinn, "Consider the lowly 2x2 Matrix", <I>IEEE Computer Graphics and Applications</I>,
*       March 1996, p. 82-88.  (This is a beautiful paper. READ IT!!!!)
* @param pB00 <= 00 coefficient of inverse
* @param pB01 <= 01 coefficient of inverse
* @param pB10 <= 10 coefficient of inverse
* @param pB11 <= 11 coefficient of inverse
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @return true iff the matrix is invertible.
* @group "Singular Value Decomposition"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiSVD_invert2x2

(
double      *pB00,
double      *pB01,
double      *pB10,
double      *pB11,
double      a00,
double      a01,
double      a10,
double      a11
)
    {
    double Ebar = a00 + a11;
    double Fbar = a00 - a11;
    double Gbar = a01 + a10;
    double Hbar = a01 - a10;
    double rootEH = 0.5 * sqrt (Ebar * Ebar + Hbar * Hbar);
    double rootFG = 0.5 * sqrt (Fbar * Fbar + Gbar * Gbar);

    double wLarge = rootEH + rootFG;    /* SUM of two sqrts, cannot be negative */
    double wSmall = rootEH - rootFG;    /* DIFFERENCE of same, absolute value
                                            cannot exceed prior sum. */

    static double relTol = 1.0e-12;

    bool    result;

    if (fabs (wSmall) <= relTol * wLarge)
        {
        *pB00 = *pB11 = 1.0;
        *pB01 = *pB10 = 0.0;
        result = false;
        }
    else
        {
        double determinant = wLarge * wSmall;   /* We know that neither of these is zero */
        double f = 1.0 / determinant;
        *pB00 = a11 * f;
        *pB01 = -a01 * f;
        *pB10 = -a10 * f;
        *pB11 = a00 * f;
        result = true;
        }
    return result;
    }

Public int bsiSVD_realEigenvalues2x2
(
double b00, 
double b01,
double b10, 
double b11,
double lambda[2],
DVec2d eigenvector[2]
)
    {
    double trace = b00 + b11;
    double determinant  = b00 * b11 - b01 * b10;
    Polynomial::Power::Degree2 quadratic (determinant, - trace, 1.0);
    int numRoots = quadratic.RealRoots (lambda);
    if (numRoots == 0)
        return 0;
    double bb = b01 * b01;
    double cc = b10 * b10;
    for (int i = 0; i < numRoots; i++)
        {
        double c00 = b00 - lambda[i];
        double c11 = b11 - lambda[i];
        eigenvector[i].Init (sqrt (bb + c11 * c11), sqrt (c00 * c00 + cc));
        if (c00 * b01 > 0.0)
            eigenvector[i].x *= -1.0;
        }
    return numRoots;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
