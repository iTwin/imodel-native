/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Solve the simultaneous equations
//! <pre>
//!               alpha + beta*c + gamma*s = 0
//!               c*c + s*s = 1
//! </pre>
//! i.e. in the (c,s) plane find intersections of the
//! line and the unit circle.
//! and return nsol=
//! <pre>
//!       0  proper equation, no solutions
//!       2  proper equation, two roots returned
//!       1  improper equation (beta=gamma=0)
//! </pre>
//! The returned value of a2 the squared distance (in
//! c,s space) from origin to the closest point on the
//! line.
//! In exact arithmetic, a2 distinguishes the root
//! cases:
//!       a2 > 1          no solutions
//!       a2 = 1          tangency. The two c,s values
//!                       are identical
//! 0 < = a2 < 1           two solutions.
//! Nearzero values  both positive and negative
//! may be interpreted as near tangencies.
//! When a is near 1, say a = 1 + e (and e is small),
//!       a^2     = 1  2e + e^2
//! For small e this is approximately 1+2e
//! The value of a^2 is computed using only addition of
//! positive quantities, so should be as accurate as
//! the inputs.
//! Therefore ...
//! testing for small absolute value of 1a^2 is a good
//! way to detect near tangencies.
//! If you choose to treat a near tangency as a tangency,
//! it is equivalent to moving the line slightly in the
//! normal direction.
//!
//! @param c1P OUT     x cosine component of first solution point
//! @param s1P OUT     y sine component of first solution point
//! @param c2P OUT     x cosine component of second solution point
//! @param s2P OUT     y sine component of second solution point
//! @param a2P OUT     squared distance to closest approach of line
//! @param alpha IN      constant coefficient on line
//! @param beta IN      x cosine coefficient on line
//! @param gamma IN      y sine coefficient on line
//! @return solution counter
//!
Public GEOMDLLIMPEXP int bsiMath_solveUnitQuadratic
(
double *c1P,
double *s1P,
double *c2P,
double *s2P,
double *a2P,
double alpha,
double beta,
double gamma
);

//!
//! Solve the simultaneous equations
//! <pre>
//!               alpha + beta*c + gamma*s = 0
//!               c*c + s*s = 1
//! </pre>
//! subject to a relative tolerance
//! The return value indicates the following cases:
//! <pre>
//!       -2 > degenerate (beta=gamma=alpha=0) and the
//!               entire unit circle solves the equation
//!       -1 > degenerate (beta=gamma=0) and no
//!               solutions.
//!        0 > no solutions. In this case (c1,s1) is
//!               the closest point on the line, (c2,s2)
//!               is the closest point on the circle
//!        1 > near tangency.  Again (c1,s1) and (c2,s2)
//!               are closest approach points on the
//!               line and circle, respectively.
//!               Application conditions may prefer
//!               to use one or the other as 'the'
//!               solution.
//!        2 > two distinct solutions.
//! </pre>
//!
//! @param c1P OUT     x cosine component of first solution point
//! @param s1P OUT     y sine component of first solution point
//! @param c2P OUT     x cosine component of second solution point
//! @param s2P OUT     y sine component of second solution point
//! @param alpha IN      constant coefficient on line
//! @param beta IN      x cosine coefficient on line
//! @param gamma IN      y sine coefficient on line
//! @param reltol IN      relative tolerance for tangencies
//! @return solution counter
//!
Public GEOMDLLIMPEXP int bsiMath_solveApproximateUnitQuadratic
(
double *c1P,
double *s1P,
double *c2P,
double *s2P,
double alpha,
double beta,
double gamma,
double reltol
);

//!
//! Find the 0, 1, or 2 roots of the quadratic aa*x*x + bb * x + c
//!
//! @param pX OUT     array of 0, 1, or 2 roots
//! @param aa IN      quadratic coefficient
//! @param bb IN      linear coefficient
//! @param cc IN      constant coefficient
//! @return number of roots
//!
Public GEOMDLLIMPEXP int bsiMath_solveQuadratic
(
double *pX,
double aa,
double bb,
double cc
);

//!
//! Find the 0, 1, or 2 root pairs of the convex quadratic
//! solver a00*x0^2 + a01*x0*x1 + a11*x1^2=0.
//! for x0+x1 = 1.
//!
//! @param pX0 OUT     array of 0, 1, or 2 roots
//! @param pX1 OUT     array of 0, 1, or 2 roots
//! @param a00 IN      coefficient of x0^2
//! @param a01 IN      coefficient of x0*x1
//! @param a11 IN      coefficient of x1^2
//! @return number of roots
//!
Public GEOMDLLIMPEXP int bsiMath_solveConvexQuadratic
(
double *pX0,
double *pX1,
double a00,
double a01,
double a11
);

//!
//! Find the angles (on a conic) where the conic is
//! tangent to a circle about the origin and through the conic point.
//! (Not necessarily unit circle, but useful for seeking near approach
//!   to unit circle.)
//! This is equivalent to projecting the origin to the conic.
//!
//! @param pCosValue OUT     0 to 4 cosine values
//! @param pSinValue OUT     0 to 4 sine values
//! @param pThetaValue OUT     0 to 4 angle values
//! @param pNumInt OUT     number of tangencies
//! @param centerx
//! @param ux
//! @param vx
//! @param centery
//! @param uy
//! @param vy
//! @param cenerw
//! @param uw
//! @param vw
//! @return -1 if conic is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int bsiMath_conicTangentUnitCircle
(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
);

//!
//! This routine will find the intersection between a general conic
//! and a unit circle. The conic is in the form of:
//! x = centerx + ux * cos(theta) + vx*sin(theta)
//! y = centery + uy * cos(theta) + vy*sin(theta)
//! w = centerw + uw * cos(theta) + vw*sin(theta)
//!   where centerx, centery, centerw, ux, uy, uw, vx, vy, vw are constants
//!   and    PI < = theta < = PI
//! A unit circle is x^2 + Y^2 = 1
//! Return values: number of solutions found.
//!               0: no intersection
//!               -1: input error or polynomial solver failed.
//!
//! @param pCosValue OUT     0 to 4 cosine values
//! @param pSinValue OUT     0 to 4 sine values
//! @param pThetaValue OUT     0 to 4 angle values
//! @param pNumInt OUT     number of intersections
//! @param centerx
//! @param ux
//! @param vx
//! @param centery
//! @param uy
//! @param vy
//! @param cenerw
//! @param uw
//! @param vw
//! @return -1 if the conic is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int bsiMath_conicIntersectUnitCircle
(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
);

//!
//! This routine finds the points of intersection between an implicit
//! conic (specified by matrix A) X^AX = 0  and the unit circle
//! x^2 + Y^2 = 1
//! Returns  : number of intersections found.
//!            -1: conic = circle or input error or polynomial solver failed.
//!
//! @param pCosValue OUT     x coordinates of intersections
//! @param pSinValue OUT     y coordinates of intersections
//! @param pThetaValue OUT     angular positions of intersections
//! @param pNumInt OUT     number of intersections
//! @param pCoefficientMatrix IN      matrix defining implicit conic
//! @return 0 if success, nonzero if error
//!
Public GEOMDLLIMPEXP StatusInt bsiMath_implicitConicIntersectUnitCircle
(
double          *pCosValue,
double    *pSinValue,
double    *pThetaValue,
int       *pNumInt,
RotMatrixCP pCoefficientMatrix
);

//!
//! Check for easy special cases of polynomial solutions.
//! @param pRoots[1...numRoots] OUT     array of (note the one-based
//!               indexing for compatibility with the full root finder)
//! @param pNumRoots OUT     number of roots
//! @param pCoff[0..degree] IN      coefficients
//! @param degree IN      degree of polynomial
//!
Public GEOMDLLIMPEXP StatusInt bsiSolve_specialCaseRealPolynomial
(
DPoint2dP pRoots,
int         *pNumRoots,
DPoint2dP pCoff,
int         degree
);

//!
//! This routine finds the 0, 1, or 2 solutions of a pair of bilinear
//! equations
//!      axy * x * y + ax * x + ay * y + a = 0
//!      bxy * x * y + bx * x + by * y + b = 0
//!
//! @param pXArray OUT     array of 0, 1, or 2 solutions.  Caller must allocate.
//! @param pYArray OUT     array of 0, 1, or 2 solutions.  Caller must allocate.
//! @param
//!
Public GEOMDLLIMPEXP void bsiMath_solveBilinear
(
double *pXArray,
double *pYArray,
int *pNumSolution,
double axy,
double ax,
double ay,
double a,
double bxy,
double bx,
double by,
double b
);

END_BENTLEY_GEOMETRY_NAMESPACE

