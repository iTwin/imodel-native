/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"







void SaveHints (bvector<CurveConstraint> &constraints)
    {
    static double s_markerSize = 0.25;
    for (auto &constraint : constraints)
        {
        switch (constraint.GetType ())
            {
            case CurveConstraint::Type::ThroughPoint:
                {
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.Location ().point}, s_markerSize);
                }
                break;
            case CurveConstraint::Type::Center:
                {
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (constraint.Location ().point, s_markerSize * 0.25));
                }
                break;
            case CurveConstraint::Type::ClosestPoint:
                {
                Check::SaveTransformed (*constraint.Location ().curve);
                }
                break;
            case CurveConstraint::Type::PointAndDirection:
                {
                Check::SaveTransformed (*constraint.Location ().curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.Location ().point}, s_markerSize);
                }
                break;
            case CurveConstraint::Type::PerpendicularNear:
            case CurveConstraint::Type::Tangent:
                {
                Check::SaveTransformed (*constraint.Location ().curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.Location ().point}, s_markerSize);
                }
                break;
            case CurveConstraint::Type::ResultFrame:
                {
                Transform transform = constraint.GetResultFrame ();
                double b = 1.0;  // use for negative coordinates to get the box asymetric.
                bvector<DPoint3d> box
                    {
                    DPoint3d::From (0,1),
                    DPoint3d::From (-b,1),
                    DPoint3d::From (-b,-b),
                    DPoint3d::From (1,-b),
                    DPoint3d::From (1,1),
                    DPoint3d::From (0,1),
                    DPoint3d::From (0,1.1),
                    DPoint3d::From (0,0),
                    DPoint3d::From (0,0,1),
                    DPoint3d::From (0,0),
                    DPoint3d::From (1.5,0)
                    };
                transform.Multiply (box, box);

                Check::SaveTransformed (box);
                }
                break;
            }
        }
    }

void RunConstraints (bvector<CurveConstraint> &constraints, void (*constructionMethod) (bvector<CurveConstraint> &, bvector<ICurvePrimitivePtr> &))
    {
    bvector<ICurvePrimitivePtr> results;
    SaveHints (constraints);
    constructionMethod (constraints, results);
    for (auto &cp: results)
        Check::SaveTransformed (*cp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,HelloLines)
    {

    DPoint3d point0 = DPoint3d::From (1,1,1);
    DPoint3d point1 = DPoint3d::From (5,2,-1);
    ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (3,1,0, 1,8,1));
    ICurvePrimitivePtr  arc0 = ICurvePrimitive::CreateArc (DEllipse3d::From (2,2,0,   1,0,0,  0,2,0, 0.0, Angle::DegreesToRadians (280)));
    ICurvePrimitivePtr  bsp0 = ICurvePrimitive::CreateBsplineCurve (
            MSBsplineCurve::CreateFromPolesAndOrder (
                bvector<DPoint3d> {
                    DPoint3d::From (2,0, -1),
                    DPoint3d::From (2,5, 2),
                    DPoint3d::From (5,8, 3),
                    DPoint3d::From (8,8)
                    },
                nullptr, nullptr, 3, false, true));
                
    double a = 10.0;
    auto hPoint0 = CurveConstraint::CreateThroughPoint (point0);
    auto hPoint1 = CurveConstraint::CreateThroughPoint (point1);
    auto hLine0 = CurveConstraint::CreateClosestPoint (line0.get ());
    auto hArc0 = CurveConstraint::CreateClosestPoint (arc0.get ());
    auto hBsp0 = CurveConstraint::CreateClosestPoint (bsp0.get ());
    bvector<CurveConstraint> allHints {hPoint0, hPoint1, hLine0, hArc0, hBsp0};

    for (size_t i1 = 0; i1 < allHints.size (); i1++)
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        for (size_t i0 = 0; i0 < allHints.size (); i0++)
            {
            SaveAndRestoreCheckTransform shifter (a,0,0);
            if (i0 != i1)
                {
                bvector<CurveConstraint> constraints {allHints[i0], allHints[i1]};
                SaveHints (constraints);
                bvector<ICurvePrimitivePtr> results;
                ConstrainedConstruction::ConstructLines (constraints, results);
                for (auto &cp: results)
                    Check::SaveTransformed (*cp);
                }
            }
        }
    Check::ClearGeometry ("Construction.HelloLines");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,PointPerpendicularNear)
    {


    DPoint3d point0 = DPoint3d::From (0.1, 0.1, 0.1);
    ICurvePrimitivePtr  arc0 = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0,
                                3,0,0,
                                0,8,0,
                                Angle::DegreesToRadians (-20),
                                Angle::DegreesToRadians (235)));
                
    double a = 10.0;
    auto hPoint0 = CurveConstraint::CreateThroughPoint (point0);

    double df = 1.0 / 16.0;
    for (double f = 0.0; f < 1.0; f += df)
        {
        SaveAndRestoreCheckTransform shifter (a,0,0);
        bvector<CurveConstraint> constraints;
        constraints.push_back (hPoint0);
        constraints.push_back (CurveConstraint::CreatePerpendicularNear (arc0.get (), f));
        SaveHints (constraints);

        bvector<ICurvePrimitivePtr> results;
        ConstrainedConstruction::ConstructLines (constraints, results);
        for (auto &cp: results)
            Check::SaveTransformed (*cp);
        }
    Check::ClearGeometry ("Construction.PointPerpendicularNear");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,HelloCircles)
    {
    double a = 6.0;
    bvector<ICurvePrimitivePtr> results;

    auto hPoint0 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0.1, 0.1));
    auto hPoint1 = CurveConstraint::CreateThroughPoint (DPoint3d::From (1,0));
    auto hPoint2 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0,2));
    auto hRadius = CurveConstraint::CreateRadius (0.8);
    auto hPointTangent0 = CurveConstraint::CreatePointAndDirection (DPoint3d::From (0.1, 0.1), DVec3d::From (1,0,0));

    auto hCenter0 = CurveConstraint::CreateCenter (DPoint3d::From (1,1));
    bvector<CurveConstraint> constraints;
    constraints.push_back (hPoint0);
    constraints.push_back (hPoint1);
    constraints.push_back (hPoint2);
    RunConstraints (constraints, ConstrainedConstruction::ConstructCircularArcs);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPoint0);
    constraints.push_back (hCenter0);
    constraints.push_back (hPoint2);
    RunConstraints (constraints, ConstrainedConstruction::ConstructCircularArcs);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPointTangent0);
    constraints.push_back (hPoint2);
    RunConstraints (constraints, ConstrainedConstruction::ConstructCircularArcs);


    bvector<Transform> transforms
        {
        Transform::FromIdentity (),
        CreateTestTransform (-1.5, 0.1, 0.2, 10.0, 22.4, 3.0)
        };
    for (auto &frame : transforms)
        {
        Check::Shift (a,0,0);
        constraints.clear ();
        constraints.push_back (hPoint1);
        constraints.push_back (hCenter0);
        constraints.push_back (CurveConstraint::CreateResultFrame (frame));
        RunConstraints (constraints, ConstrainedConstruction::ConstructCircularArcs);
        Check::Shift (a,0,0);
        constraints[0] = hRadius;
        RunConstraints (constraints, ConstrainedConstruction::ConstructCircularArcs);

        }

    Check::ClearGeometry ("Construction.HelloCircles");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,TangentCircles1)
    {


    DPoint3d point0 = DPoint3d::From (0.1, 0.1);
    DPoint3d point1 = DPoint3d::From (-1,2);
                
    double a = 40.0;

    for (double x0 : bvector<double> {0.0, 10.0, -6.0})
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        ICurvePrimitivePtr  arc0 = ICurvePrimitive::CreateArc (DEllipse3d::From (x0,0,0,
                                4,0,0,
                                0,4,0,
                                0.0, Angle::TwoPi ()));

        bvector<CurveConstraint> constraints;
        constraints.push_back (CurveConstraint::CreateThroughPoint (point0));
        constraints.push_back (CurveConstraint::CreateThroughPoint (point1));
        constraints.push_back (CurveConstraint::CreateTangent (arc0.get (), 0.0));

        SaveHints (constraints);

        bvector<ICurvePrimitivePtr> results;
        ConstrainedConstruction::ConstructCircularArcs (constraints, results);
        for (auto &cp: results)
            Check::SaveTransformed (*cp);
        constraints.pop_back ();

        Check::Shift (a,0,0);


        results.clear ();
        auto s0 = DSegment3d::From (x0, -1, 0, x0 + 5, 0, 0);
        auto s1 = DSegment3d::From (s0.FractionToPoint (-2.0), s0.FractionToPoint (3.0));
        auto line0 = ICurvePrimitive::CreateLine (s1);
        constraints.push_back (CurveConstraint::CreateTangent (line0.get (), 0.0));
        SaveHints (constraints);
        ConstrainedConstruction::ConstructCircularArcs (constraints, results);
        for (auto &cp: results)
            Check::SaveTransformed (*cp);

        }
    Check::ClearGeometry ("Construction.TangentCircles1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,TangentCircles2)
    {
    DPoint3d point0 = DPoint3d::From (0.4, 0.1);
    auto s0 = DSegment3d::From (-5,0,0,  0,-5,0);
    auto s0A = DSegment3d::From (s0.FractionToPoint (-2.0), s0.FractionToPoint (2.0));
    auto s1 = DSegment3d::From ( 4,0,0,  0,-4,0);
    auto s1A = DSegment3d::From (s1.FractionToPoint (-2.0), s1.FractionToPoint (2.0));
    bvector<ICurvePrimitivePtr> curves {
            ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (-2,2), 1.0)),
            ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (3,2),  1.5)),
            ICurvePrimitive::CreateLine (s0A),
            ICurvePrimitive::CreateLine (s1A)
            };
    double a = 100.0;
    for (size_t i0 = 0; i0 + 1 < curves.size (); i0++)
        {
        for (size_t i1 = i0 + 1; i1 < curves.size (); i1++)
            {
            SaveAndRestoreCheckTransform shifter (0,a,0);
            bvector<CurveConstraint> constraints;
            constraints.push_back (CurveConstraint::CreateTangent (curves[i0].get (), 0.0));
            constraints.push_back (CurveConstraint::CreateThroughPoint (point0));
            constraints.push_back (CurveConstraint::CreateTangent (curves[i1].get (), 0.0));

            SaveHints (constraints);

            bvector<ICurvePrimitivePtr> results;
            ConstrainedConstruction::ConstructCircularArcs (constraints, results);
            for (auto &cp: results)
                Check::SaveTransformed (*cp);
            }
        }
    Check::ClearGeometry ("Construction.TangentCircles2");
    }

// Pass g or a clip of g to Check.
// 
void SaveClipped (ICurvePrimitivePtr &g, CurveVectorCR clipper, DRange3dCR clipperRange)
    {
    DRange3d range;
    if (g.IsValid () && g->GetRange (range))
        {
        if (range.IsContained (clipperRange))
            Check::SaveTransformed (*g);
        else
            {
            CurveVectorPtr collector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
            CurveVectorPtr g1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            g1->push_back (g);
            g1->AppendSplitCurvesByRegion (clipper, collector.get (), nullptr, nullptr);
            Check::SaveTransformed (*collector);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,TangentCircles3)
    {
    double c = 10.0;
    auto clipper = CurveVector::CreateRectangle (-c, -c, c, c, 0.0);
    DRange3d clipperRange;
    clipper->GetRange (clipperRange);
    bvector<DPoint3d> centers {
        DPoint3d::From (2,2),
        DPoint3d::From (2,-2),
        DPoint3d::From (-1,0)
        };
    bvector<bvector<ICurvePrimitivePtr>> curves;
    double b = 10.0;
    double a = 40.0;
    for (auto center : centers)
        {
        curves.push_back (bvector<ICurvePrimitivePtr> ());
        curves.back ().push_back (
                ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (center, 1.0)));
        DVec3d normal = center - DPoint3d::FromZero ();
        DVec3d tangent = DVec3d::FromUnitPerpendicularXY (normal);
        curves.back ().push_back (
                ICurvePrimitive::CreateLine (
                   DSegment3d::From (center + b * tangent, center - b * tangent)));
        }

    for (size_t i0 = 0; i0 < 2; i0++)
        {
        for (size_t i1 = 0; i1 < 2; i1++)
            {
            for (size_t i2 = 0; i2 < 2; i2++)
                {
                SaveAndRestoreCheckTransform shifter (0,a,0);
                bvector<CurveConstraint> constraints;
                constraints.push_back (CurveConstraint::CreateTangent (curves[0][i0].get (), 0.0));
                constraints.push_back (CurveConstraint::CreateTangent (curves[1][i1].get (), 0.0));
                constraints.push_back (CurveConstraint::CreateTangent (curves[2][i2].get (), 0.0));

                SaveHints (constraints);
                bvector<ICurvePrimitivePtr> results;
                ConstrainedConstruction::ConstructCircularArcs (constraints, results);
                for (auto &cp: results)
                    SaveClipped (cp, *clipper, clipperRange);
                }
            }
        }
    Check::ClearGeometry ("Construction.TangentCircles3");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,TangentLines1)
    {
    bvector<Transform> transforms
        {
        Transform::FromIdentity (),
        CreateTestTransform (0.5, 0.1, 0.2, 10.0, 22.4, 3.0)
        };

    for (auto transform : transforms)
        {
        SaveAndRestoreCheckTransform shifter (15,0,0);
        auto circleA = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,1), 0.8));
        circleA->TransformInPlace (transform);
        bvector<CurveConstraint> constraints;
        constraints.push_back (CurveConstraint::CreateTangent (circleA.get (), 0.4));
        constraints.push_back (CurveConstraint::CreateThroughPoint (transform * DPoint3d::From (-1,-2)));
        SaveHints (constraints);
        bvector<ICurvePrimitivePtr> results;
        ConstrainedConstruction::ConstructLines (constraints, results);
        for (auto &cp: results)
            Check::SaveTransformed (*cp);
        }
    Check::ClearGeometry ("Construction.TangentLines1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Construction,TangentLines2)
    {
    bvector<DPoint3d> centers {
        DPoint3d::From (2,2),
        DPoint3d::From (2,-2)
        };

    bvector<Transform> transforms
        {
        Transform::FromIdentity (),
        CreateTestTransform (0.5, 0.1, 0.2, 10.0, 22.4, 3.0)
        };

    for (auto transform : transforms)
        {
        SaveAndRestoreCheckTransform shifter (100,0,0);
        for (double rB : bvector<double> {1.0, 3.0})
            {
            SaveAndRestoreCheckTransform shifter (0,15,0);
            for (double rA : bvector<double> {1.0, 3.0})
                {
                auto circleA = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (centers[0], rA));
                auto circleB = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (centers[1], rB));
                circleA->TransformInPlace (transform);
                circleB->TransformInPlace (transform);
                SaveAndRestoreCheckTransform shifter (15,0,0);
                bvector<CurveConstraint> constraints;
                constraints.push_back (CurveConstraint::CreateTangent (circleA.get (), 0.4));
                constraints.push_back (CurveConstraint::CreateTangent (circleB.get (), 0.4));
                SaveHints (constraints);
                bvector<ICurvePrimitivePtr> results;
                ConstrainedConstruction::ConstructLines (constraints, results);
                for (auto &cp: results)
                    Check::SaveTransformed (*cp);
                }
            }
        }
    Check::ClearGeometry ("Construction.TangentLines2");
    }
