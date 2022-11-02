/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* coeff2 * x^2 + coeff1 * x + coeff0 == 0
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      solve_quadratic_roots
(
double          *rt0,         /* root */
double          *rt1,         /* root */
double          coeff0,
double          coeff1,
double          coeff2,
double          tol           /* used to check if discreminent is negative */
);

/*--------------------------------------------------------------------*//**
@description
    Compute roots of a standard-form polynomial in a specific interval of the
    reals.   For numerical purposes it is desirable for the interval to be similar to 0..1,
    but extension outside that interval is ok.
@param pRootArray OUT array of roots
@param pNumRoot OUT number of roots
@param pStandardCoff IN standard form coefficients, index corresponds to power.
@param degree IN degree of polynomial.  Coefficients are 0..degree.
@param aa IN lower interval limit
@param bb IN upper interval limit
@bsimethod
----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiBezier_univariateStandardRootsInInterval
(
double          *pRootArray,
int             *pNumRoot,
double          *pStandardCoff,
int             degree,
double          aa,
double          bb
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     solve_real_roots01
(
double          *roots,    /* OUT     real roots of polynomial, starting at index 1
                                for compatibility with laguer */
int             *pNumRealRoots, /* OUT     Number of roots returned. */
DPoint2d        *a,        /* IN      polynomial coefficients, ASSUMED TO HAVE COMPLEX PART 0 */
int             m
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspcci_selectRootsInRange
(
double      *reals,         /* OUT     real number in [0, 1] range */
int         *num,           /* IN OUT  num of real numbers in [0, 1] range */
DPoint2d    *roots          /* IN      complex numbers */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bspcurv_minDistToQuadraticBez
(
double          *pMinDist,      /* OUT     mindist achieved */
double          *pParam,        /* OUT     param of closest point */
DPoint3d        *inTestPt,      /* IN      point to calculate dist from */
bool            rational,       /* IN      whether rational curve */
double          a0,             /* IN      coefficents of x */
double          a1,
double          a2,
double          b0,             /* IN      coefictents of y */
double          b1,
double          b2,
double          d0,             /* IN      coefficents of w */
double          d1,
double          d2
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspcurv_minDistToNonRatCubicBez
(
double          *pMinDist,      /* OUT     mindist achieved */
double          *pParam,        /* OUT     param of closest point */
DPoint3d        *inTestPt,      /* IN      point to calculate dist from */
double          a0,             /* IN      coefficents of x */
double          a1,
double          a2,
double          a3,
double          b0,             /* IN      coefictents of y */
double          b1,
double          b2,
double          b3
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspcci_planarXBezier_legacy
(
BezierInfo      *infoP,        /* IN      information about curves */
MSBsplineCurve  *bez0,         /* IN      Bezier curve to consider */
MSBsplineCurve  *bez1          /* IN      Bezier curve to consider */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspcci_planarXBezier
(
BezierInfo      *infoP,        /* IN      information about curves */
MSBsplineCurve  *bez0,         /* IN      Bezier curve to consider */
MSBsplineCurve  *bez1          /* IN      Bezier curve to consider */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_allIntersectsBtwCurves
(
DPoint3d        **intPts,              /* OUT     intersection point(s) on curve */
double          **param0,              /* OUT     param(s) of pts on curve0 */
double          **param1,              /* OUT     param(s) of pts on curve1 */
int             *numPoints,            /* OUT     number of intersections */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_closeIntersect
(
DPoint3d        *intPt,                /* OUT     intersection closest to testPt */
double          *param0,               /* OUT     param of xPt on curve0, or -1 */
double          *param1,               /* OUT     param of xPt on curve0, or -1 */
DPoint3d        *inTestPt,             /* IN      closest to this point */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
RotMatrix       *rotMatrix             /* IN      or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_trueIntersectsBtwCurves
(
DPoint3d        **intPts,              /* OUT     intersection point(s) on curve */
double          **param0,              /* OUT     param(s) of pts on curve0 */
double          **param1,              /* OUT     param(s) of pts on curve1 */
int             *numPoints,            /* OUT     number of intersections */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *toleranceP,
RotMatrix       *rotMatrixP,
bool            useSubdivision
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcci_selfIntersections
(
double          **param,               /* OUT     param(s) of pts on curve */
int             *numPoints,            /* OUT     number of intersections */
MSBsplineCurve  *curve,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspcci_extIntersectCurves
(
DPoint3d        **intPts,              /* OUT     intersection point(s) on curve */
double          **param0,              /* OUT     param(s) of pts on curve0 */
double          **param1,              /* OUT     param(s) of pts on curve1 */
int             *numPoints,            /* OUT     number of intersections */
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
double          *tolerance,
RotMatrix       *rotMatrix,
bool            useSubdivision

);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_countRayIntersectionsExt
(
int                 *numIntersectionP,      /* OUT     number of crossings */
int                 *numRepeatP,            /* OUT     number of crossings with repeated parameter. */
int                 *numStartP,             /* OUT     number of crossings at the ray start */
DPoint3d            *originP,
DPoint3d            *directionP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspcurv_countRayIntersections
(
DPoint3d            *originP,
DPoint3d            *directionP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
);

/*---------------------------------------------------------------------------------**//**
* Classify a point with respect to a curve. 0 -- point on curve 1 -- point not on curve; an arbitrarily chosen ray was found with an odd
* number of crossings. 2 -- point not on curve, an arbitrarily chosen ray was found with an even number of crossings (possibly zero) NOTE:
* Also returns 0 if no clear on, in, out could be found.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspcurv_classifyPoint
(
DPoint3d            *originP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_segmentCurveIntersection
(
int                 *numIntersectionP,      /* OUT     number of crossings */
double              **segmentParamPP,       /* OUT     array of parameters on segment */
double              **curveParamPP,         /* OUT     array of parameters on curve */
DPoint3d            *startP,
DPoint3d            *endP,
MSBsplineCurve      *curveP,
RotMatrix           *rotMatrixP,
double              tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

