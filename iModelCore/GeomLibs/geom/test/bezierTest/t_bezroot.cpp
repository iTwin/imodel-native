/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

void TestOrder3RootsIn01 (double a0, double a1, double a2, int numStep)
    {
    double rootA[4];
    double rootB[4];
    double bezCoffs[3];
    Polynomial::Bezier::Order3 bez(a0, a1, a2);
    DRange1d rangeA = DRange1d::From (a0, a1, a2);
    for (int i = 0; i <= numStep; i++)
        {
        double u = (double)i / (double)numStep;
        double target;
        if (rangeA.FractionToDouble (u, target, a0))
            {
            int numA;
            bezCoffs[0] = a0 - target;
            bezCoffs[1] = a1 - target;
            bezCoffs[2] = a2 - target;
            bsiBezier_univariateRoots (rootA, &numA, bezCoffs, 3);
            int numB = bez.Roots (target, rootB, true);
            char message[1024];
            sprintf (message, "Bezier (%g,%g,%g)(u)=%g (numA=%d) (numB %d)\n",
                        a0, a1, a2, target, numA, numB);
            if (Check::Int (numA, numB, "Order3 roots"))
                {
                for (int i = 0; i < numA; i++)
                    {
                    Check::Near (rootA[i], rootB[i], "root");
                    Check::Near (target, bez.Evaluate (rootB[i]), "root value");
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierOrder3,RootsIn01)
    {
    static int numStep = 7;
    TestOrder3RootsIn01 (0,1,4, numStep);
    TestOrder3RootsIn01 (0,1,2, numStep);
    TestOrder3RootsIn01 (0,2,1, numStep);

    TestOrder3RootsIn01 (1,0,-3, numStep);
    TestOrder3RootsIn01 (3,2,1, numStep);
    TestOrder3RootsIn01 (2,1,4, numStep);
    TestOrder3RootsIn01 (4,1,2, numStep);

    }

#ifdef GAUSSKRONRAD_WASMOVED
// f(x,i) = x^i
class IntegratedPowersIntegrand : public BSIVectorIntegrand
{
public:
int m_maxDegree;
IntegratedPowersIntegrand (int maxDegree) : m_maxDegree (maxDegree)
    {
    }

virtual void EvaluateVectorIntegrand (double t, double *pF) override
    {
    double f = t;
    for (int i = 0; i <= m_maxDegree; i++)
        {
        pF[i] = f / (double)(i+1);
        f *= t;
        }
    }
virtual int  GetVectorIntegrandCount ()
    {
    return m_maxDegree + 1;
    }
};


// f(x,i) = x^i
class PolynomialPowersIntegrand : public BSIVectorIntegrand
{
public:
int m_maxDegree;
PolynomialPowersIntegrand (int maxDegree) : m_maxDegree (maxDegree)
    {
    }

virtual void EvaluateVectorIntegrand (double t, double *pF) override
    {
    double f = 1.0;
    for (int i = 0; i <= m_maxDegree; i++)
        {
        pF[i] = f;
        f *= t;
        }
    }
virtual int  GetVectorIntegrandCount ()
    {
    return m_maxDegree + 1;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GaussKronrod,SimplePowers)
    {
    BSIQuadraturePoints gauss3, kronrod, gauss5, gauss7, gaussToUse;
    kronrod.InitGaussKronrod (7, gauss3);
    gauss7.InitGauss (7);
    gauss5.InitGauss (5);
    gaussToUse = gauss3;
    int maxPower = 20;
    PolynomialPowersIntegrand functions(maxPower);
    double gaussSums[200], kronrodSums[200], gauss2Sums[200], gauss4Sums[200];
    double kronrod2Sums[200];
    double gaussPower = gaussToUse.GetConvergencePower ();
    double extrapolationFactor = 1.0 / (pow (2.0, gaussPower) - 1.0);
    for (double a0 = 0.0; a0 < 4.5; a0 += 1.0)
        {
        double a1 = a0 + 1.0;
        for (int i = 0; i <= maxPower; i++)
            {
            gaussSums[i] = 0.0;
            kronrodSums[i] = 0.0;
            gauss2Sums[i] = 0.0;
            gauss4Sums[i] = 0.0;
            kronrod2Sums[i] = 0.0;
            }
        gaussToUse.AccumulateWeightedSums (functions, a0, a1, gaussSums,  1);
        gaussToUse.AccumulateWeightedSums (functions, a0, a1, gauss2Sums, 2);
        gaussToUse.AccumulateWeightedSums (functions, a0, a1, gauss4Sums, 4);
        kronrod.AccumulateWeightedSums (functions, a0, a1, kronrodSums, 1);
        kronrod.AccumulateWeightedSums (functions, a0, a1, kronrod2Sums, 2);

        printf (" Integration of simple powers on [%g,%g]\n", a0, a1);
        printf ("  i    I               eGauss1   eGauss2    eGaussX   eKronrod\n");
        for (int i = 0; i <= maxPower; i++)
            {
            double trueIntegral = (pow (a1, i+1) - pow (a0, i+1))/ (double)(i + 1);
            double eGauss = trueIntegral - gaussSums[i];
            double eKronrod = trueIntegral - kronrodSums[i];
            double eGauss2 = trueIntegral - gauss2Sums[i];
            double eGauss4 = trueIntegral - gauss4Sums[i];
            double gauss12X = gauss2Sums[i] + extrapolationFactor * (gauss2Sums[i] - gaussSums[i]);
            double gauss24X = gauss4Sums[i] + extrapolationFactor * (gauss4Sums[i] - gauss2Sums[i]);
            double eGauss12X = trueIntegral - gauss12X;
            double eGauss24X = trueIntegral - gauss24X;
            double dGK = gaussSums[i] - kronrodSums[i];
            double eKronrod2 = trueIntegral - kronrod2Sums[i];
            printf ("%3d %20.17g (G1 %7.1e)(G2 %7.1e)(X12 %7.1e )(G4 %7.1e)  (X24 %7.1e)  (K %7.1e)(K2 %7.1e) (dGK %7.1e)\n", i, trueIntegral,
                    eGauss / trueIntegral,
                    eGauss2 / trueIntegral,
                    eGauss12X/ trueIntegral,
                    eGauss4 / trueIntegral,
                    eGauss24X/ trueIntegral,
                    eKronrod / trueIntegral,
                    eKronrod2 / trueIntegral,
                    dGK / trueIntegral
                    );
            }
        }
    }
#endif

double logSeries (double u, int nMax)
    {
    double sum = 0.0;
    for (int i = nMax; i > 1; i--)
        {
        sum = u * (1.0 / (double)i - sum);
        }
    return u * (1.0 - sum);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LogSeries, Test)
    {
    printf ("\nSeries expandions of log (1+u)=u-u^2/2 + u^3/3 ....\n");
    for (double x = 0.5; x > 1.0e-6; x /= 16.0)
        {
        double f = log (1.0 + x);
        printf ("%5.3g %.15g", x, f);
        for (int i = 1; i < 6; i+= 1)
            {
            printf (" %.2le", f - logSeries (x, i));
            }
        printf ("\n");
        }
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bezier,DerivativeBasis)
    {
    double basis0[MAX_BEZIER_ORDER];
    double basis1[MAX_BEZIER_ORDER];
    double coff[MAX_BEZIER_ORDER];
    
    double dtheta = 0.9;
    double theta0 = 0.2;
    double f0, f1;
    double g0, g1;
    double du = 0.219;
    for (int order = 2; order < 25; order++)
        {
        // put poles on a sine wave.
        // shifts by integer multiples of dtheta will not hit a multiple of pi, so there will be no zeros.
        // du is large enough to introduce inflections
        for (int i = 0; i < order; i++)
            coff[i] = sin (theta0 + i * dtheta);
        double maxe0 = 0.0;
        double maxe1 = 0.0;
        double maxf0 = 0.0;
        double maxf1 = 0.0;
        for (double u = 0.0; u <= 1.0; u += du)
            {
            bsiBezier_evaluateBasisFunctions (basis0, order, u);
            bsiBezier_evaluateDerivativeBasisFunctions (basis1, order, u);
            f0 = f1 = 0.0;
            for (int i = 0; i < order; i++)
                {
                f0 += basis0[i] * coff[i];
                f1 += basis1[i] * coff[i];
                }
            bsiBezier_functionAndDerivative (&g0, &g1, coff, order, 1, u);
            maxe0 = DoubleOps::MaxAbs (maxe0, f0-g0);
            maxe1 = DoubleOps::MaxAbs (maxe1, f1-g1);
            maxf0 = DoubleOps::MaxAbs (maxf0, f0);
            maxf1 = DoubleOps::MaxAbs (maxf1, f1);
            Check::Near (f0, g0, "Bezier evaluations");
            Check::Near (f1, g1, "Bezier derivative evaluations");
            }
        static double s_e0Tol = 5.0e-15;
        static double s_e1Tol = 5.0e-15;
        if (!Check::True (maxe0 < s_e0Tol) || !Check::True (maxe1 < s_e1Tol))
            printf (" Bezier Derivative max error (order %d) (e0 %.2le / %.2le) (e1 %.2le  / %.2le)\n",
                    order, maxe0, maxf0, maxe1, maxf1);
        }
    // EDL Dec 29, 2012
// Bezier Derivative max error (order 2) (e0 0.00e+000 / 8.05e-001) (e1 0.00e+000  / 6.93e-001)
// Bezier Derivative max error (order 3) (e0 2.22e-016 / 8.94e-001) (e1 1.39e-016  / 1.39e+000)
// Bezier Derivative max error (order 4) (e0 1.11e-016 / 7.19e-001) (e1 4.44e-016  / 2.08e+000)
// Bezier Derivative max error (order 5) (e0 1.11e-016 / 6.50e-001) (e1 4.44e-016  / 3.05e+000)
// Bezier Derivative max error (order 6) (e0 5.55e-017 / 7.00e-001) (e1 4.44e-016  / 3.46e+000)
// Bezier Derivative max error (order 7) (e0 0.00e+000 / 7.43e-001) (e1 4.44e-016  / 4.16e+000)
// Bezier Derivative max error (order 8) (e0 0.00e+000 / 6.14e-001) (e1 8.88e-016  / 4.85e+000)
// Bezier Derivative max error (order 9) (e0 0.00e+000 / 5.71e-001) (e1 4.44e-016  / 5.54e+000)
// Bezier Derivative max error (order 10) (e0 0.00e+000 / 6.07e-001) (e1 8.88e-016  / 6.23e+000)
// Bezier Derivative max error (order 11) (e0 0.00e+000 / 6.16e-001) (e1 8.88e-016  / 6.93e+000)
// Bezier Derivative max error (order 12) (e0 0.00e+000 / 3.68e-001) (e1 8.88e-016  / 7.62e+000)
// Bezier Derivative max error (order 13) (e0 0.00e+000 / 2.94e-001) (e1 8.88e-016  / 8.31e+000)
// Bezier Derivative max error (order 14) (e0 0.00e+000 / 5.24e-001) (e1 1.78e-015  / 9.00e+000)
// Bezier Derivative max error (order 15) (e0 0.00e+000 / 5.09e-001) (e1 1.78e-015  / 9.70e+000)
// Bezier Derivative max error (order 16) (e0 0.00e+000 / 2.00e-001) (e1 1.78e-015  / 1.04e+001)
// Bezier Derivative max error (order 17) (e0 0.00e+000 / 2.00e-001) (e1 1.78e-015  / 1.11e+001)
// Bezier Derivative max error (order 18) (e0 0.00e+000 / 4.51e-001) (e1 1.78e-015  / 1.18e+001)
// Bezier Derivative max error (order 19) (e0 0.00e+000 / 4.19e-001) (e1 1.78e-015  / 1.25e+001)
// Bezier Derivative max error (order 20) (e0 0.00e+000 / 1.99e-001) (e1 2.22e-015  / 1.32e+001)
// Bezier Derivative max error (order 21) (e0 0.00e+000 / 1.99e-001) (e1 8.88e-016  / 1.39e+001)
// Bezier Derivative max error (order 22) (e0 0.00e+000 / 3.87e-001) (e1 1.11e-015  / 1.45e+001)
// Bezier Derivative max error (order 23) (e0 0.00e+000 / 3.44e-001) (e1 4.44e-016  / 1.52e+001)
// Bezier Derivative max error (order 24) (e0 0.00e+000 / 1.99e-001) (e1 1.78e-015  / 1.59e+001)
 
 // No significant error growth with order.   Largest errors have proportionally large function values.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TensorProduct, FastEvaluatorDPoint3d)
    {
    bvector<DPoint3d> controlPoints;
    // x == u
    // y == v^2
    // z == ??
    controlPoints.push_back (DPoint3d::From (0,0,5));
    controlPoints.push_back (DPoint3d::From (1,0,2));
    
    controlPoints.push_back (DPoint3d::From (0,0,3));
    controlPoints.push_back (DPoint3d::From (1,0,5));
    
    controlPoints.push_back (DPoint3d::From (0,1,7));
    controlPoints.push_back (DPoint3d::From (1,1,13));

    size_t uOrder = 2;
    size_t vOrder = 3;
    size_t numUOut = 5; // power of 2 steps
    size_t numVOut = 8; // nonterminating fraction in step size    bvector<double>uGrid, uBasis;
    bvector<double>vGrid, vBasis;
    bvector<double>uGrid, uBasis;
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (uGrid, uBasis, uOrder, 0.0, 1.0, numUOut);
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (vGrid, vBasis, vOrder, 0.0, 1.0, numVOut);    
    
    bvector<DPoint3d> sumDirect;
    bvector<DPoint3d> sumFast;
    Check::True (TensorProducts::EvaluateGrid2D_Direct (sumDirect, uBasis, vBasis, controlPoints, uOrder, vOrder));
    Check::True (TensorProducts::EvaluateGrid2D   (sumFast, uBasis, vBasis, controlPoints, uOrder, vOrder));
    if (Check::Size (sumFast.size (), sumDirect.size (), "Evaluation counts")
        && Check::Size (sumFast.size (), sumDirect.size (), "Template count")
        )
        {
        for (size_t i = 0; i < sumFast.size (); i++)
            {
            if (!Check::Near (sumDirect[i], sumFast[i], "Evaluation match"))
                break;
            }
        }
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TensorProduct, FastEvaluatorDouble)
    {
    size_t uOrder = 5;
    size_t vOrder = 4;
    //Polynomial::Bezier::Order5 uBezier (5,4,3,2,1);
    //Polynomial::Bezier::Order4 vBezier (1,2,3,4);

    Polynomial::Bezier::Order5 uBezier (5,4,6,2,1);
    Polynomial::Bezier::Order4 vBezier (1,2,5,4);


    bvector<double> controlPoints;
    // Form the simplistic tensor product uBezier*vBezier ...
    for (size_t j = 0; j < vOrder; j++)
        {
        for (size_t i = 0; i < uOrder; i++)
            controlPoints.push_back (uBezier.coffs[i] * vBezier.coffs[j]);
        }
    bvector<double>uGrid, uBasis;
    bvector<double>vGrid, vBasis;
    size_t numUOut = 5;
    size_t numVOut = 8;
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (uGrid, uBasis, uOrder, 0.1, 0.9, numUOut);
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (vGrid, vBasis, vOrder, 0.0, 0.95, numVOut);
    bvector<double>sumFast;
    Check::True
        (
        TensorProducts::EvaluateGrid2D   (sumFast, uBasis, vBasis, controlPoints, uOrder, vOrder),
        "Evaluate factorable 2D grid"
        );
        
    size_t numExpected = uGrid.size () * vGrid.size ();
    if (   Check::Size (numExpected, sumFast.size (), "Evaluation counts"))
        {
        for (size_t j = 0; j < vGrid.size (); j++)
            for (size_t i = 0; i < uGrid.size (); i++)
                {
                double u = uGrid[i];
                double v = vGrid[j];
                size_t k = j * uGrid.size () + i;
                double fu = uBezier.Evaluate (u);
                double fv = vBezier.Evaluate (v);
                double p = fu * fv;
                if (!Check::Near (p, sumFast[k], "fast == simpleProduct"))
                    break;
                }
        }
    }

double EvaluateBilinear4(double a[], double s, double t)
    {
    return s * t * a[0] + s * a[1] + t * a[2] + a[3];
    }
double EvaluateBilinear3(double a[], double s, double t)
    {
    return s * t * a[0] + s * a[1] + t * a[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bilinear,Hello)
    {
    double a[4] = {0.3, 1.1, 0.2, 0.0};
    double b[4] = {0.1, -0.1, 2.0, 0.0};
    for (double s = 0.0; s < 1.0; s += 0.25)
        {
        for (double t = -1.0; t < 2.0; t += 0.5)
            {
            // Create a bilinear with at least one solution ...
            a[3] = -EvaluateBilinear3 (a, s, t);
            b[3] = -EvaluateBilinear3 (b, s, t);
            double ss[2], tt[2];
            int numRoot;
            bsiMath_solveBilinear (
                            ss, tt, &numRoot,
                            a[0], a[1], a[2], a[3],
                            b[0], b[1], b[2], b[3]
                            );
            if (Check::True (numRoot >= 1, "Rigged bilinear has at least one root"))
                {
                double dMin = DPoint2d::From (s, t).Distance (DPoint2d::From (ss[0], tt[0]));
                if (numRoot == 2)
                    dMin = DoubleOps::Min (dMin, DPoint2d::From (s, t).Distance (DPoint2d::From (ss[1], tt[1])));
                for (int k = 0; k < numRoot; k++)
                    {
                    double f = EvaluateBilinear4 (a, ss[k], tt[k]);
                    double g = EvaluateBilinear4 (b, ss[k], tt[k]);
                    Check::Near (1.0, 1.0 + f, "bilinear value at root");
                    Check::Near (1.0, 1.0 + g, "bilinear value at root");
                    }
                }
            }
        }
    }

void CheckBezierEvaluationOnCircle (bvector<double> &c, bvector<double> &s, bvector<double> &w)
    {
    bvector<double> testFractions {0.0, 1.0, 0.5, 1.0/3.0, 2.0/5.0};
    int order = (int)c.size ();
    for (auto f : testFractions)
        {
        double c1, s1, w1;
        bsiBezier_evaluate (&c1, &c[0], order, 1, f);
        bsiBezier_evaluate (&s1, &s[0], order, 1, f);
        bsiBezier_evaluate (&w1, &w[0], order, 1, f);
        Check::Near (w1 * w1, c1 * c1 + s1 * s1, "Bezier point is on circle");
        }
    }
// Note args are passed by value (and modified internally, but not returned)
// Input c,s,w for which c(u) * c(u) + s (u) * s(u) == w(u) * w(u), i.e (c/w)(s/w) is always on the unit circle.
// Apply double angle trig identities to successively double the angle covered by the bezier.
void CheckBezierCircleAngleDoubling (bvector<double> c, bvector<double> s, bvector<double> w, size_t maxOrderToDouble, int noisy)
    {
    bvector<double> cc, ss, ww, cs;
    CheckBezierEvaluationOnCircle (c, s, w);
    if (noisy > 0)
        {
        GEOMAPI_PRINTF("\n\n Bezier Circle base order %d\n         c             s           w     degrees  delta\n", (int)c.size ());
        double degrees0 = 0.0;
        for (size_t i = 0; i < c.size (); i++)
            {
            double degrees1 = Angle::RadiansToDegrees (atan2 (s[i], c[i]));
            GEOMAPI_PRINTF ("( %23.17g   %23.17g )/  %23.17g   (%23.17g  %10.5f)\n", c[i], s[i], w[i], degrees1, degrees1 - degrees0);
            degrees0 = degrees1;
            }
        }
    // c,s,w are beziers for degree d circle polynomial, initially d=2;
    // form (cc-ss), (2cs), and (ww) as degree 2*d.
    for (;c.size () <= maxOrderToDouble;)
        {
        int orderA = (int)c.size ();
        int orderB = orderA * 2 - 1;
        cc.resize ((size_t)orderB);
        ss.resize ((size_t)orderB);
        ww.resize ((size_t)orderB);
        cs.resize ((size_t)orderB);

        Check::True (bsiBezier_univariateProduct
            (
            &cc[0], 0, 1,
            &c[0], orderA, 0, 1,
            &c[0], orderA, 0, 1
            ));
        Check::True (bsiBezier_univariateProduct
            (
            &ss[0], 0, 1,
            &s[0], orderA, 0, 1,
            &s[0], orderA, 0, 1
            ));
        Check::True (bsiBezier_univariateProduct
            (
            &ww[0], 0, 1,
            &w[0], orderA, 0, 1,
            &w[0], orderA, 0, 1
            ));
        Check::True (bsiBezier_univariateProduct
            (
            &cs[0], 0, 1,
            &c[0], orderA, 0, 1,
            &s[0], orderA, 0, 1
            ));
        if (noisy > 2)
            {
            GEOMAPI_PRINTF("\n         cc             ss           cs              ww\n");
            for (size_t i = 0; i < (size_t)orderB; i++)
                {
                GEOMAPI_PRINTF (" %23.17g   %23.17g   %23.17g    %23.17g\n", cc[i], ss[i], cs[i], ww[i]);
                }
            }
        
        c.clear ();
        s.clear ();
        w.clear ();
        if (noisy > 1)
            GEOMAPI_PRINTF("\n Order %d Bezier Circle Poles (c,s,w)\n", orderB);
        for (size_t i = 0; i < (size_t)orderB; i++)
            {
            c.push_back (cc[i] - ss[i]);
            s.push_back (2.0 * cs[i]);
            w.push_back (ww[i]);
            if (noisy > 1)
                GEOMAPI_PRINTF (" %23.17g   %23.17g   %23.17g\n", c[i], s[i], w[i]);
            }
        CheckBezierEvaluationOnCircle (c, s, w);
        }
    }

// Test construction of a circle from a (weighted) bezier polynomial of various degrees and angle range.
// Use double angle formulas to raise the angle coverage.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bezier,Circles)
    {
    static int s_noisyCircles = 0;
    for (double baseDegrees : bvector<double> {180.0, 90.0, 45.0})
        {
        if (s_noisyCircles > 0)
            GEOMAPI_PRINTF ("\n*** Bezier Polynomials.  Base Span Degrees %g\n", baseDegrees);
        // bezier so (c/w) (s/w) is an exact circle -- c*c+s*s = w*w
        Angle d0 = Angle::FromDegrees (0.0);
        Angle d1 = Angle::FromDegrees (baseDegrees * 0.5);
        Angle d2 = Angle::FromDegrees (baseDegrees);
        // Quadratic Bezier for baseDegrees.
        // first pole is on the circle at angle 0, weight 1
        // last pole is on the circle at angle baseDegrees, weight 1
        // direct c,s of middle pole is on the circle at angle baseDegrees/2, weight baseDegrees.Cos ()
        bvector<double> c {d0.Cos (), d1.Cos (), d2.Cos ()};
        bvector<double> s {d0.Sin (), d1.Sin (), d2.Sin ()};
        bvector<double> w {1.0,d1.Cos (),1.0};
        CheckBezierCircleAngleDoubling (c, s, w, 9, s_noisyCircles);
        for (;c.size () < 6;)
            {
            // Raise the degree by 1.  First, last poles stay on the circle with weight 1.
            int orderA = (int) c.size ();
            c.resize (c.size () + 1);
            s.resize (c.size () + 1);
            w.resize (c.size () + 1);
            int orderB = orderA + 1;
            bsiBezier_raiseDegreeInPlace (&c[0], orderA, orderB, 1);
            bsiBezier_raiseDegreeInPlace (&s[0], orderA, orderB, 1);
            bsiBezier_raiseDegreeInPlace (&w[0], orderA, orderB, 1);
            CheckBezierCircleAngleDoubling (c, s, w, 10, s_noisyCircles);
            }
        }
    }