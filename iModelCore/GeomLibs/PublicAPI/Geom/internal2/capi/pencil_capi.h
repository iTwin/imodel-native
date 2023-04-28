/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

Public GEOMDLLIMPEXP void bsiCTrig_dotProduct

(
DPoint2dP pDot, /* OUT     complex dot product */
const double *pUr,      /* IN      real part of column U */
const double *pUi,      /* IN      imaginary part of column U */
const double *pVr,      /* IN      real part of column V */
const double *pVi,      /* IN      imaginary part of column V */
int         step,       /* IN      step between adjacent members of all vectors */
int         n           /* IN      vector lengths */
);

/*-----------------------------------------------------------------*//**
* Compute the sine and cosine of the angle which is half of the
* angle whose sine and cosine are given; all values are complex.
* @param pCosA OUT     cosine of half angle
* @param pSinA OUT     sine of half angle
* @param rCos2A IN      x component of a vector
* @param rSin2A IN      y component of a vector
*/
Public GEOMDLLIMPEXP void bsiCTrig_halfAngleFunctions

(
DPoint2dP pCosA,     /* OUT     cosine(a) cosine of angle*/
      DPoint2d  *pSinA,     /* OUT     sine (a)  sine of angle*/
const DPoint2d  *pCos2A,      /* IN      cosine(2a) cosine of double angle, possibly scaled by some r*/
const DPoint2d  *pSin2A       /* IN      sine(2a) sine of double angle, possibly scaled by same r.*/
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiCTrig_dotProduct                                     |
|                                                                       |
|                                                                       |
| Dot product of vectors with real and complex parts stored as          |
| independent arrays.                                                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiCTrig_dotProduct

(
DPoint2dP pDot, /* OUT     complex dot product */
const double *pUr,      /* IN      real part of column U */
const double *pUi,      /* IN      imaginary part of column U */
const double *pVr,      /* IN      real part of column V */
const double *pVi,      /* IN      imaginary part of column V */
int         step,       /* IN      step between adjacent members of all vectors */
int         n           /* IN      vector lengths */
);

/*-----------------------------------------------------------------*//**
* Apply a complex rotation to a pair of complex vectors.
* @param pUr IN OUT  real part of vector U
* @param pUi IN OUT  complex part of vector V
* @param pVr IN OUT  real part of vector V
* @param pVi IN OUT  imaginary part of vector V
* @param step   IN      step size in vectors
* @param n      IN      number of components in vectors
* @param pCosTheta IN      complex cosine of complex rotation angle
* @param pSinTheta IN      complex sine of complex rotation angle
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
);

/*-----------------------------------------------------------------*//**
* Compute and apply a complex jacobi rotation to orthogonalize a pair of complex columns
* @param pUr    IN OUT  real part of column U
* @param pUi    IN OUT  imaginary part of column U
* @param pVr    IN OUT  real part of column V
* @param pVi    IN OUT  imaginary part of column V
* @param pQjr   IN OUT  real part of column of Qj
* @param pQji   IN OUT  real part of column of Qj
* @param pQkr   IN OUT  real part of column of Qk
* @param pQki   IN OUT  real part of column of Qk
* @param step   IN      step between adjacent members of U and V vectors
* @param stepQ  IN      step between adjacent members of Q vectors
* @param n
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiStep

(
double      *pUr,       /* IN OUT  real part of column U */
double      *pUi,       /* IN OUT  imaginary part of column U */
double      *pVr,       /* IN OUT  real part of column V */
double      *pVi,       /* IN OUT  imaginary part of column V */
double      *pQjr,      /* IN OUT  real part of column of Qj */
double      *pQji,      /* IN OUT  real part of column of Qj */
double      *pQkr,      /* IN OUT  real part of column of Qk */
double      *pQki,      /* IN OUT  real part of column of Qk */
int         step,       /* IN      step between adjacent members of U and V vectors */
int         stepQ,      /* IN      step between adjacent members of Q vectors */
int         n           /* IN      vector lengths */
);

/*-----------------------------------------------------------------*//**
* Perform a jacobi sweep to successively zero out an entire
*
* @param pAr            IN OUT  real part of matrix
* @param pAi            IN OUT  imaginary part of matrix
* @param pQr            IN OUT  real part of eigenvectors
* @param pQi            IN OUT  imaginary part of eigenvectors
* @param rowStepA       IN      row step for A matrices
* @param colStepA       IN      column step for A matrices
* @param rowStepQ       IN      row step for Q matrices
* @param colStepQ       IN      column step for Q matrices
* @param numRow         IN      number of rows in A
* @param numCol         IN      number of columns in A (hence rows, columns in Q)
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiSweep

(
double  *pAr,       /* IN OUT  real part of matrix */
double  *pAi,       /* IN OUT  imaginary part of matrix */
double  *pQr,       /* IN OUT  real part of eigenvectors */
double  *pQi,       /* IN OUT  imaginary part of eigenvectors */
int     rowStepA,   /* IN      row step for A matrices */
int     colStepA,   /* IN      column step for A matrices */
int     rowStepQ,   /* IN      row step for Q matrices */
int     colStepQ,   /* IN      column step for Q matrices */
int     numRow,     /* IN      number of rows in A */
int     numCol      /* IN      number of columns in A (hence rows, columns in Q) */
);

/*-----------------------------------------------------------------*//**
* Use one-sided complex orthogonal transformations to
* orthogonalize a complex symmetric matrix given in factored form
* @param pAr IN OUT  real part of symmetric factor
* @param pAi IN OUT  imaginary part of factor
* @param pQr IN OUT  real part of orthogonal basis
* @param pQi IN OUT  imaginary part of orthogonal basis
* @param rowStepA IN      row step in A
* @param colStepA IN      column step in A
* @param rowStepQ IN      row step in Q
* @param numRow   IN      number of rows in A
* @param numCol   IN      number of columns in A (hence rows in Q)
*/
Public GEOMDLLIMPEXP void bsiCTrig_jacobiColumnOrthogonalization

(
double  *pAr,       /* IN OUT  real part of matrix */
double  *pAi,       /* IN OUT  imaginary part of matrix */
double  *pQr,       /* IN OUT  real part of eigenvectors */
double  *pQi,       /* IN OUT  imaginary part of eigenvectors */
int     rowStepA,   /* IN      row step for A matrices */
int     colStepA,   /* IN      column step for A matrices */
int     rowStepQ,   /* IN      row step for Q matrices */
int     colStepQ,   /* IN      column step for Q matrices */
int     numRow,     /* IN      number of rows in A */
int     numCol      /* IN      number of columns in A (hence rows, columns in Q) */
);

Public GEOMDLLIMPEXP StatusInt bsiDMatrix4d_searchSingularPencil

(
DPoint4dP pSigma2,        /* OUT     array of 0 to 4 characteristics for singular surfaces in the pencil */
DMatrix4dP pB2,      /* OUT     array of 0 to 4 coordinate frames for singular surfaces in the pencil */
DMatrix4dP pB2inv,           /* OUT     array of 0 to 4 coordinate frame inverses for singular surfaces in the pencil */
int         *pNumSurface,    /* OUT     number of singular surfaces */
DPoint4dCP pSigma0,
DMatrix4dCP pB0,
DMatrix4dCP pB0inv,
DPoint4dCP pSigma1,
DMatrix4dCP pB1,
DMatrix4dCP pB1inv
);

