/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
* @bsimethod
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
            SaveAndRestoreCheckTransform shifterB (0, 10.0, 0);
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP void    bspsurf_computeZeroSurfNorm
(
DPoint3d            *normP,
const MSBsplineSurface    *surfP,
double              param,
int                 direction
);
GEOMDLLIMPEXP void    bspsurf_computeZeroSurfNormA
(
DPoint3d            *normP,
const MSBsplineSurface    *surfP,
double              param,
int                 direction
);
END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspsurf,computeZeroSurfNorm)
    {
    // A doubly curved surface ...
    auto surfaceA = HyperbolicGridSurface (3,4, 6,8,
            1.1, 1.3, 0.8,
            1,1);
    auto surfaceB = HyperbolicGridSurface (2,5, 3, 5,
            1.2, 0.8, -0.9,
            0.9, 0.95);
    for (auto &surface : {surfaceA, surfaceB})
        {
        DRange3d poleRange;
        surface->GetPoleRange(poleRange);
        SaveAndRestoreCheckTransform shifter(3.0 * poleRange.XLength (), 0, 0);
        double dy = 2.5 * ceil (poleRange.YLength () + 0.25);
        for (int direction : {0,1})
            {
            for (int edge : {0,1})
                {
                DVec3d perp;
                double param = (double)edge;
                // this obscure function is called by _computePartials when there is an apparent zero normal
                // It sums some discrete cross products as an approximate normal.
                bspsurf_computeZeroSurfNorm (&perp, surface.get (), param, direction);
                double u, v;
                if (direction == 0)
                    {
                    u = 0.5;
                    v = param;
                    }
                else
                    {
                    u = param;
                    v = 0.5;
                    }
                DPoint3d point;
                double weight;
                DVec3d dPdU, dPdV, dPdUU, dPdVV, dPdUV, normal;
                bspsurf_computePartials (&point, &weight,
                        &dPdU, &dPdV, &dPdUU, &dPdVV, &dPdUV, &normal, u, v, surface.get ());
                Check::SaveTransformed (DSegment3d::From (point, DPoint3d::FromSumOf (point, normal)));
                bspsurf_computePartials (&point, &weight,
                        &dPdU, &dPdV, &dPdUU, &dPdVV, &dPdUV, &normal, u, v, surface.get ());
                // experimentally, we observe that the ZeroSurfNorm vectors are too short to see ... scale (and negate for visual separation)
                perp.Scale (-10.0);
                Check::SaveTransformed (DSegment3d::From (point, DPoint3d::FromSumOf (point, perp)));
    //            bspsurf_computeZeroSurfNormA (&perpA, surface.get (), param, direction);
    //            Check::Near (perp, perpA);
                }
            }
        Check::SaveTransformed (surface);
        for (double angleTolerance : {0.8, 0.05})
            {
            Check::Shift (0, dy);
            IFacetOptionsPtr options = IFacetOptions::Create();
            options->SetAngleTolerance(angleTolerance);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
            builder->Add(*surface);
            Check::SaveTransformed(builder->GetClientMeshR());

            Check::Shift(0, dy);
            options->SetAngleTolerance(0.0);
            options->SetMaxEdgeLength(angleTolerance * 0.5 * poleRange.XLength ());
            IPolyfaceConstructionPtr builder1 = IPolyfaceConstruction::Create(*options);
            builder1->Add(*surface);
            Check::SaveTransformed(builder1->GetClientMeshR());


            }
        }
    Check::ClearGeometry ("bspsurf.computeZeroSurfNorm");
    
    }

enum class SurfaceAction {
    None         = 0x00,
    MakeRational = 0x01,
    ReverseU     = 0x02,
    ReverseV     = 0x04,
    SwapUV       = 0x08
    };
void ApplySurfaceAction (MSBsplineSurfacePtr &surface, SurfaceAction action)
    {
    if ((int)action & (int)SurfaceAction::MakeRational)
        surface->MakeRational ();
    if ((int)action & (int)SurfaceAction::ReverseU)
        surface->MakeReversed (0);
    if ((int)action & (int)SurfaceAction::ReverseV)
        surface->MakeReversed (1);
    if ((int)action & (int)SurfaceAction::SwapUV)
        surface->SwapUV ();
    }
template <typename T>
T BitWiseAnd (T valueA, T valueB)
    {
    return (T)((uint32_t)valueA & (uint32_t)valueB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(bspsurf,computePartials)
    {
    bvector<double> samples {0.0, 0.01, 0.25, 0.5, 0.75, 0.99, 1.0};
    double frameScale = 0.1;
    // Action bits:
    // 0x01 = make rational
    // 0x02 = 
    for (SurfaceAction action : {
                SurfaceAction::None,
                SurfaceAction::MakeRational,
                SurfaceAction::ReverseU,
                SurfaceAction::ReverseV,
                SurfaceAction::SwapUV,
                BitWiseAnd (SurfaceAction::MakeRational, SurfaceAction::ReverseU),
                BitWiseAnd (SurfaceAction::MakeRational, SurfaceAction::ReverseV),
                BitWiseAnd (SurfaceAction::MakeRational, SurfaceAction::SwapUV)
                })
        {
        SaveAndRestoreCheckTransform shifterA (0, 4, 0);
        for (auto corner11 : {
            DPoint3d::From (1.1, 1.3, 0.8),     // a "good" surface
            DPoint3d::From (1, 0, 0),           // pinch all u=1
            DPoint3d::From (0, 1, 0),           // pinch all v=1
            })
            {
            SaveAndRestoreCheckTransform shifterB (2.0, 0,0);
            auto surface = HyperbolicGridSurface (3,4, 6,8,
                    corner11.x, corner11.y, corner11.z,
                    1,1);
            ApplySurfaceAction (surface, action);
            for (double u : samples)
                {
                for (double v : samples)
                    {
                    DPoint3d point;
                    double weight;
                    DVec3d dPdU, dPdV, dPdUU, dPdVV, dPdUV, normal;
                    bspsurf_computePartials (&point, &weight,
                            &dPdU, &dPdV, &dPdUU, &dPdVV, &dPdUV, &normal, u, v, surface.get ());
                    Check::SaveTransformed (DSegment3d::From (point, DPoint3d::FromSumOf (point, dPdU, frameScale)));
                    Check::SaveTransformed (DSegment3d::From (point, DPoint3d::FromSumOf (point, dPdV, frameScale)));
                    Check::SaveTransformed (DSegment3d::From (point, DPoint3d::FromSumOf (point, normal, frameScale)));
                    }
                }
            Check::SaveTransformed (surface);
            }
        }
Check::ClearGeometry("bspsurf.computePartials");

    }
    // Compute vector from center to point.  Scale it to radius.  Add it to the point.
    // (Intended to add to center, to pin point on sphere.  But this produces a more varying surface, which is the intent)
    void ProjectFromSphereCenter(DPoint3dP points, int n, DPoint3dCR center, double radius)
        {
        for (int i = 0; i < n; i++)
            {
            auto vector = points[i] - center;
            vector.ScaleToLength(radius);
            points[i] = points[i] + vector;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(bspsurf, closestPointOnSurface)
        {
        DPoint3d center = DPoint3d::From(0.5, 0.5, -1.0);
        double radius = 2.0;
        bvector<double> samples{ 0.01, 0.25, 0.5, 0.75, 0.99 };  // avoid edge cases !!!
        double offsetDistance = 0.05;

        for (SurfaceAction action : {
            SurfaceAction::None,
                SurfaceAction::MakeRational
            })
            {
            SaveAndRestoreCheckTransform shifterA(0, 10, 0);
            for (auto corner11 : {
                DPoint3d::From(1.1, 1.3, 0.8),     // a "good" surface
                DPoint3d::From(1, 0, 0),           // pinch all u=1
                DPoint3d::From(0, 1, 0),           // pinch all v=1
                })
                {
                SaveAndRestoreCheckTransform shifterB (10.0, 0, 0);
                auto surface = HyperbolicGridSurface(3, 4, 6, 8,
                    corner11.x, corner11.y, corner11.z,
                    1, 1);
                ProjectFromSphereCenter(surface->poles, surface->GetIntNumPoles(), center, radius);
                ApplySurfaceAction(surface, action);

                for (double u : samples)
                    {
                    for (double v : samples)
                        {
                        DPoint3d point;
                        double weight;
                        DVec3d dPdU, dPdV, normal;
                        bspsurf_evaluateSurfacePoint(&point, &weight, &dPdU, &dPdV, u, v, surface.get());
                        normal.GeometricMeanCrossProduct(dPdU, dPdV);
                        Check::SaveTransformedMarker(point, 0.02);
                        DVec3d unitNormal = normal;
                        unitNormal.Normalize();
                        DPoint3d pointA = point + unitNormal * offsetDistance;
                        DPoint3d pointB;
                        DPoint2d uvB;
                        surface->ClosestPoint(pointB, uvB, pointA);
                        Check::SaveTransformedMarker(pointA, 0.04);
                        Check::SaveTransformedMarker(pointB, 0.02);
                        Check::Near(pointB, point);
                        }
                    }
                Check::SaveTransformed(surface);
                }
            }
            Check::ClearGeometry("bspsurf.closestPointOnSurface");

        }

struct IFCParabolaData
{
double xStart;
double yStart;
double gStart;
double radiusAtMinMax;
double xMinMax;
double xEnd;
double yEnd;
double gEnd;
double EvaluateIFCFormula(double x) const
    {
    double g1 = gStart + (x - xStart) / radiusAtMinMax;
    return  yStart + (x - xStart) * (gStart + g1) * 0.5;
    }
};

/**
    * @param (x0,y0) = first pole of bezier (actual start point)
    * @param (y1) = bezier y at "middle" pole of bezier. The implicit x1 is (x0+x2)/2;
    * @param (x2,y2) = last pole of bezier (actual end point)
*/
bool IFCParabolaDataFromQuadraticBezierPoles(double x0, double y0, double y1, double x2, double y2, IFCParabolaData &data)
    {
    double x1 = 0.5 * (x0 + x2);
    double dx01 = x1 - x0;
    double dx12 = x2 - x1;
    double dx02 = x2 - x0;
    double gStart, gEnd, c, radiusAtMinMax, fractionToMin;
    if (DoubleOps::SafeDivide(gStart, (y1 - y0), dx01, 0.0)
        && DoubleOps::SafeDivide(gEnd, (y2 - y1), dx12, 0.0)
        && DoubleOps::SafeDivide(c, gEnd - gStart, dx02, 0.0)
        && DoubleOps::SafeDivide(radiusAtMinMax, 1.0, c, 0.0)
        && DoubleOps::SafeDivide(fractionToMin, -gStart, gEnd - gStart, 0.0)
        )
        {
        data.xStart = x0;
        data.yStart = y0;
        data.gStart = gStart;
        data.xEnd = x2;
        data.yEnd = y2;
        data.gEnd = gEnd;
        data.radiusAtMinMax = radiusAtMinMax;
        data.xMinMax = DoubleOps::Interpolate(x0, fractionToMin, x2);
        return true;
        }
    return false;
    }

TEST(IFC, Parabola)
    {
    IFCParabolaData data;
    double x0 = 2.0;
    double y0 = 1.0;
    double x2 = 9.0;
    double y2 = 4.0;
    double x1 = (x0 + x2) / 2.0;
    double  y1 = -0.5;
    auto point0 = DPoint3d::From(x0, y0, 0.0);
    auto point1 = DPoint3d::From(x1, y1, 0.0);
    auto point2 = DPoint3d::From(x2, y2, 0.0);
    Check::SaveTransformedMarker(point0, -0.1);
    Check::SaveTransformedMarker(point1, -0.1);
    Check::SaveTransformedMarker(point2, -0.1);
    Check::SaveTransformed (std::vector<DPoint3d>({point0, point1, point2}));
    if (Check::True(IFCParabolaDataFromQuadraticBezierPoles(x0, y0, -0.5, x2, y2, data)))
        {
        bvector<DPoint3d> xyz;
        for (double f = 0.0; f <= 1.0; f += 0.0625)
            {
            double x = DoubleOps::Interpolate(x0, f, x2);
            double y = data.EvaluateIFCFormula(x);
            xyz.push_back(DPoint3d::From(x, y, 0.0));
            }
        Check::SaveTransformed(xyz);
        auto yMinMax = data.EvaluateIFCFormula (data.xMinMax);
        Check::SaveTransformedMarker (DPoint3d::From (data.xMinMax, yMinMax), 0.2);
        }
    Check::ClearGeometry("IFC.Parabola");
    }