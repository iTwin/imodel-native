/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Get the binomial coefficient iCn.  These coefficients are exact up to degree 59
//!   (That's an IEEE double fact!!!)
//! @param i IN      index within row (0..n)
//! @param n IN      row index. (apex row with singleton 1 entry is row zero.)
//!
Public GEOMDLLIMPEXP double     bsiBezier_getBinomialCoefficient
(
int i,
int n
);

//!
//! Convert coefficients of a univariate Bernstein-Bezier polynomial to
//! coefficients in the Power basis.
//!
//! @param    pPowCoff    OUT     coefficients in Power basis
//! @param    pBezCoff    IN      coefficients in Bernstein-Bezier basis
//! @param    order       IN      order of the polynomial
//!
Public GEOMDLLIMPEXP void       bsiBezier_convertBezierToPower
(
double      *pPowCoff,
double      *pBezCoff,
int         order
);

//!
//! Compute the bezier coefficients for a bezier in which the interval [0,1] corresponds
//! to the positive half of the real line for a power basis polynomial.
//! The mapping from u to x is x = u / (1-u).
//!
//! @param    pBezCoff    IN      coefficients in Bernstein-Bezier basis
//! @param    pPowCoff    OUT     coefficients in Power basis
//! @param    order       IN      order of the polynomial
//!
Public GEOMDLLIMPEXP void       bsiBezier_mapPositiveRealsToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
);

//!
//! Compute the bezier coefficients for a bezier in which the interval [0,1] corresponds
//! to the negative half of the real line for a power basis polynomial.
//! The mapping from u to x is x = -u / (1-u).
//!
//! @param    pBezCoff    IN      coefficients in Bernstein-Bezier basis
//! @param    pPowCoff    OUT     coefficients in Power basis
//! @param    order       IN      order of the polynomial
//!
Public GEOMDLLIMPEXP void       bsiBezier_mapNegativeRealsToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
);

//!
//! Convert coefficients of a univariate polynomial in the Power basis to
//! coefficients in the Bernstein-Bezier basis, with identity mapping between
//! x and u.
//!
//! @param    pPowCoff    OUT     coefficients in Bernstein-Bezier basis
//! @param    pBezCoff    IN      coefficients in Power basis
//! @param    order       IN      order of the polynomial
//!
Public GEOMDLLIMPEXP void       bsiBezier_convertPowerToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
);

//!
//! Evaluate the Bernstein-Bezier Basis Functions for a given order.
//!
//! @param    pBi     OUT     array of order basis function values.
//! @param    u       IN      parameter value for evaluation.
//! @param    order   IN      polynomial order (degree + 1)
//!
Public GEOMDLLIMPEXP void       bsiBezier_evaluateBasisFunctions
(
double      *pBi,
int         order,
double      u
);

//!
//! Evaluate basis functions for the derivatives.
//!
//! @param    pDBi     OUT     array of order derivative basis function values.
//! @param    order   IN      polynomial order (degree + 1)
//! @param    u       IN      parameter value
//!
Public GEOMDLLIMPEXP void       bsiBezier_evaluateDerivativeBasisFunctions
(
double      *pdBi,
int         order,
double      u
);

//!
//! Evaluate a Bezier curve by summing the (vector of) Bernstein basis function values.
//!
//! @param    pPoint          OUT     evaluated point
//! @param    pPoles          IN      array of order poles
//! @param    order           IN      polynomial order (degree + 1)
//! @param    numComponent    IN      number of components of each pole vector.
//! @param    u               IN      parameter value for evaluation.
//!
Public GEOMDLLIMPEXP void       bsiBezier_evaluate
(
double      *pPoint,
double      *pPoles,
int         order,
int         numComponent,
double      u
);

//!
//! Evaluate a Bezier curve by summing the (vector of) Bernstein basis function values.
//!
//! @param    pX      OUT     evaluated point
//! @param    pdXdu   OUT     partial derivative wrt u
//! @param    pdXdv   OUT     partial derivative wrt v
//! @param    pd2Xdudv OUT     mixed partial.
//! @param    pPoles      IN      array of order poles
//! @param    orderU      IN      number of u poles
//! @param    strideU     IN      stride between u poles
//! @param    orderV      IN      number of v poles
//! @param    strideV     IN      stride between v poles
//! @param    numComponet     IN      number of components per pole.
//! @param    u           IN      u parameter for evaluation
//! @param    v           IN      v parameter for evaluation
//!
Public GEOMDLLIMPEXP void       bsiBezier_evaluateTensorProduct
(
double      *pX,
double      *pdXdu,
double      *pdXdv,
double      *pd2Xdudv,
const double      *pPoles,
int         orderU,
int         strideU,
int         orderV,
int         strideV,
int         numComponent,
double      u,
double      v
);

//!
//! Compute the poles of the derivative of a Bezier curve
//!
//! @param    pDeriv          OUT     array of order - 1 (degree) poles of the derivative
//! @param    pPoles          IN      array of order poles
//! @param    order           IN      number of poles in function
//! @param    numComponent    IN      number of components of each pole vector.
//!
Public GEOMDLLIMPEXP void       bsiBezier_derivativePoles
(
double      *pDeriv,
double      *pPoles,
int         order,
int         numComponent
);

//!
//! Compute the poles of the integral
//!
//! @param    pIntegral       OUT     array of order + 1 poles of the integral.
//! @param    *pPole0         IN      constant of integration. (First pole of integral, numComponent entries)
//! @param    h               IN      x extent of full interval.
//! @param    pPoles          IN      array of (order) poles of input bezier
//! @param    inOrder         IN      number of input
//! @param    numComponent    IN      number of components of each pole vector.
//!
Public GEOMDLLIMPEXP void       bsiBezier_integralPoles
(
double      *pIntegral,
double      *pPole0,
double      h,
double      *pPoles,
int         order,
int         numComponent
);

//!
//! Compute the poles of the integral
//!
//! @param    pIntegral       OUT     array of order + 1 poles of the integral.
//! @param    f0              IN      constant of integration.
//! @param    h               IN      x extent of full interval.
//! @param    pPoles          IN      array of (order) poles of input bezier
//! @param    inOrder         IN      number of input
//!
Public GEOMDLLIMPEXP void       bsiBezier_univariateIntegralPoles
(
double      *pIntegral,
double      f0,
double      h,
double      *pPoles,
int         order
);

//!
//! Raise the degree by 1.  May be called with the same array, with output occupying
//! the input pole memory plus one additional position.
//!
//! @param    pB          OUT     order + 1 poles
//! @param    pA          IN      order poles
//! @param    order           IN      input curve order
//! @param    numComponent    IN      number of components of each pole vector.
//!
Public GEOMDLLIMPEXP void       bsiBezier_raiseDegree
(
double          *pB,
const double    *pA,
int         order,
int         numComponent
);

//!
//! Repeated degree raising in  place.
//!
//! @param    pB              IN OUT  poles
//! @param    inOrder         IN      input curve order
//! @param    outOrder        IN      output curve order
//! @param    numComponent    IN      number of components of each pole vector.
//!
Public GEOMDLLIMPEXP bool       bsiBezier_raiseDegreeInPlace
(
double          *pB,
int             orderIn,
int             orderOut,
int             numComponent
);

//!
//! Reduce degree of a univariate bezier, optionally working right to left or left to right.
//! Expect to call this twice and compare results to see if degree is actually reducible!!
//!   Do NOT call with inplace!!!
//! @param pA OUT     reduced degree coefficients.
//! @param pB IN      higher degree coefficients.
//! @param orderB IN      order of B.
//! @param numCoff IN      number of coefficients per pole.
//! @param reverseDirection IN      true to sweep from right to left.
//!
Public GEOMDLLIMPEXP bool        bsiBezier_reduceDegreeDirectional
(
      double    *pA,
const double    *pB,
      int       orderB,
      int       numCoff,
      bool      reverseDirection
);

//!
//! Reduce degree of a univariate bezier, working in both directions and averaging results.
//! @param pA OUT     reduced degree coefficients.
//! @param pB IN      higher degree coefficients.
//! @param orderB IN      order of B.
//!
Public GEOMDLLIMPEXP bool        bsiBezier_reduceDegree
(
      double    *pA,
      double    *pError,
const double    *pB,
      int       orderB,
      int       numComponent
);

//!
//! @description Compute the sum of coefficients scaled by the binomial coefficients and alternating signs
//! @param pSignedSum OUT sum of signed coefficients times binomial coefficients times (-1)^i.
//!            (This is the leading coefficient of the standard basis form of the polynomial)
//! @param pAbsUm OUT sum of absolute values of coefficients times binomial coeffients.
//!        (This is a useful indication of size of coefficient, to determine of the signed sum is
//!            close to zero in a relative sense.)
//! @param pB IN coefficients
//! @param orderB order of polynomial
//! @param numComponent number of components per pole.  Also the number of components of the output
//!        signed sum and abs sum.
//!
Public GEOMDLLIMPEXP void     bsiBezier_sumScaledCoefficients
(
      double    *pSignedSum,
      double    *pAbsSum,
const double    *pB,
      int       orderB,
      int       numComponent
);

//!
//! Return orderB-2 coefficients of the (Bezier form) coefficients of the polynomial
//! remaining after the following reductions:
//! 1) Subtract the straight line between endpoints.
//! 2) factor out (1-u)*u from the deviation.
//!
//! (OR: Return the residual after extraction of the leading (linear)part of the
//!       symmetric basis {u, 1-u, u(1-u)u, u(1-u)(1-u), ... U^k u, U^k (1-u)}
//! where the k'th pair of basis functions is U^k=u^k (1-u)^k=(u(1-u))^k.
//!
//! @param pA OUT     orderB - 2 reduced degree coefficients.
//! @param pB IN      higher degree coefficients.
//! @param orderB IN      order of B.
//!
Public GEOMDLLIMPEXP bool        bsiBezier_symmetricRemainder
(
      double    *pA,
const double    *pB,
      int       orderB,
      int       numComponent
);

//!
//! Copy (all coordinates of) poles
//!
//! @param    pDest           IN      destination pole array
//! @param    iDest           IN      pole index within destination
//! @param    pSource         IN      source bezier
//! @param    iSource         IN      index of pole in source
//! @param    numCopy         IN      number of poles to copy
//! @param    numComponent    IN      number of components per pole
//!
Public GEOMDLLIMPEXP void       bsiBezier_copyPoles
(
double      *pDest,
int         iDest,
double      *pSource,
int         iSource,
int         numCopy,
int         numComponent
);

//!
//! Copy one component from a multicomponent source to a multicomponent destination.
//!
//! @param    pDest               IN      destination pole array
//! @param    jDest               IN      destination component index
//! @param    numComponentDest    IN      number of components in the destination
//! @param    pSource             IN      source pole array
//! @param    jSource             IN      source component index
//! @param    numComponentSource  IN      number of components in the source
//! @param    numCopy             IN      number of doubles to copy (a.k.a. order)
//!
Public GEOMDLLIMPEXP void       bsiBezier_copyComponent
(
double      *pDest,
int         jDest,
int         numComponentDest,
const double      *pSource,
int         jSource,
int         numComponentSource,
int         numCopy
);

//!
//! Form a linear combination of two poles (from the same Bezier curve).
//!
//! @param    pDest           IN OUT  destination pole array
//! @param    iDest           IN      pole index within destination
//! @param    pSource         IN      source bezier
//! @param    iSource0        IN      index of first point in source
//! @param    u0              IN      scale factor for first point
//! @param    iSource1        IN      index of second point in source
//! @param    u1              IN      scale factor for second point
//! @param    numComponent    IN      number of components per pole
//!
Public GEOMDLLIMPEXP void       bsiBezier_addScaledPoles
(
double      *pDest,
int         iDest,
double      *pSource,
int         iSource0,
double      scale0,
int         iSource1,
double      scale1,
int         numComponent
);

//! Compute the function value -- like beiBezier_functionAndDerivative for special case numComponent==1
//! @param    pP              OUT     function value at u
//! @param    pD              OUT     derivative at u
//! @param    pPoles          IN      control polygon
//! @param    order           IN      number of poles
//! @param    u               IN      parameter value
Public GEOMDLLIMPEXP bool    bsiBezier_evaluateUnivariate
(
double      *pP,
double      *pD,
double      *pPoles,
int         order,
double      u
);

//!
//! Compute the function value and first derivative using deCasteljau algorithm.
//!
//! @param    pP              OUT     function value at u
//! @param    pD              OUT     derivative at u
//! @param    pK              OUT     second derivative at u
//! @param    pPoles          IN      control polygon
//! @param    order           IN      number of poles
//! @param    numComponent    IN      number of components per pole
//! @param    u               IN      parameter value
//!
Public GEOMDLLIMPEXP void       bsiBezier_functionAndDerivativeExt
(
double      *pP,
double      *pD,
double      *pK,
double      *pPoles,
int         order,
int         numComponent,
double      u
);

//!
//! Compute the function value and first derivative using deCasteljau algorithm.
//!
//! @param    pP              OUT     function value at u
//! @param    pD              OUT     derivative at u
//! @param    pPoles          IN      control polygon
//! @param    order           IN      number of poles
//! @param    numComponent    IN      number of components per pole
//! @param    u               IN      parameter value
//!
Public GEOMDLLIMPEXP void       bsiBezier_functionAndDerivative
(
double      *pP,
double      *pD,
double      *pPoles,
int         order,
int         numComponent,
double      u
);

//!
//! Compute the control points of the two polygons for curves from 0..u and u..1
//!
//! @param    pLeft           OUT     left polygon control points
//! @param    pRight          OUT     right polygon control points
//! @param    pPoles          IN      full polygon control points
//! @param    order           IN      number of poles
//! @param    numComponent    IN      number of components per pole
//! @param    u               IN      subdivision parameter
//!
Public GEOMDLLIMPEXP void       bsiBezier_subdivisionPolygons
(
double      *pLeft,
double      *pRight,
double      *pPoles,
int         order,
int         numComponent,
double      u
);

//!
//! Compute the (univariate) control points of the dot product of two vector bezier functions.
//!
//! @param    pAB             OUT     orderAB poles
//! @param    pOrderAB        OUT     order of dot product
//! @param    maxOrderAB      IN      permitted order of product.
//! @param    pA              IN      control points for bezier B
//! @param    orderA          IN      number of control points for bezier A
//! @param    firstComponentA IN      first component index to use in A
//! @param    numComponentA   IN      number of components in bezier A (may be more than components used in
//!                                   dot product)
//! @param    pB              IN      control points for bezier B
//! @param    orderB          IN      number of control points for bezier B
//! @param    firstComponentB IN      first component index to use in B
//! @param    numVectorComponent IN      number of vector components
//!
Public GEOMDLLIMPEXP bool       bsiBezier_dotProduct
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
double      *pA,
int         orderA,
int         firstComponentA,
int         numComponentA,
double      *pB,
int         orderB,
int         firstComponentB,
int         numComponentB,
int         numVectorComponent
);

//!
//! Compute the (univariate) control points of the cross product of two components
//! of vector functions, i.e the difference A[iA0] * B[iB1] - A[iA1] * B[iB0]
//!
//! (Typically, A and B will be addressed similarly, so iA0==iB0==i0 and iA1==iB1==i1, in the
//!  usual cyclic combinations of (iAB, i0, i1) of (0,1,2), (1,2,0), and (2,0,1)).
//!
//! @param    pAB             OUT     orderAB poles
//! @param    pOrderAB        OUT     order of cross product
//! @param    iAB             IN      component index in AB
//! @param    numComponentAB  IN      number of components in poles of fAB
//! @param    maxOrderAB      IN      permitted order of product.
//! @param    pA              IN      control points for bezier B
//! @param    orderA          IN      number of control points for bezier A
//! @param    iA0             IN      first component index to use in A
//! @param    iA1             IN      second component index to use in A
//! @param    numComponentA   IN      number of components in  poles of A
//! @param    pB              IN      control points for bezier B
//! @param    orderB          IN      number of control points for bezier B
//! @param    iAB             IN      first component index to use in B
//! @param    iAB             IN      second component index to use in B
//! @param    numComponentB   IN      number of components in poles of B
//!
Public GEOMDLLIMPEXP bool       bsiBezier_crossProductComponent
(
double      *pAB,
int         *pOrderAB,
int         iAB,
int         numComponentAB,
int         maxOrderAB,
double      *pA,
int         orderA,
int         iA0,
int         iA1,
int         numComponentA,
double      *pB,
int         orderB,
int         iB0,
int         iB1,
int         numComponentB
);

//!
//! Compute the (1-D) control points of the product of two univariate Bezier curves.
//! Only component componentAB is computed in the numComponentAB-dimensional output array.
//! Returns true if product curve's order is within allowable range.
//!
//! @param    pAB             OUT     (orderA - 1) + (orderB - 1) + 1 control points of product.
//! @param    componentAB     IN      component to compute in product
//! @param    numComponentAB  IN      number of components in product
//! @param    pA              IN      control points for bezier B
//! @param    orderA          IN      number of control points for bezier B
//! @param    componentA      IN      component index within bezier A
//! @param    numComponentA   IN      number of components in bezier A
//! @param    pB              IN      control points for bezier B
//! @param    orderB          IN      number of control points for bezier B
//! @param    componentB      IN      component index within bezier B
//! @param    numComponentB   IN      number of components in bezier B
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateProduct
(
double      *pAB,
int         componentAB,
int         numComponentAB,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
);

//!
//! Compute the (1-D) control points of the product of two univariate Bezier curves.
//! Only component componentAB is computed in the numComponentAB-dimensional output array.
//! Returns true if product curve's order is within allowable range.
//!
//! @param    pAB             OUT     (orderA - 1) + (orderB - 1) + 1 control points of product.
//! @param    *pOrderAB       OUT     order of product.
//! @param    maxOrderAB      IN      allowed order of product.
//! @param    componentAB     IN      component to compute in product
//! @param    numComponentAB  IN      number of components in product
//! @param    pA              IN      control points for bezier B
//! @param    orderA          IN      number of control points for bezier B
//! @param    componentA      IN      component index within bezier A
//! @param    numComponentA   IN      number of components in bezier A
//! @param    pB              IN      control points for bezier B
//! @param    orderB          IN      number of control points for bezier B
//! @param    componentB      IN      component index within bezier B
//! @param    numComponentB   IN      number of components in bezier B
//!
Public GEOMDLLIMPEXP bool       bsiBezier_univariateProductExt
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
int         componentAB,
int         numComponentAB,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
);

//!
//! Compute the (1-D) control points of the product of two univariate Bezier curves.
//! Accumulate the product to another polynomial of the same order.
//! Only component componentAB is computed in the numComponentAB-dimensional output array.
//! Returns true if product curve's order is within allowable range.
//!
//! @param    pAB             IN OUT  (orderA - 1) + (orderB - 1) + 1 control points of accumulating
//!                                   product.
//! @param    componentAB     IN      component to compute in product
//! @param    numComponentAB  IN      number of components in product
//! @param    coff            IN      scalar multiplier to apply as to accumulated terms.
//! @param    pA              IN      control points for bezier B
//! @param    orderA          IN      number of control points for bezier B
//! @param    componentA      IN      component index within bezier A
//! @param    numComponentA   IN      number of components in bezier A
//! @param    pB              IN      control points for bezier B
//! @param    orderB          IN      number of control points for bezier B
//! @param    componentB      IN      component index within bezier B
//! @param    numComponentB   IN      number of components in bezier B
//!
Public GEOMDLLIMPEXP bool       bsiBezier_accumulateUnivariateProduct
(
double      *pAB,
int         componentAB,
int         numComponentAB,
double      coff,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
);

//!
//! Compute the control points of the product of a single bezier basis function B_i^n
//! multiplying a multicomponent (curve) bezier (i.e. poles of a curve).
//!
//! @param    pAB             OUT     (orderA - 1) + (orderB - 1) + 1 control points of product.
//! @param    pOrderAB        OUT     order of product
//! @param    maxOrderAB      IN      max poles allowed in the product
//! @param    pA              IN      control points for bezier A
//! @param    orderA          IN      order of polynomials in A
//! @param    numComponent    IN      number of components in A and AB.
//! @param    coffB           IN      numeric coefficient for the basisi function.
//! @param    indexB          IN      power of u in B_i^n
//! @param    orderB          IN      order of B
//!
Public GEOMDLLIMPEXP bool     bsiBezier_basisProduct
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
const double *pA,
int         orderA,
int         numComponent,
const double coffB,
int         indexB,
int         orderB
);

//!
//! Compute the poles of the (Bezier curve) difference of two Bezier curves of
//! shared order.
//!
//! @param    pDiff           OUT     control points for bezier A - bezier B
//! @param    pA              IN      control points for bezier A
//! @param    pB              IN      control points for bezier B
//! @param    order           IN      number of control points in A, B, AB
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_subtractPoles
(
double      *pDiff,
double      *pA,
double      *pB,
int         order,
int         numComponent
);

//!
//! Zero out poles of a bezier.
//! @param order IN      number of poles.
//! @param numComponent IN      number of components per pole.
//!
Public GEOMDLLIMPEXP void       bsiBezier_zeroPoles
(
double      *pA,
int         order,
int         numComponent
);

//!
//! Add the poles of polynomial pA to those of pSum, replacing pSum.
//!
//! @param    pSum            IN OUT  control points for sum.
//! @param    pA              IN      control points for bezier A
//! @param    order           IN      number of control points in A, B, AB
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_addPolesInPlace
(
double      *pSum,
double      *pA,
int         order,
int         numComponent
);

//!
//! Add the poles of polynomial pA to those of pB, replacing pSum.
//!
//! @param    pSum            IN OUT  control points for sum.
//! @param    pA              IN      control points for bezier A
//! @param    pB              IN      control points for bezier B
//! @param    order           IN      number of control points in A, B, sum
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_addPoles
(
double      *pSum,
const double *pA,
const double *pB,
int         order,
int         numComponent
);

//!
//! Add the scaled poles of polynomial pA to those of pB
//!
//! @param    pSum            OUT     control points for sum.
//! @param    pA              IN      control points for bezier A
//! @param    scaleA          IN      scale factor for bezier A
//! @param    pB              IN      control points for bezier B
//! @param    scaleB          IN      scale factor for bezier B
//! @param    order           IN      number of control points in A, B, sum
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_addPoles
(
double      *pSum,
const double *pA,
double scaleA,
const double *pB,
double scaleB,
int         order,
int         numComponent
);


//!
//! Subtract the poles of polynomial pA from those of pSum, replacing pSum.
//!
//! @param    pSum            IN OUT  control points for sum.
//! @param    pA              IN      control points for bezier A
//! @param    order           IN      number of control points in each bezier
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_subtractPolesInPlace
(
double      *pSum,
double      *pA,
int         order,
int         numComponent
);

//!
//! Add a constant vector to all poles of a Bezier curve.
//!
//! @param    pDiff           OUT     control points for bezier + delta
//! @param    pA              IN      control points for bezier
//! @param    pDelta          IN      constant delta to apply (array must have numComponent entries)
//! @param    order           IN      number of control points
//! @param    numComponent    IN      number of components per control point.
//!
Public GEOMDLLIMPEXP void       bsiBezier_addConstant
(
double      *pDiff,
double      *pA,
double      *pDelta,
int         order,
int         numComponent
);

//!
//! Return the product of two entries in a pascal's triangle row.
//! @param i  IN      first index in row, 0<=i<=n
//! @param j  IN      second index in row, 0<=j<=n
//! @param n  IN      row index.  Each row has n+1 entries.
//! @return iCn * jCn
//!
Public GEOMDLLIMPEXP double   bsiBezier_pascalProductInRow
(
int         i,
int         j,
int         n
);

//!
//! Fill a dense, row-major matrix with values
//!       A[i][j] = iCm * jCn / ( (i+j)C(m+n) * (m + n + 1)
//!   for 0 < i < (m+1) and 0 < j < (n+1)
//!
//! @param pA IN      matrix pointer.   Filled with (m+1)*(n+1) values.
//! @param maxA IN      number of doubles the A can hold.
//! @param rowOrder IN      number of rows of A.
//! @param colOrder IN      number of columns of A.
//! @return true if all dimensions are non-negative and dimensional
//!       limits are satisfied.
//!
Public GEOMDLLIMPEXP bool     bsiBezier_pascalRegressionMatrix
(
double      *pA,
int         maxA,
int         rowOrder,
int         colOrder
);

//!
//! Compute control points for univariate interpolation.
//! @param pA OUT     array of poles.
//! @param pY IN      array of function values.
//! @return true if poles computed.
//!
Public GEOMDLLIMPEXP bool     bsiBezier_univariateInterpolationPoles
(
double          *pA,
const double    *pY,
int             n
);

//!
//! Integrate a hermite polynomial.
//! @param h IN interval size.  The bezier parameterizes the interval from 0 to 1.
//! @param fa IN array of function, first derivative, etc, at start of interval.
//! @param fb IN array of function, first derivative, etc, at end of interval.
//! @param numDerivative IN number of derivatives.  If larger than 6, only first 6 are used.  0 is valid.
//! @return true if poles computed.
//!
Public GEOMDLLIMPEXP double bsiQuadrature_hermiteIntegral
(
double h,
double fa[],
double fb[],
int numDerivative
);

//!
//! Compute control points for a hermite curve fit.
//! @param pBezCoffs OUT computed poles
//! @param h IN interval size.  The bezier parameterizes the interval from 0 to 1.
//! @param fa IN array of function, first derivative, etc, at start of interval.
//! @param fb IN array of function, first derivative, etc, at end of interval.
//! @param numDerivative IN number of derivatives.
//! @return true if poles computed.
//!
Public GEOMDLLIMPEXP int bsiBezier_univariateHermiteFitPoles
(
double *pBezCoffs,
double h,
double fa[],
double fb[],
int numDerivative
);

//!
//! Return the bezier polynomials of the nurbs basis over a single interval of a nurbs.
//! @param pBasisCoffs OUT array of order * order doubles.   Each block of order doubles
//!        are the bezier coefficient for a basis function over the interval.
//!    The bezier independent variable is a local 0..1 space within
//!        pCompleteKnots[knotIndex0] < t < pCompleteKnots[knotIndex0 + 1]
//! The bezier curve over the interval is a linear combination of the control points times
//!    these beziers,
//!        P(u} = P[K]*bezier[0] + P[K+1] * bezier[1] + .....
//!    where knot0 = K + order - 1,   i.e.    K = knot0 - order + 1
//! @param
//! @return true if basis completed.
//!
Public GEOMDLLIMPEXP bool    bsiNurbs_singleIntervalBasisFunctions
(
double *pBasisPolys,
double *pCompleteKnots,
int knotIndex0,
int order
);

//!
//! Return the bezier polynomial for a single span of a nurbs curve.
//! @param pBezierControlPoints OUT numerator control points for bezier curve.
//! @param pBezierWeights OUT denominator control points
//! @param pNURBSControlPoints OUT numerator control points for NURBS curve
//! @param pNURBSWeights OUT denominator control points for NURBS curve.  May be NULL
//! @param pCompleteKnots IN
//! @param pCompleteKnots IN complete knot vector, i.e. in clamped end case end knot has muliplicity = order
//! @return true if basis completed.
//!
#ifdef COMMENT_OUT_NOT_USED

Public GEOMDLLIMPEXP bool    bsiNurbs_singleIntervalAsBezier
(
double *pBezierControlPoints,
double *pBezierWeights,
double *pNURBSControlPoints,
double *pNURBSWeights,
int    numComponent,
int    pointIndex0,
double *pCompleteKnots,
int order
);

#endif

END_BENTLEY_GEOMETRY_NAMESPACE

