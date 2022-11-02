/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Solve a 2x2 linear system.
//! @remarks Solution process uses the SVD (Singular Value Decomposition) to compute the matrix determinant.
//!       This is pricey (2 square roots) compared to a simple Cramer's rule, but provides a clearer test for nearzero determinant.
//! @remarks Explicit SVD formulas from Jim Blinn, "Consider the lowly 2x2 Matrix", <I>IEEE Computer Graphics and Applications</I>,
//!       March 1996, p. 82-88.  (This is a beautiful paper. READ IT!!!!)
//! @param pX0 OUT     first solution parameter
//! @param pX1 OUT     second solution parameter
//! @param a00 IN      00 coefficient of matrix
//! @param a01 IN      01 coefficient of matrix
//! @param a10 IN      10 coefficient of matrix
//! @param a11 IN      11 coefficient of matrix
//! @param b0 IN      0 coefficient on RHS
//! @param b1 IN      1 coefficient on RHS
//! @return true iff the system is invertible
//! @group "Singular Value Decomposition"
//!
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
);

//!
//! @description Solve a 2x2 linear system.
//! @remarks This function does same thing as ~mbsiSVD_solve2x2 except it:
//!   <UL>
//!   <LI>uses a caller-defined tolerance for singularity test, and
//!   <LI>returns (both) singular values as output parameters.
//!   </UL>
//! @param pX0 OUT     first solution parameter
//! @param pX1 OUT     second solution parameter
//! @param pW0 OUT     larger singular value
//! @param pW1 OUT     smaller singular value
//! @param a00 IN      00 coefficient of matrix
//! @param a01 IN      01 coefficient of matrix
//! @param a10 IN      10 coefficient of matrix
//! @param a11 IN      11 coefficient of matrix
//! @param b0 IN      0 coefficient on RHS
//! @param b1 IN      1 coefficient on RHS
//! @param relTol IN      relative tolerance to apply to ratio of singular values
//! @return true iff the system is invertible.
//! @group "Singular Value Decomposition"
//!
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
);

//!
//! @description Invert a 2x2 matrix.
//! @remarks Solution process uses the SVD (Singular Value Decomposition) to compute the matrix determinant.
//!       This is pricey (2 square roots) compared to a simple Cramer's rule, but provides a clearer test for nearzero determinant.
//! @remarks Explicit SVD formulas from Jim Blinn, "Consider the lowly 2x2 Matrix", <I>IEEE Computer Graphics and Applications</I>,
//!       March 1996, p. 82-88.  (This is a beautiful paper. READ IT!!!!)
//! @param pB00 OUT     00 coefficient of inverse
//! @param pB01 OUT     01 coefficient of inverse
//! @param pB10 OUT     10 coefficient of inverse
//! @param pB11 OUT     11 coefficient of inverse
//! @param a00 IN      00 coefficient of matrix
//! @param a01 IN      01 coefficient of matrix
//! @param a10 IN      10 coefficient of matrix
//! @param a11 IN      11 coefficient of matrix
//! @return true iff the matrix is invertible.
//! @group "Singular Value Decomposition"
//!
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
);
//! Return 0, 1, or 2 eigenvalues and their eigenvectors for the matix
//!   [b00 b01]
//!   [b10 b11]
Public GEOMDLLIMPEXP int bsiSVD_realEigenvalues2x2
(
double b00, 
double b01,
double b10, 
double b11,
double lambda[2],
DVec2d eigenvector[2]
);
END_BENTLEY_GEOMETRY_NAMESPACE

