/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_rangeEpsilon = 1.0e-8;

/* NOT USED YET */
/*---------------------------------------------------------------------------------**//**
* Remove roots if they lie outside parameter range.
*
* @param pRootArray <=> initial / filtered roots
* @param pNumRoot   <=> # initial roots / # roots after filtering
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiBezier_filterRootRange
(
double  *pRootArray,
int     *pNumRoot
)
    {
    int n = 0;
    int i;
    double s0 = -s_rangeEpsilon;
    double s1 = 1.0 + s_rangeEpsilon;

    for (i = 0; i < *pNumRoot; i++)
        {
        if (pRootArray[i] > s0
            && pRootArray[i] < s1)
            {
            pRootArray[n++] = pRootArray[i];
            }
        }
    *pNumRoot = n;
    }

/*---------------------------------------------------------------------------------**//**
* Finds the first root of the control polygon for a Bezier function. For example,
* if the degree d Bezier curve C(t) = [X(t), Y(t), Z(t)] has the (weighted) poles
* b_i = [x_i, y_i, z_i], then X(t) is a degree d Bezier function with poles
* (coefficients) x_i, and the control polygon for this function has vertices at
* the points (i/d, x_i).  Similarly for Y(t) and Z(t).
*
* @param pRoot      <= the (one) root found
* @param pA         => coefficients (Bezier ordinates) of the Bezier function
* @param order      => Bezier order
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezier_firstPolygonRoot
(
double      *pRoot,
double      *pA,
int         order
)
    {
    int i;
    double a0 = pA[0];
    double s;
    for (i = 1; i < order; i++)
        {
        if (a0 * pA[i] <= 0.0)      /* sign change => root */
            {
            if (!DoubleOps::SafeDivide (s, -a0, pA[i] - a0, 0.0))
                {
                s = 0.0;
                }
            *pRoot = ((double) (i - 1) + s) / (double) (order - 1);
            return true;
            }
        a0 = pA[i];
        }
    return false;
    }

static void checkMonotonePolygon
(
double      *pA,
int         order
)
    {
    double da = pA[order-1] - pA[0];
    double da0 = pA[1] - pA[0];
    double da1 = pA[order-1] - pA[order-2];

    if (da * da0 < 0.0)
        {
        pA[1] = pA[0];
        }
    if (da * da1 < 0.0)
        {
        pA[order-2] = pA[order-1];
        }
    }

/*---------------------------------------------------------------------------------**//**
* Approximate the root (if present) of a monotonic Bezier function using smallest
* root of the control polygon as an initial guess for Newton-Raphson.
*
* @param pRoot      <= approximation to the root (if any) in [0,1].
* @param pA         => coefficients of the Bezier function
* @param order      => Bezier order
* @return true if and only if Newton-Raphson quickly converges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezier_findRootUsingMonotonicPolygon
(
double      *pRoot,
double      *pA,
int         order
)
    {
    int i;
    /* NR normally converges quadratically -- double the bits per
        iteration, and 3 to 6 shots finishes.
      At a double root, we only add one bit per iteration.
        Allow enough bits to get to IEEE double 53 precision, plus a few
        for getting started. */
    static int maxIteration = 55;
    /* Treat function values as zeros if they are significantly smaller
        than the full interval function change. */
    static double s_functionRelTol = 1.0e-22;
    double functionTolerance = s_functionRelTol * fabs (pA[order-1] - pA[0]);
    double f, df, duOld, du, uL, uH;

    /* If a NR step moves by less than 1e-10, we expect the next step to move
        by the square of that amount, i.e. 1.0e-20.  At the upper end of 0..1
        the system number representation has granularity of 1e-15, so the
        additional step is meaningless.  There may be some cases where a root
        is so close to 0 that the additional step is meaninful, but
        we'll step with the full-interval tolerance. */
    static double s_stepTolerance = 1.0e-12;

    checkMonotonePolygon (pA, order);
    /* catch nonsimple roots and avoid N-R slowdown */
    if (fabs (pA[0]) < functionTolerance)
        {
        *pRoot = 0.0;
        return true;
        }

    if (fabs (pA[order - 1]) < functionTolerance)
        {
        *pRoot = 1.0;
        return true;
        }

    /* Caller claims the function is monotone on the interval.
        If there is no sign change, it's just a wobble on one side of the x axis. */
    if (pA[order-1] * pA[0] > 0.0)
        return false;

    /* Since monotonic control polygon has root <=> curve has root, get initial
        guess from smallest root of control polygon, if there is one. */
    if (!bsiBezier_firstPolygonRoot (pRoot, pA, order))
        return false;

    /* Test initial guess *pRoot right away */
    /* Is this good? */
    bsiBezier_evaluateUnivariate (&f, &df, pA, order, *pRoot);
    if (fabs (f) < functionTolerance)
        return true;

    /* Initial guess needs refinement.  First, orient refinement scheme so
        that f(u0) < 0 (this facilitates root bracketing). */
    if (pA[0] < 0.0)
        {
        uL = 0.0;       /* curve increasing */
        uH = 1.0;
        }
    else
        {
        uL = 1.0;       /* curve decreasing */
        uH = 0.0;
        }

    du = duOld = 1.0;

    /* N-R: see Numerical Recipies */
    for (i = 0; i < maxIteration; i++)
        {
        /* if iteration goes outside range or doesn't cut duOld in half, bisect */
        if ((((*pRoot - uH) * df - f) * ((*pRoot - uL) * df - f) > 0.0) ||
                (fabs (2.0 * f) > fabs (duOld * df)))
            {
            duOld = du;
            du = 0.5 * (uH - uL);
            *pRoot = uL + du;
            }
        /* otherwise, perform N-R step */
        else
            {
            duOld = du;
            du = f / df;
            *pRoot -= du;
            }

        /* this guess didn't change much from the last guess; call it the root */
        if (fabs (du) < s_stepTolerance)
            {
            /* printf ("numits = %2i\n", i); */
            return true;
            }
        bsiBezier_evaluateUnivariate (&f, &df, pA, order, *pRoot);

        /* tighten bracket around root */
        if (f < 0.0)
            uL = *pRoot;
        else
            uH = *pRoot;
        }

    /* N-R failed to converge within maxIteration steps */
    /* printf ("numits = MAX\n"); */
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Approximate the root (if there is one) of a Bezier function in the interval
* [lowerLimit,upperLimit], on which the function is monotonic.
*
* @param pRoot      <= the root, if found in the interval
* @param pA         => coefficients of the full Bezier curve
* @param order      => Bezier order
* @param lowerLimit => low end of interval >= 0.0
* @param upperLimit => high end of interval <= 1.0
* @return true if and only if a root was found
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezier_rootInMonotonicInterval
(
double      *pRoot,
double      *pA,
int         order,
double      lowerLimit,
double      upperLimit
)
    {
    double leftCoff[MAX_BEZIER_ORDER];
    double rightCoff[MAX_BEZIER_ORDER];
    double dummy[MAX_BEZIER_ORDER];
    double localRoot, localUpperLimit;

    /* There are four cases, depending on the location of monotonicity interval
        [lowerLimit, upperLimit] within [0,1]. */

    if (lowerLimit == 0.0)
        {
        /* 1. entire curve is monotonic */
        if (upperLimit == 1.0)
            return bsiBezier_findRootUsingMonotonicPolygon (pRoot, pA, order);

        /* 2. monotone to left of upperLimit */
        else
            {
            bsiBezier_subdivisionPolygons
                    (leftCoff, dummy, pA, order, 1, upperLimit);

            if (bsiBezier_findRootUsingMonotonicPolygon (&localRoot, leftCoff, order))
                {
                *pRoot = localRoot * upperLimit;
                return true;
                }
            }
        }
    else
        {
        bsiBezier_subdivisionPolygons (leftCoff, rightCoff, pA, order, 1, lowerLimit);

        /* 3. monotone to right of lowerLimit */
        if (upperLimit == 1.0)
            {
            if (bsiBezier_findRootUsingMonotonicPolygon (&localRoot, rightCoff, order))
                {
                *pRoot = lowerLimit + localRoot * (1.0 - lowerLimit);
                return true;
                }
            }

        /* 4. monotone strictly inside (0,1) */
        else
            {
            localUpperLimit = (upperLimit - lowerLimit) / (1.0 - lowerLimit);

            bsiBezier_subdivisionPolygons
                    (leftCoff, dummy, rightCoff, order, 1, localUpperLimit);

            if (bsiBezier_findRootUsingMonotonicPolygon (&localRoot, leftCoff, order))
                {
                *pRoot = lowerLimit + localRoot * (upperLimit - lowerLimit);
                return true;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Find the range of a Bezier function and count upward and downward changes in the
* control polygon.
*
* @param pAMin      <= smallest coefficient
* @param pAMax      <= largest coefficient
* @param pNumUp     <= number of consecutive pairs of increasing coefficients
* @param pNumDown   <= number of consecutive pairs of decreasing coefficients
* @param pNumZero   <= number of consecutive pairs of equal coefficients
* @param pA         => coefficients of the Bezier function
* @param order      => Bezier order
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_evaluateRangeAndShape
(
double          *pAMin,
double          *pAMax,
int             *pNumUp,
int             *pNumDown,
int             *pNumZero,
double          *pA,
int             order
)
    {
    int i;
    double delta;
    double a;
    int numUp, numDown, numZero;
    *pAMin = *pAMax = pA[0];
    numUp = numDown = numZero = 0;
    for (i = 1; i < order; i++)
        {
        a = pA[i];
        delta = a - pA[i-1];
        if (a < *pAMin)
            *pAMin = a;
        if (a > *pAMax)
            *pAMax = a;
        if (delta > 0.0)
            numUp++;
        else if (delta < 0.0)
            numDown++;
        else
            numZero++;
        }
    *pNumUp     = numUp;
    *pNumDown   = numDown;
    *pNumZero   = numZero;
    }


/*---------------------------------------------------------------------------------**//**
* Find the (up to order-1) roots of a univariate Bezier curve over [0,1], not counting
* multiplicities.  If identically zero, return order "roots" equispaced in [0,1].
*
* @param pRootArray     <= array of roots.  Caller must allocate with size >= order
* @param pNumRoot       <= number of roots
* @param pA             => coefficients of the bezier
* @param order          => bezier order > 1
* @param bPreferAnalytic => true to favor using analytic solutions for cubic and quartic.
* @return   false if order invalid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRootsOptionalAnalytic
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order,
bool            bPreferAnalytic
)
    {
    /* Do not use any tolerance to find near roots (except in
        bsiMath_solveConvexQuadratic and
        bsiBezier_findRootUsingMonotonicPolygon).
        Tolerancing within epsilon can be handled by caller if this
        routine is called (twice) on translations of the coefficient
        vector by +/- epsilon. */
    static int sTrustQuarticSolver = 0;
    static int sTrustCubicSolver   = 1;

    int     i, n, numUp, numDown, numZero;
    double  aMin, aMax;
    *pNumRoot = 0;

    if (order < 2 || order > MAX_BEZIER_ORDER)
        return false;

    bsiBezier_evaluateRangeAndShape
            (&aMin, &aMax, &numUp, &numDown, &numZero, pA, order);

    /* No roots since polygon entirely above/below axis */
    if (aMin * aMax > 0.0)
        {
        *pNumRoot = 0;
        return true;
        }

    /* Constant Bezier function identically zero; return order "roots" */
    else if (numZero == order - 1)
        {
        *pNumRoot = order;
        pRootArray[0] = 0.0;
        pRootArray[numZero] = 1.0;
        for (i = 1; i < numZero; i++)
            pRootArray[i] = (double) i / (double) numZero;
        }

    /* Linear Bezier function with guaranteed unique root in [0,1] */
    else if (order == 2)
        {
        *pNumRoot = 1;
        if (pA[0] == 0.0)
            pRootArray[0] = 0.0;
        else if (pA[1] == 0.0)
            pRootArray[0] = 1.0;
        else
            pRootArray[0] = - pA[0] / (pA[1] - pA[0]);
        }

    /* Quadratic Bezier function; 0, 1 or 2 roots (not necessarily in [0,1]).
        If the function has any roots at all, one must theoretically lie in
        [0,1], but range-check them all just to be safe. */
    else if (order == 3)
        {
        double dummy[2];
        *pNumRoot = bsiMath_solveConvexQuadratic
                (dummy, pRootArray, pA[0], 2.0 * pA[1], pA[2]);

        /* strip roots not in [0,1] */
        for (i = n = 0; i < *pNumRoot; i++)
            if (pRootArray[i] >= 0.0 && pRootArray[i] <= 1.0)
            pRootArray[n++] = pRootArray[i];
        *pNumRoot = n;

        /* if 2 roots, coalesce or order them */
        if (*pNumRoot == 2)
            {
            if (pRootArray[0] == pRootArray[1])
                *pNumRoot = 1;
            else if (pRootArray[0] > pRootArray[1])
                {
                double temp = pRootArray[0];
                pRootArray[0] = pRootArray[1];
                pRootArray[1] = temp;
                }
            }
        }
    else if (order == 4 && bPreferAnalytic && sTrustCubicSolver)
        {
        *pNumRoot = AnalyticRoots::SolveBezierCubic (pA[0], pA[1], pA[2], pA[3], 0.0, pRootArray, true);
        }
    else if (order == 5 && bPreferAnalytic && sTrustQuarticSolver)
        {
        *pNumRoot = AnalyticRoots::SolveBezierQuartic (pA[0], pA[1], pA[2], pA[3], pA[4], 0.0, pRootArray, true);
        }
    /* Strictly monotonic Bezier function with unique root in [0,1],
        since control polygon is nonconstant, monotonic & and crosses axis */
    else if (numUp == 0 || numDown == 0)
        {
        double rt;
        if (bsiBezier_findRootUsingMonotonicPolygon (&rt, pA, order))
            {
            *pNumRoot = 1;
            pRootArray[0] = rt;
            }
        else
            {
            *pNumRoot = 0;
            return false;
            }
        }

    /* Bezier function has order > 3 and is not monotonic.  Look for roots
        of the derivative as splitters to subdivide into monotone segments. */
    else
        {
        int orderM1 = order - 1;
        double dCoff[MAX_BEZIER_ORDER];
        double dRoot[MAX_BEZIER_ORDER];
        double dFunc[MAX_BEZIER_ORDER];

        double lowerLimit, upperLimit;
        double root, lastRoot, f0, f1, f2;
        static double s_doubleRootRelTol = 1.0e-8;
        static double s_doubleRootRelTol1 = 1.0e-12;
        double doubleRootTol = s_doubleRootRelTol * (aMax - aMin);
        double doubleRootTol1 = s_doubleRootRelTol1 * (aMax - aMin);
        bool    isDoubleRoot;
        int numSplit;


        for (i = 0; i < orderM1; i++)       /* Bezier with these coeffs     */
            dCoff[i] = pA[i+1] - pA[i];     /* shares roots with true deriv */

        /* recurse on derivative */
        bsiBezier_univariateRootsOptionalAnalytic (dRoot, &numSplit, dCoff, orderM1, bPreferAnalytic);

        /* Guaranteed at this point:
            dRoot contains numSplit > 0 unique, ordered split points in [0,1]
            these split points (together with 0.0 and 1.0) determine
            subintervals on which the Bezier function is monotonic. */

        /* force the right end into the split array if needed */
        if (numSplit == 0 || dRoot[numSplit - 1] < 1.0)
            dRoot[numSplit++] = 1.0;

        for (i = 0; i < numSplit; i++)
            {
            bsiBezier_evaluateUnivariate (&dFunc[i], NULL, pA, order, dRoot[i]);
            }

        /* Look for a possible root in each interval of monotonicity.
            Skip first split point if 0.0.
            Do not count roots at split points twice.

           Even degree multiple roots are slippery.
           Look for **internal** mutiple roots with the special properties:
                * adjacent minima or endpoints have same sign
                * f at minima is small within tolerance.
            This is modestly expensive -- it requires direct evaluation
                of function value at all minima.  With enough code, this
                can be done using endpoint coefficients in the subdivisions
                within rootInMonotonicInterval.
        */
        *pNumRoot = 0;
        lowerLimit = 0.0;
        lastRoot = -1.0;
        f0 = pA[0];
        for (i = (dRoot[0] != 0.0) ? 0 : 1;
            lowerLimit < 1.0 && i < numSplit && *pNumRoot < order;
            i++)
            {
            isDoubleRoot = false;
            upperLimit = dRoot[i];
            f1 = dFunc[i];
            if (    i < numSplit - 1
                &&  lowerLimit < dRoot[i]
                &&  dRoot[i] < dRoot[i+1]
                )
                {
                f2 = dFunc[i+1];
                if (f0 * f2 > 0.0 && fabs (f1) < (f0 * f1 > 0.0 ? doubleRootTol : doubleRootTol1))
                    {
                    /* dRoot[i] is a root .. */
                    pRootArray[(*pNumRoot)++] = lastRoot = dRoot[i];
                    /* and dRoot[i+1] is the upper limit of
                        the expanded interval containing the
                        double root */
                    i++;
                    upperLimit = dRoot[i];
                    f1 = f2;
                    isDoubleRoot = true;
                    }
                }

            if (  !isDoubleRoot
               && bsiBezier_rootInMonotonicInterval
                        (&root, pA, order, lowerLimit, upperLimit)
               && root != lastRoot
               )
                {
                pRootArray[(*pNumRoot)++] = lastRoot = root;
                }
            lowerLimit = upperLimit;
            f0 = f1;
            }
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Find the (up to order-1) roots of a univariate Bezier curve over [0,1], not counting
* multiplicities.  If identically zero, return order "roots" equispaced in [0,1].
*
* @param pRootArray     <= array of roots.  Caller must allocate with size >= order
* @param pNumRoot       <= number of roots
* @param pA             => coefficients of the bezier
* @param order          => bezier order > 1
* @return   false if order invalid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRoots
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order
)
    {
    static int      sbPreferAnalytic = 0;
    double testRoot[100];
    int numTestRoot;
    static double sErrorCount = 0;

    static int sCounter = 0;
    static int sCounter2 = 0;
    static int sCounter3 = 0;
    static int sCounter4 = 0;
    static int sCounter5 = 0;
    static int sCounter6 = 0;
    static int sCounterX = 0;
    static int sNumRoot0 = 0;
    static int sNumRootX = 0;
    static int sNumRoot1 = 0;
    sCounter++;
    if (order == 2)
        sCounter2++;
    else if (order == 3)
        sCounter3++;
    else if (order == 4)
        sCounter4++;
    else if (order == 5)
        sCounter5++;
    else if (order == 6)
        sCounter6++;
    else
        sCounterX++;

    if (sbPreferAnalytic == 2)
        {
        // For debugging -- solve both ways, compare solutions.  Error counter makes easy place to stop in debugger.
        static double sRootTol = 1.0e-12;
        bsiBezier_univariateRootsOptionalAnalytic (testRoot, &numTestRoot, pA, order, 0);
        bsiBezier_univariateRootsOptionalAnalytic (pRootArray, pNumRoot, pA, order, 0 != sbPreferAnalytic);
        sErrorCount = 0;
        if (*pNumRoot != numTestRoot)
            {
            sErrorCount++;
            }
        else
            {
            for (int i = 0; i < numTestRoot; i++)
                {
                if (fabs (testRoot[i] - pRootArray[i]) > sRootTol)
                    sErrorCount++;
                }
            }
        }
    bool    stat =  bsiBezier_univariateRootsOptionalAnalytic (pRootArray, pNumRoot, pA, order, 0 != sbPreferAnalytic);
    if (*pNumRoot == 0)
        sNumRoot0++;
    else if (*pNumRoot == 1)
        sNumRoot1++;
    else
        sNumRootX += *pNumRoot;
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* Find the (up to degree) roots of a univariate polynomial over the entire real axis.
* The polynomial is summation of pStandardCoff[k]*x^k.
*
* @param pRootArray     <= array of roots.  Caller must allocate with size >= order
* @param pNumRoot       <= number of roots
* @param pStandarddCoff => degree + 1 standard form coefficients of the polynomial
* @param degree         => degree of polynomial
* @return   false if order invalid
* @bsimethod
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateStandardRoots
(
double          *pRootArray,
int             *pNumRoot,
double          *pStandardCoff,
int             degree
)
    {
    int order = degree + 1;
    int i, totalRoot, numMappedRoot;
    double u;
    double bezierCoff[MAX_BEZIER_ORDER];
    double bezierRoot[MAX_BEZIER_ORDER];

    totalRoot = *pNumRoot = 0;

    if (order < 2 || order > MAX_BEZIER_ORDER)
        return false;

    /* Remap so negative-x roots are in [0,1] for a bezier.
       Ignore roots at zero -- they will be computed in the positive half.
       Copy to output in reverse order so the x values are sorted. */
    bsiBezier_mapNegativeRealsToBezier (bezierCoff, pStandardCoff, order);
    bsiBezier_univariateRoots (bezierRoot, &numMappedRoot, bezierCoff, order);

    for (i = numMappedRoot - 1; i >= 0; i--)
        {
        u = bezierRoot[i];
        if (u > 0.0 && u < 1.0 && totalRoot < degree)
            {
            pRootArray[totalRoot++] = - u / (1.0 - u);
            }
        }


    /* Remap so positive-x roots are in [0,1] for a bezier: */
    bsiBezier_mapPositiveRealsToBezier (bezierCoff, pStandardCoff, order);
    bsiBezier_univariateRoots (bezierRoot, &numMappedRoot, bezierCoff, order);

    for (i = 0; i < numMappedRoot; i++)
        {
        u = bezierRoot[i];
        if (u < 1.0 && totalRoot < degree)
            {
            pRootArray[totalRoot++] = u / (1.0 - u);
            }
        }

    *pNumRoot = totalRoot;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Find the (up to degree) roots of a bezier polynomial over the entire real axis.
*
* @param pRootArray     <= array of roots.  Caller must allocate with size >= order
* @param pNumRoot       <= number of roots
* @param pBezCoff       => degree + 1 bezier coefficients of the polynomial
* @param degree         => degree of polynomial
* @return   false if order invalid
* @bsimethod
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateRootsExt
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order
)
    {
    double powerCoffs[MAX_BEZIER_ORDER];

    if (pNumRoot)
        *pNumRoot = 0;

    if (order > MAX_BEZIER_ORDER)
        return false;

    bsiBezier_convertBezierToPower (powerCoffs, pA, order);
    return bsiBezier_univariateStandardRoots (pRootArray, pNumRoot, powerCoffs, order - 1);
    }

/*---------------------------------------------------------------------------------**//**
* Conditionally look for a simple roots very just outside an endpoint; ADD the root to
*  the root array.  A root is considered simple if a simple newton iteration leads
*   there VERY quickly, and it is within specified tolerance of the endpoint.
*
*
* @param pRootArray     <= array of roots.  Caller must allocate with size >= order
* @param pNumRoot       <= number of roots
* @param pBezCoff       => degree + 1 bezier coefficients of the polynomial
* @param degree         => degree of polynomial
* @param select         => 0 or 1, selects corresponding endpoint of 01 interval.
* @param insideEpsilon  => skip if there is already a root this close to endpoint.
*                           If 0 is given, a default will be used.
* @param outsideEpsilon => accept new root of on outside and this close.
*                           If 0 is given, a default will be used.
* @return   true if a root was added.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_addRootNearEndpoint
(
double          *pRootArray,
int             *pNumRoot,
double          *pA,
int             order,
int             select,
double          insideEpsilon,
double          outsideEpsilon
)
    {
    double a0, a1;
    double s, step;
    double u0, u1;  /* Limits of insideEpsilon interval */
    double v0, v1;  /* Limits of outsideEpsilon interval */
    static double s_startupFactor = 1.0e-10;
    static int s_maxNewtonStep = 5;
    static int s_avoidInternalRoots = 0;
    static double s_maxStep = 1.0e-4;
    static double s_insideEpsilon = 1.0e-10;
    static double s_outsideEpsilon = 1.0e-10;
    static double s_convergenceFactor = 0.25;
    double f, df;
    int i;
    double stepTol;

    if (*pNumRoot >= order - 1)
        return false;

    if (insideEpsilon < s_insideEpsilon)
        insideEpsilon = s_insideEpsilon;
    if (outsideEpsilon < s_outsideEpsilon)
        outsideEpsilon = s_outsideEpsilon;

    stepTol = s_convergenceFactor * s_outsideEpsilon;

    // u0,u1 = search zone for prior roots.
    // v0,v1 = live zone for new roots.
    if (select == 0)
        {
        a0 = pA[0];
        a1 = pA[1];
        s = 0.0;
        u0 = v0 = insideEpsilon;
        u1 = v1 = -outsideEpsilon;
        }
    else if (select == 1)
        {
        a0 = pA[order - 1];
        a1 = pA[order - 2];
        s = 1.0;
        u0 = v0 = s - insideEpsilon;
        u1 = v1 = s + outsideEpsilon;
        }
    else
        return false;

    for (i = 0; i < *pNumRoot; i++)
        if ((pRootArray[i] - u0) * (pRootArray[i] - u1) <= 0.0)
            return false;

    /* Near-endpoint roots are only interesting if the
        final polygon edge does not cross zero but aims
        sharply at 0... */
    if (s_avoidInternalRoots && a0 * a1 <= 0.0)
        return false;


    if (fabs (a0) < s_startupFactor * fabs (a1))
        {
        /* Polygon suggests a root near the endpoint.  Do some Newton steps: */
        for (i = 0; i < s_maxNewtonStep; i++)
            {
            bsiBezier_evaluateUnivariate (&f, &df, pA, order, s);
            if (fabs (f) >= s_maxStep * fabs (df))
                return false;
            step = f / df;
            s -= step;
            if (step < stepTol)
                {
                if ((s-v0) * (s-v1) <= 0.0)
                    {
                    pRootArray[*pNumRoot] = s;
                    *pNumRoot += 1;
                    return true;
                    }
                else
                    return false;
                }
            }
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE