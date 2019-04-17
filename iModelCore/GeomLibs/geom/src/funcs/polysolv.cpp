/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/
#define MR                  8
#define MT                  10
#define MAXIT               (MT*MR)
#define EPS                 1.0e-14
#define MAX_POLY_ORDER      20
#define SOLVER_RELATIVE_TOL 1.0e-4
#define SOLVER_ABSOLUTE_TOL 1.0e-14

/*----------------------------------------------------------------------+
|                                                                       |
| name          isComplexZero                                           |
|                                                                       |
| author        LuHan                                   6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    isComplexZero

(
DPoint2d        *pP     /* => complex number */
)
    {
    return (pP->x == 0.0 && pP->y == 0.0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          laguer_root                                             |
|                                                                       |
| author        LuHan                                   6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     laguer_root

(
DPoint2d        *x,           /* <=> convereg the start guess to a root */
int             *its,         /* <=  number of iterations taken */
DPoint2d        *a,           /*  => polynomial coefficients */
int             m             /*  => polynomial degree */
)
    {
    int         iter, j;
    double      abx, abp, abm, err, max;
    DPoint2d    dx, x1, b, d, f, g, h, sq, gp, gm, g2, cmp, pro, diff;
    double      frac[MR+1] = {0.0, 0.5, 0.25, 0.75, 0.13, 0.38, 0.62, 0.88, 1.0};
    static double s_bTol = 1.0e-14;
    static double s_realTol = 1.0e-14;
    static double s_complexTol = 1.0e-14;
    static int   minIteration = 1;

    for (iter = 1; iter <= MAXIT; iter++)
        {
        *its = iter;
        b = a[m];
        err = bsiComplex_abs(&b);
        d.x = d.y = f.x = f.y = 0.0;
        abx = bsiComplex_abs(x);
        for (j = m-1; j >= 0; j--)
            {
            bsiComplex_multiply(&pro, x, &f);
            bsiComplex_add(&f, &pro, &d);
            bsiComplex_multiply(&pro, x, &d);
            bsiComplex_add(&d, &pro, &b);
            bsiComplex_multiply(&pro, x, &b);
            bsiComplex_add(&b, &pro, &a[j]);
            err = bsiComplex_abs(&b) + abx * err;
            }

        err *= s_bTol;
        if (isComplexZero (&b) || (iter > minIteration && bsiComplex_abs(&b) <= err))
            return (SUCCESS);

        bsiComplex_divide(&g, &d, &b);
        bsiComplex_multiply(&g2, &g, &g);
        bsiComplex_divide(&pro, &f, &b);
        bsiComplex_realMultiplyComplex(&diff, 2.0, &pro);
        bsiComplex_subtract(&h, &g2, &diff);

        bsiComplex_realMultiplyComplex(&pro, (double) m, &h);
        bsiComplex_subtract(&diff, &pro, &g2);
        bsiComplex_realMultiplyComplex(&pro, (double) (m-1), &diff);
        bsiComplex_sqrt(&sq, &pro);

        bsiComplex_add(&gp, &g, &sq);
        bsiComplex_subtract(&gm, &g, &sq);
        abp = bsiComplex_abs(&gp);
        abm = bsiComplex_abs(&gm);
        if (abp < abm)
            gp = gm;

        max = (abp > abm) ? abp : abm;
        if (max > 0.0)
            {
            cmp.x = (double) m;
            cmp.y = 0.0;
            bsiComplex_divide(&dx, &cmp, &gp);
            }
        else
            {
            cmp.x = cos((double) iter);
            cmp.y = sin((double) iter);
            bsiComplex_realMultiplyComplex(&dx, exp(log(1.0 + abx)), &cmp);
            }

        bsiComplex_subtract(&x1, x, &dx);
        if (iter > minIteration && fabs(x->x - x1.x) < s_realTol && fabs(x->y - x1.y) < s_complexTol)
            return (SUCCESS);

        if (iter % MT)
            *x = x1;
        else
            {
            bsiComplex_realMultiplyComplex(&pro, frac[iter/MT], &dx);
            bsiComplex_subtract(x, x, &pro);
            }
        }

    /* Very unusual: still not converging after all iterations */
    return ERROR;
    }

/* MAP bsiSolve_polynomialEquation=Geom.polynomialEquation ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiSolve_polynomialEquation                             |
|                                                                       |
| author        LuHan                                   6/93            |
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
DPoint2d        *roots,    /* <= all real and complex roots of polynomial */
DPoint2d        *a,        /* => polynomial coefficients */
int             m,         /* => degree of polynomial >= 1 */
int             polish     /* => If true, roots are polished (improved) */
)

    {
    int         its, j, jj, status = ERROR;
    DPoint2d    x, b, c, pro, *ad;
    if (m < 1)
        return ERROR;
    if (NULL == (ad = (DPoint2d *) _alloca ((m+2) * sizeof (DPoint2d))))
        return  ERROR;

    memcpy (ad, a, (m+1)*sizeof(DPoint2d));
    for (j = m; j >= 1; j--)
        {
        x.x = x.y = 0.0;
        if (SUCCESS != (status = laguer_root(&x, &its, ad, j)))
            goto wrapup;

        if (fabs(x.y) <= 2.0 * EPS * fabs(x.x))
            x.y = 0.0;
        roots[j] = x;
        b = ad[j];
        for (jj = j-1; jj >= 0; jj--)
            {
            c = ad[jj];
            ad[jj] = b;
            bsiComplex_multiply(&pro, &x, &b);
            bsiComplex_add(&b, &pro, &c);
            }
        }

    /* Improve the roots by calling Laguer's method again */
    if (polish)
        for (j = 1; j <= m; j++)
            {
            if (SUCCESS != (status = laguer_root(&roots[j], &its, a, m)))
                goto wrapup;
            }

wrapup:
    return (status);
    }


/*---------------------------------------------------------------------------------**//**
* Note: Given degree m and complex coefficient a[0]...a[m], find all m complex roots by Laguer's method. a[0] is constant term. a[m] is
* coefficint for x^m term. If DPoint2d->y == 0, it is a real number.
* IMPORTANT!!! Output roots stars from roots[1] untill roots[m]!!! Input a[m] SHOULD NOT BE ZERO!!!
* ALSO: Even m can be any number >= 1, for m=1, 2, it is better call formula directly. MAX_POLY_ORDER can be set up to 100 and still produce
* stable results!!!
* @bsimethod                                                    Lu.Han          06/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      solve_polynomial_roots
(
DPoint2d        *roots,    /* <= all real and complex roots of polynomial */
DPoint2d        *a,        /* => polynomial coefficients */
int             m,         /* => degree of polynomial >= 1 */
int             polish     /* => If true, roots are polished (improved) */
)
    {
    return bsiSolve_polynomialEquation (roots, a, m, polish);
    }

#ifdef CompileAllPolysolve
/*----------------------------------------------------------------------+
|                                                                       |
| name          isCloseToZero                                           |
|                                                                       |
| author        LuHan                                   6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    isCloseToZero

(
const   double  value,
const   double  relative
)
    {
    if (fabs(value) < 1.0e-12 || fabs(value) <= 1.0e-8 * fabs(relative))
        return  true;
    else
        return  false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          sortLambdasByAbsoluteValues
|                                                                       |
| author        LuHan                                   6/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static void    sortLambdasByAbsoluteValues

(
double          *uP /* array of three */
)
    {
    double  tmp, tmp0, aP[3];

    aP[0] = fabs(uP[0]); aP[1] = fabs(uP[1]); aP[2] = fabs(uP[2]);
    if (aP[0] >= aP[1] && aP[0] >= aP[2])
        {
        if (aP[2] > aP[1])
            {
            tmp = uP[2];
            uP[2] = uP[1];
            uP[1] = tmp;
            }
        }
    else if (aP[0] < aP[1] && aP[0] > aP[2])
        {
        tmp = uP[0];
        uP[0] = uP[1];
        uP[1] = tmp;
        }
    else if (aP[0] > aP[1] && aP[0] < aP[2])
        {
        tmp = uP[2];
        tmp0 = uP[0];
        uP[2] = uP[1];
        uP[0] = tmp;
        uP[1] = tmp0;
        }
    else if (aP[0] < aP[1] && aP[0] < aP[2])
        {
        if (aP[1] > aP[2])
            {
            tmp = uP[2];
            tmp0 = uP[1];
            uP[2] = uP[0];
            uP[1] = tmp;
            uP[0] = tmp0;
            }
        else
            {
            tmp = uP[2];
            uP[2] = uP[0];
            uP[0] = tmp;
            }
        }
    }
#endif


END_BENTLEY_GEOMETRY_NAMESPACE

BEGIN_BENTLEY_NAMESPACE



/**
@description Computes (complex) eigenvalues of a matrix.
@param pLambda0 <= eigenvalue; may be complex
@param pLambda1 <= eigenvalue; may be complex
@param pLambda2 <= eigenvalue; may be complex
@param pMatrix => The matrix whose eigenvalues are computed.
@return int
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiRotMatrix_getEigenvalues
(
DPoint2d            *pLambda0,
DPoint2d            *pLambda1,
DPoint2d            *pLambda2,
RotMatrixCP pMatrix
)
    {
    int         status;
    DPoint2d    co[4], roots[4];

    /* set up coefficents for solving polynomial equation    */
    co[3].x = -1.0;
    co[2].x = pMatrix->form3d[0][0] + pMatrix->form3d[1][1] + pMatrix->form3d[2][2];
    co[1].x = pMatrix->form3d[0][2] * pMatrix->form3d[2][0] +
              pMatrix->form3d[0][1] * pMatrix->form3d[1][0] +
              pMatrix->form3d[1][2] * pMatrix->form3d[2][1] -
              pMatrix->form3d[0][0] * pMatrix->form3d[1][1] -
              pMatrix->form3d[0][0] * pMatrix->form3d[2][2] -
              pMatrix->form3d[1][1] * pMatrix->form3d[2][2];
    co[0].x = pMatrix->Determinant ();
    co[0].y = co[1].y = co[2].y = co[3].y = 0.0;

    /* call the solver */
    if (SUCCESS == (status = bsiSolve_polynomialEquation (roots, co, 3, true)))
        {
        *pLambda0 = roots[1];
        *pLambda1 = roots[2];
        *pLambda2 = roots[3];
        }

    return  status;
    }

static double s_mediumRelTol = 1.0e-12;


/*-----------------------------------------------------------------*//**
@description Compute the eigenvalues of a symmetric matrix.   The symmetry
assures that all eigenvalues are real.

@param pLambda0 <= first eigenvalue
@param pLambda1 <= second eigenvalue
@param pLambda2 <= third eigenvalue
@param pMatrix => The matrix whose eigenvalues are computed
@return int
@group "RotMatrix Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiRotMatrix_getEigenvaluesFromSymmetric


(
double              *pLambda0,
double              *pLambda1,
double              *pLambda2,
RotMatrixCP pMatrix
)
    {
    int         status;
    DPoint2d    root[3];
    double  absTol = 1.0e-20;
    double  relTol = s_mediumRelTol;

    if (SUCCESS == (status = bsiRotMatrix_getEigenvalues (&root[0], &root[1], &root[2], pMatrix)))
        {
        if ((fabs(root[0].y) < absTol || fabs(root[0].y) <= relTol * fabs(root[0].x)) &&
            (fabs(root[1].y) < absTol || fabs(root[1].y) <= relTol * fabs(root[1].x)) &&
            (fabs(root[2].y) < absTol || fabs(root[2].y) <= relTol * fabs(root[2].x)))
            {
            *pLambda0 = root[0].x;
            *pLambda1 = root[1].x;
            *pLambda2 = root[2].x;
            return  SUCCESS;
            }
        else
            return  ERROR;
        }
    return  status;
    }

END_BENTLEY_NAMESPACE
