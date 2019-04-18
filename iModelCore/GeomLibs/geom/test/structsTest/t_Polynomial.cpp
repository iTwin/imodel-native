/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

void CheckQuadratic (double root0, double root1, double c, bool safeDivide)
    {
    // y = c * (x-root0) * (x-root1)
    Polynomial::Power::Degree2 q = Polynomial::Power::Degree2::FromRootsAndC2(root0, root1, c);

    if (c != 0.0)
        {
        double ss[2];
        if (safeDivide)
            Check::Int (2, q.RealRootsWithSafeDivideCheck (ss));
        else 
            Check::Int (2, q.RealRoots (ss));
        double midRoot = 0.5 * (root0 + root1);
        Check::True (ss[0] < ss[1], "sorted roots");
        Check::Near (ss[0], DoubleOps::Min (root0, root1), "Min root");
        Check::Near (ss[1], DoubleOps::Max (root0, root1), "Max root");
        double x0, y0, c0;
        Check::True (q.TryGetVertexFactorization (x0, y0, c0), "Vertex factorization");
        Check::Near (0.0, q.Evaluate (root0), "root0");
        Check::Near (0.0, q.Evaluate (root1), "root1");
        Check::Near (midRoot, x0, "vertex x");
        
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolynomialPowerDegree2, Test0)
    {
    for (int pass = 0; pass < 2; pass++)
        {
        CheckQuadratic (1,2,-3, pass != 0);
        CheckQuadratic (1,2, 5, pass != 0);
        CheckQuadratic (-20, 40, 0.23423, pass != 0);
        }
    }
    
void CheckLinear (double root, double slope, bool safeDivide)
    {
    // y = c * (x-root0) * (x-root1)
    Polynomial::Power::Degree2 q = Polynomial::Power::Degree2(-root * slope, slope, 0.0);
    if (slope != 0.0)
        {
        double ss[2];
        if (safeDivide)
            Check::Int (1, q.RealRootsWithSafeDivideCheck (ss));
        else 
            Check::Int (1, q.RealRoots (ss));
        Check::Near (ss[0], root, "Linear case root");
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolynomialPowerDegree2, Test1)
    {
    for (int pass = 0; pass < 2; pass++)
        {
        CheckLinear (1,-3, pass != 0);
        CheckLinear (1, 5, pass != 0);
        CheckLinear (-20, 0.23423, pass != 0);
        }
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolynomialPowerDegree2, BigRoots)
    {
    double dx = 1.0;
    for (double center = 10.0; center < 1e8; center*= 10)
        {
        double root0 = center - dx;
        double root1 = center + dx;
        for (double c = 0.00001; c < 10.0; c *= 10.0)
            {
            double s0[2], s1[2], s2[2];
            Polynomial::Power::Degree2 q = Polynomial::Power::Degree2::FromRootsAndC2(root0, root1, c);
            int n0 = q.RealRoots (s0);
            int n1 = q.RealRootsWithSafeDivideCheck (s1);
            Polynomial::Power::Degree2 qneg = Polynomial::Power::Degree2::FromRootsAndC2(-root0, -root1, c);
            int n2 = qneg.RealRootsWithSafeDivideCheck (s2);

            char buffer[1000];
            sprintf (buffer, " (center %g) (c %g) (n0 %d) (n1 %d)", center, c, n0, n1);
            Check::StartScope (buffer);


            Check::Int (n1, n2, "positive/negative match");
            Check::Near (-s1[1], s2[0], "+- root0");
            Check::Near (-s1[0], s2[1], "+- root0");
            if (c > 1e-8 * center)
                {
                if (c > 1.0e-6 * center)
                    Check::Int (2, n0, "quadeqn");
                if (Check::Int (2, n1, "safe quadeqn"))
                    {
                    if (c > 1.0e-5 * center)
                        {
                        Check::Near (root0, s1[0], "root0");
                        Check::Near (root1, s1[1], "root1");
                        }
                    }
                }
            Check::EndScope ();
            }        
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolynomialPowerDegree2, EvaluateThetaPhiDistance)
    {
    Polynomial::Implicit::Torus tor(30, 5, false);  // "old style".  Internal convention is now true!!!
    DPoint3d point3D = tor.EvaluateThetaPhiDistance(60, 15, 5);
    DPoint3d pointSecond3D = DPoint3d::From(-24.9547062661, -7.98651391025, 3.25143920079);
    Check::Near(point3D, pointSecond3D);
    }
