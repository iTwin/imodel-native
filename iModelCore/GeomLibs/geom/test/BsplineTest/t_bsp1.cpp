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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

#ifdef CompileBoresite
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
#endif
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, GrevilleSpiralFit)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSpirals (geometry);
    TestGrevilleFit (geometry, 1, 19, 0);
    //TestGrevilleFit(geometry, 1, 12, 1);
    }

#include <GeomSerialization/GeomSerializationApi.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, GrevilleArcs)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddArcs(geometry);
    TestGrevilleFit(geometry, 3, 8, 0);
    //TestGrevilleFit(geometry, 3, 8, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface,TightPrincipalExtentsWeighted)
    {
    auto surface = SurfaceBubbleWithMidsideWeights ();
    testTightSurfaceExtents (surface);
    Transform rotate = YawPitchRollAngles::FromDegrees (12.0, 40.0, -20.0).ToTransform (DPoint3d::From (1,1,0));
    surface->TransformSurface (rotate);
    testTightSurfaceExtents (surface);
    Check::ClearGeometry ("MSBsplineSurface.TightPrincipalExtentsWeighted");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSInterpolatingCurve,EndConditions)
    {
    bvector<bool> truefalse {true, false};
    for (auto &q : bvector <bvector<double>>
                {
                // greville-like spacing
                bvector<double>{0, 1, 3, 6, 9, 11, 12},
                // uniform spacing
                bvector<double>{0, 2, 4, 6, 8, 10, 12}
                }
            )
        {
        SaveAndRestoreCheckTransform shifter (0, 20, 0);
        bvector<DPoint3d> points;
        for (auto x : q)
            {
            double theta = Angle::Pi () * (x - q[0]) / (q.back () - q.front ());
            points.push_back (DPoint3d::From (cos (theta), sin(theta), 0.0));
            }
        for (bool closedCurve : truefalse)
            {
            for (bool colinearTangents : truefalse)
                {
                for (bool chordLengthTangents : truefalse)
                    {
                    for (bool naturalTangents : truefalse)
                        {
                        SaveAndRestoreCheckTransform shifter (10, 0, 0);
                        MSInterpolationCurve curve;
                        auto s = curve.InitFromPointsAndEndTangents (points, false, 0.0, nullptr,
                            closedCurve, colinearTangents, chordLengthTangents, naturalTangents);
                        if (s == SUCCESS)
                            {
                            auto cp = ICurvePrimitive::CreateInterpolationCurveSwapFromSource (curve);
                            auto cp1 = ICurvePrimitive::CreateBsplineCurve (cp->GetProxyBsplineCurvePtr ());
                            Check::SaveTransformed (*cp1);
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("MSInterpolatingCurve.EndConditions");
    }
void DrawLine (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    Check::SaveTransformed (bvector<DPoint3d>{xyz0, xyz1});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,OffsetConstruction0)
    {
    double r0 = 100.0;
    double length0 = 50.0;
    double sweep0 = Angle::DegreesToRadians (20.0);
    Check::SaveTransformed (bvector<DPoint3d>{DPoint3d::From (-30.0, 0,0), DPoint3d::From (30,-0,0)});
    auto spiral0 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (10,
                            Angle::PiOver2 (), 0.0, length0, r0,
                            Transform::FromIdentity (), 0.0, 1.0);
    Check::SaveTransformed (*spiral0);
    auto endRay0 = spiral0->FractionToPointAndUnitTangent (1.0).Value ();
    auto startRay0 = spiral0->FractionToPointAndUnitTangent (0.0).Value ();
    DEllipse3d arc0 = DEllipse3d::FromStartTangentNormalRadiusSweep (endRay0.origin, endRay0.direction, DVec3d::UnitZ (), r0, sweep0);
    DPoint3d midPoint0;
    DVec3d midTangent0, d2;
    arc0.FractionParameterToDerivatives (midPoint0, midTangent0, d2, 0.5);
    double halfTurn = DVec3d::UnitY ().AngleTo (midTangent0);
    DVec3d outboundTangent = DVec3d::FromRotateVectorAroundVector (midTangent0, DVec3d::UnitZ (), Angle::FromRadians (halfTurn));

    Check::SaveTransformed (bvector<DPoint3d> {arc0.center, DPoint3d::FromInterpolate (arc0.center, 3.0, endRay0.origin)});
    auto midray0 = DRay3d::FromOriginAndTarget (arc0.center, midPoint0);
    Check::SaveTransformed (bvector<DPoint3d> {arc0.center, DPoint3d::FromInterpolate (arc0.center, 3.0, midPoint0)});

    auto refray = DRay3d::FromOriginAndVector (startRay0.origin, DVec3d::UnitY());
    DPoint3d piA, piB;
    double fiA, fiB;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay (fiA, fiB, piA, piB, midray0, refray);
    DrawLine (startRay0.origin, piB);       
    
    Check::SaveTransformed (arc0);
    bvector<DPoint3d> refMarks;
//    for (double radiusFactor : bvector<double> {0.8, 0.85, 0.9, 0.95, 1.05, 1.10, 1.15, 1.20})
    for (double radiusFactor : bvector<double> {0.8, 1.20})
        {
        double r1 = radiusFactor * r0;
        double offset = r1 - r0;
        DEllipse3d arc1 = DEllipse3d::FromVectors
            (
            arc0.center,
            arc0.vector0 * radiusFactor,
            arc0.vector90 * radiusFactor,
            0.0, sweep0
            );
        Check::SaveTransformed (arc1);
        DPoint3d refStart = DPoint3d::From (offset, 0,0);

        Transform frame1 = Transform::From (refStart);
        refMarks.push_back (refStart);

        ICurvePrimitivePtr spiral2 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (10,
                                Angle::PiOver2 (), 0.0, length0 * radiusFactor, r1,
                                frame1, 0.0, 1.0);

        auto endRay2 = spiral2->FractionToPointAndUnitTangent (1.0).Value ();
        //DPoint3d center2 = endRay2.origin + r1 * DVec3d::FromUnitPerpendicularXY (endRay2.direction);
        DEllipse3d arc2 = DEllipse3d::FromStartTangentNormalRadiusSweep (endRay2.origin, endRay2.direction, DVec3d::UnitZ (), r1, sweep0);

        // translate the whole spiral+arc so the arc midpoint is on midray0
        DPoint3d midpoint2;
        arc2.FractionParameterToPoint (midpoint2, 0.5);

        auto midPointShiftRay = DRay3d::FromOriginAndVector (midpoint2, DVec3d::UnitY());
        DPoint3d piA, piB;
        double fiA, fiB;
        DRay3d::ClosestApproachUnboundedRayUnboundedRay (fiA, fiB, piA, piB, midray0, midPointShiftRay);

        DVec3d shiftVector = piB - midpoint2;
        auto shiftTransform = Transform::From (shiftVector);
        spiral2->TransformInPlace (shiftTransform);
        DPoint3d start2;
        spiral2->FractionToPoint (0.0, start2);
        DrawLine (start2, start2 + 2.0 * DVec3d::UnitX ());
        shiftTransform.Multiply (arc2);
        Check::SaveTransformed (*spiral2);    
        Check::SaveTransformed (arc2);

        DRay3d startRay2 = DRay3d::FromOriginAndVector (start2, DVec3d::UnitY ());
        DRay3d::ClosestApproachUnboundedRayUnboundedRay (fiA, fiB, piA, piB, midray0, startRay2);
        DrawLine (refStart, piB);
        DrawLine (piB, piB + 100 * outboundTangent);
        refMarks.push_back (piB);
        }
    Check::SaveTransformedMarkers (refMarks, 0.4);
    Check::ClearGeometry ("Spiral.OffsetConstruction0");
    }
// curvature along length . . .
//       o------------------o k
//      /|                  |
//     / |                  |
//    /  |                  |
//   +-a-+----b-------------+
// a = spiral length
// b = arc length to middle of arc portion.
// total turn = a * k/2 + b * k
// consider both a and b variable.  this makes the angle at transition vary.
// (But total turn is fixed.   So db = -da/2)
// As a and b vary, the arc midpoint moves both along the mid ray and away from it.
// For newton condition ... let the end float along, but not perpendicular.
// f0 = a*k/2 + b*k - totalTurn
// f1 = distance from midray = (Xend - rayOrigin)
//
void ConstructTransition
(
bvector<DPoint3d> xyz,
double arcRadius,
double spiralLength,
DPoint3dR   xyzLineTangent,
DEllipse3dR arc
)
    {
    DSpiral2dBaseP spiralA = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DPoint3d xyzA, xyzB, xyzC, xyzD;
    DPoint3d xyz0, xyz1;
    DVec3d uv0, uv1;
    double ticLength = 2.0;
    DVec3d crossVec = DVec3d::UnitZ () * ticLength;
    if (Check::True (
        DSpiral2dBase::LineSpiralArcSpiralLineTransition (xyz[0], xyz[2], xyz[1],
                arcRadius, spiralLength, spiralLength, *spiralA, *spiralB, 
                xyzA, xyzB, xyzC, xyzD, arc
                )))
        {
        xyzLineTangent = xyzA;
        Check::SaveTransformed (xyz);
        Transform frameA = Transform::From (xyzA);
        Transform frameB = Transform::From (xyzB);
        auto cpA = ICurvePrimitive::CreateSpiral (*spiralA, frameA, 0.0, 1.0);
        auto cpB = ICurvePrimitive::CreateSpiral (*spiralB, frameB, 1.0, 0.0);
        for (auto cp : bvector<ICurvePrimitivePtr> {cpA, cpB})
            {
            Check::SaveTransformed (*cp);
            cp->GetStartEnd (xyz0, xyz1, uv0, uv1);
            DrawLine (xyz0, xyz0 + DVec3d::FromCrossProduct (crossVec, uv0));
            DrawLine (xyz1, xyz1 + DVec3d::FromCrossProduct (crossVec, uv1));
            }
        Check::SaveTransformed (arc);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,OffsetConstruction1)
    {
    bvector<DPoint3d> pi0 = bvector<DPoint3d> {DPoint3d::From (0,0,0), DPoint3d::From (0,60,0), DPoint3d::From (40,110,0)};
    double r0 = 70.0;
    double length0 = 20.0;
    DEllipse3d arc, arc0;
    DPoint3d lineTangent0, lineTangent1;
    ConstructTransition (pi0, r0, length0, lineTangent0, arc0);
    bvector<DPoint3d> centers;
    centers.push_back (arc0.center);
    double markerSize = 1.0;
    for (double offsetDistance : bvector<double> {14.0, 8.0, -8.0, -15.0})
        {
        double r1 = r0 - offsetDistance;
        double fraction = r1 / r0;
        double length1 = fraction * length0;
        bvector<DPoint3d> pi1;
        PolylineOps::OffsetLineString (pi1, pi0, offsetDistance, DVec3d::UnitZ (), false, 2.0);
        ConstructTransition (pi1, r1, length1, lineTangent1, arc);
        centers.push_back (arc.center);
        }
    Check::SaveTransformedMarkers (centers, markerSize);
    Check::ClearGeometry ("Spiral.OffsetConstruction1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,OffsetConstruction2)
    {
    bvector<DPoint3d> pi0 = bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,60,0),
        DPoint3d::From (40,110,0),
        DPoint3d::From (50,160,0)
        };
    double r0 = 70.0;
    double length0 = 20.0;

    for (size_t i0 = 0; i0 + 2 < pi0.size (); i0++)
        {
        auto primaryCV = CurveVector::ConstructSpiralArcSpiralTransition (pi0[i0], pi0[i0 + 1], pi0[i0 + 2], r0, length0);
        Check::SaveTransformed (*primaryCV);
        for (double offsetDistance : bvector<double> {12.0, 8.0, -6.0, -10.0})  // assymetric magnitudes to expose right-left flip
            {
            auto offsetCV = CurveVector::ConstructSpiralArcSpiralTransitionPseudoOffset (pi0[i0], pi0[i0 + 1], pi0[i0 + 2], r0, length0, offsetDistance);
            if (offsetCV.IsValid ())
                Check::SaveTransformed (*offsetCV);
            }
        }
    Check::ClearGeometry ("Spiral.OffsetConstruction2");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,PartialSpiralEvaluation)
    {
    double f0 = 0.1, f1 = 0.2;
    double totalLength = 100.0;
    double snippetLength = (f1 - f0) * totalLength;
    auto spiral = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
        (10,
            0.0, 0.0,  // x axis
            totalLength, 100.0,   // obvious length
            Transform::FromIdentity (),
            f0, f1
            );
    bvector<DPoint3d> xyzC;
    for (double u : bvector<double>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0})
        {
        DPoint3d xyz;
        spiral->FractionToPoint (u, xyz);
        xyzC.push_back (xyz);
        }
    double lengthB;
    spiral->Length (lengthB);
    double lengthC = PolylineOps::Length (xyzC);
    Check::True (lengthC <= snippetLength);
    Check::Near (lengthB, snippetLength);
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv->push_back (spiral);
    auto path = CurveVectorWithDistanceIndex::Create ();
    path->SetPath (cv);

    IFacetOptionsPtr curveOptions = IFacetOptions::CreateForCurves();

    bvector<PathLocationDetail> locations;
    path->Stroke(locations, *curveOptions);

    double prevDropDist = -DBL_MAX;
    bvector<double> dropDistances;
    for (PathLocationDetailCR pntDetail : locations)
        {
        double distanceAlong = pntDetail.DistanceFromPathStart();

        //Sometimes we get duplicate drop distances at the ends, as we walk the stroked points make sure we filter out any consecutive duplicates
        if(!DoubleOps::AlmostEqual(prevDropDist, distanceAlong))
            {
            dropDistances.push_back(distanceAlong);
            Check::LessThanOrEqual (prevDropDist, distanceAlong);
            Check::LessThanOrEqual (distanceAlong, snippetLength);
            }

        prevDropDist = distanceAlong;
        }
    Check::Near (prevDropDist, lengthB);
    }

void sample (ICurvePrimitivePtr &cp, double f0, double df, int numFraction, bvector<DPoint3d> &points)
    {
    points.clear ();
    for (int i = 0; i < numFraction; i++)
        {
        DPoint3d xyz;
        cp->FractionToPoint (f0 + i * df, xyz);
        points.push_back (xyz);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineCurve,ClosedToOpen)
    {
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (5,0,0));
    poles.push_back (DPoint3d::From (8,0,0));
    poles.push_back (DPoint3d::From (8,1,0));
    poles.push_back (DPoint3d::From (0,1,0));
    bvector<DPoint3d> polesA, polesB, pointC;
    double dX = 15.0;
    double dY = 10.0;
    double dYA = 2.0;
    for (int order = 2; order < 6; order++)
        {
        SaveAndRestoreCheckTransform shifter (0, dY, 0);
        MSBsplineCurvePtr curveA = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, order, true, true);
        auto cp = ICurvePrimitive::CreateBsplineCurve(curveA);
        Check::SaveTransformed (*cp);
        Check::Shift (0, dYA);
        curveA->GetPoles (polesA);
        Check::SaveTransformed (polesA);

        Check::Shift (dX, -dYA, 0);
        curveA->MakeOpen (0.0);
        sample (cp, 0.0, 0.1, 4, pointC);
        Check::SaveTransformed (*cp);
        curveA->GetPoles (polesB);

        Check::Shift (0, dYA);
        Check::SaveTransformed (polesB);
        Check::Shift (0, dYA);
        Check::SaveTransformed (pointC);
        }
    Check::ClearGeometry("MSBsplineCurve.ClosedToOpen");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, CleanKnots)
    {
    bvector<DPoint3d> poles = bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,6,0),
        DPoint3d::From (4,11,0),
        DPoint3d::From (5,16,0),
        DPoint3d::From (6,10,0),
        DPoint3d::From (8,0,0)
        };
    for (int order : {3,4})
        {
        SaveAndRestoreCheckTransform shifter (20,0, 0);
        MSBsplineCurvePtr curveA = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, order, false, false);
        Check::SaveTransformed (curveA);
        curveA->CleanKnots ();
        Check::Shift (0,10,0);
        Check::SaveTransformed (curveA);
        auto bezier = curveA->CreateCopyBezier ();
        Check::Shift (0,10,0);
        Check::SaveTransformed (bezier);
        bezier->CleanKnots ();
        Check::Shift (0,10,0);
        Check::SaveTransformed (bezier);
        }

    Check::ClearGeometry ("BsplineCurve.CleanKnots");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, Fit1)
    {
    bvector<DPoint3d> poles = bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,6,0),
        DPoint3d::From (4,8,0),
        DPoint3d::From (5, 12, 0),
        DPoint3d::From (6,10,0),
        DPoint3d::From (8,0,0)
        };
    int order = 5;
    MSBsplineCurvePtr curveA = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, order, false, false);
    
    Check::SaveTransformed (curveA, true);
    Check::Shift (10, 0, 0);
    for (double tolerance : {1.0e-3, 1.0e-2, 1.0e-1})
        {
        SaveAndRestoreCheckTransform shifter (10, 0, 0);
        MSBsplineCurvePtr curveB = MSBsplineCurve::CreatePtr ();
        curveB->ApproximateAnyCurve (curveA.get (), tolerance, 4, 2, true);
        Check::SaveTransformed (curveA);
        for (double f = 0.0; f <= 1.0; f += 0.0625)
            {
            DPoint3d xyz;
            curveA->FractionToPoint (xyz, f);
            Check::SaveTransformedMarker (xyz, -tolerance);
            }
        Check::SaveTransformed (curveB, true);
        }
    Check::ClearGeometry ("BsplineCurve.Fit1");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, Fit2)
    {
    bvector<DPoint3d> poles = bvector<DPoint3d> {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,6,0),
        DPoint3d::From (4,8,0),
        DPoint3d::From (5, 12, 0),
        DPoint3d::From (6,10,0),
        DPoint3d::From (8,0,0)
        };
    int order = 5;
    static double s_paramFraction = 10.0;
    static double s_pointFraction = 0.01;
    MSBsplineCurvePtr curveA = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, order, false, false);
    Check::SaveTransformed (curveA, true);
    Check::Shift (10, 0, 0);
    for (double tolerance : {1.0e-3, 1.0e-2, 1.0e-1})
        {
        SaveAndRestoreCheckTransform shifter (10, 0, 0);
        MSBsplineCurvePtr curveB = MSBsplineCurve::CreatePtr ();
        MSBsplineCurve::ApproximateG1Curve (
            curveB.get (),
            curveA.get (),
            3,  // out degree
            false, // end tangents
            3,
            tolerance, s_paramFraction * tolerance, s_pointFraction * tolerance);
        Check::SaveTransformed (curveA);
        for (double f = 0.0; f <= 1.0; f += 0.0625)
            {
            DPoint3d xyz;
            curveA->FractionToPoint (xyz, f);
            Check::SaveTransformedMarker (xyz, -tolerance);
            }
        Check::SaveTransformed (curveB, true);
        }
    Check::ClearGeometry ("BsplineCurve.Fit2");
    }
