/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include <testHarness.h>
#include <Geom/XYRangeTree.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL


static bool s_noisy = false;
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
virtual int  GetVectorIntegrandCount () override
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
    double gaussRatio = pow (2.0, (int)(gaussPower - 1));
    int    maxGaussRatioPower = (int)gaussPower + 3;  // expect really good error ratios for a few powers past the stated limit ...
    static double s_gaussRatioFuzzFactor = 0.75;
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

        if(s_noisy)
            {
            printf (" Integration of simple powers on [%g,%g]\n", a0, a1);
            printf ("  i    I               eGauss1   eGauss2    eGaussX   eKronrod\n");
            }
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

            if (s_noisy)
                {
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
            // if the full interval gauss error is "clearly nonzero", we expect consistent ratios with 2X and 4X ..
            // SPECIFICALLY -- with n-point gauss rule, the ratio is near to  2^(2n-1):  (3=>64)
            static double s_testThreshold = 1.0e-12;
            if (fabs (eGauss) > s_testThreshold)
                {
                double ratio4, ratio2;
                DoubleOps::SafeDivide (ratio4, eGauss2, eGauss4, 0.0);
                DoubleOps::SafeDivide (ratio2, eGauss, eGauss2, 0.0);
                if (i < maxGaussRatioPower)
                    Check::True (ratio4 > s_gaussRatioFuzzFactor * gaussRatio, "Interval halving convergence measure");
                if (s_noisy)
                    printf ("      (2X factor %.4g) (4X factor (%.4g)\n", ratio2, ratio4);
                }


            }
        }
    }

// f(x,i) = elliptic arc length differential at theta, for axis lengths (1,b) 
// where b is 1, factor, factor^2 etc
class EllipticArcQuadrantIntegrand : public BSIVectorIntegrand
{
public:
int m_maxFactor;
double m_factor;
EllipticArcQuadrantIntegrand (int maxDivide, double factor) : m_maxFactor (maxDivide), m_factor(factor)
    {
    
    }

virtual void EvaluateVectorIntegrand (double theta, double *pF) override
    {
    double b = 1.0;
    double s = sin (theta);
    double c = cos (theta);
    double ss = s * s;
    double cc = c * c;
    for (int i = 0; i < m_maxFactor; i++)
        {
        // x = cos (theta)   dx = -sin(theta) dtheta
        // y = b sin (theta)  dy =  b cos (theta) dtheta
        double dd = ss + b * b * cc;
        pF[i] = sqrt (dd);
        b *= m_factor;
        }
    }
virtual int  GetVectorIntegrandCount () override
    {
    return m_maxFactor;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GaussKronrod,EllipticArcLength)
    {
    BSIQuadraturePoints gauss3, kronrod, gauss5, gauss7, gaussToUse;
    kronrod.InitGaussKronrod (7, gauss3);
    gauss7.InitGauss (7);
    gauss5.InitGauss (5);
    gaussToUse = gauss3;
    int numEllipse= 6;
    double axisFactor = 0.5;
    EllipticArcQuadrantIntegrand functions(numEllipse, axisFactor);
    double gaussSums[200], kronrodSums[200], gauss2Sums[200], gauss4Sums[200];
    double kronrod2Sums[200];
    double gaussPower = gaussToUse.GetConvergencePower ();
    double extrapolationFactor = 0.5 / (pow (2.0, gaussPower) - 1.0);
    double baseAngleStep = 0.10;
    double sweep = 0.5;
    for (double a0 = -0.2; a0 < 1.0; a0 += baseAngleStep)
        {
        double a1 = a0 + sweep;
        for (int i = 0; i < numEllipse; i++)
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
        if (s_noisy)
            {
            printf (" Integration of elliptic arcs on [%g,%g]\n", a0, a1);
            printf ("   Gauss extrapolation power %g    factor %g\n", gaussPower, extrapolationFactor);
            printf ("  i   b        I                  eGauss1      eGauss2         eGaussX        eKronrod\n");
            }
        for (int i = 0; i < numEllipse; i++)
            {
            double b = pow (axisFactor, i);
            double trueIntegral = bsiGeom_ellipseArcLength (1.0, b, a0, a1 - a0);
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
            if (s_noisy)
                {
                printf ("%3d %8g %20.16g (G1 %7.1e)(G2 %7.1e)(X12 %7.1e)(G4 %7.1e)(X24 %7.1e)  (K %7.1e)(K2 %7.1e) (dGK %7.1e)\n", i, b, trueIntegral,
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
    }

     // f(x,i) = x^i
    class PolynomialPowersIntegrandXY : public BSIVectorIntegrandXY
    {
    public:
    int m_maxDegree;
    PolynomialPowersIntegrandXY (int maxDegree) : m_maxDegree (maxDegree)
        {
        }

    virtual void EvaluateVectorIntegrand (double x, double y, double *pF) override
        {
        int k = 0;
        for (int i = 0; i <= m_maxDegree; i++)
            {
            for (int j = 0; i + j <=  m_maxDegree; j++)
                {
                pF[k++] = pow (x, i) * pow(y,j);
                }
            }
        }
    virtual int  GetVectorIntegrandCount () override
        {
        return (m_maxDegree + 1) * (m_maxDegree + 2) / 2;
        }
    };    
    
    
     // f(x,i) = x^i
    class ExponentialFunctionIntegrandXY : public BSIVectorIntegrandXY
    {
    public:
    int m_maxDegree;
    ExponentialFunctionIntegrandXY () : m_maxDegree (0)
        {
        }

    virtual void EvaluateVectorIntegrand (double x, double y, double *pF) override
        {
        pF[0] = cos (x);
        pF[1] = sin (x);
        pF[2] = exp (x);
        }
    virtual int  GetVectorIntegrandCount () override
        {
        return 3;
        }
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, Strang1)
    {
    int power  = 3;
    int selector = 1;
    double pSums1 [10];
    double pSums2 [10];
    double pSums3 [10];
    double pSums4 [10];
    double pSums5 [10];
    double pSums6 [10];
    
    double one = 1.0 / 2.0;
    double xANDy = 1.0 / 6.0;
    double x2ANDy2 = 1.0 / 12.0;
    double xy = 1.0 / 24.0;
    double x3ANDy3 = 1.0 / 20.0;
    double xy2ANDx2y = 1.0 / 60.0;
    
    for (int i = 0; i < 10; i++)
        {
        pSums1[i] = pSums2[i] = pSums3[i] = pSums4[i] = pSums5[i] = pSums6[i] =  0;
        }
        
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums1);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums2); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums3);
    selector++;   
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums4);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums5); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums6);     
     
     if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                  Approximate Integral (1, 2, 3)                        Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = 1   :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[0], pSums2[0], pSums3[0], one);
        printf ("f(x,y) = y   :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[1], pSums2[1], pSums3[1], xANDy);
        printf ("f(x,y) = y2  :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[2], pSums2[2], pSums3[2], x2ANDy2);
        printf ("f(x,y) = y3  :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[3], pSums2[3], pSums3[3], x3ANDy3);
        printf ("f(x,y) = x   :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[4], pSums2[4], pSums3[4], xANDy);
        printf ("f(x,y) = xy  :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[5], pSums2[5], pSums3[5], xy);
        printf ("f(x,y) = xy2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[6], pSums2[6], pSums3[6], xy2ANDx2y);
        printf ("f(x,y) = x2  :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[7], pSums2[7], pSums3[7], x2ANDy2);
        printf ("f(x,y) = x2y :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[8], pSums2[8], pSums3[8], xy2ANDx2y);
        printf ("f(x,y) = x3  :  %22.5f %22.5f %22.5f %22.5f\n", pSums1[9], pSums2[9], pSums3[9], x3ANDy3);
    
        printf ("\n");
        printf ("Function                                  Approximate Integral (4, 5, 6)                        Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = 1   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[0], pSums5[0], pSums6[0], one);
        printf ("f(x,y) = y   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[1], pSums5[1], pSums6[1], xANDy);
        printf ("f(x,y) = y2  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[2], pSums5[2], pSums6[2], x2ANDy2);
        printf ("f(x,y) = y3  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[3], pSums5[3], pSums6[3], x3ANDy3);
        printf ("f(x,y) = x   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[4], pSums5[4], pSums6[4], xANDy);
        printf ("f(x,y) = xy  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[5], pSums5[5], pSums6[5], xy);
        printf ("f(x,y) = xy2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[6], pSums5[6], pSums6[6], xy2ANDx2y);
        printf ("f(x,y) = x2  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[7], pSums5[7], pSums6[7], x2ANDy2);
        printf ("f(x,y) = x2y :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[8], pSums5[8], pSums6[8], xy2ANDx2y);
        printf ("f(x,y) = x3  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[9], pSums5[9], pSums6[9], x3ANDy3);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, Strang2)
    {
    int power  = 5;
    int selector = 4;
    double pSums4 [100];
    double pSums5 [100];
    double pSums6 [100];
    
    double x4ANDy4 = 1.0 / 30.0;
    double x5ANDy5 = 1.0 / 42.0;
    double xy3 = 1.0 / 120.0;
    double xy4 = 1.0 / 210.0;
    double x2y2 = 1.0 / 180.0;
    double x2y3 = 1.0 / 420.0;
    double x3y = xy3;
    double x3y2 = x2y3;
    double x4y = 1 / 30.0;
    
    for (int i = 0; i < 100; i++)
        {
        pSums4[i] = pSums5[i] = pSums6[i] =  0;
        }
        
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums4);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums5); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums6);     

     if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                  Approximate Integral (4, 5, 6)                        Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = y4   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[4], pSums5[4], pSums6[4], x4ANDy4);
        printf ("f(x,y) = y5   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[5], pSums5[5], pSums6[5], x5ANDy5);
        printf ("f(x,y) = xy3  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[9], pSums5[9], pSums6[9], xy3);
        printf ("f(x,y) = xy4  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[10], pSums5[10], pSums6[10], xy4);
        printf ("f(x,y) = x2y2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[13], pSums5[13], pSums6[13], x2y2);
        printf ("f(x,y) = x2y3 :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[14], pSums5[14], pSums6[14], x2y3);
        printf ("f(x,y) = x3y  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[16], pSums5[16], pSums6[16], x3y);
        printf ("f(x,y) = x3y2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[17], pSums5[17], pSums6[17], x3y2);
        printf ("f(x,y) = x4   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[18], pSums5[18], pSums6[18], x4ANDy4);
        printf ("f(x,y) = x4y  :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[19], pSums5[19], pSums6[19], x4y);
        printf ("f(x,y) = x5   :  %22.5f %22.5f %22.5f %22.5f\n", pSums4[20], pSums5[20], pSums6[20], x5ANDy5);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, Strang3)
    {
    int power  = 8;
    int selector = 7;
    double pSums7 [100];
    double pSums8 [100];
    double pSums9 [100];
    double pSums10 [100];
    
    double x5 = 1.0 / 42.0;
    double y5 = x5;
    double x6 = 1.0 / 56.0;
    double y6 = x6;
    double x7 = 1.0 / 72.0;
    double y7 = x7;
    double x8 = 1.0 / 90.0;
    double y8 = x8;
    double xy4 = 1.0 / 210.0;
    double xy5 = 1.0 / 336.0;
    double xy6 = 1.0 / 504.0;
    double xy7 = 1.0 / 720.0;
    
    for (int i = 0; i < 100; i++)
        {
        pSums7[i] = pSums8[i] = pSums9[i] = pSums10[i] =  0;
        }
        
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums7);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums8); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums9);  
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums10);   

         if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                           Approximate Integral (7, 8, 9, 10)                                    Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = y5  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[5], pSums8[5], pSums9[5], pSums10[5], y5);
        printf ("f(x,y) = y6  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[6], pSums8[6], pSums9[6], pSums10[6], y6);
        printf ("f(x,y) = y7  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[7], pSums8[7], pSums9[7], pSums10[7], y7);
        printf ("f(x,y) = y8  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[8], pSums8[8], pSums9[8], pSums10[8], y8);
        printf ("f(x,y) = xy4 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[13], pSums8[13], pSums9[13], pSums10[13], xy4);
        printf ("f(x,y) = xy5 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[14], pSums8[14], pSums9[14], pSums10[14], xy5);
        printf ("f(x,y) = xy6 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[15], pSums8[15], pSums9[15], pSums10[15], xy6);
        printf ("f(x,y) = xy7 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[16], pSums8[16], pSums9[16], pSums10[16], xy7);
        printf ("f(x,y) = x5  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[35], pSums8[35], pSums9[35], pSums10[35], x5);
        printf ("f(x,y) = x6  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[39], pSums8[39], pSums9[39], pSums10[39], x6);
        printf ("f(x,y) = x7  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[42], pSums8[42], pSums9[42], pSums10[42], x7);
        printf ("f(x,y) = x8  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[44], pSums8[44], pSums9[44], pSums10[44], x8);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, Strang4)
    {
    int selector = 7;
    double pSums7 [3];
    double pSums8 [3];
    double pSums9 [3];
    double pSums10 [3];
    
    for (int i = 0; i < 3; i++)
        {
        pSums7[i] = pSums8[i] = pSums9[i] = pSums10[i] =  0;
        }
        
    ExponentialFunctionIntegrandXY f = ExponentialFunctionIntegrandXY ();
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums7);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums8); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums9);  
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSums (f, pSums10);   
    if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                           Approximate Integral (7, 8, 9, 10)                                    Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = cos(x)  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[0], pSums8[0], pSums9[0], pSums10[0], 0.45969769413186028259906339255702);
        printf ("f(x,y) = sin(x)  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[1], pSums8[1], pSums9[1], pSums10[1], 0.1585290151921034933474976783697);
        printf ("f(x,y) = exp(x)  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[2], pSums8[2], pSums9[2], pSums10[2], 0.718281828459045235360287471352);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, MappedStrangConsistency)
    {
    int power  = 8;
    int selector = 9;
    double pSumsv1 [100];
    double pSumsv2 [100];
    double pSumsv3 [100];
    double pSumsv4 [100];
     double pSumsv5 [100];
    
    double ax = 1.0;
    double ay = 1.0;
    double bx = 11.0 / 10.0;
    double by = 1.0;
    double cx = 1.0;
    double cy = 11.0 / 10.0;
    
    for (int i = 0; i < 100; i++)
        {
        pSumsv1[i] = pSumsv2[i] = pSumsv3[i] = pSumsv4[i] = pSumsv5[i] = 0;
        }
        
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSumsv1, ax, ay, bx, by, cx, cy);
    test.AccumulateWeightedSumsMapped (f, pSumsv2, bx, by, ax, ay, cx, cy); 
    test.AccumulateWeightedSumsMapped (f, pSumsv3, cx, cy, ax, ay, bx, by);  
    test.AccumulateWeightedSumsMapped (f, pSumsv4, bx, by, cx, cy, ax, ay);
    test.AccumulateWeightedSumsMapped (f, pSumsv5, cx, cy, bx, by, ax, ay);   
     if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                   Approximate integrals (different directions of vectors)        \n");
        printf ("\n");
        printf ("f(x,y) = 1   :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[0], pSumsv2[0], pSumsv3[0], pSumsv4[0], pSumsv5[0]);
        printf ("f(x,y) = y   :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[1], pSumsv2[1], pSumsv3[1], pSumsv4[1], pSumsv5[1]);
        printf ("f(x,y) = y2  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[2], pSumsv2[2], pSumsv3[2], pSumsv4[2], pSumsv5[2]);
        printf ("f(x,y) = y5  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[5], pSumsv2[5], pSumsv3[5], pSumsv4[5], pSumsv5[5]);
        printf ("f(x,y) = y8  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[8], pSumsv2[8], pSumsv3[8], pSumsv4[8], pSumsv5[8]);
        printf ("f(x,y) = x   :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[9], pSumsv2[9], pSumsv3[9], pSumsv4[9], pSumsv5[9]);
        printf ("f(x,y) = xy3 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSumsv1[12], pSumsv2[12], pSumsv3[12], pSumsv4[12], pSumsv5[12]);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, MappedStrang1)
    {
    int power  = 8;
    int selector = 7;
    double pSums7 [100];
    double pSums8 [100];
    double pSums9 [100];
    double pSums10 [100];
    
    double ax = 1.0;
    double ax5 = pow(ax, 5);
    double ax6 = ax5 * ax;
    double ay = 1.0;
    double ay5 = pow(ay, 5);
    double ay6 = ay5 * ay;
    double ay7 = ay6 * ay;
    double ay8 = ay7 * ay;
    double bx = 2.0;
    double bx5 = pow(bx, 5);
    double bx6 = bx5 * bx;
    double by = 1.0;
    double cx = 1.0;
    double cy = 2.0;
    double cy5 = pow(cy, 5);
    double cy6 = cy5 * cy;
    double cy7 = cy6 * cy;
    double cy8 = cy7 * cy;
    double a2x = bx;
    double a2y = cy;
    
    double xpar = (1.0 / 2.0) * (bx*bx - ax*ax);
    
    double y5 = (1.0 / 6.0) * (bx - ax) * (cy6 - ay6);
    double y6 = (1.0 / 7.0) * (bx - ax) * (cy6*cy - ay6*ay);
    double y7 = (1.0 / 8.0) * (bx - ax) * (cy6*cy*cy - ay6*ay*ay);
    double y8 = (1.0 / 9.0) * (bx - ax) * (cy6*cy*cy*cy - ay6*ay*ay*ay);
    double x5 = (1.0 / 6.0) * (bx6 - ax6) * (cy - ay);
    double x6 = (1.0 / 7.0) * (bx6*bx - ax6*ax) * (cy - ay);
    double x7 = (1.0 / 8.0) * (bx6*bx*bx - ax6*ax*ax) * (cy - ay);
    double x8 = (1.0 / 9.0) * (bx6*bx*bx*bx - ax6*ax*ax*ax) * (cy - ay);
    double xy4 = xpar * (1.0 / 5.0) * (cy5 - ay5);
    double xy5 = xpar * (1.0 / 6.0) * (cy6 - ay6);
    double xy6 = xpar * (1.0 / 7.0) * (cy7 - ay7);
    double xy7 = xpar * (1.0 / 8.0) * (cy8 - ay8);
    
    for (int i = 0; i < 100; i++)
        {
        pSums7[i] = pSums8[i] = pSums9[i] = pSums10[i] =  0;
        }
        
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums7, ax, ay, bx, by, cx, cy);
    test.AccumulateWeightedSumsMapped (f, pSums7, a2x, a2y, cx, cy, bx, by);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums8, ax, ay, bx, by, cx, cy); 
    test.AccumulateWeightedSumsMapped (f, pSums8, a2x, a2y, cx, cy, bx, by);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums9, ax, ay, bx, by, cx, cy); 
    test.AccumulateWeightedSumsMapped (f, pSums9, a2x, a2y, cx, cy, bx, by); 
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums10, ax, ay, bx, by, cx, cy);  
    test.AccumulateWeightedSumsMapped (f, pSums10, a2x, a2y, cx, cy, bx, by); 

     if (s_noisy)
        {
        printf ("\n");
        printf ("Function                                           Approximate Integral (7, 8, 9, 10)                                 Exact Integral\n");
        printf ("\n");
        printf ("f(x,y) = y5  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[5], pSums8[5], pSums9[5], pSums10[5], y5);
        printf ("f(x,y) = y6  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[6], pSums8[6], pSums9[6], pSums10[6], y6);
        printf ("f(x,y) = y7  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[7], pSums8[7], pSums9[7], pSums10[7], y7);
        printf ("f(x,y) = y8  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[8], pSums8[8], pSums9[8], pSums10[8], y8);
        printf ("\n");
        printf ("f(x,y) = xy4 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[13], pSums8[13], pSums9[13], pSums10[13], xy4);
        printf ("f(x,y) = xy5 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[14], pSums8[14], pSums9[14], pSums10[14], xy5);
        printf ("f(x,y) = xy6 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[15], pSums8[15], pSums9[15], pSums10[15], xy6);
        printf ("f(x,y) = xy7 :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[16], pSums8[16], pSums9[16], pSums10[16], xy7);
        printf ("\n");
        printf ("f(x,y) = x5  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[35], pSums8[35], pSums9[35], pSums10[35], x5);
        printf ("f(x,y) = x6  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[39], pSums8[39], pSums9[39], pSums10[39], x6);
        printf ("f(x,y) = x7  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[42], pSums8[42], pSums9[42], pSums10[42], x7);
        printf ("f(x,y) = x8  :  %22.5f %22.5f %22.5f %22.5f %22.5f\n", pSums7[44], pSums8[44], pSums9[44], pSums10[44], x8);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (GaussKronrod, MappedStrang2)
    {
    int power  = 2;
    double pSums7 [10];
    double pSums8 [10];
    
    for (int i = 0; i < 5; i++)
    {
    int selector = 7;
    
     for (int j = 0; j < 10; j++)
      {
      pSums7[j] = pSums8[j] =  0;
      }   
    double ax = 5.0 + i;
    double ay = 7.0 + i;
    double bx = 12.0 + i;
    double by = 13.0 + i;
    double cx = 4.0 + i;
    double cy = 11.0 + i;
    
    double Ux = bx - ax;
    double Uy = by - ay;
    double Vx = cx - ax;
    double Vy = cy - ay;
    
    double ar = 0.5 * (Ux * Vy - Uy * Vx);
    double minxm = ay * ar;
    double minxmm = ay * minxm;
    double maxxm = by * ar;
    double maxxmm = by * maxxm;
    double minym = cx * ar;
    double minymm = cx * minym;
    double maxym = bx * ar;
    double maxymm = bx * maxym;
    
    PolynomialPowersIntegrandXY f = PolynomialPowersIntegrandXY (power);
    BSITriangleQuadraturePoints test;
    
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums7, ax, ay, bx, by, cx, cy);
    selector++;
    test.InitStrang(selector);
    test.AccumulateWeightedSumsMapped (f, pSums8, ax, ay, bx, by, cx, cy); 
    
     if (s_noisy)
        {
        printf ("\n");
        printf ("Function                          Approximate Integral (7, 8)                         Min, Max\n");
        printf ("\n");
        printf ("f(x,y) = y  :  %22.5f %22.5f %22.5f %22.5f\n", pSums7[1], pSums8[1], minym, maxym);
        printf ("f(x,y) = y2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums7[2], pSums8[2], minymm, maxymm);
        printf ("f(x,y) = x  :  %22.5f %22.5f %22.5f %22.5f\n", pSums7[3], pSums8[3], minxm, maxxm);
        printf ("f(x,y) = x2 :  %22.5f %22.5f %22.5f %22.5f\n", pSums7[5], pSums8[5], minxmm, maxxmm);
        }

    Check::True (pSums7[1] <= maxym && minym <= pSums7[1], "checking between"); 
    Check::True (pSums7[2] <= maxymm && minymm <= pSums7[2], "checking between"); 
    Check::True (pSums7[3] <= maxxm && minxm <= pSums7[3], "checking between"); 
    Check::True (pSums7[5] <= maxxmm && minxmm <= pSums7[5], "checking between"); 
    }
    
    }
#ifdef TestlambdaFunctions
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPP,LambdaFunction)
    {
    double a = -1.0;
    auto f = [&a](double x) -> double {return a * x;};
    double x = 2.0;
    for (a = 1.0; a < 5.0; a += 3.0)
        {
        printf ("Hello Lambda %g{a=%g} --> %g\n", x, a, f(x));
        }
    }
#endif

struct SimpleFunctions : BSIIncrementalVectorIntegrand
{
virtual void EvaluateVectorIntegrand (double x, double *f) override
    {
    f[0] = 1.0;
    f[1] = x;
    f[2] = x * x;
    f[3] = x * x * x;
    f[4] = sin(x);
    f[5] = cos(x);
    }
virtual int  GetVectorIntegrandCount () override {return 6;}
void EvaluateTrueIntegrals (double a, double b, bvector<double> &F)
    {
    F.clear ();
    F.push_back (b - a);
    F.push_back ((b*b - a*a) / 2.0);
    F.push_back ((b*b*b - a*a*a) / 3.0);
    F.push_back ((b*b*b *b - a*a*a *a) / 4.0);
    F.push_back (-(cos(b) - cos(a)));
    F.push_back ( sin(b) - sin(a));
    }
double m_lastResult[10];
virtual bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    int n = GetVectorIntegrandCount ();
    for (int i = 0; i < n; i++)
        {
        m_lastResult[i] = pIntegrals[i];
        }
    return true;
    }
void GetLastIntegrals (bvector<double> &data)
    {
    int n = GetVectorIntegrandCount ();
    data.clear ();
    for (int i = 0; i < n; i++)
        {
        data.push_back (m_lastResult[i]);
        }
    }

};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GaussRules,IntegrateWithRombergExtrapolation)
    {
    BSIQuadraturePoints gauss5, gauss2;
    gauss5.InitGauss (5);
    gauss2.InitGauss (2);

    bvector<double> F5(10), FExact(10), F2(10);
    SimpleFunctions functions;
    double a = 0.4;
    double b = 0.8;
    double e5, e2;
    gauss5.IntegrateWithRombergExtrapolation (functions, a, b, 3, e5);
    functions.GetLastIntegrals (F5);
    gauss2.IntegrateWithRombergExtrapolation (functions, a, b, 3, e2);
    functions.GetLastIntegrals (F2);
    functions.EvaluateTrueIntegrals (a, b, FExact);
    double e5Max = DoubleOps::MaxAbsDiff (F5, FExact);
    double e2Max = DoubleOps::MaxAbsDiff (F2, FExact);
    Check::LessThanOrEqual (e5Max, 1.0e-14);
    Check::LessThanOrEqual (e5Max, e2Max);

    for (int numGauss = 1; numGauss < 6; numGauss++)
        {
        BSIQuadraturePoints gauss;
        gauss.InitGauss (numGauss);
        bvector<double> eMaxGauss2, eeGauss2;
        double e, eBound;
        for (int numInterval = 2; numInterval < 20; numInterval *= 2)
            {
            //eBound;
            gauss.IntegrateWithRombergExtrapolation (functions, a, b, numInterval, eBound);
            functions.GetLastIntegrals (F2);
            e = DoubleOps::MaxAbsDiff (F2, FExact);
            eeGauss2.push_back (eBound);
            eMaxGauss2.push_back (e);
            // WE EXPECT -- the error bound is an overestimate . ..
            Check::LessThanOrEqual (e, DoubleOps::Max (eBound, 1.0e-15));
            }
        for (size_t i = 0; i + 1 < eeGauss2.size (); i++)
            Check::LessThanOrEqual (eeGauss2[i+1], DoubleOps::Max (eeGauss2[i], 1.0e-15));
        }
    }