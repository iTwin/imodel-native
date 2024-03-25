/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Remove roots if they lie outside parameter range.
//!
//! @param pRootArray IN OUT  initial / filtered roots
//! @param pNumRoot   IN OUT  # initial roots / # roots after filtering
//!
Public GEOMDLLIMPEXP void bsiBezier_filterRootRange
(
double  *pRootArray,
int     *pNumRoot
);

//!
//! Finds the first root of the control polygon for a Bezier function. For example,
//! if the degree d Bezier curve C(t) = [X(t), Y(t), Z(t)] has the (weighted) poles
//! b_i = [x_i, y_i, z_i], then X(t) is a degree d Bezier function with poles
//! (coefficients) x_i, and the control polygon for this function has vertices at
//! the points (i/d, x_i).  Similarly for Y(t) and Z(t).
//!
//! @param pRoot      OUT     the (one) root found
//! @param pA         IN      coefficients (Bezier ordinates) of the Bezier function
//! @param order      IN      Bezier order
//!
Public GEOMDLLIMPEXP bool    bsiBezier_firstPolygonRoot
(
double      *pRoot,
double      *pA,
int         order
);

//!
//! Approximate the root (if present) of a monotonic Bezier function using smallest
//! root of the control polygon as an initial guess for Newton-Raphson.
//!
//! @param pRoot      OUT     approximation to the root (if any) in [0,1].
//! @param pA         IN      coefficients of the Bezier function
//! @param order      IN      Bezier order
//! @return true if and only if Newton-Raphson quickly converges.
//!
Public GEOMDLLIMPEXP bool    bsiBezier_findRootUsingMonotonicPolygon
(
double      *pRoot,
double      *pA,
int         order
);

//!
//! Approximate the root (if there is one) of a Bezier function in the interval
//! [lowerLimit,upperLimit], on which the function is monotonic.
//!
//! @param pRoot      OUT     the root, if found in the interval
//! @param pA         IN      coefficients of the full Bezier curve
//! @param order      IN      Bezier order
//! @param lowerLimit IN      low end of interval >= 0.0
//! @param upperLimit IN      high end of interval OUT     1.0
//! @return true if and only if a root was found
//!
Public GEOMDLLIMPEXP bool    bsiBezier_rootInMonotonicInterval
(
double      *pRoot,
double      *pA,
int         order,
double      lowerLimit,
double      upperLimit
);

//!
//! Find the range of a Bezier function and count upward and downward changes in the
//! control polygon.
//!
//! @param pAMin      OUT     smallest coefficient
//! @param pAMax      OUT     largest coefficient
//! @param pNumUp     OUT     number of consecutive pairs of increasing coefficients
//! @param pNumDown   OUT     number of consecutive pairs of decreasing coefficients
//! @param pNumZero   OUT     number of consecutive pairs of equal coefficients
//! @param pA         IN      coefficients of the Bezier function
//! @param order      IN      Bezier order
//!
Public GEOMDLLIMPEXP void       bsiBezier_evaluateRangeAndShape
(
double          *pAMin,
double          *pAMax,
int             *pNumUp,
int             *pNumDown,
int             *pNumZero,
double          *pA,
int             order
);

//!
//! Find the (up to order-1) roots of a univariate Bezier curve over [0,1], not counting
//! multiplicities.  If identically zero, return order "roots" equispaced in [0,1].
//!
//! @param pRootArray     OUT     array of roots.  Caller must allocate with size >= order
//! @param pNumRoot       OUT     number of roots
//! @param pA             IN      coefficients of the bezier
//! @param order          IN      bezier order > 1
//! @param bPreferAnalytic IN      true to favor using analytic solutions for cubic and quartic.
//! @return   false if order invalid
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRootsOptionalAnalytic
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order,
bool            bPreferAnalytic
);

//!
//! Find the (up to order-1) roots of a univariate Bezier curve over [0,1], not counting
//! multiplicities.  If identically zero, return order "roots" equispaced in [0,1].
//!
//! @param pRootArray     OUT     array of roots.  Caller must allocate with size >= order
//! @param pNumRoot       OUT     number of roots
//! @param pA             IN      coefficients of the bezier
//! @param order          IN      bezier order > 1
//! @return   false if order invalid
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRoots
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order
);

//!
//! Find the (up to degree) roots of a univariate polynomial over the entire real axis.
//! The polynomial is summation of pStandardCoff[k]*x^k.
//!
//! @param pRootArray     OUT     array of roots.  Caller must allocate with size >= order
//! @param pNumRoot       OUT     number of roots
//! @param pStandarddCoff IN      degree + 1 standard form coefficients of the polynomial
//! @param degree         IN      degree of polynomial
//! @return   false if order invalid
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateStandardRoots
(
double          *pRootArray,
int             *pNumRoot,
double          *pStandardCoff,
int             degree
);

//!
//! Find the (up to degree) roots of a bezier polynomial over the entire real axis.
//!
//! @param pRootArray     OUT     array of roots.  Caller must allocate with size >= order
//! @param pNumRoot       OUT     number of roots
//! @param pBezCoff       IN      degree + 1 bezier coefficients of the polynomial
//! @param degree         IN      degree of polynomial
//! @return   false if order invalid
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRootsExt
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order
);

//!
//! Conditionally look for a simple roots very just outside an endpoint; ADD the root to
//!  the root array.  A root is considered simple if a simple newton iteration leads
//!   there VERY quickly, and it is within specified tolerance of the endpoint.
//!
//!
//! @param pRootArray     OUT     array of roots.  Caller must allocate with size >= order
//! @param pNumRoot       OUT     number of roots
//! @param pBezCoff       IN      degree + 1 bezier coefficients of the polynomial
//! @param degree         IN      degree of polynomial
//! @param select         IN      0 or 1, selects corresponding endpoint of 01 interval.
//! @param insideEpsilon  IN      skip if there is already a root this close to endpoint.
//!                           If 0 is given, a default will be used.
//! @param outsideEpsilon IN      accept new root of on outside and this close.
//!                           If 0 is given, a default will be used.
//! @return   true if a root was added.
//!
Public GEOMDLLIMPEXP bool       bsiBezier_addRootNearEndpoint
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order,
int             select,
double          insideEpsilon,
double          outsideEpsilon
);

//!
//! This routine finds the solutions of a quartic bezier expressed
//! as a quadric form over quadratic bezier basis functions, i.e.
//! f(u) = sum A[i][j]*B[i](u)*B[j](u)
//! where sums are over 0..2, A[i][j] is a coefficient from the matrix,
//! and B[i](u) is the i'th quadratic bezier basis function at u.
//! Returns  : number of intersections found.
//!
//! @param pCosArray OUT     x coordinates of intersections
//! @param pSinArray OUT     y coordinates of intersections
//! @param pAngleArray OUT     angular positions of intersections
//! @param pNumInt OUT     number of intersections
//! @param pCoff0 OUT     array of bezier coefficients for upper half circle.
//! @param pCoff1 OUT     array of bezier coefficients for lower half circle.
//! @param pA IN      matrix defining implicit conic
//! @return -1 if matrix pA is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int bsiBezier_solveTrigForm
(
double      *pCosArray,
double      *pSinArray,
double      *pAngleArray,
int         *pNumAngle,
double      *pCoff0,
double      *pCoff1,
RotMatrixCP  pA
);

//!
//! This routine finds the points of intersection between an implicit
//! conic (specified by matrix A) X^AX = 0  and the unit circle
//! x^2 + Y^2 = 1
//! Returns  : number of intersections found.
//!            -1: conic = circle or input error or polynomial solver failed.
//!
//! @param pCosArray OUT     x coordinates of intersections in unit circle system
//! @param pSinArray OUT     y coordinates of intersections in unit circle system
//! @param pAngleArray OUT     angular positions of intersections  in unit circle
//! @param pNumAngle OUT     number of intersections. (0 to 4)
//! @param pCoefficientMatrix IN      matrix defining implicit conic
//! @return -1 if matrix pA is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int    bsiBezier_implicitConicIntersectUnitCircle
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
double     *pCoff0,
double     *pCoff1,
RotMatrixCP pCoefficientMatrix
);

//!
//! This routine finds the points of intersection between an implicit
//! conic (specified by matrix A) X^AX = 0  and the unit circle
//! x^2 + Y^2 = 1
//! Returns  : number of intersections found.
//!            -1: conic = circle or input error or polynomial solver failed.
//!
//! @param pCosArray OUT     x coordinates of intersections in unit circle system
//! @param pSinArray OUT     y coordinates of intersections in unit circle system
//! @param pAngleArray OUT     angular positions of intersections  in unit circle
//! @param pNumAngle OUT     number of intersections. (0 to 4)
//! @param pCoefficientMatrix IN      matrix defining implicit conic
//! @return -1 if matrix pA is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int    bsiBezier_implicitConicIntersectUnitCircleShort
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
RotMatrixCP pCoefficientMatrix
);

//!
//! This routine finds the points of intersection between a parametric
//! conic specified as the (2d, homogeneous) points
//!   B.column[0]*cos(theta) + B.column[1]*sin(theta) + B.column[2]
//!  and the unit circle
//!       X^2 + Y^2 = W^2
//! Returns  : number of intersections found.
//!            -1: conic = circle or input error or polynomial solver failed.
//!
//! @param pCosArray OUT     cosine coordinates of intersections in conic frame
//! @param pSinArray OUT     sine coordinates of intersections in conic frame
//! @param pAngleArray OUT     angular positions of intersections in conic parameter space
//! @param pNumAngle OUT     number of intersections
//! @param pB IN      matrix defining parametric conic.
//! @return -1 if pB is (exactly) a unit circle,
//!               else number of intersections.
//!
Public GEOMDLLIMPEXP int    bsiBezier_conicIntersectUnitCircle
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
double     *pCoff0,
double     *pCoff1,
RotMatrixCP pB
);

END_BENTLEY_GEOMETRY_NAMESPACE

