/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PreciseSum, Test1)
    {
    bvector<double> data;
    size_t nMax = 10;
    for (size_t i = 0; i < nMax; i++)
        {
        data.push_back (1.0 + i);
        }
    for (size_t i = 1; i < nMax; i++)
        {
        double s0 = DoubleOps::Sum (&data[0],(int)i);
        double s1 = DoubleOps::PreciseSum (&data[0], i);
        Check::Near (s0, s1, "partial sum");
        }
    
    }
double MatchRoots (bvector<double> &target, bvector<double> &actual)
    {
    double eMax = DBL_MAX;
    if (Check::Size (target.size (), actual.size (), "root count"))
        {
        std::sort (target.begin (), target.end ());
        std::sort (actual.begin (), actual.end ());
        eMax = 0.;
        for (size_t i = 0; i < target.size (); i++)
            {
            double e= target[i] - actual[i];
            eMax = DoubleOps::MaxAbs (e, eMax);
            }
        }
    return eMax;
    }

// check solutions of (x-a) (x^2 + b^2)
void CheckCubic (double a, double b)
    {
    double coffs [4];
    coffs[3] = 1.0;
    coffs[2] = - a;
    coffs[1] = b * b;
    coffs[0] = - b * b * a;
    double roots[4];
    bvector<double>target, actual;
    target.push_back (a);
    double xSafe;
    int numRoots = AnalyticRoots::SolveCubic (coffs, roots, xSafe);
    if (Check::Int (1, numRoots, "cubic root count"))
        {
        for (int i = 0; i < numRoots; i++)
            actual.push_back (roots[i]);
            double eMax = MatchRoots(target, actual);
        Check::True (eMax < 1.0e-14 * (1.0 + DoubleOps::MaxAbs (target)), "root error");
        printf ("  (target %#.17g) (b %#.17g)\n", a, b);
        printf ("  (actual %#.17g)              (eMax %#.3g)\n", a, eMax);
        }
    }

double ErrorAtClosestRoot (bvector<double> &target, bvector<double> &actual, double targetRoot)
    {
    double e = DBL_MAX;   // error at closest root
    double d = DBL_MAX;   // distance to closest root in "actual"
    if (target.size () != actual.size ())
        return e;
    for (size_t i = 0; i < target.size (); i++)
        {
        double a = fabs (actual[i] - targetRoot);
        if (a < d)
            {
            d = a;
            e = fabs (actual[i] - target[i]);
            }
        }
    return e;
    }
double NewtonStep3 (double coffs[4], double u)
    {
    double f = coffs[0] + u * (coffs[1] + u * (coffs[2] + u * coffs[3]));
    double df = coffs[1] + u * (2.0 * coffs[2] + u * 3.0 * coffs[3]);
    return f / df;
    }

void CheckCubic (double u0, double u1, double u2, bool printAll)
    {
    double coffs [4];
    coffs[3] = 1.0;
    coffs[2] = - DoubleOps::PreciseSum (u0, u1, u2);
    coffs[1] = DoubleOps::PreciseSum (u0 * u1, u1 * u2, u0 * u2);
    coffs[0] = - u0 * u1 * u2;
    double roots[4];
    bvector<double>target, actual;
    target.push_back (u0);
    target.push_back (u1);
    target.push_back (u2);
    double xSafe;
    int numRoots = AnalyticRoots::SolveCubic (coffs, roots, xSafe);
    if (numRoots == 2)
        numRoots = 3;
    for (int i = 0; i < numRoots; i++)
        actual.push_back (roots[i]);

    if (Check::Int (3, numRoots, "cubic root count"))
        {
        double uMax = DoubleOps::MaxAbs (target);
        double eMax = MatchRoots(target, actual) / uMax;
        double eSafe = ErrorAtClosestRoot (target, actual, xSafe) / uMax;
        static double s_printTrigger = 1.0e-12;

        if (printAll || eSafe >= s_printTrigger * uMax)
            {
//        Check::True (eMax < 1.0e-14 * DoubleOps::MaxAbs (target), "root error");
            printf ("Cubic root variances.  These may be expected behavior under extreme origin conditions\n");
            printf ("  (known roots %#.17g %#.17g %#.17g) (eMax %#.3g) (eSafe %#.3g)\n", target[0], target[1], target[2], eMax, eSafe);
            printf ("  (computed roots %#.17g %#.17g %#.17g) \n", actual[0], actual[1], actual[2]);
            printf ("  (Correction by newton from computed root  %#.3g %#.3g %#.3g) \n",
                    NewtonStep3 (coffs, actual[0]),
                    NewtonStep3 (coffs, actual[1]),
                    NewtonStep3 (coffs, actual[2]));
            printf ("  (Correction by newton from known root %#.3g %#.3g %#.3g) \n",
                    NewtonStep3 (coffs, target[0]),
                    NewtonStep3 (coffs, target[1]),
                    NewtonStep3 (coffs, target[2]));
            }
        }
    else if (numRoots == 1)
        {
        printf (" ** SINGLETON *** (u target %#.17g %#.17g %#.17g) (u %#17g) (spread factor %.4g)\n", u0, u1, u2, actual[0], (u1-u0) / u2);
        printf ("       NewtonDX %#.3g\n", NewtonStep3(coffs, actual[0]));
        }
    }


    static bool s_printAll = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots, Cubic3)
    {
    static double s_rangeScale = 1.0e6;
    for (double e = 1.0; e > 1.0e-10; e *= 0.1)
      {
      if (s_printAll)
          printf ("\n\n  e %#.3g\n", e);
      for (double x0 = 0.0; x0 < DoubleOps::Max (s_rangeScale * e, 20.0); x0 = 10.0 * (x0 + 1.0))
        CheckCubic (x0-e, x0, x0 + 1.0, s_printAll);
      }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots, Cubic3X)
    {
    // root e = small positive -- e.g. in range 01 for sure,like the one interesting root for a cubic bezier 
    // x0, x0+1 = two roots "somewhere else" -- not interesting in bezier case.
    for (double e = 1.0; e > 1.0e-10; e *= 0.1)
      for (double x0 = 0.0; x0 < 1100; x0 = 10.0 * (x0 + 1.0))
        CheckCubic (e, x0, x0 + 1.0, s_printAll);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/

TEST(AnalyticRoots, Cubic1)
    {
    double b = 100.0;
    for (double x0 = 0.0; x0 < 1000; x0 = 10.0 * (x0 + 1.0))
      {
      CheckCubic (x0, b);
      }
    }

// newton step to improve a root of a quartic . . .
double NewtonStep4 (double coffs[5], double u)
    {
    double f = coffs[0] + u * (coffs[1] + u * (coffs[2] + u * (coffs[3] + u * coffs[4])));
    double df = coffs[1] + u * (2.0 * coffs[2] + u * (3.0 * coffs[3] + u * 4.0 * coffs[4]));
    return f / df;
    }

void CheckQuartic (double u0, double u1, double u2, double u3, double tolerance)
    {
    double coffs [5];
    coffs[4] = 1.0;
    coffs[3] = - DoubleOps::PreciseSum (u0, u1, u2, u3);
    double xx[10];
    int numxx = 0;
    xx[numxx++] = u0 * u1;
    xx[numxx++] = u0 * u2;
    xx[numxx++] = u0 * u3;
    xx[numxx++] = u1 * u2;
    xx[numxx++] = u1 * u3;
    xx[numxx++] = u2 * u3;
    coffs[2] =  u0 * (u1 + u2 + u3) + u1 * (u2 + u3) + u2 * u3;
    coffs[2] = DoubleOps::PreciseSum (xx, numxx);

    coffs[1] = -( u0 * u1 * u2 + u0 * u1 * u3 + u0 * u2 * u3 + u1 * u2 * u3);
    coffs[1] = -DoubleOps::PreciseSum (u0 * u1 * u2, u0 * u1 * u3, u0 * u2 * u3, u1 * u2 *u3);
    coffs[0] = u0 * u1 * u2 * u3;
    double roots[4];
    bvector<double>target, actual;
    target.push_back (u0);
    target.push_back (u1);
    target.push_back (u2);
    target.push_back (u3);

    int numRoots = AnalyticRoots::SolveQuartic (coffs, roots);
    for (int i = 0; i < numRoots; i++)
        actual.push_back (roots[i]);

    if (Check::Int (4, numRoots, "quartic root count"))
        {
        double uMax = DoubleOps::MaxAbs (target);
        double eMax = MatchRoots(target, actual) / uMax;
        bool ok = Check::True (eMax < tolerance, "Quartic Root tolerance");

//        Check::True (eMax < 1.0e-14 * DoubleOps::MaxAbs (target), "root error");
        bool print = !ok || s_printAll;
        if (print)
            {
            printf ("\n  (target %#.17g %#.17g %#.17g %#.17g) \n", target[0], target[1], target[2], target[3]);
            printf ("  (actual %#.17g %#.17g %#.17g %#.17g) (eMax %#.3g)\n", actual[0], actual[1], actual[2], actual[3], eMax);
            }
        for (int step = 0; step < 10 && eMax > 1.0e-14; step++)
            {
            if (print)
                printf ("  (actualDX   %#.3g %#.3g %#.3g %#.3g) \n",
                    NewtonStep4 (coffs, actual[0]),
                    NewtonStep4 (coffs, actual[1]),
                    NewtonStep4 (coffs, actual[2]),
                    NewtonStep4 (coffs, actual[3])
                    );
            for (int k = 0; k < numRoots; k++)
                actual[k] -= NewtonStep4 (coffs, actual[k]);
            eMax = MatchRoots(target, actual) / uMax;
            if (print)
                printf ("  (actual %#.17g %#.17g %#.17g %#.17g)  (eMax %#.3g)\n", actual[0], actual[1], actual[2], actual[3], eMax);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots, Quartic4)
    {
    static int s_applyFactor1 = false;
    double tightTol = 1.0e-15;
    CheckQuartic (0,1,2,3, tightTol);
    double mediumTol = 1.0e-10;
    for (auto delta: bvector<double>{1, 3,7,10})
        {
        CheckQuartic (-11, -10, 10, 10 + delta, mediumTol);
        }
    // symmetry with varying spread
    for (auto delta: bvector<double>{0.1, 1,5, 10})
        CheckQuartic (-100, -100 + delta, 100 - delta, 100, mediumTol);

    double a = 100.0;
    double b = 1000.0;
    double e = 1.0;
    double looseTol = 1.0e-5;
    
    // We'd prefer the last factor to be 100, but it fails on optimized iOSArm64.
    for (auto factor : bvector<double>{1, 0.1, 3, 6, 10})
        {
        CheckQuartic (a, a + e, b, b + e/factor, looseTol);
        if (s_applyFactor1)
            {
            // this has a bad tolerance problem for factor==3 with factor1 applied .. (cubic disriminant is near root.)
            double factor1 = 1.0 / 64.0;
            CheckQuartic (a * factor1, (a + e) * factor1, b * factor1, (b + e / factor) * factor1, looseTol);
            }
        }
    }

void CompareAnalyticQuarticWithBezier (double c0, double c1, double c2, double c3, double c4)
    {
    double coffs[5];
    coffs[0] = c0;
    coffs[1] = c1;
    coffs[2] = c2;
    coffs[3] = c3;
    coffs[4] = c4;
    double roots1[4];
    int numRoots1 = AnalyticRoots::SolveQuartic (coffs, roots1);
    std::sort (roots1, roots1 + numRoots1);
    double roots2[10];
    int numRoots2;
    bsiBezier_univariateStandardRoots (roots2, &numRoots2, coffs, 4);
    std::sort (roots2, roots2 + numRoots2);
    if (Check::Int (numRoots1, numRoots2, "Quartic root counts"))
        {
        for (int i = 0; i < numRoots1; i++)
            Check::Near (roots1[i], roots2[i], "Compare roots");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots,Quartic)
    {
    CompareAnalyticQuarticWithBezier (3, -11.2, 6.279999999999999, 8.24, -3.32);
    CompareAnalyticQuarticWithBezier (3.2125, -11.87, 7.039999999999999, 8.06, -3.6300000000000003);
    }


void SolveBezierQuadratic(double a0, double a1, double a2, double target)
    {
    double uVals[2];
    Polynomial::Bezier::Order3 bezier (a0, a1, a2);
    for (bool restrict01 : { false, true})
        {
        int n = AnalyticRoots::SolveBezierQuadric (a0, a1, a2, target, uVals, restrict01);
        for (int i = 0; i < n; i++)
            {
            double u = uVals[i];
            double f = bezier.Evaluate (u);
            Check::Near (target, f);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots,BezierQuadric)
    {
    for (double target : {0.0, -1.0, 1.0})
        {
        SolveBezierQuadratic (3, -11.2, 6.279999999999999, target);
        SolveBezierQuadratic (3.2125, -11.87, 7.039999999999999, target );
        }
    }

void SolveBezierCubic (double a0, double a1, double a2, double a3, double target)
    {
    double uVals[5];
    Polynomial::Bezier::Order4 bezier (a0, a1, a2, a3);
    for (bool restrict01 : { false, true})
        {
        int n = AnalyticRoots::SolveBezierCubic (a0, a1, a2, a3, target, uVals, restrict01);
        for (int i = 0; i < n; i++)
            {
            double u = uVals[i];
            double f = bezier.Evaluate (u);
            Check::Near (target, f);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots,BezierCubic)
    {
    for (double target : {0.0, -1.0, 1.0})
        {
        SolveBezierCubic (-3, 1, -1, 3, target);
        SolveBezierCubic (3.2125, -11.87, 7.039999999999999, 20.0, target );
        }
    }


void SolveBezierQuartic (double a0, double a1, double a2, double a3, double a4, double target)
    {
    double uVals[5];
    Polynomial::Bezier::Order5 bezier (a0, a1, a2, a3, a4);
    for (bool restrict01 : { false, true})
        {
        int n = AnalyticRoots::SolveBezierQuartic (a0, a1, a2, a3, a4, target, uVals, restrict01);
        for (int i = 0; i < n; i++)
            {
            double u = uVals[i];
            double f = bezier.Evaluate (u);
            Check::Near (target, f);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AnalyticRoots,BezierQuartic)
    {
    for (double target : {0.0, -1.0, 1.0})
        {
        SolveBezierQuartic (-3, 1, -1, 3, 4, target);
        SolveBezierQuartic (3.2125, -11.87, 7.039999999999999, 20.0, 25.0, target );
        }
    }