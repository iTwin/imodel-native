/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////////
//
// Roots.cpp - Global functions for finding the roots of quadradic, cubic
//             and quartic equations.  Adopted directly from  Graphic Gems I
//             and left in its original form.
//
// Author: John Lynch           November 1997
//
//
///////////////////////////////////////////////////////////////////////////////////


/*
 *  Roots3And4.c
 *
 *  Utility functions to find cubic and quartic roots,
 *  coefficients are passed like this:
 *
 *      c[0] + c[1]*x + c[2]*x^2 + c[3]*x^3 + c[4]*x^4 = 0
 *
 *  The functions return the number of non-complex roots and
 *  put the values into the s array.
 *
 *  Author:         Jochen Schwarze (schwarze@isa.de)
 *
 *  Jan 26, 1990    Version for Graphics Gems
 *  Oct 11, 1990    Fixed sign problem for negative q's in SolveQuartic
 *                  (reported by Mark Podlipec),
 *                  Old-style function definitions,
 *                  IsZero() as a macro
 *  Nov 23, 1990    Some systems do not declare acos() and cbrt() in
 *                  <math.h>, though the functions exist in the library.
 *                  If large coefficients are used, EQN_EPS should be
 *                  reduced considerably (e.g. to 1E-30), results will be
 *                  correct but multiple roots might be reported more
 *                  than once.
 */
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define _USE_MATH_DEFINES
#define DIVIDE_ONCE 1
END_BENTLEY_GEOMETRY_NAMESPACE
#include    <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* epsilon surrounding for near zero values */

// TODO: merge this with the fuzz handling in GeomUtil (or at least review it).
static double EQN_EPS = 1.0e-9;
static bool IsZero (double x)
    {
    return fabs (x) < EQN_EPS;
    }
static bool IsSmallRatio (double x, double y, double abstol = 1.0e-9,  double reltol = 8.0e-16)
    {
    return fabs (x) <= abstol || fabs (x) < reltol * fabs (y);
    }



#define NOCBRT      // none in VC that I can find.
#ifdef NOCBRT
#define     cbrt(x)     ((x) > 0.0 ? pow((double)(x), 1.0/3.0) : \
                          ((x) < 0.0 ? -pow((double)-(x), 1.0/3.0) : 0.0))
#endif

static double s_safeDivideFactor = 1.0e-14;
bool AnalyticRoots::SafeDivide
(
double &result,
double numerator,
double denominator,
double defaultResult
)
    {
    if (fabs (denominator) > s_safeDivideFactor * fabs (numerator))
        {
        result = numerator / denominator;
        return true;
        }
    result = defaultResult;
    return false;
    }

int AnalyticRoots::SolveLinear(double c[2], double s[1])
    {
    // c0 + c1*x= 0
    // x = - c0 / c1
    if (AnalyticRoots::SafeDivide (s[0], -c[0], c[1], 0.0))
        return 1;
    return 0;
    }


int AnalyticRoots::SolveQuadric(double c[3], double s[2])
{
    double p, q, D;

    /* normal form: x^2 + 2 p x + q = 0 */

#ifdef DIVIDE_ONCE
    double divFactor;
    if (!AnalyticRoots::SafeDivide (divFactor, 1.0, c[2], 0.0))
        return AnalyticRoots::SolveLinear (c, s);
    p = 0.5 * c[1] * divFactor;
    q = c[0] * divFactor;
#else
    if (   !AnalyticRoots::SafeDivide (p, 0.5 * c[1], c[2], 0.0)
        || !AnalyticRoots::SafeDivide (q,       c[0], c[2], 0.0))
        return AnalyticRoots::SolveLinear (c + 1, s);
#endif

    D = p * p - q;

    if (IsZero(D))
    {
            s[ 0 ] = - p;
        return 1;
    }
    else if (D < 0)
    {
            return 0;
    }
    else if (D > 0)
    {
            double sqrt_D = sqrt(D);

            s[ 0 ] =   sqrt_D - p;
            s[ 1 ] = - sqrt_D - p;
            return 2;
    }

    return 0;
}

static void check (double a, double b){}
// In 1 root case, farSolution is the one root.
// In 3 root case, farSolution is the root farthest from the inflection.
int AnalyticRoots::SolveCubic(double c[4], double s[3])
    {
    double farSolution;
    return SolveCubic (c, s, farSolution);
    }
// In 1 root case, farSolution is the one root.
// In 3 root case, farSolution is the root farthest from the inflection.
int AnalyticRoots::SolveCubic(double c[4], double s[3], double &farSolution)
{
    int     i, num;
    double  sub;
    double  A, B, C;
    double  sq_A, p, q;
    double  cb_p, D;

    /* normal form: x^3 + Ax^2 + Bx + C = 0 */
#ifdef DIVIDE_ONCE
    double coffScale;
    if (!AnalyticRoots::SafeDivide (coffScale, 1.0, c[3], 0.0))
        return AnalyticRoots::SolveQuadric (c, s);
    A = c[2] * coffScale;
    B = c[1] * coffScale;
    C = c[0] * coffScale;
#else
    if (   !AnalyticRoots::SafeDivide (A, c[2], c[3], 0.0)
        || !AnalyticRoots::SafeDivide (B, c[1], c[3], 0.0)
        || !AnalyticRoots::SafeDivide (C, c[0], c[3], 0.0))
        return AnalyticRoots::SolveQuadric (c, s);
#endif

    /*  substitute x = y - A/3 to eliminate quadric term:
        f = y^3 +3py + 2q = 0
        f' = 3y^2 + p
            local min/max at Y = +-sqrt (-p)
            f(+Y) = -p sqrt(-p) + 3p sqrt (-p) + 2q = 2 p sqrt (-p) + 2q

    */

    sq_A = A * A;
    double p1 = 1.0/3 * (- 1.0/3 * sq_A + B);
    p = (3.0 * B - sq_A) / 9.0;
    check (p, p1);
    double q1 = 1.0/2 * (2.0/27 * A * sq_A - 1.0/3 * A * B + C);
    q = 0.5 * DoubleOps::PreciseSum (2.0/27 * A * sq_A, - 1.0/3 * A * B, C);
    double q2 = A * (sq_A / 27.0  - B / 6.0) + 0.5 * C;
    check (q, q1);
    check (q1, q2);
    /* use Cardano's formula */

    cb_p = p * p * p;
    D = q * q + cb_p;

    if (D >= 0.0 && IsZero(D))
        {
        if (IsZero(q)) /* one triple solution */
            {
            s[0] = s[1] = s[2] = 0;
            num = 1;
            }
        else /* one single and one double solution */
            {
            double u = cbrt(-q);
            s[0] = 2 * u;
            s[1] = s[2] = - u;
            num = 2;
            }
    }
    else if (D <= 0) /* Casus irreducibilis: three real solutions */
    {
            double phi = 1.0/3 * acos(-q / sqrt(-cb_p));
            double t = 2 * sqrt(-p);

            s[0] =   t * cos(phi);
            s[1] = - t * cos(phi + M_PI / 3);
            s[2] = - t * cos(phi - M_PI / 3);
            num = 3;
    }
    else /* one real solution */
    {
            double sqrt_D = sqrt(D);
            double u = cbrt(sqrt_D - q);
            double v = - cbrt(sqrt_D + q);

            s[0] = s[1] = s[2] = u + v;
            num = 1;
    }

    /* resubstitute */

    sub = 1.0/3 * A;
#ifdef CheckConstants
    double a0 = DoubleOps::PreciseSum (sub * sub * sub, 3.0 * p * sub, 2.0 * q);
    check (a0, c[0]);
#endif
    farSolution = s[0];
    for (i = 0; i < num; i++)
        {
        if (fabs (s[i]) > fabs (farSolution))
            farSolution = s[i];
        }
    for (i = 0; i < 3; ++i)   // all three root spots get values even if fewer roots.
        s[ i ] -= sub;
    farSolution -= sub;
    return num;
}


int AnalyticRoots::SolveQuartic(double c[5], double s[4])
{
    double  coeffs[ 4 ];
    double  z, u, v, sub;
    double  A, B, C, D;
    double  sq_A, p, q, r;
    int     i, num;

    /* normal form: x^4 + Ax^3 + Bx^2 + Cx + D = 0 */

#ifdef DIVIDE_ONCE
    double coffScale;
    if (!AnalyticRoots::SafeDivide (coffScale, 1.0, c[4], 0.0))
        return AnalyticRoots::SolveCubic (c, s);
    A = c[3] * coffScale;
    B = c[2] * coffScale;
    C = c[1] * coffScale;
    D = c[0] * coffScale;
#else
    if (   !AnalyticRoots::SafeDivide (A, c[3], c[4], 0.0)
        || !AnalyticRoots::SafeDivide (B, c[2], c[4], 0.0)
        || !AnalyticRoots::SafeDivide (C, c[1], c[4], 0.0)
        || !AnalyticRoots::SafeDivide (D, c[0], c[4], 0.0))
        return AnalyticRoots::SolveCubic (c, s);
#endif
    /*  substitute x = y - A/4 to eliminate cubic term:
        x^4 + px^2 + qx + r = 0 */

    sq_A = A * A;
    p = - 3.0/8 * sq_A + B;
    q = 1.0/8 * sq_A * A - 1.0/2 * A * B + C;
    r = - 3.0/256*sq_A*sq_A + 1.0/16*sq_A*B - 1.0/4*A*C + D;

    if (IsZero(r))
    {
            /* no absolute term: y(y^3 + py + q) = 0 */

            coeffs[ 0 ] = q;
            coeffs[ 1 ] = p;
            coeffs[ 2 ] = 0;
            coeffs[ 3 ] = 1;

            num = SolveCubic(coeffs, s);

            s[ num++ ] = 0;
    }
    else
    {
            /* solve the resolvent cubic ... */

            coeffs[ 0 ] = 1.0/2 * r * p - 1.0/8 * q * q;
            coeffs[ 1 ] = - r;
            coeffs[ 2 ] = - 1.0/2 * p;
            coeffs[ 3 ] = 1;
            double sMax;
            // choose the "safe" (far from min/max) solution ...
            (void) SolveCubic(coeffs, s, sMax);
            z = sMax;

            /* ... to build two quadric equations */

            u = z * z - r;
            v = 2 * z - p;

            if (IsSmallRatio (u, r))
                u = 0;
            else if (u > 0)
                u = sqrt(u);
            else
                return 0;

            if (IsSmallRatio(v, p))
                v = 0;
            else if (v > 0)
                v = sqrt(v);
            else
                return 0;

            coeffs[ 0 ] = z - u;
            coeffs[ 1 ] = q < 0 ? -v : v;
            coeffs[ 2 ] = 1;

            num = SolveQuadric(coeffs, s);

            coeffs[ 0 ]= z + u;
            coeffs[ 1 ] = q < 0 ? v : -v;
            coeffs[ 2 ] = 1;

            num += SolveQuadric(coeffs, s + num);
    }

    /* resubstitute */

    sub = 1.0/4 * A;

    for (i = 0; i < num; ++i)
        s[ i ] -= sub;

    return num;
}

static double s_quadricRelTol = 1.0e-14;
// Find solutions (u values) of the bezier-form quadratic
// y0 (1-u)^2 + 2 y1 u(1-u) + y2 u^2 = e
// i.e. y0, y1, y2 are coefficients of bezier-basis polynomial, e is y level whose crossings
// are needed.
//
// Remark: This effect of the e term can be obtained by subtracting e from the y values, viz
//      (y0-e) (1-u)^2 + 2 (y1-e) u(1-u) + (y2-e) u^2 = 0
// The separate argument for e saves the carrying out this detail (and there might be
//      numerical benefit from keeping it all internal)
//
int AnalyticRoots::SolveBezierQuadric
        (
        double y0,
        double y1,
        double y2,
        double e,
        double uvals[2],
        bool bRestrictSolutionsTo01
        )
    {
    // These are coefficients of (fluid-mechanics style) quadratic
    // a u^2 + 2 b u + c = 0
    // (Note the linear coefficient stated as 2*b -- this makes the factors of 2 and 4
    //    disappear from the solution formula.)
    double a = y0 - 2.0 * y1 + y2;
    double b = y1 - y0;
    double c = y0 - e;
    // Compute the discriminant from the original coefficients -- it is not affected by bits
    // lost in a,b,c computation:
    double y11 = y1 * y1;
    double y02 = y0 * y2;
    double ea = e * a;
    double discriminant = y11 - y02 + ea;
    double refScale = fabs (y11) + fabs (y02) + fabs (ea);
    double eps = s_quadricRelTol * refScale;
    int numOut;

    if (discriminant < -eps)
        {
        numOut = 0;
        }
    else if (discriminant > eps)
        {
        double q = b > 0
                    ? - b - sqrt (discriminant)
                    : - b + sqrt (discriminant);
        uvals[0] = q / a;
        uvals[1] = c / q;
        if (uvals[0] > uvals[1])
            {
            double temp = uvals[0];
            uvals[0] = uvals[1];
            uvals[1] = temp;
            }
        numOut = 2;
        if (bRestrictSolutionsTo01)
            {
            numOut = 0;
            for (int i = 0; i < 2; i++)
                {
                if (uvals[i] >= 0 && uvals[i] <= 1.0)
                    uvals[numOut++] = uvals[i];
                }
            }
        }
    else
        {
        uvals[0] = -b / a;
        numOut = 1;
        if (bRestrictSolutionsTo01
            && (uvals[0] < 0.0 || uvals[1] > 1.0))
            numOut = 0;
        }
    return numOut;
    }

static int packAndSortRoots
(
double *pRoots,
int     numRoots,
bool    bRestrict
)
    {
    int i, j;
    if (bRestrict)
        {
        int numPacked = 0;
        for (i = 0; i < numRoots; i++)
            if (pRoots[i] >= 0.0 && pRoots[i] <= 1.0)
                pRoots[numPacked++] = pRoots[i];
        numRoots = numPacked;
        }

    for (i = 0; i < numRoots; i++)
        {
        for (j = 1; j < numRoots; j++)
            {
            if (pRoots[j] < pRoots[i])
                {
                double temp = pRoots[i];
                pRoots[i] = pRoots[j];
                pRoots[j] = temp;
                }
            }
        }
    return numRoots;
    }

void checkRootsNearEndpoints (double *pA, int order, double *pRoots, int numRoots)
    {
    static double sTestWindow = 1.0e-6;
    for (int i = 0; i < numRoots; i++)
        {
        double u = pRoots[i];
        if (fabs (u) < sTestWindow || fabs (u-1.0) < sTestWindow)
            {
            double f, df, du;
            bsiBezier_functionAndDerivativeExt (&f, &df, 0, pA, order, 1, u);
            if (DoubleOps::SafeDivide (du, f, df, 0.0))
                {
                if (fabs (du) < sTestWindow)
                    pRoots[i] -= du;
                }
            }
        }
    }
// Find solutions (u values) of the bezier-form cubic
// y0 (1-u)^3 + 3 y1 u(1-u)^2 + 3 y2 u^2 (1-u) + y3 u^3= e
// i.e. y0, y1, y2, y3 are coefficients of bezier-basis polynomial, e is y level whose crossings
// are needed.
//
int AnalyticRoots::SolveBezierCubic
        (
        double y0,
        double y1,
        double y2,
        double y3,
        double e,
        double roots[3],
        bool bRestrictSolutionsTo01
        )
    {
    // Get direct solutions in standard basis.
    double cc[4];

    double yRef = fabs (y0) + fabs (y1) + fabs (y2) + fabs (y3);
    double cScale;
    if (!SafeDivide (cScale, 1.0, yRef, 1.0))
        return 0;

    cc[0] = cScale * (y0 - e);
    cc[1] = 3.0 * cScale * (y1 - y0);
    cc[2] = 3.0 * cScale * (y0 - 2.0 * y1 + y2);
    cc[3] = cScale * (- y0 + 3.0 * y1 - 3.0 * y2 + y3);
    //double roots[3];
    int numRoots = SolveCubic (cc, roots);
    //checkRootsNearEndpoints (bezCoff, 4, roots, numRoots);
    return packAndSortRoots (roots, numRoots, bRestrictSolutionsTo01);
    }

// Find solutions (u values) of the bezier-form quartic
// y0 (1-u)u^4 + etc = e
//
int AnalyticRoots::SolveBezierQuartic
        (
        double y0,
        double y1,
        double y2,
        double y3,
        double y4,
        double e,
        double roots[4],
        bool bRestrictSolutionsTo01
        )
    {
    // Get direct solutions in standard basis.
    double cc[5];
    double yRef = fabs (y0) + fabs (y1) + fabs (y2) + fabs (y3) + fabs (y4);
    double cScale;
    if (!SafeDivide (cScale, 1.0, yRef, 1.0))
        return 0;
    cc[0] = cScale * (y0 - e);
    cc[1] = 4.0 * cScale * (-y0 + y1);
    cc[2] = 6.0 * cScale * (y0 - 2.0 * y1 + y2);
    cc[3] = 4.0 * cScale * (-y0 + 3.0 * y1 - 3.0 * y2 + y3);
    cc[4] = cScale * (y0 - 4.0 * y1 + 6.0 * y2 - 4.0 * y3 + y4);

    int numRoots = SolveQuartic (cc, roots);
    return packAndSortRoots (roots, numRoots, bRestrictSolutionsTo01);
    }


/*-----------------------------------------------------------------*//**
* Solve the simultaneous equations
* <pre>
*               alpha + beta*c + gamma*s = 0
*               c*c + s*s = 1
*
* @param c1P OUT x cosine component of first solution point
* @param s1P OUT y sine component of first solution point
* @param c2P OUT x cosine component of second solution point
* @param s2P OUT y sine component of second solution point
* @param solutionType OUT One of the following values:
<pre>
    -2 -- all coefficients identically 0.   The entire c,s plane -- and therefore
        the entire unit circle -- is a solution.
    -1 -- beta,gamma are zero, alpha is not.   There is no line defined.  There are
        no solutions.
    0 -- the line is well defined, but passes completely outside the unit circle.
            In this case, (c1,s1) is the circle point closest to the line
            and (c2,s2) is the line point closest to the circle.
    1 -- the line is tangent to the unit circle.  As tangency is identified at
             numerical precision, faithful interpretation of the coefficients
             may allow for some distance from line to circle. (c1,s1) is returned
             as the closest circle point, (c2,s2) the line point.  These are
             nominally the same but may differ due to the tolerance
             decision.
    2 -- two simple intersections.
</pre>

* @param alpha => constant coefficient on line
* @param beta => x cosine coefficient on line
* @param gamma => y sine coefficient on line
* @param reltol => relative tolerance for tangencies
* @return the (nonnegative) solution count.

@remarks Here is an example of the tangible meaning of the coefficients and
the cryptic 5-way solution type separation.
Point X on a 3D ellipse at parameter space angle theta is given by
    X = C + U cos(theta) + V sin(theta)
where C,U,V are (respectively) center, 0 degree, and 90 degree vectors.
A plane has normal N and is at distance a from the origin.  X is on the plane if
    X.N = a
i.e.
    C.N + U.N cos(theta) + V.N sin(theta) = a
i.e.
    C.N - a + U.N cos(theta) + V.N sin(theta) = 0
i.e.
    alpha = C.N - a
    beta =  U.N
    gamma = V.N
If the ellipse is parallel to the plane, both beta and gamma are zero.  These are
the two degenerat cases.  If alpha is also zero the entire ellipse is completely
in the plane.   If alpha is nonzero the ellipse is completely out of plane.

If the ellipse plane is NOT parallel, there are zero, one, or two solutions according as
the ellipse is completly on one side, tangent or is properly split by the plane.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
int AnalyticRoots::IntersectImplicitLineUnitCircle
(
double &c1,
double &s1,
double &c2,
double &s2,
int    &solutionType,
double alpha,
double beta,
double gamma,
double reltol
)
    {
    double twoTol;
    double delta2 = beta*beta + gamma*gamma;
    double alpha2 = alpha*alpha;
    int nSolution;
    solutionType = 0;

    if ( reltol < 0.0 )
        {
        twoTol = 0.0;
        }
    else
        {
        twoTol = 2.0*reltol;
        }


    if ( delta2 <= 0.0 )
        {
        nSolution = 0;
        solutionType = alpha == 0.0 ? -2 : -1;
        c1 = s1 = c2 = s2 = 0.0;
        }
    else
        {
        double lambda = - alpha / delta2;
        double a2 = alpha2 / delta2;
        double D2 = 1.0 - a2;
        if ( D2 < -twoTol )
            {
            double delta = sqrt(delta2);
            double iota = (alpha < 0) ? 1.0 / delta : - 1.0 / delta;
            nSolution = 0;
            c1 = lambda*beta;
            s1 = lambda*gamma;
            c2 = beta * iota;
            s2 = gamma * iota;
            solutionType = 0;
            }
        else if ( D2 < twoTol )
            {
            double delta = sqrt(delta2);
            double iota = (alpha < 0) ? 1.0 / delta : - 1.0 / delta;
            nSolution = 1;
            c1 = lambda*beta;
            s1 = lambda*gamma;
            c2 = beta * iota;
            s2 = gamma * iota;
            solutionType = 1;
            }
        else
            {
            double mu = sqrt ( D2 / delta2 );
            /* c0,s0 = closest approach of line to origin */
            double c0 = lambda*beta;
            double s0 = lambda*gamma;
            nSolution = 2;
            c1 = c0 - mu * gamma;
            s1 = s0 + mu * beta;
            c2 = c0 + mu * gamma;
            s2 = s0 - mu * beta;
            solutionType = 2;
            }
        }
    return nSolution;
    }

bool AnalyticRoots::NormalizeCosine
(
double beta,
double gamma,
double &thetaMin,
double &thetaMax,
double &amplitude
)
    {
    if (beta == 0.0 && gamma == 0.0)
        return false;
    //    f  = beta * cos + gamma * sin
    //    f' = - beta * sin + gamma * cos
    //    f' = 0 at sin/cos = gamma/beta
    double thetaBar = atan2 (gamma, beta);
    double h = beta * cos (thetaBar) + gamma * sin (thetaBar);
    if (h > 0)
        {
        thetaMax = thetaBar;
        thetaMin = thetaBar > 0.0 ? (thetaBar - msGeomConst_pi) : (thetaBar + msGeomConst_pi);
        }
    else
        {
        thetaMin = thetaBar;
        thetaMax = thetaBar > 0.0 ? (thetaBar - msGeomConst_pi) : (thetaBar + msGeomConst_pi);
        }
    amplitude = fabs (h);
    return true;
    }

//! Return 0 or 2 angle roots of  trig form {alpha + beta * cos + gamma * sin} for a bounded interval.
//! There is no test for tangency --- a (numerically) exact tangency goes in twice (unnoticed).
//! @param [in] alpha constant coff
//! @param [in] beta  cosine coff
//! @param [in] gamma sine coff
//! @param [in] theta0 start angle of interval
//! @param [in] sweep interval sweep
//! @param [out] angles 
//! @param [out] count angle count.  0 if no amplitude.
//! @param [out] thetaMax angle where maximum value alpha + amplitude occurs.
//! @param [out] thetaMin angle where minimum value alpha - almplitude occurs.
//! @param [out] amplitude (absolute) amplitude of wave.
bool AnalyticRoots::LinearTrigFormRoots (double alpha, double beta, double gamma, double *angles, size_t &count, double &thetaMax, double &thetaMin, double &amplitude)
    {
    count = 0;
    if (!NormalizeCosine (beta, gamma, thetaMin, thetaMax, amplitude))
        return false;
    if (fabs (alpha) <= amplitude)
        {
        double delta = acos (-alpha / amplitude);
        angles[count++] = thetaMax - delta;
        angles[count++] = thetaMax + delta;
        }
    return true;
    }

#ifdef USE_PREHISTORIC_CUBIC_SOLIVER
// REMARKS: EDL 1/19/2011
// This code was found in quicksilver source in mstn\mscore\mathutil.cpp, which is the hg image of mstn\library\soruce\mathutil.c.
// The change logs record the developer as "Prehistoric".
// This is the shortest cubic solver I have seen, with the caveat that it does not have the single root case, so it's essentially wrong.
// RBB says BCC is Brian Brian Clendenin, aka the noodle.
/*----------------------------------------------------------------------+
|									|
| name		mdlMath_findCubicRoots - Finds roots of			|
|		x^3 + ax^2 + bx + c = 0 if all roots are real. From	|
|		numerical recipes in C.					|
|									|
| author	BCC				 	 12/91		|
|									|
+----------------------------------------------------------------------*/
Public int mdlMath_findCubicRoots
(
double		*roots,		/* <= Array of 3 doubles to hold roots. */
double		a,		/* => */
double		b,		/* => */
double		c		/* => */
)	
    {
    double	q, r, magnitude, aOver3, theta, qCubed;

    q = (a*a - 3.0*b) / 9.0;
    r = (2.0*a*a*a - 9.0*a*b + 27.0*c) / 54.0;
    qCubed = q*q*q;

    if (qCubed - r*r < 0.0)
    	return ERROR;		/* Not all roots are real */

    theta = acos ( r / sqrt (qCubed));

    magnitude = -2.0 * sqrt (q);
    aOver3 = a / 3.0;
    roots[0] = magnitude * cos ( theta / 3.0) - aOver3;
    roots[1] = magnitude * cos ( (theta + 2 * fc_pi) / 3.0) - aOver3;
    roots[2] = magnitude * cos ( (theta + 4 * fc_pi) / 3.0) - aOver3;
#if defined (debug)
    {
    int	    i;

    printf ("A: %f, B: %f, C: %f\n", a, b, c);
    for (i=0; i<3; i++)
	printf ("f (%f) = %f\n", roots[i], roots[i] * roots[i] * roots[i] +
					  roots[i] * roots[i] * a +
					  roots[i] * b +
					  c);
    }
#endif

    return SUCCESS;
    }

#endif
END_BENTLEY_GEOMETRY_NAMESPACE
