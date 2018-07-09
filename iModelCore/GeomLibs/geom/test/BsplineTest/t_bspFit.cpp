//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

void SineWave (uint32_t numPerPeriod, double numPeriod, bvector<double> &theta, bvector<DPoint3d> &xyz)
    {
    double thetaA = numPeriod * Angle::TwoPi ();
    double dTheta = thetaA / (numPerPeriod * numPeriod);
    for (double q = 0; q <= thetaA; q += dTheta)
        {
        theta.push_back (q);
        xyz.push_back (DPoint3d::From (q, sin (q)));
        }
    }
void ApplyLinearFactorToY (bvector<DPoint3d> &xyz, double f0, double f1)
    {
    double du = 1.0 / (xyz.size () - 1.0);
    for (size_t i = 0; i < xyz.size (); i++)
        {
        xyz[i].y *= DoubleOps::Interpolate (f0, i * du, f1);
        }
    }
//static int s_noisy = 0;
#ifdef TestLeastSquares_FailsInGaussPartialPivot
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,LeastSquaresToCurve)
    {
    uint32_t numPerPeriod = 30;
    int numPoles = 12;
    for (double f1 : {1.0, 3.0})
        {
        SaveAndRestoreCheckTransform shifterA (20.0, 0,0);
        for (double numPeriod : {1.0, 3.0})
            {
            SaveAndRestoreCheckTransform shifterA (0, 10.0, 0);
            bvector<DPoint3d> xyz;
            bvector<double>   theta;
            SineWave (numPerPeriod, numPeriod, theta, xyz);
            ApplyLinearFactorToY (xyz, 1.0, f1);
            Check::SaveTransformed (xyz);
            MSBsplineCurve curve;
            double averageDistance;
            double maxDistance;
            curve.Zero ();
            curve.params.numPoles = numPoles;
            curve.params.order = 4;
            if (Check::True (SUCCESS == bspcurv_leastSquaresToCurve (
                &curve, &averageDistance, &maxDistance,
                xyz.data (), theta.data (), (int)xyz.size ()), "LeastSquares SUCCESS"))
                {
                auto bcurve = curve.CreateCapture ();
                Check::SaveTransformed (bcurve);
                }
            }
        }
    Check::ClearGeometry ("MSBsplineCurve.LeastSquaresToCurve");
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspcurv,CubicFit)
    {
    uint32_t numPerPeriod = 30;
    for (double tolerance : {0.2, 0.02, 0.01})
        {
        SaveAndRestoreCheckTransform shifterA (0, 30, 0);
        for (double f1 : {1.0, 3.0})
            {
            SaveAndRestoreCheckTransform shifterB (20.0, 0,0);
            for (double numPeriod : {1.0, 3.0})
                {
                SaveAndRestoreCheckTransform shifterC (0, 10.0, 0);
                bvector<DPoint3d> xyz;
                bvector<double>   theta;
                SineWave (numPerPeriod, numPeriod, theta, xyz);
                ApplyLinearFactorToY (xyz, 1.0, f1);
                Check::SaveTransformedMarkers (xyz, -tolerance);
                Check::SaveTransformed (xyz);
                MSBsplineCurve curve;
                curve.Zero ();
                if (Check::True (SUCCESS == bspconv_cubicFitAndReducePoints (
                    &curve,
                    xyz.data (), (int)xyz.size (), tolerance), "LeastSquares SUCCESS"))
                    {
                    auto bcurve = curve.CreateCapture ();
                    Check::SaveTransformed (bcurve);
                    }
                }
            }
        }
    Check::ClearGeometry ("bspcurv.CubicFit");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspcurv,CubicFitPeriodic)
    {
    bvector<DPoint3d> poleA {
        DPoint3d::From (0,0),
        DPoint3d::From (10,0),
        DPoint3d::From (10,5),
        DPoint3d::From (8,5),
        DPoint3d::From (8,2),
        DPoint3d::From (6,2),
        DPoint3d::From (1,5)
        };
    auto curveA = MSBsplineCurve::CreateFromPolesAndOrder (poleA, nullptr, nullptr, 4, true, true);
    bvector<DPoint3d> xyzB;
    bvector<double> paramB;
    size_t numB = 60;
    curveA->StrokeFixedNumberWithEqualFractionLength (xyzB, paramB, numB);
    for (double tolerance : {0.2, 0.02, 0.01})
        {
        SaveAndRestoreCheckTransform shifterB (0.0, 10,0);
        bvector<DPoint3d> xyz;
        bvector<double>   theta;
        Check::SaveTransformedMarkers (xyzB, -tolerance);
        Check::SaveTransformed (xyzB);
        MSBsplineCurve curve;
        curve.Zero ();
        if (Check::True (SUCCESS == bspconv_cubicFitAndReducePoints (
            &curve,
            xyzB.data (), (int)xyzB.size (), tolerance), "LeastSquares SUCCESS"))
            {
            auto bcurve = curve.CreateCapture ();
            Check::SaveTransformed (bcurve);
            }
        }

    Check::ClearGeometry ("bspcurv.CubicFitPeriodic");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspcurv,Helix)
    {
    double r0 = 1.0;
    double r1 = 1.0;
    double pitch = 0.5;
    DPoint3d startPoint = DPoint3d::From (0,0,0);
    DPoint3d axis1 = DVec3d::From (5, 0, 0);
    DPoint3d axis2 = DVec3d::From (5,0,1.0);

    for (int valueIsHeight : { 0, 1})
        {
        SaveAndRestoreCheckTransform shifterB (0.0, 10,0);
        MSBsplineCurve curve;
        Check::SaveTransformed (bvector<DPoint3d> {startPoint, axis1, axis2});
        if (Check::Int (0, bspcurv_helix (&curve, r0, r1, pitch, &startPoint, &axis1, &axis2, valueIsHeight)))
            {
            auto bcurve = curve.CreateCapture ();
            Check::SaveTransformed (bcurve);
            }
        }
    Check::ClearGeometry ("bspcurv.Helix");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspcurv,blendCurves)
    {
    DPoint3d sharedPoint = DPoint3d::From (10,1,0);
    auto curve0 = MSBsplineCurve::CreateFromPolesAndOrder (
        bvector <DPoint3d> {
            DPoint3d::From (0,0,0), DPoint3d::From (2,2,0),
            DPoint3d::From (4,0,1), DPoint3d::From (6,0,1),
            DPoint3d::From (8,0,1), sharedPoint}, nullptr, nullptr,
        4, false, true);
    auto curve1 = MSBsplineCurve::CreateFromPolesAndOrder (
        bvector <DPoint3d> {
            sharedPoint, DPoint3d::From (10, 3, 0),
            DPoint3d::From (9, 5, 0),
            DPoint3d::From (9,7,0),
            DPoint3d::From (10,9,0),
            DPoint3d::From (10,11,0)}, nullptr, nullptr,
    4, false, true);
    MSBsplineCurve blendCurve;
    for (double mag : {0.20, 0.50, 1.0})
        {
        SaveAndRestoreCheckTransform shifterA (15.0, 0,0);
        for (int degree : {1,2})
            {
            SaveAndRestoreCheckTransform shifterB (0.0, 20,0);
            Check::SaveTransformed (curve0);
            Check::SaveTransformed (curve1);
            for (double u : { 0.1, 0.3, 0.5, 0.7})
                {
                if (Check::Int (0, bspcurv_blendCurve (
                    &blendCurve,
                    curve0.get (),
                    curve1.get (),
                    u, 1.0 - u,
                    degree,
                    mag, mag)))
                    {
                    auto bcurve = blendCurve.CreateCapture ();
                    Check::SaveTransformed (bcurve);
                    }
                }
            }
        }
    Check::ClearGeometry ("bspcurv.blendCurves");
    }