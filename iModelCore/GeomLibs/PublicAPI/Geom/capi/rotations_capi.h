/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Given trig functions (cosine and sine) of some (double) angle 2A, find trig functions for the angle A.
//! @remarks Inputs are <EM>not</EM> assumed to be a unit (2D) vector -- first step is to compute r = c^2 + s^2.
//! @param pCosA OUT     cosine of angle
//! @param pSinA OUT     sine of angle
//! @param rCos2A IN      cosine of double angle, possibly scaled by some r
//! @param rSin2A IN      sine of double angle, possibly scaled by same r.
//! @group "Trigonometric Rotations"
//!
Public GEOMDLLIMPEXP void bsiTrig_halfAngleFunctions
(
double      *pCosA,
double      *pSinA,
double      rCos2A,
double      rSin2A
);

//!
//! @description Find a vector that differs from (x0,y0) by a multiple of 90 degrees,
//!   x1 is positive, and y1 is as small as possible in absolute value, i.e. points to the right.
//! @param pX1 OUT     rotated x component
//! @param pY1 OUT     rotated y component
//! @param x0 IN      initial x component
//! @param x1 IN      initial y component
//! @group "Trigonometric Rotations"
//!
Public GEOMDLLIMPEXP void bsiTrig_rotate90UntilSmallAngle
(
double      *pX1,
double      *pY1,
double      x0,
double      y0
);

//!
//! Construct the givens rotation trig values (cosine, sine) so as to
//! transfer the weight of entries to the "a" position, and make "b" zero.
//!
//! @param pCosine OUT     cosine of givens angle
//! @param pSine OUT     sine of givens angle
//! @param a IN      x part of vector
//! @param b IN      y part of vector
//!
Public GEOMDLLIMPEXP void bsiTrig_constructGivensWeights
(
double      *pCosine,
double      *pSine,
double      a,
double      b
);

//!
//! @param pA OUT     modified x part of vector
//! @param pB OUT     modified y part of vector
//! @param a IN      x part of vector
//! @param b IN      y part of vector
//! @param cosine OUT     cosine of givens angle
//! @param sine OUT     sine of givens angle
//!
Public GEOMDLLIMPEXP void bsiTrig_applyGivensWeights
(
double      *pA,
double      *pB,
double      a,
double      b,
double      cosine,
double      sine
);

//!
//! Compute the cosine and sine of a rotation matrix which can be used to post-multiply
//! a 2-column matrix so that the resulting colums become perpendicular.
//! The (2x2) matrix has first row (c,-s), second row (s,c)
//!
//! @param      pCosine OUT     cosine of rotation angle
//! @param      pSine   OUT     sine of rotation angle
//! @param      pU      IN      first vector (column of matrix being rotated)
//! @param      pV      IN      second vector (column of matrix being rotated)
//!
Public GEOMDLLIMPEXP void        bsiTrig_constructOneSided3DJacobiRotation
(
double    *pCosine,
double    *pSine,
DVec3dP     pU,
DVec3dP     pV
);

//!
//! Compute the cosine and sine of a rotation matrix which can be used to post-multiply
//! a 2-column matrix so that the resulting colums become perpendicular.
//! The (2x2) matrix has first row (c,-s), second row (s,c)
//!
//! @param      pCosine OUT     cosine of rotation angle
//! @param      pSine   OUT     sine of rotation angle
//! @param      pU      IN      first vector (column of matrix being rotated)
//! @param      pV      IN      second vector (column of matrix being rotated)
//!
Public GEOMDLLIMPEXP void        bsiTrig_constructOneSided4DJacobiRotation
(
double    *pCosine,
double    *pSine,
DPoint4dCP pU,
DPoint4dCP pV
);

//!
//! @param pA OUT     modified x part of vector
//! @param pB OUT     modified y part of vector
//! @param a IN      x part of vector
//! @param b IN      y part of vector
//! @param cosine OUT     cosine of givens angle
//! @param sine OUT     sine of givens angle
//!
Public GEOMDLLIMPEXP bool     bsiTrig_constructHyperbolicWeights
(
double      *pSecant,
double      *pTangent,
double      a,
double      b
);

//!
//! @param pA OUT     modified x part of vector
//! @param pB OUT     modified y part of vector
//! @param a IN      x part of vector
//! @param b IN      y part of vector
//! @param cosine OUT     cosine of givens angle
//! @param sine OUT     sine of givens angle
//!
Public GEOMDLLIMPEXP void bsiTrig_applyHyperbolicWeights
(
double      *pA,
double      *pB,
double      a,
double      b,
double      secant,
double      tangent
);

//!
//! Apply a matrix to sine and cosine of a single angle, and return the mapped trig functions and
//! angle.
//!
//! @param    pMappedTrigFuncs OUT     (optional) vector of weighted cosine, weighted sine, weight values.
//! @param    pPhi             OUT     (optional) mapped angle.
//! @param    pDerivNumerator  OUT     (optional) numerator of ratio form of derivative
//! @param    pDirivDenominator OUT     (optional) denominator of ratio form of derivative
//!
Public GEOMDLLIMPEXP void     bsiTrig_mapAngle
(
DPoint3dP pMappedTrigFuncs,
double    *pPhi,
double    *pDerivNumerator,
double    *pDerivDenominator,
RotMatrixCP pMatrix,
double    theta
);

//!
//! Apply a matrix to sine and cosine of two angles, and return the corresponding
//! mapped angles.
//!
//!
Public GEOMDLLIMPEXP void     bsiTrig_mapLimits
(
double    *pPhi0,
double    *pPhi1,
RotMatrixCP pMatrix,
double    theta0,
double    theta1
);

//!
//! Compute the least-squares solution to the overdetermined problem AX = B
//! where A is 3 rows and 2 columns and B is one column.
//!
//! @param pS0 OUT     1st solution parameter
//! @param pS1 OUT     2nd solution parameter
//! @param pA0 IN      First column of matrix
//! @param pA1 IN      Second column of matrix
//! @param pB IN      right hand side
//! @return rank deficiency of A.
//!                   0 means A is full rank, i.e. normal solution.
//!                   1 means columns of A are either parallel or one is zero.
//!                   2 means A has nothing but zeros.
//!
Public GEOMDLLIMPEXP int bsiTrig_solve3x2
(
double  *pS0,
double  *pS1,
DPoint3dCP pA0,
DPoint3dCP pA1,
DPoint3dCP pB
);

//!
//!
//! @param pB OUT     matrix containing coefficients that map a unit circle to the
//!               standard parabola y=x^2
//! @param pBinverse OUT     inverse matrix
//!
Public GEOMDLLIMPEXP void     bsiTrig_getStandardParabola
(
RotMatrixP pB,
RotMatrixP pBinverse
);

//!
//!
//! @param pB OUT     matrix containing coefficients that map a unit circle to the
//!               standard parabola y=1/x
//! @param pBinverse OUT     inverse matrix
//!
Public GEOMDLLIMPEXP void     bsiTrig_getStandardHyperbola
(
RotMatrixP pB,
RotMatrixP pBinverse
);

//!
//! @description Compute matrices B, C, Q such that:
//! <UL>
//! </LI>w*B*C*Q = I
//! </LI>(wc ws w1) * B = (0 0 1)
//! </LI>If F is a trig point (F.x*F.x + F.y * F.y = F.w * F.w), then Q*F is also.
//! </UL>
//! @param pw OUT     scale factor
//! @param pB OUT     transformation to purely cartesian weights.
//! @param pC OUT     basis matrix for a standard ellipse, parabola, or hyperbola.
//! @param pQ OUT     angle transformation
//! @param pCurveType OUT     1 for ellipse, 0 for parabola, -1 for hyperbola.
//! @param wc IN      weight of conic's vector0
//! @param ws IN      weight of conic's vector90
//! @param ws IN      weight of conic's center
//! @return true if |wc| + |ws| + |w1| > 0
//!
Public GEOMDLLIMPEXP bool    bsiTrig_factorConicWeights
(
double      *pw,
RotMatrixP pB,
RotMatrixP pC,
RotMatrixP pQ,
int         *pCurveType,
double      wc,
double      ws,
double      w1
);

END_BENTLEY_GEOMETRY_NAMESPACE

