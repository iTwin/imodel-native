//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
static int s_noisy = 0;

void freeCurves (MSBsplineCurveP curves, int n)
    {
    for (int i = 0; i < n; i++)
        curves[i].ReleaseMem ();
    BSIBaseGeom::Free (curves);
    }
    
void freeSurfaces (MSBsplineSurfaceP surfaces, int n)
    {
    for (int i = 0; i < n; i++)
        surfaces[i].ReleaseMem ();
    BSIBaseGeom::Free (surfaces);
    }


TEST(BsplineCurve,SegmentDisjoint)
    {
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (1,1,1));
    poles.push_back (DPoint3d::From (2,2,1));
    poles.push_back (DPoint3d::From (1,1,0));
    poles.push_back (DPoint3d::From (2,2,0));
    bvector<double> knot0;
    knot0.push_back (0);
    knot0.push_back (0);
    knot0.push_back (1);
    knot0.push_back (2);
    knot0.push_back (4);
    knot0.push_back (4);
    MSBsplineCurvePtr curve0 = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, &knot0, 2, false, true);
    MSBsplineCurveP split1, split2;
    int num1, num2;
    bspcurv_segmentDisjointCurve (&split1, &num1, curve0.get ());
    curve0->knots[3] = curve0->knots[2];   // force a discontinuity !!!!
    bspcurv_segmentDisjointCurve (&split2, &num2, curve0.get ());
    Check::Int (1, num1, "noop split");
    Check::Int (2, num2, "real split");
    freeCurves (split1, num1);
    freeCurves (split2, num2);    
    }
    
    
TEST(BsplineSurface,SegmentDisjoint)
    {
    bvector<DPoint3d> poles;
    int numX = 8;
    int numY = 10;
    for (double y = 0.0; y < numY; y++)
        {
        for (double x = 0.0; x < numX; x++)
            {
            poles.push_back (DPoint3d::From (x, y, 0));
            }
        }
    MSBsplineSurfacePtr surface0 = MSBsplineSurface::CreateFromPolesAndOrder (poles, NULL,
                            NULL, 3, numX, false,
                            NULL, 4, numY, false,
                            true);
    MSBsplineSurfaceP split1, split2, split3;
    int num1, num2, num3;
    bspsurf_segmentDisjointSurface (&split1, &num1, surface0.get ());
    Check::Int (1, num1, "noop split");
    surface0->uKnots[4] = surface0->uKnots[5];   // not a discontinuity yet !!
    bspsurf_segmentDisjointSurface (&split2, &num2, surface0.get ());
    Check::Int (1, num2, "noop split");

    surface0->uKnots[6] = surface0->uKnots[5];   // now its discontinuous
    bspsurf_segmentDisjointSurface (&split3, &num3, surface0.get ());
    Check::Int (2, num3, "u split");

    freeSurfaces (split1, num1);
    freeSurfaces (split2, num2);
    freeSurfaces (split3, num3);
    }    
    
TEST(BsplineSurface,ImposeBoundary)
    {    
    bvector<DPoint3d> surfacePoints;
    int numX = 11;
    int numY = 11;
    double ax = 2.0;
    double ay = 2.0;
    for (double y = 0.0; y < numY; y++)
        {
        for (double x = 0.0; x < numX; x++)
            {
            surfacePoints.push_back (DPoint3d::From (ax * x, ay * y, 0));
            }
        }
    MSBsplineSurfacePtr surface0 = MSBsplineSurface::CreateFromPolesAndOrder (surfacePoints, NULL,
                            NULL, 2, numX, false,
                            NULL, 2, numY, false,
                            true);

    bvector<DPoint3d> boxPoles;
    boxPoles.push_back (DPoint3d::From (1,1,0));
    boxPoles.push_back (DPoint3d::From (1,2,0));
    boxPoles.push_back (DPoint3d::From (2,2,0));
    boxPoles.push_back (DPoint3d::From (2,1,0));
    MSBsplineCurvePtr curve0 = MSBsplineCurve::CreateFromPolesAndOrder (boxPoles, NULL, NULL, 2, true, false);
    double edgeLength = 0.5;
    DPoint3d *imposedPoints;
    int     numImposedPoints;
    Check::True (SUCCESS == bspsurf_imposeBoundaryBySweptCurve (surface0.get (), curve0.get (), edgeLength, NULL, &imposedPoints, &numImposedPoints),
                "impose boundary completes");
    Check::Int (1, surface0->numBounds, "1 boundary created");

    bsputil_free (imposedPoints);    
    }

void PrintMinMaxRatio(int order, int numPoles, UsageSums &sums, char const *name)
    {
    Check::PrintHeading (name);
    Check::Print (order, "order");
    Check::Print (numPoles, "numPoles");
    //Check::Print(sums.Min(), "Min");
    //Check::Print(sums.Max(), "Max");
    Check::Print(sums.Min() / sums.Max(), "ratio");
    }

UsageSums ScanTangentMagnitudes (MSBsplineCurveCR curve, size_t numSample)
    {
    UsageSums tangentMagnitude;
    for (size_t i = 0; i <= numSample; i++)
        {
        double f = i / (double)numSample;
        DPoint3d xyz;
        DVec3d tangent;
        curve.FractionToPoint(xyz, tangent, f);
        tangentMagnitude.Accumulate(tangent.Magnitude());
        }
    PrintMinMaxRatio (curve.GetIntOrder (), curve.GetIntNumPoles (), tangentMagnitude, "tangent magnitude");
    return tangentMagnitude;
    }

UsageSums ScanTangentMagnitudes(ICurvePrimitiveCR cp, MSBsplineCurveCR curve, size_t numSample)
    {
    UsageSums tangentMagnitude;
    UsageSums tangentialError;
    UsageSums perpendicularError;
    for (size_t i = 0; i <= numSample; i++)
        {
        double f = i / (double)numSample;
        DPoint3d xyz, xyz1;
        DVec3d tangent, tangent1;
        CurveLocationDetail detail2;
        curve.FractionToPoint(xyz, tangent, f);
        cp.FractionToPoint(f, xyz1, tangent1);
        cp.ClosestPointBounded (xyz, detail2);
        tangentMagnitude.Accumulate(tangent.Magnitude());
        tangentialError.Accumulate (xyz.Distance (xyz1));
        perpendicularError.Accumulate (xyz.Distance (detail2.point));
        }
    PrintMinMaxRatio(curve.GetIntOrder(), curve.GetIntNumPoles(), tangentMagnitude, "tangent magnitude");
    Check::Print (tangentialError.Max (), "Et");
    Check::Print(perpendicularError.Max (), "Ep");
    return tangentMagnitude;
    }

TEST(BsplineCurve, GrevilleLine)
    {
    size_t numSample = 100;
    for (int order = 3; order < 7; order++)
        {
        if (s_noisy)
            {
            Check::PrintIndent (0);
            Check::PrintIndent(0);
            Check::Print (order, "GREVILLE POLE PLACMENT ORDER");
            }
        for (int numPoles = 4; numPoles < 12; numPoles++)
            {
            // Generate uniformly spaced poles:
            bvector<DPoint3d> poles;
            int numSegment = numPoles - 1;
            for (int i = 0; i < numPoles; i++)
                {
                double f = i / (double)numSegment;
                poles.push_back(DPoint3d::From(f, 0,0));
                }
            auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, 4, false, false);
            auto uniformMagnitude = ScanTangentMagnitudes (*bcurve, numSample);
            if (s_noisy)
                {
                Check::PrintIndent (2);
                Check::Print (numPoles, "NumPoles");
                PrintMinMaxRatio (4, numPoles, uniformMagnitude, "Uniform Poles tangent magnitude");
                }
            bvector<double>grevilleKnots;
            bcurve->ComputeGrevilleAbscissa (grevilleKnots);
            bvector<DPoint3d> grevillePoles;
            for (auto x : grevilleKnots)
                grevillePoles.push_back (DPoint3d::From (x,0,0));
            auto grevilleCurve = MSBsplineCurve::CreateFromPolesAndOrder(grevillePoles, nullptr, nullptr, 4, false, false);
            auto grevilleMagnitude = ScanTangentMagnitudes(*grevilleCurve, numSample);
            if (s_noisy)
                {
                PrintMinMaxRatio(4, numPoles, grevilleMagnitude, "Greville Poles tangent magnitude");
                }
            Check::LessThanOrEqual (grevilleMagnitude.Min (), grevilleMagnitude.Max (), "Greville Interpolation has near constant Tangent Magnitude");

            // Confirm greville knots are interpolated at the knots.
            KnotData knots;
            knots.LoadCurveKnots(*grevilleCurve);
            double a;
            size_t multiplicity;
            for (size_t i = 0; knots.GetKnotByActiveIndex (i, a, multiplicity); i++)
                {
                DPoint3d xyz;
                double f = grevilleCurve->KnotToFraction(a);
                grevilleCurve->FractionToPoint (xyz, f);
                Check::Near (f, xyz.x);
                }
            }
        }
    }



TEST(BsplineCurve,GrevilleMovingAverages)
    {
    bvector<double>gridDistances;
    size_t numPole = 8;
    // Trivial counts make uniform grid points ...
    DoubleOps::MovingAverages (gridDistances, 1.0, (double)numPole, numPole, 1, 1, 1);
    for (size_t i = 1; i <=numPole; i++)
        Check::Near ((double)i, gridDistances[i-1]);

    bvector<double>all3, average3A, average3B;
    // all3 has 3 copies at end ...
    double aMax = (double)numPole - 4;;
    // simulate a typical cubic knot vector -- 4 leading and trailing clamps, but only 3 are used in averages
    DoubleOps::MovingAverages(all3, 1.0, aMax, numPole, 1, 4, 4);
    // summing through the array and implicitly should produce the same results ..
    DoubleOps::MovingAverages (average3A, all3, 3, 1, 1);
    DoubleOps::MovingAverages(average3B, all3.front (), all3.back (), average3A.size (), 3, 3, 3);
    Check::Print (all3, "Raw knots");
    Check::Print (average3A, "direct averages");
    Check::Print(average3B, "implicit averages");
    UsageSums diffSum;
    diffSum.AccumulateAbsBMinusA (average3A, average3B);
    Check::Near (0.0, diffSum.Max (), "Same moving averages from explicit and implicit");
    }


void TestGrevilleFit (bvector<IGeometryPtr> &geometry, uint32_t minPoleMultiplier, uint32_t maxPoleMultiplier, int knotSelector)
    {
    Check::PrintIndent(0);
    Check::Print(knotSelector, " ******* KNOT SELECTOR ********");
    for (auto g : geometry)
        {
        auto cp = g->GetAsICurvePrimitive();
        if (cp.IsValid())
            {
            Check::Print(*cp);
            for (int order = 4; order < 7; order++)
                {
                Check::PrintIndent(0);
                Check::Print(order, "*** ORDER ****");
                for (size_t numPoles = order * minPoleMultiplier; numPoles < order * maxPoleMultiplier; numPoles += (numPoles - 1))
                    {
                    auto bcurve = MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots(*cp, numPoles, order, false, knotSelector);
                    if (bcurve.IsValid())
                        {
                        auto cp1 = ICurvePrimitive::CreateBsplineCurve(bcurve);
                        ScanTangentMagnitudes(*cp, *bcurve, 100);
                        if (s_noisy)
                            {
                            Check::Print(*cp1);
                            }
                        }
                    }
                }
            }
        }
    }


TEST(BsplineCurve,GrevilleLineFit)
    {
    auto cp0 = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 10,0,0));
    for (int knotSelector : bvector<int>{0,1})
        {
        Check::PrintIndent (0);
        Check::Print (knotSelector, " ******* KNOT SELECTOR ********");
        auto bcurve = MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots(*cp0, 8, 4, false, knotSelector);
        if (bcurve.IsValid ())
            {
            auto cp =ICurvePrimitive::CreateBsplineCurve (bcurve);
            Check::Print (*cp);
            ScanTangentMagnitudes(*bcurve, 100);
            }
        }
    }

TEST(BsplineCurve, GrevilleSpiralFit)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSpirals (geometry);
    TestGrevilleFit (geometry, 1, 19, 0);
    //TestGrevilleFit(geometry, 1, 12, 1);
    }
#include <GeomSerialization/GeomSerializationApi.h>

TEST(BsplineCurve, GrevilleArcs)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddArcs(geometry);
    TestGrevilleFit(geometry, 3, 8, 0);
    //TestGrevilleFit(geometry, 3, 8, 1);
    }



TEST(bsiBezierDPoint4d,saturateKnotsInInterval)
    {
    DPoint4d points[4] =
        {
        DPoint4d::From (1,11,21,31),
        DPoint4d::From (2,12,22,32),
        DPoint4d::From (3,13,23,33),
        DPoint4d::From (4,14,24,34)
        };

    double   knots[6] = // bspBezierDPoint4d does NOT want the extra clamp knots
        {
        0,1,2,
        3,4,5,
        };

    bool isNull;
    bsiBezierDPoint4d_saturateKnotsInInterval (points, knots, 4, isNull);
    }

void SaveRectangle (DPoint3dCP xyz, int i0, int i1, int i2, int i3)
    {
    Check::SaveTransformed (bvector<DPoint3d>{xyz[i0], xyz[i1], xyz[i2], xyz[i3], xyz[i0]});
    }

void SaveBox (TransformCR transform, DRange3dCR localRange)
    {
    DPoint3d corners[8];
    localRange.Get8Corners (corners);
    transform.Multiply (corners, corners, 8);
    SaveRectangle (corners, 0,1,3,2);
    SaveRectangle (corners, 4,5,7,6);
    SaveRectangle (corners, 0,2,6,4);
    SaveRectangle (corners, 1,5,7,3);
    }
bool IsFinalPointOfStringer (DPoint3dDoubleUVArrays const &data, size_t i)
    {
    if (i + 1 >=data.m_f.size ())
        return true;
    if (data.m_f[i] != data.m_f[i + 1])   // patch break 
        return true;
    if (data.m_uv[i].x > data.m_uv[i + 1].x)   // u jumps back down to next stringer.
        return true;
    return false;
    }

void SavePointsFromPatches (DPoint3dDoubleUVArrays const &data)
    {
    bvector <DPoint3d> points;
    for (size_t i = 0; i < data.m_xyz.size (); i++)
        {
        points.push_back (data.m_xyz[i]);
        if (IsFinalPointOfStringer (data, i))
            {
            Check::SaveTransformed (points);
            points.clear ();
            }
        }
    }

void testTightSurfaceExtents (MSBsplineSurfacePtr &surface)
    {
    DRange3d globalRange;
    surface->GetPoleRange (globalRange);
    double dX = globalRange.XLength () * 1.2;
    double dY = globalRange.YLength () * 1.2;
    SaveAndRestoreCheckTransform shifter (dX, 0, 0);
    Transform cornerBox, localToWorld, worldToLocal;
    DRange3d localRange;
    DRange3d unitRange;
    unitRange.low = DPoint3d::From (0,0,0);
    unitRange.high = DPoint3d::From (1,1,1);
    surface->TightPrincipalExtents (cornerBox, localToWorld, worldToLocal, localRange);
    DPoint3dDoubleUVArrays cloud;
    surface->EvaluatePoints (cloud);
    Transform cornerBoxA;
    Check::SaveTransformed (surface);
    if (surface->GetPrincipalExtents (cornerBoxA))
        SaveBox (cornerBoxA, unitRange);

    Check::Shift (0, dY ,0);
    SavePointsFromPatches (cloud);
    SaveBox (cornerBox, unitRange);


    Check::Shift (0, dY ,0);
    SavePointsFromPatches (cloud);
    SaveBox (localToWorld, localRange);
    }

TEST(MSBsplineSurface,TightPrincipalExtents)
    {
    double q0I = -0.3;
    double q0J = 0.5;
    int order = 4;
    int numX = 11;
    int numY = 12;
    auto surface = SurfaceWithSinusoidalControlPolygon (order, order, numX, numY, q0I, 0.522, q0J, 0.98);
    testTightSurfaceExtents (surface);
    Transform rotate = YawPitchRollAngles::FromDegrees (12.0, 40.0, -20.0).ToTransform (DPoint3d::From (3,4,0));
    surface->TransformSurface (rotate);
    testTightSurfaceExtents (surface);
    Check::ClearGeometry ("MSBsplineSurface.TightPrincipalExtents");
    }

TEST(MSBsplineSurface,TightPrincipalExtentsWeighted)
    {
    auto surface = SurfaceBubbleWithMidsideWeights ();
    testTightSurfaceExtents (surface);
    Transform rotate = YawPitchRollAngles::FromDegrees (12.0, 40.0, -20.0).ToTransform (DPoint3d::From (1,1,0));
    surface->TransformSurface (rotate);
    testTightSurfaceExtents (surface);
    Check::ClearGeometry ("MSBsplineSurface.TightPrincipalExtentsWeighted");
    }