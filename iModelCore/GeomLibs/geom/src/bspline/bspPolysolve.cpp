/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspPolysolve.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// REMARK: These functions pulled from bspcci.c when recursive subdivision was replaced.
// Called only by bsprsurf.
#define fc_coeffTol	    1.0e-20
#define                     MINIMUM_Parameter               -1.0E-4
#define                     MAXIMUM_Parameter               1.0001
#define fc_realNumberTol    0.05

/*---------------------------------------------------------------------------------**//**
* coeff2 * x^2 + coeff1 * x + coeff0 == 0
* @bsimethod                                                    Lu.Han          06/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      solve_quadratic_roots
(
double          *rt0,         /* root */
double          *rt1,         /* root */
double          coeff0,
double          coeff1,
double          coeff2,
double          tol           /* used to check if discreminent is negative */
)
    {
    double      det, denom;

    /* Check if only linear equation */
    if (coeff2 == 0.0 || fabs (coeff0/coeff2) > 1.0E12 || fabs (coeff1/coeff2) > 1.0E12)
        {
        if (fabs(coeff1) < fc_coeffTol)
            {
            if (fabs(coeff0) < fc_coeffTol)
                return SOLUTION_ALL;
            else
                return SOLUTION_NONE;
            }
        else
            {
            *rt0 = *rt1 = -coeff0 / coeff1;
            return SOLUTION_SAME;
            }
        }

    denom = -2.0 * coeff2;
    det = (coeff1 * coeff1 - 4.0 * coeff2 * coeff0) / (denom * denom);
    if (det < tol)
        return SOLUTION_NONE;
    else
        {
        if (det <= 0.0)
            {
            *rt0 = *rt1 = coeff1 / denom;
            return SOLUTION_SAME;
            }
        else
            {
            det = sqrt(det);
            *rt0 = coeff1 / denom - det;
            *rt1 = coeff1 / denom + det;
            return SOLUTION_TWO;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          06/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspcci_selectRootsInRange
(
double      *reals,         /* <= real number in [0, 1] range */
int         *num,           /* <=> num of real numbers in [0, 1] range */
DPoint2d    *roots          /* => complex numbers */
)
    {
    int         j;
    double      *rP;
    DPoint2d    *pP, *pEnd;

    for (rP = reals, pP = roots+1, pEnd = roots + *num, j = 0;
         pP <= pEnd; pP++)
        {
        if ((pP->x > MINIMUM_Parameter && pP->x < MAXIMUM_Parameter) &&
            (fabs(pP->y) < fc_realNumberTol))
            {
            *rP = pP->x;
            if (*rP < 0.0)
                *rP = 0.0;
            else if (*rP > 1.0)
                *rP = 1.0;

            rP++;
            j++;
            }
        }
    *num = j;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     solve_real_roots01
(
double          *roots,    /* <= real roots of polynomial, starting at index 1
                                for compatibility with laguer */
int             *pNumRealRoots, /* <= Number of roots returned. */
DPoint2d        *a,        /* => polynomial coefficients, ASSUMED TO HAVE COMPLEX PART 0 */
int             m
)
    {
    double realCoffs[100];
    double realRoots[100];
    int numReal, i;

    for (i = 0; i <=m; i++)
        realCoffs[i] = a[i].x;

    bsiBezier_univariateStandardRootsInInterval (realRoots, &numReal, realCoffs, m,
                    MINIMUM_Parameter, MAXIMUM_Parameter);
    if (numReal < m)
        {
        *pNumRealRoots = numReal;
        for (i = 0; i < numReal; i++)
            roots[i] = realRoots[i];
        }
    else
        {
        // identically zero.
        *pNumRealRoots = 0;
        }
    return SUCCESS;
    }

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
@bsimethod                              EarlinLutz      04/04
----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiBezier_univariateStandardRootsInInterval
(
double          *pRootArray,
int             *pNumRoot,
double          *pStandardCoff,
int             degree,
double          aa,
double          bb
)
    {
    int order = degree + 1;
    int i, totalRoot, numMappedRoot;
    double u;
    double bezierCoff[MAX_BEZIER_ORDER];
    double bezierRoot[MAX_BEZIER_ORDER];
    double leftCoff[MAX_BEZIER_ORDER];
    double rootCoff[MAX_BEZIER_ORDER];  // bezier polynomial, over its own 0..1 which corresponds to aa.bb
                                        // after (a) nonlinear mapping of [x in 0..infinity] to [u in 0..1]
                                        // and   (b) linear mappings of [u in u(aa),u(bb)] to [t in 0..1]
                                        // this is the polynomial which is actually sent to the root finder.
                                        // Its local solutions are then remapped to the [u in 0..infinity] parameter
                                        // and then back to the original [x in aa,bb] parameter.
    double dummyCoff[MAX_BEZIER_ORDER];
    double u0, u1, x0, x1, tt;
    totalRoot = *pNumRoot = 0;

    if (order < 2 || order > MAX_BEZIER_ORDER)
        return false;

    /* Remap so positive-x roots are in [0,1] for a bezier: */
    // The original polynomial in s becomes a polynomial in u where
    //      x = u / (1-u)
    //      u = x / (1+x)
    // Find u values at expanded interval ends:
    bsiBezier_mapPositiveRealsToBezier (bezierCoff, pStandardCoff, order);
    x0 = aa;
    x1 = bb;
    u0 = x0 / (1.0 + x0);
    u1 = x1 / (1.0 + x1);

    // Remap so 0..1 maps to u=0..u1
    bsiBezier_subdivisionPolygons (leftCoff, dummyCoff, bezierCoff, order, 1, u1);
    // and again so 0..1 maps to u=u0..u1
    tt = u0 / u1;
    bsiBezier_subdivisionPolygons (dummyCoff, rootCoff, leftCoff, order, 1, tt);

    bsiBezier_univariateRoots (bezierRoot, &numMappedRoot, rootCoff, order);

    for (i = 0; i < numMappedRoot; i++)
        {
        u = u0 + bezierRoot[i] * (u1 - u0);
        if (u > u0 && u < u1 && totalRoot < degree)
            {
            pRootArray[totalRoot++] = u / (1.0 - u);
            }
        }

    *pNumRoot = totalRoot;
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
