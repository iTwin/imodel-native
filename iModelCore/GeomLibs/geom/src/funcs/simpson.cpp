/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/simpson.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/bezeval.fdf>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define MAX_DEPTH           10
static int s_maxDepth = 0;  // running counter of maximum depth reached.
static double s_relTolFactor = 1.414;
static double s_absTolFactor = 0.707;

/*----------------------------------------------------------------------+
|                                                                       |
| name          newtonCotes5PointQuadrature                             |
|                                                                       |
| author        EarlinLutz                              08/98           |
|                                                                       |
| Estimate an integral by 5 point newton cotes rule, and estimate the   |
| absolute value integral by 4-part trapezoid rule.                     |
+----------------------------------------------------------------------*/
static void nc5Quad

(
double          *sumP,      /* <= simpson's rule estimate of integral */
double          *absP,      /* <= trapezoidal estimate of integral of absolute value */
double          f0,         /* => integrand at 0 */
double          f1,         /* => integrand at 0.25 */
double          f2,         /* => integrand at 0.5 */
double          f3,         /* => integrand at 0.75 */
double          f4,         /* => integrand at 1 */
double          delta       /* => interval width */
)
    {
    *sumP = delta * (7.0 * (f0 + f4) + 32.0 * (f1 + f3) + 12.0 * f2) / 90.0;
    *absP = delta * 0.25 *  (       fabs (f0)
                            + 2.0 * (fabs (f1) + fabs (f2) + fabs (f3))
                            +       fabs (f4)
                            );
    }

#ifdef CompileSimpson
/*----------------------------------------------------------------------+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          simpQuad                                                |
|                                                                       |
| author        EarlinLutz                              08/98           |
|                                                                       |
| Estimate an integral by simpson's rule, and estimate the absolute     |
| value integral by 2-part trapezoid rule.                              |
+----------------------------------------------------------------------*/
static void simpQuad

(
double          *sumP,      /* <= simpson's rule estimate of integral */
double          *absP,      /* <= trapezoidal estimate of integral of absolute value */
double          f0,         /* => integrand at 0 */
double          fM,         /* => integrand at midpoint */
double          f1,         /* => integrand at 1 */
double          delta       /* => interval width */
)
    {
    *sumP = delta * (f0 + 4.0 * fM + f1) / 6.0;
    *absP = delta * 0.25 * ( fabs (f0) + 2.0 * fabs (fM) + fabs (f1));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          recursiveSubdivision                                    |
|                                                                       |
| author        LuHan                                   3/95            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool     recursiveSimpson

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
int             *pCount,            /* <= number of function calls */
double          x0,                 /* => lower limit for integration */
double          f0,                 /* => integrand at lower limit */
double          xM,                 /* => midpoint of interval */
double          fM,                 /* => integrand at midpoint */
double          x1,                 /* => upper limit for integration */
double          f1,                 /* => integrand at lower limit */
double          g01,                /* => simpson's rule estimate for overall integral */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand   evaluateFunc,   /* => evaluate function for integrand */
void            *userDataP,         /* => passed through to evaluateFunc */
int             depth
)
    {
    double      error;
    double      x2 = 0.5 * (x0 + xM);
    double      x3 = 0.5 * (xM + x1);
    double      f2, f3;
    double      g0, g1, g;
    double      a0, a1;
    double      h   = 0.5 * (x1 - x0);


    (*evaluateFunc) (&f2, x2, userDataP);
    (*evaluateFunc) (&f3, x3, userDataP);
    (*pCount) += 2;

    simpQuad (&g0, &a0, f0, f2, fM, h);
    simpQuad (&g1, &a1, fM, f3, f1, h);

    g = g0 + g1;

    error = fabs (g01 - g);

    if (depth > MAX_DEPTH || error < absTol + relTol * (fabs (a0) + fabs (a1)) )
        {
        *integralP += g;
        *errorP += error;
        return true;
        }

    depth++;
    if (depth > s_maxDepth)
        s_maxDepth = depth;

    recursiveSimpson (integralP, errorP, pCount,
            x0, f0, x2, f2, xM, fM, g0,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, userDataP, depth);

    recursiveSimpson (integralP, errorP, pCount,
            xM, fM, x3, f3, x1, f1, g1,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, userDataP, depth);

    return true;
    }



typedef bool    (*IntegrationControlFunction)

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
double          x0,                 /* => lower limit for integration */
double          x1,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* => evaluate function for integrand */
void            *userDataP          /* => passed through to evaluateFunc */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveSimpson                                |
|                                                                       |
| author        LuHan                                   3/95            |
|                                                                       |
| Note          This is an adaptive method and no value is evaluated    |
|               twice during the calculation                            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveSimpson

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
int             *pCount,            /* <= number of function calls */
double          x0,                 /* => lower limit for integration */
double          x1,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* => evaluate function for integrand */
void            *userDataP          /* => passed through to evaluateFunc */
)
    {
    double f0, f1, fM, g01, a01;
    double xM;

    xM = 0.5 * (x0 + x1);
    (*evaluateFunc) (&f0, x0, userDataP);
    (*evaluateFunc) (&fM, xM, userDataP);
    (*evaluateFunc) (&f1, x1, userDataP);
    *pCount = 3;
    simpQuad (&g01, &a01, f0, fM, f1, x1 - x0);
    *integralP = *errorP = 0.0;
    return (recursiveSimpson (integralP, errorP, pCount,
        x0, f0, xM, fM, x1, f1, g01,
        absTol, relTol, evaluateFunc,
        userDataP, 0));
    }

#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          invokeNC5ExtendFunc                                     |
|                                                                       |
| author        EarlinLutz                              08/98           |
|                                                                       |
| Invoke the "extend" function which reports addition of one            |
| lowest-level interval to the accepted integral.                       |
+----------------------------------------------------------------------*/
static void invokeNC5ExtendFunc

(
double          g0,         /* => accepted integral to left. */
double          dg,         /* => previously computed NC5 quadrature for this interval */
double          x0,         /* => x values */
double          x1,
double          x2,
double          x3,
double          x4,
double          f0,         /* => corresponding integrands */
double          f1,
double          f2,
double          f3,
double          f4,
PFExtendScalarIntegration   extendFunc,
void            *userDataP
)
    {
    /*
        Form Lagrange quartic polynomials with spikes at 0, 1, 2, 3, 4.
        Integrate from 0 to 1,2,3,4.
    */
    static int numerator[5][4] =
        {
           {  502,  464,  486,  448},   /* Spike at 0, integrated to 1, 2, 3, 4 */
           { -323, -496, -459, -512},   /* Spike at 1, .... */
           {  -88,   64,  216,  128},   /* Spike at 2, .... */
           {  -53,  -16, -189, -512},   /* Spike at 3, .... */
           {  -38,  -16,  -54,  448},   /* Spike at 4, .... */
        };

    static double weight[5][4];
    int i, j;

    static int denominator[5] = { 1440, -360, 240, -360, 1440 };

    static int firstTime = 1;

    double xx[5];
    double ff[5];
    double gg[5];
    double h = x4 - x0;

    /* The weights are defined as exact integer ratios.
       On first time through this function, divide out to floating point form:
    */

    if (firstTime)
        {
        for (i = 0; i < 5; i++)
            for (j = 0; j < 4; j++)
                weight[i][j] = 0.25 * (double)numerator[i][j] / (double)denominator[i];
        firstTime = 0;
        }

    xx[0] = x0;
    xx[1] = x1;
    xx[2] = x2;
    xx[3] = x3;
    xx[4] = x4;

    ff[0] = f0;
    ff[1] = f1;
    ff[2] = f2;
    ff[3] = f3;
    ff[4] = f4;

    gg[0] = 0.0;

    for (j = 1; j <= 4; j++)
        {
        gg[j] = 0.0;

        for (i = 0; i < 5; i++)
            gg[j] += weight[i][j-1] * ff[i];

        gg[j] *= h;
        }
    /* Last weight sum should match dg --- just reuse it */
    gg[4] = dg;

    extendFunc (g0, xx, ff, gg, 5, userDataP);
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          recursiveNC5                                            |
|                                                                       |
| author        EarlinLutz                              05/99           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool     recursiveNC5

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
int             *pCount,            /* <= number of function calls */
double          x0,                 /* => lower limit for integration */
double          f0,                 /* => integrand at lower limit */
double          x2,                 /* => 0.25 parameter */
double          f2,                 /* => f(0.25) */
double          x4,                 /* => 0.50 parameter */
double          f4,                 /* => f(0.50) */
double          x6,                 /* => 0.75 parameter */
double          f6,                 /* => f(0.75) */
double          x8,                 /* => upper limit for integration */
double          f8,                 /* => integrand at lower limit */
double          g01,                /* => prior estimate for overall integral */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand   evaluateFunc,
PFExtendScalarIntegration   extendFunc,
void            *userDataP,         /* => passed through to evaluateFunc */
int             depth
)
    {
    double      error;
    double      x1 = 0.5 * (x0 + x2);
    double      x3 = 0.5 * (x2 + x4);
    double      x5 = 0.5 * (x4 + x6);
    double      x7 = 0.5 * (x6 + x8);
    double      f1, f3, f5, f7;
    double      g0, g1, g;
    double      a0, a1;
    double      h   = 0.5 * (x8 - x0);


    (*evaluateFunc) (&f1, x1, userDataP);
    (*evaluateFunc) (&f3, x3, userDataP);
    (*evaluateFunc) (&f5, x5, userDataP);
    (*evaluateFunc) (&f7, x7, userDataP);
    (*pCount) += 4;

    nc5Quad (&g0, &a0, f0, f1, f2, f3, f4, h);
    nc5Quad (&g1, &a1, f4, f5, f6, f7, f8, h);

    g = g0 + g1;

    error = fabs (g01 - g);

    if (depth > MAX_DEPTH || error <= absTol + relTol * (fabs (a0) + fabs (a1)) )
        {
        if (extendFunc)
            {
            invokeNC5ExtendFunc ( *integralP, g0,
                                  x0, x1, x2, x3, x4,
                                  f0, f1, f2, f3, f4,
                                  extendFunc, userDataP);

            invokeNC5ExtendFunc ( *integralP, g1,
                                  x4, x5, x6, x7, x8,
                                  f4, f5, f6, f7, f8,
                                  extendFunc, userDataP);
            }
        *integralP += g;
        *errorP += error;
        return true;
        }

    depth++;
    if (depth > s_maxDepth)
        s_maxDepth = depth;

    recursiveNC5 (integralP, errorP, pCount,
            x0, f0, x1, f1, x2, f2, x3, f3, x4, f4, g0,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, extendFunc, userDataP, depth);

    recursiveNC5 (integralP, errorP, pCount,
            x4, f4, x5, f5, x6, f6, x7, f7, x8, f8, g1,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, extendFunc, userDataP, depth);

    return true;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveSimpson                                |
|                                                                       |
| author        LuHan                                   3/95            |
|                                                                       |
| Note          This is an adaptive method and no value is evaluated    |
|               twice during the calculation                            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
int             *pCount,            /* <= function count */
double          x0,                 /* => lower limit for integration */
double          x8,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* => evaluate function for integrand */
void            *userDataP          /* => passed through to evaluateFunc */
)
    {
    double f0, f2, f4, f6, f8, g01, a01;
    double x2, x4, x6;

    x4 = 0.5 * (x0 + x8);
    x2 = 0.5 * (x0 + x4);
    x6 = 0.5 * (x4 + x8);
    (*evaluateFunc) (&f0, x0, userDataP);
    (*evaluateFunc) (&f2, x2, userDataP);
    (*evaluateFunc) (&f4, x4, userDataP);
    (*evaluateFunc) (&f6, x6, userDataP);
    (*evaluateFunc) (&f8, x8, userDataP);

    nc5Quad (&g01, &a01, f0, f2, f4, f6, f8, x8 - x0);
    *integralP = *errorP = 0.0;
    *pCount = 5;
    return (recursiveNC5 (integralP, errorP, pCount,
        x0, f0, x2, f2, x4, f4, x6, f6, x8, f8, g01,
        absTol, relTol, evaluateFunc, NULL,
        userDataP, 0));
    }
#ifdef CompileSimposonIntegrationContext
typedef struct
    {
    int numPoint;
    double g;
    double ga;
    double absTol;
    double relTol;
    int count;
    double *pXArray;
    double *pFArray;
    } IntegrationGrid;

#define MAX_INTEGRATION_STACK_SIZE 1000

typedef void (*IC_EvaluateFunc)
                    (
                    void *pUserData,
                    double *pF,         /* <= function value */
                    double x            /* => independent var */
                    );

typedef bool    (*IC_IntegrateFunc)
                    (
                    void *pUserData,
                    double *pG,         /* <= returned integral */
                    double *pAbsG,      /* <= returned integral of absolute value, or
                                                other value to be scaled by relTol to determine
                                                termination tolerances */
                    double *pX,         /* => x grid */
                    double *pF,         /* => function grid */
                    int numPoint        /* => number of points */
                    );

typedef bool    (*IC_IntervalFunc)
                    (
                    void *pUserData,
                    double *pX,         /* => x grid */
                    double *pF,         /* => function grid */
                    int numPoint,       /* => number of points */
                    double g            /* => integral evaluated on interval. */
                    );

typedef bool    (*IC_FinalNodeFunc)
                    (
                    void *pUserData,
                    double gParent,     /* => integral in parent */
                    double gLeft,       /* => integral in left child */
                    double gRight       /* => integral in right child */
                    );

typedef struct
    {
    int maxDepthPermitted;

    double globalRelTol;
    double globalAbsTol;

    double absTolFactor;
    double relTolFactor;
    void *pUserData;

    IC_EvaluateFunc     cbEvaluate;
    int                 numCallEvaluate;

    IC_IntegrateFunc    cbIntegrate;
    int                 numCallIntegrate;

    IC_IntervalFunc     cbEnterInterval;
    int                 numCallEnterInterval;

    IC_FinalNodeFunc    cbFinalNode;
    int                 numCallFinalNode;

    IC_IntervalFunc     cbLeaf;
    int                 numCallLeaf;

    int numPointPerGrid;

    int stackCount;
    double globalError;
    int majorErrorCount;
    int maxDepthReached;
    int numDepthLimitReached;

    double stack[MAX_INTEGRATION_STACK_SIZE];
    } IntegrationContext;


static void initIntegrationContext

(
IntegrationContext *pContext,
void *pUserData,
int numPointPerGrid,
int maxDepthPermitted,
double globalAbsTol,
double globalRelTol,
double absTolFactor,
double relTolFactor,
IC_EvaluateFunc     cbEvaluate,
IC_IntegrateFunc    cbIntegrate,
IC_IntervalFunc     cbEnterInterval,
IC_FinalNodeFunc    cbFinalNode,
IC_IntervalFunc     cbLeaf
)
    {
    memset (pContext, 0, sizeof (IntegrationContext));

    pContext->cbEvaluate        = cbEvaluate;
    pContext->cbIntegrate       = cbIntegrate;
    pContext->cbEnterInterval   = cbEnterInterval;
    pContext->cbFinalNode       = cbFinalNode;
    pContext->cbLeaf            = cbLeaf;

    pContext->globalAbsTol = globalAbsTol;
    pContext->globalRelTol = globalRelTol;

    pContext->absTolFactor = absTolFactor;
    pContext->relTolFactor = relTolFactor;

    pContext->pUserData = pUserData;

    pContext->maxDepthPermitted     = maxDepthPermitted;
    pContext->numPointPerGrid       = numPointPerGrid;
    }


static void closeIntegrationContext

(
IntegrationContext *pContext
)
    {
    }


static int getStackMarker

(
IntegrationContext *pContext
)
    {
    return pContext->stackCount;
    }

static void popToMarker

(
IntegrationContext *pContext,
int marker
)
    {
    if (marker >= 0 && marker < MAX_INTEGRATION_STACK_SIZE)
        pContext->stackCount = marker;
    }

static bool    initRootGridFromStack

(
IntegrationContext  *pContext,
IntegrationGrid     *pRoot,
double              x0,
double              x1
)
    {
    int n = pContext->numPointPerGrid;
    int i, i0, i1;
    int newStackCount = pContext->stackCount + n;
    bool    retvalue = true;
    double dx = (x1 - x0) / (double)(n - 1);

    if (newStackCount > MAX_INTEGRATION_STACK_SIZE)
        {
        pRoot->count = 0;
        pRoot->pXArray = NULL;
        pRoot->pFArray = NULL;
        return false;
        }

    pRoot->pXArray = pContext->stack + pContext->stackCount;
    pContext->stackCount += n;

    pRoot->pFArray = pContext->stack + pContext->stackCount;
    pContext->stackCount += n;

    pRoot->count = n;

    pRoot->absTol = pContext->globalAbsTol;
    pRoot->relTol = pContext->globalRelTol;

    i0 = 0;
    i1 = n - 1;
    pRoot->pXArray[i0] = x0;
    pRoot->pXArray[i1] = x1;

    for (i = i0 + 1; i < i1; i++)
        {
        pRoot->pXArray[i] = x0 + i * dx;
        }

    for (i = i0; i <= i1; i++)
        {
        pContext->cbEvaluate (pContext->pUserData, &pRoot->pFArray[i], pRoot->pXArray[i]);
        pContext->numCallEvaluate++;
        }

    retvalue = pContext->cbIntegrate
            (
            pContext->pUserData,
            &pRoot->g,
            &pRoot->ga,
            pRoot->pXArray,
            pRoot->pFArray,
            n
            );
    pContext->numCallIntegrate++;

    return retvalue;
    }

static bool    initChildGridFromStack

(
IntegrationContext  *pContext,
IntegrationGrid     *pChild,
IntegrationGrid     *pParent,
bool                childIsRightHalf
)
    {
    int n = pContext->numPointPerGrid;
    int k, i0, i1;
    int newStackCount = pContext->stackCount + 2 * n;
    bool    retvalue = true;

    if (newStackCount > MAX_INTEGRATION_STACK_SIZE)
        {
        pChild->count = 0;
        pChild->pXArray = NULL;
        pChild->pFArray = NULL;
        return false;
        }
    pChild->pXArray = pContext->stack + pContext->stackCount;
    pContext->stackCount += n;
    pChild->pFArray = pContext->stack + pContext->stackCount;
    pContext->stackCount += n;
    pChild->count = n;

    pChild->absTol = pParent->absTol * pContext->absTolFactor;
    pChild->relTol = pParent->relTol * pContext->relTolFactor;

    if (pParent)
        {
        if (childIsRightHalf)
            {
            i0 = n >> 1;    /* Divide by 2 -- n is odd, we want the truncation */
            }
        else
            {
            i0 = 0;
            }
        pChild->pXArray[0] = pParent->pXArray[i0];
        pChild->pFArray[0] = pParent->pFArray[i0];
        for (i1 = i0 + 1, k = 1; k < n; i0 = i1++)
            {
            pChild->pXArray[k] = 0.5 * (pParent->pXArray[i0] + pParent->pXArray[i1]);
            pContext->cbEvaluate
                    (
                    pContext->pUserData,
                    &pChild->pFArray[k],
                    pChild->pXArray[k]
                    );
            pContext->numCallEvaluate++;
            k++;
            pChild->pXArray[k] = pParent->pXArray[i1];
            pChild->pFArray[k] = pParent->pFArray[i1];
            k++;
            }

        retvalue = pContext->cbIntegrate
                (
                pContext->pUserData,
                &pChild->g, &pChild->ga,
                pChild->pXArray,
                pChild->pFArray,
                n
                );
        pContext->numCallIntegrate++;
        }
    return retvalue;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          continueRecursiveSubdivision                            |
|                                                                       |
| enterIntervalFunc (pGrid, pUserData) - test if need to enter          |
|       the interval.  Return true to enter, false to skip quietly.     |
|       If no function, act as if true.
|                                                                       |
| evaluateFunc (pUserData, &f, x) -- evaluate function at x.  No        |
|   control flow changes possible -- must evaluate.                     |
| integrateFunc (&f, &fa, pXArray, pFArray, n, pUserData)               |
|   Evaluate integral and absolute integral (for tolerance scaling)     |
|   over interval.  Return true to continue, false to abort.            |
|                                                                       |
| pFinalNodeFunc (pUserData, fNode, fLeft, fRight) -- called with       |
|   integral values where a node and its children are being accepted.   |
|   return false to abort.                                              |
| pLeafFunc (pUserData, pXArray, pFArray, n, g) -- process grid         |
|   at an accepted leaf.  These calls are made in strict                |
|   start-to-finish order.                                              |
|                                                                       |
| author        EarlinLutz                              03/00           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        continueRecursiveSubdivision

(
IntegrationContext  *pContext,
IntegrationGrid     *pParentGrid,
int                 depth
)
    {
    double      error;
    IntegrationGrid leftGrid;
    IntegrationGrid rightGrid;
    double      g, ga;
    bool    continueProcessing = true;
    bool    converged;
    int stackMarker = getStackMarker (pContext);
//    pContext->numCallEnterInterval++;

    if (pContext->cbEnterInterval
        && !(pContext->cbEnterInterval
                (
                pContext->pUserData,
                pParentGrid->pXArray,
                pParentGrid->pFArray,
                pParentGrid->count,
                pParentGrid->g
                )))
        {
        continueProcessing = true;
        }
    else if (   !initChildGridFromStack (pContext, &leftGrid, pParentGrid,  false)
             || !initChildGridFromStack (pContext, &rightGrid, pParentGrid, true))
        {
        continueProcessing = false;
        }
    else
        {
        g = leftGrid.g + rightGrid.g;
        ga = leftGrid.ga + rightGrid.ga;
        error = fabs (pParentGrid->g - g);
        converged = error < pParentGrid->absTol + pParentGrid->relTol * ga;

        if (    converged || depth > pContext->maxDepthPermitted)
            {
            if (!converged)
                pContext->numDepthLimitReached++;

            pContext->numCallFinalNode++;
            if (pContext->cbFinalNode)
                continueProcessing = pContext->cbFinalNode
                            (
                            pContext->pUserData,
                            pParentGrid->g,
                            leftGrid.g,
                            rightGrid.g
                            );

            pContext->numCallLeaf += 2;
            if (pContext->cbLeaf)
                {
                if (continueProcessing)
                    continueProcessing = pContext->cbLeaf
                                                (
                                                pContext->pUserData,
                                                leftGrid.pXArray,
                                                leftGrid.pFArray,
                                                leftGrid.count,
                                                leftGrid.g
                                                );

                if (continueProcessing)
                    continueProcessing = pContext->cbLeaf
                                                (
                                                pContext->pUserData,
                                                rightGrid.pXArray,
                                                rightGrid.pFArray,
                                                rightGrid.count,
                                                rightGrid.g
                                                );
                }
            }
        else
            {
            depth++;
            if (depth > pContext->maxDepthReached)
                pContext->maxDepthReached = depth;
            continueProcessing =
                    continueRecursiveSubdivision (pContext, &leftGrid, depth)
                &&  continueRecursiveSubdivision (pContext, &rightGrid, depth);
            }
        }

    popToMarker (pContext, stackMarker);
    return continueProcessing;
    }

typedef struct
    {
    void *pUserData;
    PFScalarIntegrand cbEvaluate;
    double x0;
    double x1;
    double u0;
    double u1;
    double globalIntegral;
    double globalError;
    double partialIntegral;
    } FunctionContext;

static void cbNCEvaluateUserFunc

(
FunctionContext *pFunctionContext,
double *pF,
double x
)
    {
    pFunctionContext->cbEvaluate (pF, x, pFunctionContext->pUserData);
    }

static bool    cbNCIntegrate

(
FunctionContext *pFunctionContext,
double *pIntegral,
double *pAbsIntegral,
const double *pXArray,
const double *pFArray,
int         n
)
    {
    double a;
    if (n == 3)
        {
        a = (pXArray[2] - pXArray[0]) / 6.0;
        *pIntegral = a * (pFArray[0] + 4.0 * pFArray[1] + pFArray[2]);
        *pAbsIntegral = a * (fabs (pFArray[0]) + 4.0 * fabs (pFArray[1]) + fabs (pFArray[2]));
        return true;
        }
    if (n == 5)
        {
        double h = pXArray[4] - pXArray[0];
        nc5Quad (pIntegral, pAbsIntegral,
                pFArray[0], pFArray[1], pFArray[2], pFArray[3], pFArray[4], h);
        return true;
        }

    return false;
    }

static bool    cbNCEnterInterval

(
FunctionContext *pFunctionContext,
const double *pXArray,
const double *pFArray,
int         n,
double      g
)
    {
    if (pFunctionContext->u1 < pXArray[0])
        return false;
    if (pFunctionContext->u0 > pXArray[n-1])
        return false;
    return true;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Leaf level integation.  The integral result is declared to be the     |
| integral under the polynomial through the given function values,      |
| evaluated at the u0 and u1 limits.   If the leaf is entirely inside   |
| u0..u1, the previously computed integral is accepted directly.        |
| Otherwise, explicitly form the bezier polynomial, integrate it to     |
| another bezier polynomial, and evaluate that at the limits.           |
+----------------------------------------------------------------------*/
static bool    cbNCLeaf

(
FunctionContext *pFunctionContext,
const double *pXArray,
const double *pFArray,
int         n,
double      g
)
    {
    double fraction0, fraction1;
    double x0 = pXArray[0];
    double x1 = pXArray[n - 1];
    double x01 = x1 - x0;
    double bezierCoffs[20];
    double bezierIntegral[21];
    double g0, g1;
    //int functionOrder = n;
    int integralOrder = n + 1;

    if (pFunctionContext->u0 <= pXArray[0]
        && pFunctionContext->u1 >= pXArray[n-1])
        {
        pFunctionContext->partialIntegral += g;
        }
    else
        {
        fraction0 = (pFunctionContext->u0 - x0) / x01;
        fraction1 = (pFunctionContext->u1 - x0) / x01;

        if (fraction0 < 0.0)
            fraction0 = 0.0;
        if (fraction1 > 1.0)
            fraction1 = 1.0;
        /* Back out quietly if nothing left ... */
        if (fraction0 < fraction1)
            {
            bsiBezier_univariateInterpolationPoles (bezierCoffs, pFArray, n);
            bsiBezier_univariateIntegralPoles (bezierIntegral, 0.0, x01, bezierCoffs, n);

            if (fraction0 > 0.0)
                bsiBezier_functionAndDerivative (&g0, NULL, bezierIntegral, integralOrder, 1, fraction0);
            else
                g0 = bezierIntegral[0]; /* Yup, that outght to be zero */

            if (fraction1 < 1.0)
                bsiBezier_functionAndDerivative (&g1, NULL, bezierIntegral, integralOrder, 1, fraction1);
            else
                g1 = bezierIntegral[n];

            pFunctionContext->partialIntegral += g1 - g0;
            }
        }
    return true;
    }


static bool    cbNCFinalNode

(
FunctionContext *pFunctionContext,
double      parentIntegral,
double      rightIntegral,
double      leftIntegral
)
    {
    double childIntegral = rightIntegral + leftIntegral;
    pFunctionContext->globalIntegral += childIntegral;
    pFunctionContext->globalError    += parentIntegral - childIntegral;
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
+----------------------------------------------------------------------*/
static double sortLimits

(
double *pX0,
double *pX1,
double  x0,
double  x1
)
    {
    double a;
    if (x0 < x1)
        {
        a = 1.0;
        *pX0 = x0;
        *pX1 = x1;
        }
    else
        {
        a = -1.0;
        *pX0 = x1;
        *pX1 = x0;
        }
    return a;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotesIntegration         |
|                                                                       |
| Integrate from u0 to u1, using polynomial approximations in binary    |
| subdivision of the interval x0 to x1.                                 |
| author        EarlinLutz                                  03/00       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiMath_recursiveNewtonCotesIntegration

(
double          *pIntegral,         /* <= resulting integration */
double          *pError,            /* <= achived absolute error */
int             *pCount,            /* <= function count */
void    *pUserData,
PFScalarIntegrand cbEvaluate,
int     numPoints,
double  x0,
double  x1,
double u0,
double u1,
double absErr,
double relErr
)
    {
    IntegrationContext  integrationContext;
    FunctionContext     functionContext;
    IntegrationGrid     rootGrid;
    double xSign, uSign;

    functionContext.pUserData = pUserData;
    functionContext.cbEvaluate = cbEvaluate;
    functionContext.x0 = x0;
    functionContext.x1 = x1;
    functionContext.u0 = u0;
    functionContext.u1 = u1;
    functionContext.partialIntegral = 0.0;
    functionContext.globalIntegral  = 0.0;
    functionContext.globalError     = 0.0;

    xSign = sortLimits (&x0, &x1, x0, x1);
    uSign = sortLimits (&u0, &u1, u0, u1);

    if (   u0 < x0 || u0 > x1 || u1 < x0 || u1 > x1)
        {
        /* The u range is not contained in the x range. Just integrate
                on the
        /* This is not fully repeatable, but it gives a result: */
        x0 = u0;
        x1 = u1;
        }

    initIntegrationContext
                (&integrationContext,
                &functionContext,
                numPoints,
                MAX_DEPTH,
                absErr,
                relErr,
                s_absTolFactor,
                s_relTolFactor,
                (IC_EvaluateFunc)cbNCEvaluateUserFunc,
                (IC_IntegrateFunc)cbNCIntegrate,
                (IC_IntervalFunc)cbNCEnterInterval,
                (IC_FinalNodeFunc)cbNCFinalNode,
                (IC_IntervalFunc)cbNCLeaf
                );

    if (   initRootGridFromStack (&integrationContext, &rootGrid, x0, x1)
        && continueRecursiveSubdivision (&integrationContext, &rootGrid, 0)
        )
        {
        if (pIntegral)
            *pIntegral = xSign * uSign * functionContext.partialIntegral;
        if (pError)
            *pError    = functionContext.globalError;
        if (pCount)
            *pCount = integrationContext.numCallEvaluate;

        closeIntegrationContext (&integrationContext);
        return true;
        }

    closeIntegrationContext (&integrationContext);
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMath_recursiveIncrementalNewtonCotes5               |
|                                                                       |
| author        EarlinLutz                                  05/99       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveIncremantalNewtonCotes5

(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
int             *pCount,            /* <= function count */
double          x0,                 /* => lower limit for integration */
double          x8,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
PFScalarIntegrand evaluateFunc,     /* => evaluate function for integrand */
PFExtendScalarIntegration extendFunc,/* => function to receive incremental
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
void            *userDataP          /* => passed through to evaluateFunc */
)
    {
    double f0, f2, f4, f6, f8, g01, a01;
    double x2, x4, x6;

    x4 = 0.5 * (x0 + x8);
    x2 = 0.5 * (x0 + x4);
    x6 = 0.5 * (x4 + x8);
    (*evaluateFunc) (&f0, x0, userDataP);
    (*evaluateFunc) (&f2, x2, userDataP);
    (*evaluateFunc) (&f4, x4, userDataP);
    (*evaluateFunc) (&f6, x6, userDataP);
    (*evaluateFunc) (&f8, x8, userDataP);

    nc5Quad (&g01, &a01, f0, f2, f4, f6, f8, x8 - x0);
    *integralP = *errorP = 0.0;
    *pCount = 5;
    return (recursiveNC5 (integralP, errorP, pCount,
        x0, f0, x2, f2, x4, f4, x6, f6, x8, f8, g01,
        absTol, relTol, evaluateFunc, extendFunc,
        userDataP, 0));
    }
#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          nc5QuadVector                                           |
|                                                                       |
| author        EarlinLutz                              08/98           |
|                                                                       |
| Estimate an integral by 5 point newton cotes rule, and estimate the   |
| absolute value integral by 4-part trapezoid rule.                     |
+----------------------------------------------------------------------*/
static void nc5QuadVector

(
double          *pSum,      /* <= simpson's rule estimate of integral */
double          *pAbs,      /* <= trapezoidal estimate of integral of absolute value */
double          *pF0,       /* => integrand at 0    */
double          *pF1,       /* => integrand at 0.25 */
double          *pF2,       /* => integrand at 0.5  */
double          *pF3,       /* => integrand at 0.75 */
double          *pF4,       /* => integrand at 1    */
double          delta,      /* => interval width */
int             numFunc
)
    {
    int i;
    for (i = 0; i < numFunc; i++)
        {
        nc5Quad (pSum + i, pAbs + i,
                    pF0[i], pF1[i], pF2[i], pF3[i], pF4[i], delta);
        }
    }

#define MAX_FUNCTION 32
typedef struct
    {
    PFVectorIntegrand evaluateFunc;
    void    *userDataP;
    int numFunc;
    int count;
    double a0[MAX_FUNCTION];
    double a1[MAX_FUNCTION];
    double localError[MAX_FUNCTION];
    /* Global sums -- only add to these after testing for convergence or depth limit */
    double g[MAX_FUNCTION];
    double error[MAX_FUNCTION];
    } VectorIntegrationParams;

/*----------------------------------------------------------------------+
|                                                                       |
| name          recursiveNC5Vector                                      |
|                                                                       |
| author        EarlinLutz                                  05/99       |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        sumLeftAndRightParts

(
double          *pG,                /* <= summed integrals */
double          *pError,            /* <= error estimate */
double          *pG0,               /* => left contribution */
double          *pG1,               /* => right contribution */
double          *pA0,               /* => approximate integral of absolute values on left */
double          *pA1,               /* => approximate integral of absolute values on right */
double          *pG01,              /* => approximation from next level up */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
int             numFunc
)
    {
    bool    withinTol = true;
    int i;
    double tol;
    for (i = 0; i < numFunc; i++)
        {
        pG[i] = pG0[i] + pG1[i];
        pError[i] = pG[i] - pG01[i];
        if (withinTol)
            {
            tol = absTol + relTol * (fabs (pA0[i]) + fabs (pA1[i]));
            withinTol = fabs (pError[i]) <= fabs (tol);
            }
        }
    return withinTol;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          accumulateResults                                       |
|                                                                       |
| author        EarlinLutz                                  05/99       |
|                                                                       |
+----------------------------------------------------------------------*/
static void    accumulateResults

(
double          *pGlobalG,          /* <=> accumulating integrals */
double          *pGlobalError,      /* <=> error estimate */
double          *pLocalG,           /* => local contribution */
double          *pLocalError,       /* => local contribution */
int             numFunc
)
    {
    int i;
    for (i = 0; i < numFunc; i++)
        {
        pGlobalG[i] += pLocalG[i];
        pGlobalError[i] += pLocalError[i];
        }
    }


#define MAX_FUNCTION 32
struct VectorIntegrationContext
    {
    BSIVectorIntegrand &integrand;
    int numFunc;
    int count;
    double a0[MAX_FUNCTION];
    double a1[MAX_FUNCTION];
    double localError[MAX_FUNCTION];
    /* Global sums -- only add to these after testing for convergence or depth limit */
    double globalSum[MAX_FUNCTION];
    double globalError[MAX_FUNCTION];

VectorIntegrationContext (BSIVectorIntegrand &_integrand)
    : integrand (_integrand)
    {
    numFunc = integrand.GetVectorIntegrandCount();
    }

bool     RunRecursion
(
double          x0,                 /* => lower limit for integration */
double          *pF0,               /* => integrand at lower limit */
double          x2,                 /* => 0.25 parameter */
double          *pF2,               /* => f(0.25) */
double          x4,                 /* => 0.50 parameter */
double          *pF4,               /* => f(0.50) */
double          x6,                 /* => 0.75 parameter */
double          *pF6,               /* => f(0.75) */
double          x8,                 /* => upper limit for integration */
double          *pF8,               /* => integrand at lower limit */
double          *pG01,              /* => prior estimate for overall integral */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
int             depth
)
    {
    double      x1 = 0.5 * (x0 + x2);
    double      x3 = 0.5 * (x2 + x4);
    double      x5 = 0.5 * (x4 + x6);
    double      x7 = 0.5 * (x6 + x8);
    double      f1[MAX_FUNCTION], f3[MAX_FUNCTION], f5[MAX_FUNCTION], f7[MAX_FUNCTION];

    double      g0[MAX_FUNCTION], g1[MAX_FUNCTION];
    double      g[MAX_FUNCTION];
    double      h   = 0.5 * (x8 - x0);

    bool        withinTol;


    integrand.EvaluateVectorIntegrand(x1, f1);
    integrand.EvaluateVectorIntegrand(x3, f3);
    integrand.EvaluateVectorIntegrand(x5, f5);
    integrand.EvaluateVectorIntegrand(x7, f7);
    count += 4;

    nc5QuadVector (g0, a0, pF0, f1, pF2, f3, pF4, h, numFunc);
    nc5QuadVector (g1, a1, pF4, f5, pF6, f7, pF8, h, numFunc);

    withinTol = sumLeftAndRightParts
                        (
                        g, localError,
                        g0, g1,
                        a0, a1, pG01,
                        absTol, relTol, numFunc);


    if (depth > MAX_DEPTH || withinTol)
        {
        accumulateResults (globalSum, globalError,  g, localError, numFunc);
        }
    else
        {
        bool    leftOK, rightOK;
        depth++;
        if (depth > s_maxDepth)
            s_maxDepth = depth;

        leftOK  = RunRecursion (
                  x0, pF0, x1, f1, x2, pF2, x3, f3, x4, pF4, g0,
                  s_absTolFactor * absTol, s_relTolFactor * relTol, depth);

        rightOK = RunRecursion (
                  x4, pF4, x5, f5, x6, pF6, x7, f7, x8, pF8, g1,
                  s_absTolFactor * absTol, s_relTolFactor * relTol, depth);

        withinTol = leftOK & rightOK;
        }

    return withinTol;
    }
};
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotes5                           |
|                                                                       |
| author        EarlinLutz                                  05/99       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5Vector

(
double          *pIntegral,         /* <= resulting integration */
double          *pError,            /* <= achived absolute error */
int             *pCount,            /* <= function count */
double          x0,                 /* => lower limit for integration */
double          x8,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
BSIVectorIntegrand &integrand
)
    {
    double f0[MAX_FUNCTION], f2[MAX_FUNCTION], f4[MAX_FUNCTION], f6[MAX_FUNCTION], f8[MAX_FUNCTION];
    double g01[MAX_FUNCTION], a01[MAX_FUNCTION];

    double x2, x4, x6;
    VectorIntegrationContext params (integrand);
    bool    boolstat = false;

    if (params.numFunc <= MAX_FUNCTION)
        {
        memset (params.globalSum, 0, MAX_FUNCTION * sizeof (double));
        memset (params.globalError, 0, MAX_FUNCTION * sizeof (double));
        x4 = 0.5 * (x0 + x8);
        x2 = 0.5 * (x0 + x4);
        x6 = 0.5 * (x4 + x8);

        params.integrand.EvaluateVectorIntegrand(x0, f0);
        params.integrand.EvaluateVectorIntegrand(x2, f2);
        params.integrand.EvaluateVectorIntegrand(x4, f4);
        params.integrand.EvaluateVectorIntegrand(x6, f6);
        params.integrand.EvaluateVectorIntegrand(x8, f8);

        nc5QuadVector (g01, a01, f0, f2, f4, f6, f8, x8 - x0, params.numFunc);

        params.count = 5;

        boolstat = params.RunRecursion (
            x0, f0, x2, f2, x4, f4, x6, f6, x8, f8, g01,
            absTol, relTol, 0);
        if (pIntegral)
            memcpy (pIntegral, params.globalSum, params.numFunc * sizeof (double));
        if (pError)
            memcpy (pError, params.globalError, params.numFunc * sizeof (double));
        if (pCount)
            *pCount = params.count;
        }
    return boolstat;
    }


#ifdef abc
// DEPRECATED BY HARD TO STOMP . . . 


/*----------------------------------------------------------------------+
|                                                                       |
| name          recursiveNC5Vector                                      |
|                                                                       |
| author        EarlinLutz                                  05/99       |
|                                                                       |
+----------------------------------------------------------------------*/
static bool     recursiveNC5Vector

(
VectorIntegrationParams *pParams,
double          x0,                 /* => lower limit for integration */
double          *pF0,               /* => integrand at lower limit */
double          x2,                 /* => 0.25 parameter */
double          *pF2,               /* => f(0.25) */
double          x4,                 /* => 0.50 parameter */
double          *pF4,               /* => f(0.50) */
double          x6,                 /* => 0.75 parameter */
double          *pF6,               /* => f(0.75) */
double          x8,                 /* => upper limit for integration */
double          *pF8,               /* => integrand at lower limit */
double          *pG01,              /* => prior estimate for overall integral */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
int             depth
)
    {
    double      x1 = 0.5 * (x0 + x2);
    double      x3 = 0.5 * (x2 + x4);
    double      x5 = 0.5 * (x4 + x6);
    double      x7 = 0.5 * (x6 + x8);
    double      f1[MAX_FUNCTION], f3[MAX_FUNCTION], f5[MAX_FUNCTION], f7[MAX_FUNCTION];

    double      g0[MAX_FUNCTION], g1[MAX_FUNCTION];
    double      g[MAX_FUNCTION];
    double      h   = 0.5 * (x8 - x0);

    bool        withinTol;


    pParams->evaluateFunc (f1, x1, pParams->userDataP, pParams->numFunc);
    pParams->evaluateFunc (f3, x3, pParams->userDataP, pParams->numFunc);
    pParams->evaluateFunc (f5, x5, pParams->userDataP, pParams->numFunc);
    pParams->evaluateFunc (f7, x7, pParams->userDataP, pParams->numFunc);
    pParams->count += 4;

    nc5QuadVector (g0, pParams->a0, pF0, f1, pF2, f3, pF4, h, pParams->numFunc);
    nc5QuadVector (g1, pParams->a1, pF4, f5, pF6, f7, pF8, h, pParams->numFunc);

    withinTol = sumLeftAndRightParts
                        (
                        g, pParams->localError,
                        g0, g1,
                        pParams->a0, pParams->a1, pG01,
                        absTol, relTol, pParams->numFunc);


    if (depth > MAX_DEPTH || withinTol)
        {
        accumulateResults (pParams->g, pParams->error,  g, pParams->localError, pParams->numFunc);
        }
    else
        {
        bool    leftOK, rightOK;
        depth++;
        if (depth > s_maxDepth)
            s_maxDepth = depth;

        leftOK  = recursiveNC5Vector (pParams,
                  x0, pF0, x1, f1, x2, pF2, x3, f3, x4, pF4, g0,
                  s_absTolFactor * absTol, s_relTolFactor * relTol, depth);

        rightOK = recursiveNC5Vector (pParams,
                  x4, pF4, x5, f5, x6, pF6, x7, f7, x8, pF8, g1,
                  s_absTolFactor * absTol, s_relTolFactor * relTol, depth);

        withinTol = leftOK & rightOK;
        }

    return withinTol;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiMath_recursiveNewtonCotes5                           |
|                                                                       |
| author        EarlinLutz                                  05/99       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsiMath_recursiveNewtonCotes5Vector

(
double          *pIntegral,         /* <= resulting integration */
double          *pError,            /* <= achived absolute error */
int             *pCount,            /* <= function count */
double          x0,                 /* => lower limit for integration */
double          x8,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
PFVectorIntegrand evaluateFunc,     /* => evaluate function for integrand */
void            *userDataP,         /* => passed through to evaluateFunc */
int             numFunc             /* => number of functions being integrated */
)
    {
    double f0[MAX_FUNCTION], f2[MAX_FUNCTION], f4[MAX_FUNCTION], f6[MAX_FUNCTION], f8[MAX_FUNCTION];
    double g01[MAX_FUNCTION], a01[MAX_FUNCTION];

    double x2, x4, x6;
    VectorIntegrationParams params;
    bool    boolstat = false;

    memset (params.g, 0, MAX_FUNCTION * sizeof (double));
    memset (params.error, 0, MAX_FUNCTION * sizeof (double));


    if (numFunc <= MAX_FUNCTION)
        {
        x4 = 0.5 * (x0 + x8);
        x2 = 0.5 * (x0 + x4);
        x6 = 0.5 * (x4 + x8);

        (*evaluateFunc) (f0, x0, userDataP, numFunc);
        (*evaluateFunc) (f2, x2, userDataP, numFunc);
        (*evaluateFunc) (f4, x4, userDataP, numFunc);
        (*evaluateFunc) (f6, x6, userDataP, numFunc);
        (*evaluateFunc) (f8, x8, userDataP, numFunc);

        nc5QuadVector (g01, a01, f0, f2, f4, f6, f8, x8 - x0, numFunc);

        params.count = 5;
        params.evaluateFunc = evaluateFunc;
        params. numFunc = numFunc;
        params.userDataP = userDataP;

        boolstat = recursiveNC5Vector (&params,
            x0, f0, x2, f2, x4, f4, x6, f6, x8, f8, g01,
            absTol, relTol, 0);
        if (pIntegral)
            memcpy (pIntegral, params.g, numFunc * sizeof (double));
        if (pError)
            memcpy (pError, params.error, numFunc * sizeof (double));
        if (pCount)
            *pCount = params.count;
        }
    return boolstat;
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
