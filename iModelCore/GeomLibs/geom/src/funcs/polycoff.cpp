/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/internal2/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_init

(
PolyCoffs *pA,      /* <= initialized polynomial */
int       degree    /* => polynomial degree. (degree + 1 coffs are zeroed */
)

    {
    StatusInt status = SUCCESS;
    int numCoff = degree + 1;
    if (numCoff < 0)
        numCoff = 0;
    if (numCoff > POLYCOFF_MAX_COFFS)
        {
        status = ERROR;
        numCoff = POLYCOFF_MAX_COFFS;
        }

    if (numCoff > 0)
        {
        memset (pA->coff, 0, numCoff * sizeof (double));
        }
    pA->numCoff = numCoff;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_raiseDegree

(
PolyCoffs *pA,      /* <=> polynomial */
int       degree    /* => degree of polynomial */
)

    {
    StatusInt status = SUCCESS;
    int newNumCoff = degree + 1;

    if (newNumCoff > pA->numCoff)
        {
        if (newNumCoff > POLYCOFF_MAX_COFFS)
            {
            newNumCoff = POLYCOFF_MAX_COFFS;
            status = ERROR;
            }
        memset (pA->coff + pA->numCoff, 0, (newNumCoff - pA->numCoff) * sizeof (double));
        pA->numCoff = newNumCoff;
        }

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_setCoff

(
PolyCoffs *pA,      /* <=> polynomial */
double    coff,     /* => coefficient to set */
int       degree    /* => degree of term */
)

    {
    StatusInt status = SUCCESS;
    if (   degree < pA->numCoff
        || SUCCESS == bsiPolycoffs_raiseDegree (pA, degree))
        {
        pA->coff[degree] = coff;
        }
    else
        {
        status = ERROR;
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_addToCoff

(
PolyCoffs *pA,      /* <=> polynomial */
double    coff,     /* => value to add */
int       degree    /* => degree of term */
)

    {
    StatusInt status = SUCCESS;
    if (   degree < pA->numCoff
        || SUCCESS == bsiPolycoffs_raiseDegree (pA, degree))
        {
        pA->coff[degree] += coff;
        }
    else
        {
        status = ERROR;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_multiply        /* degree of product = sum of degrees  */

(
PolyCoffs *pC,  /* <= product coefficients. */
const PolyCoffs *pA,  /* => factor coefficients */
const PolyCoffs *pB   /* =. factor coefficients */
)
    {
    int i, j;
    int nA = pA->numCoff;
    int nB = pB->numCoff;
    PolyCoffs product;
    StatusInt status = bsiPolycoffs_init (&product, nA + nB - 2);

    if (SUCCESS == status)
        {
        for (i = 0; i < nA; i++)
            {
            for (j = 0; j < nB; j++)
                product.coff[i+j] += pA->coff[i] * pB->coff[j];
            }
        }

    *pC = product;
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initLinear

(
PolyCoffs *pA,      /* <= product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* <= constant coefficient */
double  c1          /* <= linear coefficient */
)

    {
    pA->numCoff = 2;
    pA->coff[0] = c0;
    pA->coff[1] = c1;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initQuadratic

(
PolyCoffs *pA,      /* <= product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* <= constant coefficient */
double  c1,         /* <= linear coefficient */
double  c2          /* <= quadratic coefficient */
)

    {
    pA->numCoff = 3;
    pA->coff[0] = c0;
    pA->coff[1] = c1;
    pA->coff[2] = c2;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initSquaredLinear

(
PolyCoffs *pA,      /* <= product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* <= constant coefficient */
double  c1          /* <= linear coefficient */
)

    {
    pA->numCoff = 3;
    pA->coff[0] = c0 * c0;
    pA->coff[1] = 2.0 * c0 * c1;
    pA->coff[2] = c1 * c1;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_add

(
PolyCoffs *pC,  /* <= summed coefficients. */
const PolyCoffs *pA,  /* => term coefficients */
const PolyCoffs *pB   /* =. term coefficients */
)

    {
    int i;
    PolyCoffs sum;

    if (pA->numCoff >= pB->numCoff)
        {
        sum = *pA;
        for (i = 0; i < pB->numCoff; i++)
            sum.coff[i] += pB->coff[i];
        }
    else
        {
        sum = *pB;
        for (i = 0; i < pA->numCoff; i++)
            sum.coff[i] += pA->coff[i];
        }
    *pC = sum;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_addScaled

(
PolyCoffs *pC,  /* <= summed coefficients. */
const PolyCoffs *pA,  /* => term coefficients */
const PolyCoffs *pB,   /* =. term coefficients */
double scale    /* scale factor for pB */
)

    {
    int numCoff = pA->numCoff > pB->numCoff ? pA->numCoff : pB->numCoff;
    int i;
    PolyCoffs sum;

    bsiPolycoffs_init (&sum, numCoff - 1);

    for (i = 0; i < pA->numCoff; i++)
        {
        sum.coff[i] = pA->coff[i];
        }

    for (i = 0; i < pB->numCoff; i++)
        {
        sum.coff[i] += pB->coff[i] * scale;
        }

    *pC = sum;
    }


/*---------------------------------------------------------------------------------**//**
* Construct the (degree 4) polynomial for the intersection of a line and a unit torus. (Torus major circle is unit circle in xy plane, minor
* circle radius given)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initLineUnitTorus

(
PolyCoffs       *pA,      /* <= polynomial */
DPoint4dCP pPoint0, /* => line start point */
DPoint4dCP pPoint1, /* => line end point */
double          r         /* => minor radius */
)

    {
    PolyCoffs XX, YY, ZZ, WW, PP, QQ, UU;
    bsiPolycoffs_initSquaredLinear (&XX, pPoint0->x, pPoint1->x - pPoint0->x);
    bsiPolycoffs_initSquaredLinear (&YY, pPoint0->y, pPoint1->y - pPoint0->y);
    bsiPolycoffs_initSquaredLinear (&ZZ, pPoint0->z, pPoint1->z - pPoint0->z);
    bsiPolycoffs_initSquaredLinear (&WW, pPoint0->w, pPoint1->w - pPoint0->w);

    bsiPolycoffs_add (&UU, &XX, &YY);
    bsiPolycoffs_add (&PP, &UU, &ZZ);
    bsiPolycoffs_addScaled (&PP, &PP, &WW, 1.0 - r*r);

    bsiPolycoffs_multiply (&PP, &PP, &PP);
    bsiPolycoffs_multiply (&QQ, &UU, &WW);

    bsiPolycoffs_addScaled (pA, &PP, &QQ, -4.0);
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void        panic

(
)
    {
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void        bsiPolycoffs_realRootsLaguer

(
PolyCoffs     *pRoot,     /* <= coefficients are roots */
const PolyCoffs     *pA         /* => polynomial to solve */
)

    {
    DPoint2d complexCoff[POLYCOFF_MAX_COFFS];
    DPoint2d complexRoot[POLYCOFF_MAX_COFFS];
    double rho, rhoMax;
    static double s_relZero = 1.0e-12;  /* For real root testing */
    double complexZero;
    double maxCoff;
    double coff;
    StatusInt status;
    int iMin;
    double complexPart, minComplexPart = DBL_MAX;

    int i;
    int degree;

    maxCoff = 0.0;
    for (i = 0, degree = -1; i < pA->numCoff; i++)
        {
        coff = complexCoff[i].x = pA->coff[i];
        complexCoff[i].y = 0.0;
        if (fabs (coff) > maxCoff)
            maxCoff = fabs (coff);
        if (coff != 0.0)
            degree = i;
        }

    bsiPolycoffs_init (pRoot, 0);
    pRoot->numCoff = 0;
    if ( degree > 0)
        {
        double rMax = 1.0 / maxCoff;
        for (i = 0; i <= degree; i++)
            {
            complexCoff[i].x *= rMax;
            }

        status = bsiSolve_polynomialEquation (complexRoot, complexCoff, degree, true);

        /* Find largest magnitude of any root */
        rhoMax = 0.0;
        for (i = 1; i <= degree; i++)
            {
            rho = complexRoot[i].x * complexRoot[i].x + complexRoot[i].y * complexRoot[i].y;
            if (rho > rhoMax)
                rhoMax = rho;
            }

        complexZero = s_relZero * sqrt (rhoMax);
        /* Extract the real roots */
        iMin = 0;
        for (i = 1; i <= degree; i++)
            {
            complexPart = fabs (complexRoot[i].y);
            if (complexPart <= complexZero)
                {
                pRoot->coff[pRoot->numCoff++] = complexRoot[i].x;
                }
            else if (iMin == 0 || complexPart < minComplexPart)
                {
                iMin = i;
                minComplexPart = complexPart;
                }
            }
        if (!(degree & 1) && (pRoot->numCoff & 1) && iMin != 0)
            {
            /* We need one more real root to make things even. */
            pRoot->coff[pRoot->numCoff++] = complexRoot[iMin].x;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void        bsiPolycoffs_realRootsBezier

(
PolyCoffs     *pRoot,     /* <= coefficients are roots */
const PolyCoffs     *pA         /* => polynomial to solve */
)
    {
    double roots[POLYCOFF_MAX_COFFS];
    int numRoot, i;

    bsiPolycoffs_init (pRoot, 0);
    pRoot->numCoff = 0;
    if (bsiBezier_univariateStandardRoots (roots, &numRoot, (double*)pA->coff, pA->numCoff - 1))
        {
        for (i = 0; i < numRoot; i++)
            {
            pRoot->coff[pRoot->numCoff++] = roots[i];
            }
        }
#ifdef COMPARE_TO_LAGUER
        {
        PolyCoffs testRoot;
        static int s_errors = 0;
        bsiPolycoffs_realRootsLaguer (&testRoot, pA);
        if (testRoot.numCoff != pRoot->numCoff)
            s_errors++;
        }
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void           bsiPolycoffs_realRoots

(
PolyCoffs     *pRoot,     /* <= coefficients are roots */
const PolyCoffs     *pA         /* => polynomial to solve */
)
    {
    static int s_solver = 0;
    if (s_solver == 0)
        bsiPolycoffs_realRootsBezier (pRoot, pA);
    else
        bsiPolycoffs_realRootsLaguer (pRoot, pA);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bsiPolycoffs_getCoff

(
double    *pCoff,               /* <= i'th coefficient */
const PolyCoffs *pA,            /* => coefficient array */
int       i
)

    {
    StatusInt status;
    if (i >= 0 && i < pA->numCoff)
        {
        *pCoff = pA->coff[i];
        status = SUCCESS;
        }
    else
        {
        *pCoff = 0.0;
        status = ERROR;
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @return the degree of the polynomial.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiPolycoffs_getDegree

(
const PolyCoffs *pA             /* => polynomial whose degree is queried */
)
    {
    return pA->numCoff - 1;
    }


/*---------------------------------------------------------------------------------**//**
* Copy a polynomial and raise the output polynomial degree.  If
* input polynomial is a null pointer, set the output to a default constant
* value.
* @return SUCCESS if the copy and degree raising were completed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bsiPolycoffs_copyOrInit

(
PolyCoffs *pOut,                /* <= destination polynomial */
const   PolyCoffs *pIn,                 /* => source polynomial */
double    defaultConstValue,    /* => constant term (constant polynomial) to use when pIn is null */
        int       defaultdegree         /* => minimum degree of output. */
)
    {
    if (pIn)
        {
        *pOut = *pIn;
        }
    else
        {
        pOut->numCoff = 1;
        pOut->coff[0] = defaultConstValue;
        }
    return bsiPolycoffs_raiseDegree (pOut, defaultdegree);
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the polynomial at the specified point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    bsiPolycoffs_evaluate

(
const   PolyCoffs *pA,      /* => polynomial to evaluate. */
double     x        /* => point for evaluation */
)
    {
    double f = 0.0;
    int i;
    if (pA  && pA->numCoff >= 0)
        {
        i = pA->numCoff -1;
        f = pA->coff[i];
        while (--i >= 0)
            {
            f = f * x + pA->coff[i];
            }
        }
    return f;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
