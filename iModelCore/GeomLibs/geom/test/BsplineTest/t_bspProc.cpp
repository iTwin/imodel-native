/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
//static int s_noisy = 0;
#ifdef compile_bspcurv_offset
// edl -- april 24 2018 remove bspcurv_offset
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  05/18
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
                bcurve->AllTangents (curvePoints, curveParams, spacePoint);
                for (auto &xyz: curvePoints)
                    Check::SaveTransformed (bvector<DPoint3d>{spacePoint, xyz});
                }
            }

        }
    Check::ClearGeometry ("BsplineCurve.InflectionsXYZ");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
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
* @bsimethod                                                     Earlin.Lutz  05/18
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
* @bsimethod                                                     Earlin.Lutz  05/18
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
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,DConic4d)
    {
    // with zero z, the full and projected curves both have cusps. 
    for (double w0 : {0.0, 0.1, 0.3, 0.7})
        {
        SaveAndRestoreCheckTransform shifter (5, 0, 0);
        for (double w90 : {0.0, 0.3, 0.5, 0.7})
            {
            SaveAndRestoreCheckTransform shifter (0, 5, 0);
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
* @bsimethod                                                     Earlin.Lutz  05/18
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
* @bsimethod                                                     Earlin.Lutz  05/18
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

