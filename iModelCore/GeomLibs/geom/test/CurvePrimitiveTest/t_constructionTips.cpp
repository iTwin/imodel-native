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
                {
                Check::SaveTransformed (*constraint.Location ().curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.Location ().point}, s_markerSize);
                }
                break;
            }
        }
    }


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

TEST (Construction,HelloCircles)
    {
    double a = 10.0;
    bvector<ICurvePrimitivePtr> results;

    auto hPoint0 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0.1, 0.1));
    auto hPoint1 = CurveConstraint::CreateThroughPoint (DPoint3d::From (1,0));
    auto hPoint2 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0,2));
    auto hPointTangent0 = CurveConstraint::CreatePointAndDirection (DPoint3d::From (0.1, 0.1), DVec3d::From (1,0,0));

    auto hCenter0 = CurveConstraint::CreateCenter (DPoint3d::From (1,1));
    bvector<CurveConstraint> constraints;
    constraints.push_back (hPoint0);
    constraints.push_back (hPoint1);
    constraints.push_back (hPoint2);
    SaveHints (constraints);

    ConstrainedConstruction::ConstructCircularArcs (constraints, results);
    for (auto &cp: results)
        Check::SaveTransformed (*cp);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPoint0);
    constraints.push_back (hCenter0);
    constraints.push_back (hPoint2);
    SaveHints (constraints);

    ConstrainedConstruction::ConstructCircularArcs (constraints, results);
    for (auto &cp: results)
        Check::SaveTransformed (*cp);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPointTangent0);
    constraints.push_back (hPoint2);
    SaveHints (constraints);

    ConstrainedConstruction::ConstructCircularArcs (constraints, results);
    for (auto &cp: results)
        Check::SaveTransformed (*cp);



    Check::ClearGeometry ("Construction.HelloCircles");
    }

