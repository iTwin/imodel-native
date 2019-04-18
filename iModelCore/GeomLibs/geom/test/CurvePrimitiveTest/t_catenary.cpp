/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Catenary,HelloWorld)
    {
    static double s_catenaryStrokeTolerance = 1.0e-3;       // hm... really crude, but seems to be so . .
    auto options = IFacetOptions::CreateForCurves ();
    options->SetAngleTolerance (0.1);
    DPoint3dDVec3dDVec3dCR identity = DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,0),  DVec3d::From (1,0,0), DVec3d::From (0,1,0));

    for (double a: bvector<double>{0.5,1.0,2.0})
        {
        for (double intervalFactor : bvector<double>{1,2, 4})
            {
            double x1 = intervalFactor * a;      // small a ==> hard curvature approximation at origin
            
            Check::PrintIndent (2);
            Check::Print (a, "Catenary Constant");
            Check::Print(x1, "interval length");

            auto cp0 = ICurvePrimitive::CreateCatenary (a, identity, 0.0, x1);
            auto cp = cp0->Clone ();

            auto bcurve = cp0->CloneAsBspline();
            // NOW ... We know that cp0's bspline has been created.  We would like the cloned cp to never evaluate its own.
            double bcurveLength, strokeLength, catenaryPrimitiveLength;
            Check::True ((int)cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Catenary);
            double trueCatenaryLength = x1;
            Check::True (bcurve->Length (bcurveLength));
            Check::True (cp->Length (catenaryPrimitiveLength));
            bvector<DPoint3d> points;
            cp->AddStrokes (points, *options);
            auto bcurve0 = cp->CloneAsBspline();
            //Check::Print (points, "Catenary Stroke Points");
            strokeLength = PolylineOps::Length (points);
            Check::PrintIndent (4);
            Check::Print((uint64_t)points.size (), "stroke count");

            Check::PrintIndent (4);
            Check::Print(strokeLength, "stroke length");
            Check::Print (strokeLength-trueCatenaryLength, "Estroke");

            Check::PrintIndent (4);
            Check::Print(bcurveLength, "bcurve length");
            Check::Print (bcurveLength-trueCatenaryLength, "Ebc");

            Check::Shift(10, 0, 0);

            // We know the catenary is y = a*cosh(x/a) . . .
            UsageSums error;
            for (auto xyz : points)
                {
                double yA = xyz.y;
                double yB = a * cosh(xyz.x / a);
                error.Accumulate (fabs (yA-yB));
                }
            Check::PrintIndent (4);
            Check::Print (error.Max (), "Max Deviation");
            Check::True (error.Max () < s_catenaryStrokeTolerance);
            }
        }
    }

#ifdef OriginalFormGamma
void TestCoshIntersectLine (double gamma, double x0, double x1, bool print = false)
    {
    // ASSUME x0 != x1 ...
    // Evaluate two points on the cosh .. .
    double y0 = gamma * cosh (x0);
    double y1 = gamma * cosh (x1);
    // compute slope and intercept of the chord ...
    double beta = (y1 - y0) / (x1 - x0);
    double alpha = y0 - beta * x0;
    bvector<double> roots;
    bvector<double> knownRoots {x0, x1};
    if (DCatenaryXY::CoshIntersectLine (alpha, beta, -gamma, roots))
        {
        // double xx[2] = {x0, x1};
        if (print)
            {
            Check::PrintIndent (2);
            Check::Print (alpha, "alpha");
            Check::Print (beta, "beta");
            Check::Print (gamma, "gamma");
            Check::PrintIndent (2);
            Check::Print (knownRoots, "KnownRoots");
            Check::Print (roots, "Roots");
            }
        if (Check::Size (knownRoots.size (), roots.size ()))
            {
            std::sort (roots.begin (), roots.end());
            std::sort (knownRoots.begin (), knownRoots.end());
            for (size_t i = 0; i < roots.size (); i++)
                {
                Check::Near (knownRoots[i], roots[i]);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Cosh,IntersectLine)
    {
    bool print = true;
    TestCoshIntersectLine (1.0, -2.0, 2.0, print);
    TestCoshIntersectLine (1.5, -2.0, 2.0, print);
    TestCoshIntersectLine (1.5, -3.0, 2.0, print);
    TestCoshIntersectLine (1.2, 1.2, 2.0, print);
    TestCoshIntersectLine (-1.2, 1.2, 2.0, print);
    for (double x1 = 2.0; x1 < 100; x1 *= 1.5)
        TestCoshIntersectLine (1.0, -2.0, x1, print);

    }
#else
void TestCoshIntersectLine (double x0, double x1, bool print = false)
    {
    double maxReasonableRoot = 15.0;    // cosh gets too big too fast after this
    // ASSUME x0 != x1 ...
    // Evaluate two points on the cosh .. .
    double y0 = cosh (x0);
    double y1 = cosh (x1);
    double maxRoot = DoubleOps::MaxAbs (x0, x1);
    // homogeneous form of points ...
    auto h0 = DVec3d::From (x0, y0, 1.0);
    auto h1 = DVec3d::From (x1, y1, 1.0);
    // And the homogeneous form of the line is just the cross product ...
    DVec3d hLine = DVec3d::FromCrossProduct (h0, h1);
    bvector<double> roots;
    bvector<double> knownRoots {x0, x1};
    if (DCatenaryXY::CoshIntersectHomogeneousLine (hLine, roots))
        {
        // double xx[2] = {x0, x1};
        if (print)
            {
            Check::PrintIndent (2);
            Check::Print (hLine, "hLine");
            Check::PrintIndent (2);
            Check::Print (knownRoots, "KnownRoots");
            Check::Print (roots, "Roots");
            }
        // don't demand root count if request is nasty . ..  (But still check small roots)
        if (maxRoot > maxReasonableRoot || Check::Size (knownRoots.size (), roots.size ()))
            {
            std::sort (roots.begin (), roots.end());
            std::sort (knownRoots.begin (), knownRoots.end());
            for (size_t i = 0; i < roots.size (); i++)
                {
                if (fabs (knownRoots[i]) < maxReasonableRoot)
                    Check::Near (knownRoots[i], roots[i]);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Cosh,IntersectHomogeneousLine)
    {
    bool print = true;
    TestCoshIntersectLine (-2.0, 2.0, print);
    TestCoshIntersectLine (-2.0, 2.0, print);
    TestCoshIntersectLine (-3.0, 2.0, print);
    TestCoshIntersectLine (1.2, 2.0, print);
    TestCoshIntersectLine (1.2, 2.0, print);
    for (double x1 = 2.0; x1 < 100; x1 *= 1.5)
        TestCoshIntersectLine (-2.0, x1, print);

    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Catenary, TrimCatenary) 
    {
    DPoint3dDVec3dDVec3d dTri3d = DPoint3dDVec3dDVec3d(DPoint3d::From(0, 0, 0), DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0));
    //double a = 2;
    //double x1 = 8;
    auto cp0 = ICurvePrimitive::CreateCatenary(10, dTri3d, 2, 20);
    auto bspline = cp0->CloneAsBspline();
    Check::SaveTransformed(*bspline);
    Check::Shift(2, 0, 0);
    

    DCatenary3dPlacement dp;
    Check::True(cp0->TryGetCatenary(dp));
    Check::ExactDouble(2, dp.StartDistance());
    Check::ExactDouble(20, dp.EndDistance());
    auto cpReversed = ICurvePrimitive::CreateCatenary(10, dTri3d, 20, 2);
    DCatenary3dPlacement dpReversed;
    Check::True(cpReversed->TryGetCatenary(dpReversed));

    double tol = 1.0e-3;
    dp.ReverseInPlace();
    Check::True(dp.AlmostEqual(dpReversed, tol));

    
    double len, lenFraction;
    Check::True(bspline->Length(len));
    DCatenary3dPlacement fractionedCatenary = dp.CloneBetweenFractions(0.2, 0.7);
    
    double parameter;
    DSegment1d startEnd;
    DPoint3dDVec3dDVec3d identity;
    fractionedCatenary.Get(parameter, identity, startEnd);
    auto fractionCatenary = ICurvePrimitive::CreateCatenary(10, identity, startEnd.GetStart(), startEnd.GetEnd());
    
    auto bsplineFraction = fractionCatenary->CloneAsBspline();
    Check::SaveTransformed(*bsplineFraction);
    Check::True(bsplineFraction->Length(lenFraction));
   
    Check::Near(Rounding::Round(lenFraction, Rounding::RoundingMode_Up, 8.5, 9), 0.5 * Rounding::Round(len, Rounding::RoundingMode_Up, 17.5, 18));
    Check::ClearGeometry("Catenary.TrimCatenary");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Catenary, CloneBetweenFractions_Reverse) 
    {
    DPoint3dDVec3dDVec3d dTri3d = DPoint3dDVec3dDVec3d(DPoint3d::From(0, 0, 0), DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0));

    auto cp0 = ICurvePrimitive::CreateCatenary(10, dTri3d, 2, 20);
    
    //cloning
    auto cloned = cp0->CloneBetweenFractions(0.2, 0.6, false);
    auto bsplineFraction = cp0->CloneAsBspline();
    auto bsplineCloned = cloned->CloneAsBspline();
    Check::SaveTransformed(*bsplineFraction);
    Check::Shift(10, 0, 0);
    Check::SaveTransformed(*bsplineCloned);
    Check::ClearGeometry("Catenary.CloneBetweenFractions_Reverse");
    double length, lengtCloned;
    Check::True(cloned->Length(lengtCloned));
    Check::True(cp0->Length(length));
    Check::Near(0.4*length, lengtCloned);

    double lengthBefore, lengthAfter, leng;
    cp0->Length(lengthBefore);
    Check::True(cp0->ReverseCurvesInPlace());

    cp0->Length(lengthAfter);
    Check::True(lengthBefore == lengthAfter);
    cp0->CloneAsBspline();
    Check::True(1 == cp0->NumComponent());
    RotMatrix rotMat = RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians(60));
    Check::True(false == cp0->Length(&rotMat, leng));
    Check::True(0.0 == leng);
    }
