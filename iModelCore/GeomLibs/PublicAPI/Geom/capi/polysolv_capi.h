/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiSolve_polynomialEquation                             |
|                                                                       |
|                                                                       |
| Note:         Given degree m and complex coefficient a[0]...a[m],     |
|               find all m complex roots by Laguer's method.            |
|               a[0] is constant term. a[m] is coefficint for x^m term. |
|               If DPoint2d->y == 0, it is a real number.               |
|                                                                       |
| IMPORTANT!!!  Output roots stars from roots[1] untill roots[m]!!!     |
|               Input a[m] SHOULD NOT BE ZERO!!!                        |
|                                                                       |
| ALSO:         Even m can be any number >= 1, for m=1, 2, it is better |
|               call formula directly. MAX_POLY_ORDER can be set        |
|               up to 100 and still produce stable results!!!           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int        bsiSolve_polynomialEquation
(
DPoint2d        *roots,    /* OUT     all real and complex roots of polynomial */
DPoint2d        *a,        /* IN      polynomial coefficients */
int             m,         /* IN      degree of polynomial >= 1 */
int             polish     /* IN      If true, roots are polished (improved) */
);

//!
//! Note: Given degree m and complex coefficient a[0]...a[m], find all m complex roots by Laguer's method. a[0] is constant term. a[m] is
//! coefficint for x^m term. If DPoint2d->y == 0, it is a real number.
//! IMPORTANT!!! Output roots stars from roots[1] untill roots[m]!!! Input a[m] SHOULD NOT BE ZERO!!!
//! ALSO: Even m can be any number >= 1, for m=1, 2, it is better call formula directly. MAX_POLY_ORDER can be set up to 100 and still produce
//! stable results!!!
//!
Public GEOMDLLIMPEXP int      solve_polynomial_roots
(
DPoint2d        *roots,    /* OUT     all real and complex roots of polynomial */
DPoint2d        *a,        /* IN      polynomial coefficients */
int             m,         /* IN      degree of polynomial >= 1 */
int             polish     /* IN      If true, roots are polished (improved) */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiSolve_quadraticIntersectUnitCircle                   |
|                                                                       |
|                                                                       |
| This routine will find the intersection between a general quadratic   |
| and a unit circle. The quadratic is in the form of:                   |
| Ax^2 + Cxy + By^2 + Ex + Fy + GGG = 0                                 |
| The unit circle is x^2 + Y^2 = 1                                      |
|                                                                       |
| Return values: number of solutions found.                             |
|               0: no intersection                                      |
|               -1: input error or polynomial solver failed.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsiSolve_quadraticIntersectUnitCircle
(
DPoint2d            *pInters,       /* array of 4 elements */
double              AAA,
double              BBB,
double              CCC,
double              EEE,
double              FFF,
double              GGG
);

END_BENTLEY_GEOMETRY_NAMESPACE

