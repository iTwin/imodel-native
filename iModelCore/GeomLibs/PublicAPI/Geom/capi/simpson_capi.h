/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveSimpson                                |
|                                                                       |
|                                                                       |
| Note          This is an adaptive method and no value is evaluated    |
|               twice during the calculation                            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveSimpson
(
double          *integralP,         /* OUT     resulting integration */
double          *errorP,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     number of function calls */
double          x0,                 /* IN      lower limit for integration */
double          x1,                 /* IN      upper limit for integration */
double          absTol,             /* IN      relative tolerance required */
double          relTol,             /* IN      relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* IN      evaluate function for integrand */
void            *userDataP          /* IN      passed through to evaluateFunc */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveSimpson                                |
|                                                                       |
|                                                                       |
| Note          This is an adaptive method and no value is evaluated    |
|               twice during the calculation                            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5
(
double          *integralP,         /* OUT     resulting integration */
double          *errorP,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     function count */
double          x0,                 /* IN      lower limit for integration */
double          x8,                 /* IN      upper limit for integration */
double          absTol,             /* IN      relative tolerance required */
double          relTol,             /* IN      relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* IN      evaluate function for integrand */
void            *userDataP          /* IN      passed through to evaluateFunc */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotesIntegration         |
|                                                                       |
| Integrate from u0 to u1, using polynomial approximations in binary    |
| subdivision of the interval x0 to x1.                                 |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiMath_recursiveNewtonCotesIntegration
(
double          *pIntegral,         /* OUT     resulting integration */
double          *pError,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     function count */
void    *pUserData,
PFScalarIntegrand cbEvaluate,
int     numPoints,
double  x0,
double  x1,
double u0,
double u1,
double absErr,
double relErr
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMath_recursiveIncrementalNewtonCotes5               |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveIncremantalNewtonCotes5
(
double          *integralP,         /* OUT     resulting integration */
double          *errorP,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     function count */
double          x0,                 /* IN      lower limit for integration */
double          x8,                 /* IN      upper limit for integration */
double          absTol,             /* IN      relative tolerance required */
double          relTol,             /* IN      relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* IN      evaluate function for integrand */
PFExtendScalarIntegration extendFunc,/* IN      function to receive incremental
results for further interpretation by the caller.
Parameters of each call are
extendFunc (x, f, g, num, userDataP)
                                          where
                                                g0 = integral prior to x[0].
                                                x[0..n-1] is a vector of x values.
                                                f[0..n-1] are corresponsing integrand values
                                                g[0..n-1] are corresponding partial integrations
                                                            from x[0] to x[i]. (Yes, g[0] is 0)
                                                userDataP is as in evaluateFunc
                                        The extend function is called in strictly left-to-right
                                            sequence; the x, f, and g values will cover the entire
                                            interval.
                                          */
void            *userDataP          /* IN      passed through to evaluateFunc */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotes5                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5Vector
(
double          *pIntegral,         /* OUT     resulting integration */
double          *pError,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     function count */
double          x0,                 /* IN      lower limit for integration */
double          x8,                 /* IN      upper limit for integration */
double          absTol,             /* IN      relative tolerance required */
double          relTol,             /* IN      relative tolerance required */
PFVectorIntegrand evaluateFunc,     /* IN      evaluate function for integrand */
void            *userDataP,         /* IN      passed through to evaluateFunc */
int             numFunc             /* IN      number of functions being integrated */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotes5                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5Vector
(
double          *pIntegral,         /* OUT     resulting integration */
double          *pError,            /* OUT     achived absolute error */
int             *pCount,            /* OUT     function count */
double          x0,                 /* IN      lower limit for integration */
double          x8,                 /* IN      upper limit for integration */
double          absTol,             /* IN      relative tolerance required */
double          relTol,             /* IN      relative tolerance required */
BSIVectorIntegrand &integrand
);


END_BENTLEY_GEOMETRY_NAMESPACE

