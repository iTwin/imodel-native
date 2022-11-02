/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
//static int s_noisy = 0;
#ifdef compile_bspcurv_offset
// edl -- april 24 2018 remove bspcurv_offset
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBspline,RecursiveOffset)
    {
    auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
        (
        bvector<DPoint3d>
            {
            DPoint3d::From (0,0),
            DPoint3d::From (3,0),
            DPoint3d::From (5,0),
            DPoint3d::From (7,2),
            DPoint3d::From (9,6),
            DPoint3d::From (9,10),
            DPoint3d::From (11,14),
            DPoint3d::From (13,15)
            },
        nullptr,
        nullptr,
        3,
        false,
        true
        );
    auto cpCurve = ICurvePrimitive::CreateBsplineCurve (bcurve);

    double tolerance = 1.0e-4;
    for (int select = 0; select < 3; select++)
        {
        Check::SaveTransformed (*cpCurve);
        for (double offsetDistance = 1.0; fabs (offsetDistance) < 8.0;
                    offsetDistance = 
                        offsetDistance > 0 ? -offsetDistance : -offsetDistance + 1.0
                        )
            {
            MSBsplineCurve offsetCurve;

            if (select == 0 && Check::Int (SUCCESS, bsprcurv_offset
                    (
                    &offsetCurve,
                    bcurve.get(),
                    offsetDistance,
                    0,
                    2,
                    tolerance,
                    nullptr
                    )))
                {
                auto cpOffset = ICurvePrimitive::CreateBsplineCurveSwapFromSource (offsetCurve);
                Check::SaveTransformed (*cpOffset);
                }
            if (select == 1 && Check::Int (SUCCESS, bsprcurv_offset
                    (
                    &offsetCurve,
                    bcurve.get(),
                    offsetDistance,
                    0,
                    1,
                    tolerance,
                    nullptr
                    )))
                {
                auto cpOffset = ICurvePrimitive::CreateBsplineCurveSwapFromSource (offsetCurve);
                Check::SaveTransformed (*cpOffset);
                }
            else if (select == 2)
                {
                CurveOffsetOptions options (offsetDistance);
                auto bcurve1 = bcurve->CreateCopyOffsetXY (offsetDistance, offsetDistance, options);
                if (bcurve1.IsValid ())
                    {
                    auto cpOffset = ICurvePrimitive::CreateBsplineCurve (bcurve1);
                    Check::SaveTransformed (*cpOffset);
                    }
                }
            }
        Check::Shift (20,0,0);
        }
    Check::ClearGeometry ("MSBsplineCurve.RecursiveOffset");
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,InflectionsXYZ)
    {
    for (double zTwist : {0.0, 0.7, 1.1})
        {
        SaveAndRestoreCheckTransform shifter (20, 0, 0);
        auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            bvector<DPoint3d>
                {
                DPoint3d::From (0,0),
                DPoint3d::From (3,0),
                DPoint3d::From (5,1, zTwist),
                DPoint3d::From (7,1,   2.0 * zTwist * zTwist),
                DPoint3d::From (9,0),
                DPoint3d::From (11,0)
                },
            nullptr,
            nullptr,
            4,
            false,
            true
            );
        bvector<DPoint3d> inflections;
        bvector<double> inflectionParams;
        bcurve->ComputeInflectionPoints (inflections, inflectionParams);
        Check::SaveTransformed (bcurve);
        Check::SaveTransformedMarkers (inflections, 0.04);
        Check::Shift (0,10,0);

        bcurve->ComputeInflectionPointsXY (inflections, inflectionParams, nullptr);
        Check::SaveTransformed (bcurve);
        Check::SaveTransformedMarkers (inflections, 0.04);
        Check::Shift (0,15,0);




        Check::SaveTransformed (bcurve);
        for (double z : { 0, 1,3,-2})
            {
            for (auto & spacePoint : bvector<DPoint3d> {
                    DPoint3d::From (3,4,z), 
                    DPoint3d::From (5,0.2, z),
                    DPoint3d::From (-3,1, z)
                    })
                {
                DPoint3d curvePoint;
                double curveParam;
                bcurve->ClosestPoint (curvePoint, curveParam, spacePoint);
                Check::SaveTransformed (bvector<DPoint3d>{spacePoint, curvePoint});
                }
            }

        Check::Shift (0,10,0);
        Check::SaveTransformed (bcurve);
        for (double z : { 0, 1,3,-2})
            {
            for (auto & spacePoint : bvector<DPoint3d> {
                    DPoint3d::From (3,4,z), 
                    DPoint3d::From (5,0.2, z),
                    DPoint3d::From (-3,1, z)
                    })
                {
                DPoint3d curvePoint;
                double curveParam;
                double xyDistance;
                bcurve->ClosestPointXY (curvePoint, curveParam, xyDistance, spacePoint, nullptr);
                Check::SaveTransformed (bvector<DPoint3d>{spacePoint, curvePoint});
                }
            }

        Check::Shift (0,10,0);
        Check::SaveTransformed (bcurve);
        for (double z : { 0, 1,3,-2})
            {
            for (auto & spacePoint : bvector<DPoint3d> {
                    DPoint3d::From (3,1,z), 
                    DPoint3d::From (6,0.4, z),
                    DPoint3d::From (-3,1, z)
                    })
                {
                bvector<DPoint3d> curvePoints;
                bvector<double> curveParams;
                bcurve->AllTangentsXY (curvePoints, curveParams, spacePoint, nullptr);
                for (auto &xyz: curvePoints)
                    Check::SaveTransformed (bvector<DPoint3d>{spacePoint, xyz});
                }
            }

        Check::Shift (0,20,0);
        Check::SaveTransformed (bcurve);
        for (double z : { 0, 1,3,-2})
            {
            for (auto & spacePoint : bvector<DPoint3d> {
                    DPoint3d::From (3,1,z), 
                    DPoint3d::From (6,0.4, z),
                    DPoint3d::From (-3,1, z)
                    })
                {
                bvector<DPoint3d> curvePoints;
                bvector<double> curveParams;
                bcurve->AllTangents (curvePoints, curveParams, spacePoint);
                for (auto &xyz: curvePoints)
                    {
                    Check::SaveTransformed (DSegment3d::From (spacePoint, xyz));
                    auto biasPoint = DPoint3d::FromInterpolate (xyz, 0.05, spacePoint);
                    Check::SaveTransformedMarker (biasPoint, -0.03);
                    DPoint3d curvePointA;
                    double curveParamA;
                    bcurve->ClosestTangent (curvePointA, curveParamA, spacePoint, biasPoint);
                    Check::SaveTransformedMarker (curvePointA, 0.05);
                    Check::Near (curvePointA, xyz, "tangency point recheck");
                    }
                Check::Shift (0,10,0);
                }
            }

        }
    Check::ClearGeometry ("BsplineCurve.InflectionsXYZ");
    }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(BsplineCurve, AllParallelTangents)
        {
        bvector<DPoint3d> poles;
        // in xz plane, concave to the right ...
        poles.push_back(DPoint3d::From(0, 1, 0));
        poles.push_back(DPoint3d::From(1, -1, 0));
        poles.push_back(DPoint3d::From(3, 1, 0));
        poles.push_back(DPoint3d::From(5, -1, 0));
        poles.push_back(DPoint3d::From(7, 1, 0));
        poles.push_back(DPoint3d::From(9, 6, 0));

        MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(poles, NULL, NULL, 3, false, false);
        for (auto tangentDirection : {
            DVec3d::From(1,-0.1,0),
            DVec3d::From(1,0.1,0)
            })
            {
            SaveAndRestoreCheckTransform shifterA(0, 10, 0);
            bvector<DPoint3d> points;
            bvector<double> fractions;
            curve->AllParallellTangentsXY(points, fractions, tangentDirection);
            Check::SaveTransformed(*curve);
            DPoint3d xyz = poles[0];
            Check::SaveTransformed(
                DSegment3d::From(xyz, xyz + tangentDirection));
            for (size_t i = 0; i < points.size(); i++)
                {
                Check::SaveTransformedMarker(points[i]);
                Check::SaveTransformed(DSegment3d::From(points[i], points[i] + tangentDirection));
                double weight;
                DVec3d tangent;
                curve->FractionToPoint(xyz, weight, tangent, fractions[i]);
                Check::SaveTransformed(DSegment3d::From(xyz, xyz + 0.2 * tangent));
                Check::Parallel(tangent, tangentDirection);
                Check::Near(xyz, points[i]);
                }
            }
        Check::ClearGeometry("MSBsplineCurve.AllParallelTangents");
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(BsplineCurve, CloseTangentXY)
        {
        bvector<DPoint3d> poles;
        // in xz plane, concave to the right ...
        poles.push_back(DPoint3d::From(0, 1, 0));
        poles.push_back(DPoint3d::From(1, -1, 0));
        poles.push_back(DPoint3d::From(3, 1, 0));
        poles.push_back(DPoint3d::From(5, -1, 0));
        poles.push_back(DPoint3d::From(7, 1, 0));
        poles.push_back(DPoint3d::From(9, 6, 0));
        MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(poles, NULL, NULL, 3, false, false);
        Check::SaveTransformed (*curve);
        for (double fractionA: {0.25, 0.50, 0.75})
            {
            DPoint3d xyzA, xyzB, xyzC;
            DVec3d tangentA, tangentB;
            double fractionC;
            curve->FractionToPoint (xyzA, tangentA, fractionA);
            double fractionB = fractionA + 0.02;
            curve->FractionToPoint(xyzB, tangentB, fractionB);
            DPoint3d biasPoint = DPoint3d::FromInterpolateAndPerpendicularXY (xyzA, 0.1, xyzB, 0.5);
            DPoint3d spacePoint = xyzA + 0.5 * tangentA;
            curve->ClosestTangentXY(xyzC, fractionC, spacePoint, biasPoint, nullptr);
            Check::SaveTransformedMarker (xyzA);
            Check::SaveTransformedMarker(biasPoint);
            Check::SaveTransformed (DSegment3d::From (spacePoint, xyzC));
            Check::Near (xyzA, xyzC);
            Check::Near (fractionA, fractionC);
            }
        Check::ClearGeometry("MSBsplineCurve.ClosestTangentXY");
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(BsplineCurve, PointsAtFractions)
        {
        bvector<DPoint3d> poles;
        // in xz plane, concave to the right ...
        poles.push_back(DPoint3d::From(0, 1, 0));
        poles.push_back(DPoint3d::From(1, -1, 0));
        poles.push_back(DPoint3d::From(3, 16, 0));
        poles.push_back(DPoint3d::From(5, -1, 0));
        poles.push_back(DPoint3d::From(7, 1, 0));
        poles.push_back(DPoint3d::From(9, 6, 0));
        MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(poles, NULL, NULL, 3, false, false);
        bvector<DPoint3d> points;
        bvector<double> fractions;
        size_t n = 20;
        Check::SaveTransformed(*curve);
        curve->PointsAtUniformFractions (points, fractions, n);
        Check::SaveTransformedMarkers(points, 0.05);
        for (size_t i = 0; i + 2 < fractions.size(); i++)
            {
            Check::Near (fractions[i+1]-fractions[i], fractions[i+2] - fractions[i+1], "uniform fractions");
            }
        curve->PointsAtUniformArcLength (points, fractions, n);
        Check::Shift (0,20,0);
        Check::SaveTransformed(*curve);
        Check::SaveTransformedMarkers(points, 0.05);
        for (size_t i = 0; i + 2 < fractions.size(); i++)
            {
            double d01 = curve->LengthBetweenFractions (fractions[i], fractions[i+1]);
            double d12 = curve->LengthBetweenFractions(fractions[i+1], fractions[i + 2]);
            // test with loose tolerance
            Check::Near(d01, d12,  "Uniform arc length", 1000.0);
            }

        Check::Shift(0, 20, 0);
        Check::SaveTransformed(*curve);
        bvector<DVec3d> derivatives;
        points.clear ();
        derivatives.clear ();
        fractions.clear ();
        curve->AddStrokes (n,
            points, 
            &derivatives, 
            &fractions, true, 
            CURVE_PARAMETER_MAPPING_CurveFraction);
        Check::SaveTransformed (points);
        double a = 0.5 / n;
        for (size_t i = 0; i < fractions.size (); i++)
            Check::SaveTransformed (DSegment3d::From (points[i], points[i] + a * derivatives[i]));
        Check::ClearGeometry("MSBsplineCurve.PointsAtUniformFractions");
        }
    /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,MakeBezier)
    {
    for (double zTwist : {0.0, 1.1})
        {
        SaveAndRestoreCheckTransform shifter (20, 0, 0);
        bvector<DPoint3d> poles
                {
                DPoint3d::From (0,0),
                DPoint3d::From (3,0),
                DPoint3d::From (5,1, zTwist),
                DPoint3d::From (7,1,   2.0 * zTwist * zTwist),
                DPoint3d::From (9,0),
                DPoint3d::From (11,0)
                };
        auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            poles,
            nullptr,
            nullptr,
            4,
            false,
            true
            );
        // Simple curve ...
        Check::SaveTransformed (bcurve);

        // With weights ...
        bcurve->MakeRational ();
        Check::Shift (0,4,0);
        Check::SaveTransformed (bcurve);

        // single bezier ...
        auto singleBezier =  bcurve->CreateCopyBezier ();
        Check::Shift (0,4,0);
        Check::SaveTransformed (singleBezier);

        // many beziers ...
        bvector<MSBsplineCurvePtr> multipleBeziers;
        bcurve->CopyAsBeziers (multipleBeziers);
        Check::Shift (0,4,0);
        for (auto &b : multipleBeziers)
            Check::SaveTransformed (b);

        Check::Shift (0,4,0);
        auto curveC = MSBsplineCurve::CreateFromBeziers (multipleBeziers);
        Check::SaveTransformed (curveC);

        }
    Check::ClearGeometry ("BsplineCurve.MakeBezier");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,Cusps)
    {
    // with zero z, the full and projected curves both have cusps. 
    for (double zTwist : {0.0, 1.1})
        {
        SaveAndRestoreCheckTransform shifter (20, 0, 0);
        bvector<DPoint3d> poles
                {
                DPoint3d::From (0,0),
                DPoint3d::From (1,1),
                DPoint3d::From (0,1, zTwist),
                DPoint3d::From (1,0)
                };
        auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            poles,
            nullptr,
            nullptr,
            4,
            false,
            true
            );
        // Simple curve ...
        Check::SaveTransformed (bcurve);

        bvector<DPoint3d> cusps;
        bvector<double> params;
        bcurve->AddCuspsXY (&cusps, &params, nullptr);
        Check::SaveTransformedMarkers (cusps, 0.04);

        Check::Shift (0,2,0);
        Check::SaveTransformed (bcurve);
        cusps.clear ();
        bcurve->AddCusps (&cusps, &params);
        Check::SaveTransformedMarkers (cusps, 0.04);
        }
    Check::ClearGeometry ("BsplineCurve.Cusps");
    }

void ShowWithStartEnd (double dy, MSBsplineCurvePtr const &curve)
    {
    Check::Shift (0, dy, 0);
    DPoint3d xyz0, xyz1;
    curve->FractionToPoint (xyz0, 0.0);
    curve->FractionToPoint (xyz1, 1.0);
    Check::SaveTransformed (curve);
    if (curve->IsClosed ())
        {
        Check::SaveTransformedMarker (xyz0, -0.03);
        }
    else
        {
        Check::SaveTransformedMarker (xyz0, 0.03);
        Check::SaveTransformedMarker (xyz1, 0.03);
        bvector<DPoint3d> strokes;
        strokes.push_back (xyz0);
        for (double f : {0.1, 0.2, 0.3})
            {
            DPoint3d xyz;
            curve->FractionToPoint (xyz, f);
            strokes.push_back (xyz);
            strokes.push_back (
                DPoint3d::FromInterpolateAndPerpendicularXY (xyz0, 1.0, xyz, 0.1));
            xyz0 = xyz;
            }
        Check::SaveTransformed (strokes);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,Closed)
    {
    // with zero z, the full and projected curves both have cusps. 
    for (double zTwist : {0.0, 1.1})
        {
        SaveAndRestoreCheckTransform shifter (5, 0, 0);
        bvector<DPoint3d> poles
                {
                DPoint3d::From (0,0),
                DPoint3d::From (1,0),
                DPoint3d::From (1,1, zTwist),
                DPoint3d::From (0,2),
                DPoint3d::From (-1,1)
                };

        auto originalCurve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            poles,
            nullptr,
            nullptr,
            4,
            true,
            true
            );

       ShowWithStartEnd (0, originalCurve);
       double knot30 = originalCurve->FractionToKnot (0.30);
       auto splitAt30 = originalCurve->CreateCopyOpenAtKnot (knot30);
       ShowWithStartEnd (4, splitAt30);

       auto closeAt30 = splitAt30->CreateCopyClosed ();
       ShowWithStartEnd (4, closeAt30);

       }
    Check::ClearGeometry ("BsplineCurve.Closed");
    }

void SweepBetweenAsymptotes (DConic4dR conic,
double theta0,
double theta1,
double fraction
)
    {
    double alpha0 = DoubleOps::Interpolate (theta0, fraction, theta1);
    double alpha1 = DoubleOps::Interpolate (theta0, 1.0 - fraction, theta1);
    conic.start = alpha0;
    conic.sweep = alpha1 - alpha0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,DConic4d)
    {
    // with zero z, the full and projected curves both have cusps. 
    for (double w0 : {0.0, 0.1, 0.3, 0.7})
        {
        SaveAndRestoreCheckTransform shifter1 (5, 0, 0);
        for (double w90 : {0.0, 0.3, 0.5, 0.7})
            {
            SaveAndRestoreCheckTransform shifter2 (0, 5, 0);
            DConic4d conic;
            DPoint4d center = DPoint4d::From (0,0,0,1);
            DPoint4d vector0 = DPoint4d::From (1,0,0,w0);
            DPoint4d vector90 = DPoint4d::From (0,1,0,w90);
            bsiDConic4d_initFrom4dVectors (&conic,
                &center,
                &vector0,
                &vector90,
                0.0, 6.0
                );
            double c0, s0, c1, s1;
            double a;
            int n = bsiMath_solveUnitQuadratic (&c0, &s0, &c1, &s1, &a, 1, w0, w90);
            if (n == 2)
                {
                double theta0 = atan2 (s0, c0);
                double theta1 = atan2 (s1, c1);
                if (theta0 > theta1)
                    std::swap (theta0, theta1);
                SweepBetweenAsymptotes (conic, theta0, theta1, 0.1);
                auto curve = MSBsplineCurve::CreateFromDConic4d (conic);
                if (curve.IsValid ())
                    Check::SaveTransformed (curve);
                SweepBetweenAsymptotes (conic, theta1, theta0 + Angle::TwoPi (), 0.1);
                curve = MSBsplineCurve::CreateFromDConic4d (conic);
                if (curve.IsValid ())
                    Check::SaveTransformed (curve);
                }
            else if (n == 0)
                {
                auto curve = MSBsplineCurve::CreateFromDConic4d (conic);
                if (curve.IsValid ())
                    Check::SaveTransformed (curve);
                }
            }
       }
    Check::ClearGeometry ("BsplineCurve.DConic4d");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,Copy)
    {
    bvector<DPoint3d> poles
            {
            DPoint3d::From (0,0),
            DPoint3d::From (3,0),
            DPoint3d::From (5,1),
            DPoint3d::From (7,2),
            DPoint3d::From (9,0),
            DPoint3d::From (10,2),
            DPoint3d::From (11,0)
            };
    auto curve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            poles.data (), (int)poles.size (),
            4,
            false
            );
    ShowWithStartEnd (0.0, curve);
    Check::SaveTransformed (curve);
    ShowWithStartEnd (4, curve->CreateCopyReversed ());

    ShowWithStartEnd (4, curve->CreateCopyBetweenKnots (0.1, 0.5));
    ShowWithStartEnd (2, curve->CreateCopyBetweenKnots (0.5, 0.1));

    auto transform = Transform::FromMatrixAndFixedPoint (
            RotMatrix::FromRowValues (
                1, -0.2, 0,
                0.2, 1, 0,
                0,  0,  1
                ),
            DPoint3d::From (5,0,0)); 
    ShowWithStartEnd (2, curve->CreateCopyTransformed(transform));
    Check::ClearGeometry ("BsplineCurve.Copy");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,FixedChordLength)
    {
    bvector<DPoint3d> poles
            {
            DPoint3d::From (0,0),
            DPoint3d::From (3,0),
            DPoint3d::From (5,1),
            DPoint3d::From (7,2),
            DPoint3d::From (9,0),
            DPoint3d::From (10,2),
            DPoint3d::From (11,0)
            };
    auto curve = MSBsplineCurve::CreateFromPolesAndOrder
            (
            poles.data (), (int)poles.size (),
            4,
            false
            );
    bvector<DPoint3d> points;
    bvector<double> params;
    for (double a : {0.5, 1.0, 2.0})
        {
        SaveAndRestoreCheckTransform shifter (0, 5, 0);
        Check::SaveTransformed (curve);
        curve->StrokeWithFixedChordLength (points, params, a);
        Check::SaveTransformed (points);
        for (size_t i = 0; i + 1 < points.size (); i++)
            Check::Near (a, points[i].Distance (points[i+1]));
        }
    Check::ClearGeometry ("BsplineCurve.FixedChordLength");
    }

static bvector<double> suspectKnots
    {
    -0.0024308355052249401,
    -0.0024308355052249401,
    0.00089260739125482001,
    0.00089260739125482001,
    0.0042160502877345799 ,
    0.0042160502877345799 ,
    0.0075394931842143404 ,
    0.0075394931842143404 ,
    0.010862936080694101  ,
    0.010862936080694101  ,
    0.014186378977173900  ,
    0.014186378977173900  ,
    0.017509821873653599  ,
    0.017509821873653599  ,
    0.020833264770133399  ,
    0.020833264770133399  ,
    0.024156707666613101  ,
    0.024156707666613101  ,
    0.027480150563092900  ,
    0.027480150563092900  ,
    0.030803593459572599  ,
    0.030803593459572599
    };
static bvector<DPoint3d> suspectPoles
    {
    DPoint3d::From (0.055946978976323204, -0.058240828923063002, 0.20045848066128399),
    DPoint3d::From (0.056256486230119897, -0.058147835327057203, 0.20061563595716800),
    DPoint3d::From (0.056977613413905705, -0.060213870216702700, 0.20053002636673803),
    DPoint3d::From (0.057227305809723496, -0.060003592517550504, 0.20068364148617701),
    DPoint3d::From (0.057872297267850897, -0.060996594470500903, 0.20041917724786101),
    DPoint3d::From (0.058070086293077500, -0.060739718314781005, 0.20057987955019499),
    DPoint3d::From (0.059926794936194398, -0.061719512943375202, 0.19994781818507201),
    DPoint3d::From (0.060005150852925605, -0.061419585381460598, 0.20013662492827200),
    DPoint3d::From (0.061064130593255800, -0.061653109810302101, 0.19959095501770199),
    DPoint3d::From (0.061076236342387097, -0.061357139488029198, 0.19980046644785002),
    DPoint3d::From (0.062931726537669902, -0.060705251149900004, 0.19883436670744001),
    DPoint3d::From (0.062834838911504598, -0.060465719418793598, 0.19908711712514501),
    DPoint3d::From (0.063643723196896304, -0.059830369600018800, 0.19844373746551699),
    DPoint3d::From (0.063505194005905596, -0.059642905928736806, 0.19871861667006399),
    DPoint3d::From (0.064237928792320709, -0.057763267548976600, 0.19786870657736302),
    DPoint3d::From (0.064064685281096906, -0.057698633375515403, 0.19817665723980202),
    DPoint3d::From (0.064114141004545003, -0.056582066454766301, 0.19769010642247301),
    DPoint3d::From (0.063948203723952601, -0.056587520709190299, 0.19800876389096600),
    DPoint3d::From (0.063092199924651093, -0.054598572121023599, 0.19765189914422998),
    DPoint3d::From (0.062986186232764002, -0.054721562290896801, 0.19797415565242801),
    DPoint3d::From (0.062205022936941398, -0.053815093748746698, 0.19778996883852698),
    DPoint3d::From (0.062150946242236903, -0.053984438929068804, 0.19810511859053900),
    DPoint3d::From (0.060160903131923904, -0.053089764162026903, 0.19828790157958401),
    DPoint3d::From (0.060226122104957903, -0.053302010555301302, 0.19857499670414200),
    DPoint3d::From (0.059022880493195699, -0.053155026591640106, 0.19864142244228000),
    DPoint3d::From (0.059154325878906698, -0.053363415930436704, 0.19890783450321201),
    DPoint3d::From (0.057144939378006701, -0.054112283562450000, 0.19936502070591999),
    DPoint3d::From (0.057385366350658898, -0.054264044668977801, 0.19958811718489700),
    DPoint3d::From (0.056425118481298601, -0.054993230004181798, 0.19972948814954403),
    DPoint3d::From (0.056707171327040000, -0.055092851446330700, 0.19993041196664701),
    DPoint3d::From (0.055822091553107599, -0.057067093171181596, 0.20027823623492699),
    DPoint3d::From (0.056138904624494203, -0.057043777534755299, 0.20044601886164701),
    DPoint3d::From (0.055946978976323204, -0.058240828923063002, 0.20045848066128399),
    DPoint3d::From (0.056256486230119897, -0.058147835327057203, 0.20061563595716800),
    DPoint3d::From (0.056977613413905705, -0.060213870216702700, 0.20053002636673803),
    DPoint3d::From (0.057227305809723496, -0.060003592517550504, 0.20068364148617701)
    };

void ExamineBCurvePatches (MSBsplineCurveCR curve)
    {
    BCurveSegment segment;
    for (size_t bezierIndex = 0; curve.AdvanceToBezier(segment, bezierIndex, true); bezierIndex++)
        {
        printf (" [%d] %s ", (int)bezierIndex, segment.IsNullU() ? "null" : "----");
        for (size_t i = 0; i < segment.GetNumKnots (); i++)
            {
            printf (" %12.9lf", segment.GetKnotP()[i]);
            }
        printf ("\n");
        }
    }
void SaveNet(DPoint3d *points, int numU, int numV)
    {
    for (int i = 0; i < numV; i++)
        Check::SaveTransformed (points + i * numU, numU);
    bvector<DPoint3d> column;
    for (int i = 0; i < numU; i++)
        {
        column.clear ();
        for (int j = 0; j < numV; j++)
            column.push_back (*(points + j * numU + i));
        Check::SaveTransformed (column);
        }
    }
void SaveNet (MSBsplineSurfaceCR surface)
    {
    SaveNet (surface.poles, surface.uParams.numPoles, surface.vParams.numPoles);
    }
bool IsClampedKnotVector(BsplineParam const &params, double const *knots)
    {
    if (!params.closed)
        {
        int numKnots = params.NumberAllocatedKnots ();
        int order = params.order;
        double tolerance = KNOT_TOLERANCE_BASIS;
        double knot0 = knots[order-1];
        double knot1 = knots[numKnots - order];
        for (int i = 0; i + 1 < order; i++)
            {
            if (fabs (knot0 - knots[i]) > tolerance)
                return false;
            if (fabs(knot1 - knots[numKnots - 1 - i]) > tolerance)
                return false;
            }
        return true;
        }
    return false;
    }
void CheckSurface (MSBsplineSurfaceR surface)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetAngleTolerance (0.20);
    DRange3d surfaceRange;
    surface.GetPoleRange(surfaceRange);
    SaveNet(surface);
    // Check::Shift(0, 0, 1.5 * surfaceRange.ZLength());


    bvector<bvector<MSBsplineSurface>> beziers;
    surface.MakeBeziers(beziers);
    Check::Shift(0, 0, 1.5 * surfaceRange.ZLength());
    for (auto &slice : beziers)
        {
        for (auto &patch : slice)
            Check::SaveTransformed(patch);
        }
    MSBsplineSurface::ClearBeziers(beziers);

    Check::Shift(0, 0, 3 * surfaceRange.ZLength());
    SaveNet(surface);
    Check::Shift(0, 0, 1.5 * surfaceRange.ZLength());

    SaveAndRestoreCheckTransform shifter (
        DoubleOps::Max (surfaceRange.XLength (), surfaceRange.high.x ) * 2, 0, 0);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->Add(surface);
    Check::SaveTransformed(builder->GetClientMeshR());
#ifdef testProppedSurfaces
    MSBsplineSurface preppedSurface;
    preppedSurface.Zero();
    int numUSegs, numVSegs;
    int *uStart = nullptr, *vStart = nullptr;
    bspproc_prepareSurface(&preppedSurface, &numUSegs, &numVSegs, &uStart, &vStart,
        &surface);

    Check::True (IsClampedKnotVector (preppedSurface.uParams, preppedSurface.uKnots));
    Check::True(IsClampedKnotVector(preppedSurface.vParams, preppedSurface.vKnots));

    SaveNet(preppedSurface);
    preppedSurface.ReleaseMem();

    Check::Shift(0, 0, 2.0 * surfaceRange.ZLength());
    surface.SwapUV ();
    bspproc_prepareSurface(&preppedSurface, &numUSegs, &numVSegs, &uStart, &vStart,
        &surface);
    Check::True(IsClampedKnotVector(preppedSurface.uParams, preppedSurface.uKnots));
    Check::True(IsClampedKnotVector(preppedSurface.vParams, preppedSurface.vKnots));

    SaveNet(preppedSurface);
#endif
    }
// Sawthooth along X
// Start at x0,y0,z0
// Advance x by dx
// negate y each step
bvector<DPoint3d> SawtoothAlongX (int numPoints, double x0, double y0, double z0, double dx)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From(x0, y0, z0));
    for (int i = 1; i < numPoints; i++)
        {
        auto xyz = points.back ();
        points.push_back (DPoint3d::From (xyz.x + dx, -xyz.y, z0));
        }
    return points;
    }
// make copies of a polyline as V direction poles
bvector<DPoint3d> PolylineCopiesV (int numUStep, bvector<DPoint3d> const &pointsV, DVec3dCR stepU)
    {
    bvector<DPoint3d> gridPoints;
    for (auto xyz : pointsV)
        {
        for (int i = 0; i < numUStep; i++)
            {
            xyz = xyz + stepU;
            gridPoints.push_back (xyz);
            }
        }
    return gridPoints;
    }

TEST(MSBspline, KnotRangeTests)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();

    Check::SaveTransformed (suspectPoles);
    Check::Shift (0,0.1,0);
    Check::SaveTransformed(bvector<DPoint3d> { suspectPoles[0], suspectPoles[1] });
    for (size_t j = 0; 2 * j + 3 < suspectPoles.size(); j++)
        {
        Check::SaveTransformed(
            bvector<DPoint3d> { suspectPoles[2 * j], suspectPoles[2 * j + 2],
            suspectPoles[2 * j + 3], suspectPoles[2 * j + 1]
            });
        }

    static int s_doFixup = 1;
    DRange3d surfaceRange = DRange3d::From (suspectPoles);

    Check::Shift(3 * surfaceRange.XLength (), 0, 0);

    auto clampedKnots = suspectKnots;
    size_t k1 = clampedKnots.size() - 1;
    clampedKnots[0] = 0;
    for (size_t i = 1; i < 4; i++)
        {
        clampedKnots[i] = clampedKnots[0];
        clampedKnots[k1 - i] = clampedKnots[k1];
        }
    double knotOriginA = suspectKnots[0];
    double knotOriginB = suspectKnots[3];
    double knotDelta = suspectKnots.back () - knotOriginA;
    bvector<double> positiveKnotsA, positiveKnotsB;
    for (auto knot : suspectKnots)
        {
        positiveKnotsA.push_back ((knot - knotOriginA)/knotDelta);
        positiveKnotsB.push_back((knot - knotOriginB) / knotDelta);
        }
    // v knots stricly interior
    bvector<double> positiveInteriorKnots;
    for (size_t i = 0; 2 * i < suspectKnots.size(); i++)
        {
        double q = 0.2 + i * 0.05;
        positiveInteriorKnots.push_back (q);
        positiveInteriorKnots.push_back (q);
        }
    if (s_doFixup)
        {
        double a = 0.02;
        auto gridPoints = PolylineCopiesV (2, SawtoothAlongX
         ((int)suspectPoles.size () / 2, 0,  a, 0, a), DVec3d::From (0,0,a));
         for (auto &knots : {positiveInteriorKnots, clampedKnots, positiveKnotsA, positiveKnotsB, suspectKnots})
            {
            SaveAndRestoreCheckTransform shifter (0, 10.0 * a, 0);
            auto bsurf1 = MSBsplineSurface::CreateFromPolesAndOrder(
                gridPoints, nullptr,
                nullptr, 2, 2, false,
                &knots, 4, 18, false, true);
            CheckSurface(*bsurf1);
            auto bsurf2 = MSBsplineSurface::CreateFromPolesAndOrder(
                suspectPoles, nullptr,
                nullptr, 2, 2, false,
                &knots, 4, 18, false, true);
            CheckSurface(*bsurf2);
             }
        }
    Check::ClearGeometry("BsplineSurface.KnotRangeTests");
    }



