/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifndef Public
#define Public
#endif
#ifndef Private
#define Private
#endif
#ifndef nativeCode
#define nativeCode
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
#include <Geom/quadeqn.fdf>
#include <Geom/bezroot.fdf>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*------------------------------------------------------+
|TITLE   Numerically Stable Solution of Quadratic Equations
| These functions solve quadratic equations
| in a form that occurs frequently in graphics.
|                                                       |
| The quadratic is required to be formulated as the     |
| intersection of a straight line with a unit           |
| circle.  This usual interpretation of this is         |
| that the "coordinates" of the solution point are      |
| the cosine and sine of an angle in a parameterization |
| of an ellipse.                                        |
| Specifically, the functions find the (0, 1, or 2)     |
| pairs of c and s values that simultaneously satisfy   |
|<pre>                                                  |
|               alpha + beta*c + gamma*s = 0            |
|               c*c + s*s = 1                           |
|</pre>                                                 |
| where alpha, beta, and gamma are coefficients of      |
| the straight line.                                    |
|                                                       |
| bsiMath_solveUnitQuadratic makes no attempt to recognize |
| approximate 0 discriminants that occur with a near-   |
| tangency.   However, it returns a numeric indicator   |
| that can be reinterpretted.                           |
|                                                       |
| bsiMath_solveApproximateUnitQuadratic applies a user-    |
| specified relative tolerance to distinguish           |
| tangencies.                                           |
+------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* Solve the simultaneous equations
* <pre>
*               alpha + beta*c + gamma*s = 0
*               c*c + s*s = 1
* </pre>
* i.e. in the (c,s) plane find intersections of the
* line and the unit circle.
* and return nsol=
* <pre>
*       0  proper equation, no solutions
*       2  proper equation, two roots returned
*       1  improper equation (beta=gamma=0)
* </pre>
* The returned value of a2 the squared distance (in
* c,s space) from origin to the closest point on the
* line.
* In exact arithmetic, a2 distinguishes the root
* cases:
*       a2 > 1          no solutions
*       a2 = 1          tangency. The two c,s values
*                       are identical
* 0 < = a2 < 1           two solutions.
* Nearzero values  both positive and negative
* may be interpreted as near tangencies.
* When a is near 1, say a = 1 + e (and e is small),
*       a^2     = 1  2e + e^2
* For small e this is approximately 1+2e
* The value of a^2 is computed using only addition of
* positive quantities, so should be as accurate as
* the inputs.
* Therefore ...
* testing for small absolute value of 1a^2 is a good
* way to detect near tangencies.
* If you choose to treat a near tangency as a tangency,
* it is equivalent to moving the line slightly in the
* normal direction.
*
* @param c1P <= x cosine component of first solution point
* @param s1P <= y sine component of first solution point
* @param c2P <= x cosine component of second solution point
* @param s2P <= y sine component of second solution point
* @param a2P <= squared distance to closest approach of line
* @param alpha => constant coefficient on line
* @param beta => x cosine coefficient on line
* @param gamma => y sine coefficient on line
* @return solution counter
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_solveUnitQuadratic

(
double *c1P,
double *s1P,
double *c2P,
double *s2P,
double *a2P,
double alpha,
double beta,
double gamma
)
    {
    double delta2 = beta*beta + gamma*gamma;
    double alpha2 = alpha*alpha;
    int nSolution;

    if ( delta2 <= 0.0 )
        {
        nSolution = 0;
        }
    else
        {
        double lambda = - alpha / delta2;
        double D2;
        *a2P = alpha2 / delta2;
        D2 = 1.0 - *a2P;
        if ( D2 < 0.0 )
            {
            nSolution = 0;
            }
        else
            {
            double mu = sqrt ( D2 / delta2 );
            /* c0,s0 = closest approach of line to origin */
            double c0  =  lambda*beta;
            double s0 =  lambda*gamma;
            nSolution = 2;
            *c1P = c0   - mu * gamma;
            *s1P = s0  + mu * beta;
*c2P = c0   + mu * gamma;
            *s2P = s0  - mu * beta;
            }
        }
    return nSolution;
    }

/* MAP jmdlMath_computeQuadsol=Geom.solveApproximateUnitQuadratic ENDMAP */

/*-----------------------------------------------------------------*//**
* Solve the simultaneous equations
* <pre>
*               alpha + beta*c + gamma*s = 0
*               c*c + s*s = 1
* </pre>
* subject to a relative tolerance
* The return value indicates the following cases:
* <pre>
*       -2 > degenerate (beta=gamma=alpha=0) and the
*               entire unit circle solves the equation
*       -1 > degenerate (beta=gamma=0) and no
*               solutions.
*        0 > no solutions. In this case (c1,s1) is
*               the closest point on the line, (c2,s2)
*               is the closest point on the circle
*        1 > near tangency.  Again (c1,s1) and (c2,s2)
*               are closest approach points on the
*               line and circle, respectively.
*               Application conditions may prefer
*               to use one or the other as 'the'
*               solution.
*        2 > two distinct solutions.
*</pre>
*
* @param c1P <= x cosine component of first solution point
* @param s1P <= y sine component of first solution point
* @param c2P <= x cosine component of second solution point
* @param s2P <= y sine component of second solution point
* @param alpha => constant coefficient on line
* @param beta => x cosine coefficient on line
* @param gamma => y sine coefficient on line
* @param reltol => relative tolerance for tangencies
* @return solution counter
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_solveApproximateUnitQuadratic

(
double *c1P,
double *s1P,
double *c2P,
double *s2P,
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
        if ( alpha == 0.0 )
            {
            nSolution = -1;
            }
        else
            {
            nSolution = -2;
            }
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
            *c1P = lambda*beta;
            *s1P = lambda*gamma;
            *c2P = beta * iota;
            *s2P = gamma * iota;
            }
        else if ( D2 < twoTol )
            {
            double delta = sqrt(delta2);
            double iota = (alpha < 0) ? 1.0 / delta : - 1.0 / delta;
            nSolution = 1;
            *c1P = lambda*beta;
            *s1P = lambda*gamma;
            *c2P = beta * iota;
            *s2P = gamma * iota;
            }
        else
            {
            double mu = sqrt ( D2 / delta2 );
            /* c0,s0 = closest approach of line to origin */
            double c0 = lambda*beta;
            double s0 = lambda*gamma;
            nSolution = 2;
            *c1P = c0 - mu * gamma;
            *s1P = s0 + mu * beta;
            *c2P = c0 + mu * gamma;
            *s2P = s0 - mu * beta;
            }
        }
    return nSolution;
    }

/*------------------------------------------------------------------+
|                                                                   |
| name      jmdlMath_computeQuadsol
|                                                                   |
| author    EarlinLutz                                  10/96       |
|                                                                   |
| Carry out the numerically sensitive steps in solving a quadratic. |
| Called internally by quadratic solvers after discriminant is      |
| determined to be positive.                                        |
|                                                                   |
| This step is separated out so that it can be invoked from both    |
| regular quadratic solver (for ax^2 + bx + c=0) and symmetric      |
| solver a00*x0^2 + a01*x0*x1 + a11*x1^2=0.                         |
|                                                                   |
| (The latter case DEPENDS ON the fact that pXplus corresponds to   |
| the ADDED discriminant, pXminus subtracted, even if the are       |
| computed in the opposite order and with another formula.          |
+------------------------------------------------------------------*/
static void jmdlMath_computeQuadsol

(
double *pXplus,     /* <= root (-b + D) / 2a */
double *pXminus,    /* <= root (-b - D) / 2a */
double aa,          /* => quadratic coefficient. ASSUMED NONZERO        */
double bb,          /* => linear coefficient            */
double cc,          /* => constant coefficient          */
double D            /* => precomputed and square-rooted discriminant. ASSUMED POSITIVE  */
)

    {
    /* Calculate b+-D only for the one that does not cause cancelation.
       Use c/q for the other one. */
    double qq;
    if (bb > 0.0)
        {
        qq = 0.5 * (-bb - D);
        *pXminus = qq / aa;
        *pXplus  = cc / qq;
        }
    else
        {
        qq = 0.5 * (-bb + D);
        *pXplus  = qq / aa;
        *pXminus = cc / qq;
        }
    }

/* MAP isCloseToZero=Geom.solveQuadratic ENDMAP */

/*-----------------------------------------------------------------*//**
* Find the 0, 1, or 2 roots of the quadratic aa*x*x + bb * x + c
*
* @param pX <= array of 0, 1, or 2 roots
* @param aa => quadratic coefficient
* @param bb => linear coefficient
* @param cc => constant coefficient
* @return number of roots
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_solveQuadratic

(
double *pX,
double aa,
double bb,
double cc
)
    {
    static double relTol = 1.0e-14;
    double epsilon = relTol * (fabs (aa) + fabs(bb) + fabs(cc));
    int numRoot = 0;

    if (fabs(aa) > epsilon)
        {
        double b2 = bb * bb;
        double disc = b2 - 4.0 * aa * cc;
        double discTol = fabs (relTol * b2);

        if (disc < 0.0 && fabs (bb) < epsilon && fabs (cc) < epsilon)
            {
            /* Degenerate case aa * x^2 = 0, based on coefficient magnitudes.
                Call it a double root at 0.  For similarly small positive discriminant,
                allow other cases to possibly assign two very near roots.
            */
            numRoot = 1;
            pX[0] = 0.0;
            }
        else if (disc < - discTol)
            {
            /* discriminant is clearly negative */
            numRoot = 0;
            }
        else if (disc > discTol)
            {
            /* discriminant is clearly positive */
            double delta = sqrt (disc);
            jmdlMath_computeQuadsol (&pX[0], &pX[1], aa, bb, cc, delta);
            numRoot = 2;
            }
        else
            {
            numRoot = 1;
            pX[0] = -0.5 * bb / aa;
            }
        }
    else if (fabs (bb) > epsilon)
        {
        numRoot = 1;
        pX[0] = - cc / bb;
        }
    return numRoot;
    }



/*-----------------------------------------------------------------*//**
* Find the 0, 1, or 2 root pairs of the convex quadratic
* solver a00*x0^2 + a01*x0*x1 + a11*x1^2=0.
* for x0+x1 = 1.
*
* @param pX0 <= array of 0, 1, or 2 roots
* @param pX1 <= array of 0, 1, or 2 roots
* @param a00 => coefficient of x0^2
* @param a01 => coefficient of x0*x1
* @param a11 => coefficient of x1^2
* @return number of roots
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_solveConvexQuadratic

(
double *pX0,
double *pX1,
double a00,
double a01,
double a11
)
    {
    static double relTol = 1.0e-10;
    double epsilon = relTol * (fabs (a00) + fabs(a01) + fabs(a11));
    double A = a00 - a01 + a11;
    double B;
    int numRoot = 0;

    if (fabs(A) > epsilon)
        {
        double b2 = a01 * a01;
        double disc = b2 - 4.0 * a00 * a11;
        double discTol = fabs (relTol * b2);
        double B0 = a01 - 2.0 * a11;
        double B1 = a01 - 2.0 * a00;

        if (disc < - discTol)
            {
            /* discriminant is clearly negative */
            numRoot = 0;
            }
        else if (disc > discTol)
            {
            /* discriminant is clearly positive */
            double delta = sqrt (disc);
            double C0 = a11;
            double C1 = a00;
            jmdlMath_computeQuadsol (&pX0[0], &pX0[1], A, B0, C0, delta);
            jmdlMath_computeQuadsol (&pX1[1], &pX1[0], A, B1, C1, delta);
            numRoot = 2;
            }
        else
            {
            numRoot = 1;
            pX0[0] = -0.5 * B0 / A;
            pX1[0] = -0.5 * B1 / A;
            }
        }
    else if (fabs (B = (a00 - a11)) > epsilon)
        {
        numRoot = 1;
        pX0[0] = - a11 / B;
        pX1[0] =   a00 / B;
        }
    return numRoot;
    }


#ifdef TESTEXACT
void main()
    {
    double alpha, beta,gamma;
    double D2;
    int n;
    double c1,s1,c2,s2;
    while ( 3 == scanf( "%lf %lf %lf",&alpha,&beta,&gamma) )
        {
        n = bsiMath_solveUnitQuadratic( &c1,&s1, &c2,&s2, &D2,
                                alpha,beta,gamma );
        printf("alpha = %lf; beta = %lf; gamma = %lf;\n",alpha, beta,gamma);
        if( n > 0 )
            {
            printf(" 2 solutions, D2=%82.le\n",D2);
            printf(" solution 1 = (%lf,%lf) ==> (%lf,%lf)\n",
                                c1,s1, alpha + c1*beta + s1*gamma,
                                c1*c1 + s1*s1);
            printf(" solution 2 = (%lf,%lf) ==> (%lf,%lf)\n",
                                c2,s2, alpha + c2*beta + s2*gamma,
                                c2*c2 + s2*s2);
            }
        else if ( n == 0 )
            {
            printf(" no solutions, D2=%8.2le\n",D2);
            }
        else
            {
            printf(" degenerate\n");
            }


        }

    }

#elif defined (TESTPROGRAM)
/* Test the toleranced solver */
void main()
    {
    double reltol = 1.0e-12;
    double alpha, beta,gamma;
    double D2;
    int n;
    double c1,s1,c2,s2;
    if ( ! scanf ( "%lf",&reltol ) )
        return;
    while ( 3 == scanf( "%lf %lf %lf",&alpha,&beta,&gamma) )
        {
        n = bsiMath_solveApproximateUnitQuadratic( &c1,&s1, &c2,&s2,
                                alpha,beta,gamma, reltol );
        printf("alpha = %lf; beta = %lf; gamma = %lf;\n",alpha, beta,gamma);
        if( n >= 0 )
            {
            double dc,ds;
            printf(" %d solutions\n",n);
            printf(" solution 1 = (%lf,%lf) ==> (%lf,%lf)\n",
                                c1,s1, alpha + c1*beta + s1*gamma,
                                c1*c1 + s1*s1);
            printf(" solution 2 = (%lf,%lf) ==> (%lf,%lf)\n",
                                c2,s2, alpha + c2*beta + s2*gamma,
                                c2*c2 + s2*s2);
            dc = c2-c1;
            ds = s2-s1;
            printf(" separation = %8.2le\n", sqrt( dc*dc + ds*ds ));
            }
        else
            {
            printf(" degenerate\n");
            }


        }

    }
#endif



#ifdef Compile_bsiSolve_specialCaseRealPolynomial
/*-----------------------------------------------------------------*//**
* Check for easy special cases of polynomial solutions.
* @param pRoots[1...numRoots] <= array of (note the one-based
*               indexing for compatibility with the full root finder)
* @param pNumRoots <= number of roots
* @param pCoff[0..degree] => coefficients
* @param degree => degree of polynomial
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiSolve_specialCaseRealPolynomial

(
DPoint2dP pRoots,
int         *pNumRoots,
DPoint2dP pCoff,
int         degree
)
    {
    int i;
    double sum = fabs (pCoff[0].x);
    int numRoots = 0;
    static double s_smallCoff = 1.0e-10;
    for (i = 1; i <= degree; i++)
        {
        sum += fabs (pCoff[i].x);
        }

    if (degree == 4)
        {
        double smallCoff = s_smallCoff * sum;
        if (   fabs (pCoff[1].x) < smallCoff
            && fabs (pCoff[3].x) < smallCoff)
            {
            /* The odd powers coffs are essentially zero.  Solve the equation
                    a0 + a2 XX + a4 XX^2
                    for XX, and expand each XX solution as two roots x= +-sqrt (XX)
            */
            double xx[2];
            double x;
            int numQuadSol = bsiMath_solveQuadratic (xx, pCoff[4].x, pCoff[2].x, pCoff[0].x);
            numRoots = 0;
            for (i = 0; i < numQuadSol; i++)
                {
                if (xx[i] >= 0)
                    {
                    x = sqrt (xx[i]);
                    numRoots++;
                    pRoots [numRoots].x = x;
                    pRoots [numRoots].y = 0.0;
                    numRoots++;
                    pRoots [numRoots].x = -x;
                    pRoots [numRoots].y = 0.0;
                    }
                }
            *pNumRoots = numRoots;
            return SUCCESS;
            }
        }

    /* Fall-through to full iterative solution */
    if (SUCCESS == bsiSolve_polynomialEquation (pRoots, pCoff, degree, true))
        {
        *pNumRoots = degree;
        return SUCCESS;
        }

    *pNumRoots = 0;
    return ERROR;
    }
#endif

/*-----------------------------------------------------------------*//**
* This routine finds the 0, 1, or 2 solutions of a pair of bilinear
* equations
*      axy * x * y + ax * x + ay * y + a = 0
*      bxy * x * y + bx * x + by * y + b = 0
* with the particular elimination order:
*
*      Isolate both x term....
*         x * (axy * y + ax) = -(ay * y + a)
*        x * (bxy * y + bx) = -(by * y + b)
*      Take ratio of left, right sides.  x and negatives cancel, leaving quadratic in y.  Cross multiply:
*          (axy * y + ax)(by * y + b)  - (ay * y + a)(bxy * y + bx) = 0
*         (axy * by - ay * bxy) * y^2 + (axy * b + ax * by - ay * bx - a * bx) * y + ax * b - a * bx = 0
*
*     Solve this for y.  Then substitute in the "isolated x" equations to get
*               x = -(ay * y + a) / (axy * y + ax)
*   -- or --    x = -(by * y + b) / (bxy * y + bx)
*      Within these two, choose the one with larger denominator.
*       (Surprise surprise -- when axy=ax=0, the first denominator really does vanish!!)
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void bsiMath_solveBilinear_inOrder

(
double *pXArray,
double *pYArray,
int *pNumSolution,
double axy,
double ax,
double ay,
double a,
double bxy,
double bx,
double by,
double b
)
    {
    double cyy, cy, c;
    double x, y;
    double quadroot[2];
    int numQuadroot;
    int i;
    double d0, d1;
    cyy = axy * by - ay * bxy;
    cy  = axy * b + ax * by - ay * bx - a * bxy;
    c   = ax * b - a * bx;
    numQuadroot = bsiMath_solveQuadratic (quadroot, cyy, cy, c);
    *pNumSolution = 0;
    for (i = 0; i < numQuadroot; i++)
        {
        y = quadroot [i];
        d0 = axy * y + ax;
        d1 = bxy * y + bx;
        if (fabs (d0) >= fabs (d1))
            {
            if (DoubleOps::SafeDivide (x, -(ay * y + a), d0, 0.0))
                {
                pXArray[*pNumSolution] = x;
                pYArray[*pNumSolution] = y;
                *pNumSolution += 1;
                }
            }
        else
            {
            if (DoubleOps::SafeDivide (x, -(by * y + b), d1, 0.0))
                {
                pXArray[*pNumSolution] = x;
                pYArray[*pNumSolution] = y;
                *pNumSolution += 1;
                }
            }
        }
    }
/*-----------------------------------------------------------------*//**
* This routine finds the 0, 1, or 2 solutions of a pair of bilinear
* equations
*      axy * x * y + ax * x + ay * y + a = 0
*      bxy * x * y + bx * x + by * y + b = 0
*
* @param pXArray <= array of 0, 1, or 2 solutions.  Caller must allocate.
* @param pYArray <= array of 0, 1, or 2 solutions.  Caller must allocate.
* @param
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiMath_solveBilinear

(
double *pXArray,
double *pYArray,
int *pNumSolution,
double axy,
double ax,
double ay,
double a,
double bxy,
double bx,
double by,
double b
)
    {
    /* Someday figure out which elimination order is better? */
    bsiMath_solveBilinear_inOrder (pXArray, pYArray, pNumSolution, axy, ax, ay, a, bxy, bx, by, b);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
