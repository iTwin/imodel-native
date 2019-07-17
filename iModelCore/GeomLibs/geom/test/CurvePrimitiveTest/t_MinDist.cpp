/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
#include "ConstructPointRadiusTangentPointTangentRadius.h"

// Make a smooth bcurve.
// evaluate its frenet frame at curveFraction
// Make a perpendicular circle there;
//   evaluate the circle at circleFraction.
//   interpolate at radius fraction from the circle center to the circle ponit.
//   make the bcurve pass throught there perpendicular to the circle's plane
// evaluate the closest approach -- should be at curveFraction and on that radial line.
static void testBsplineCircleProximity (double curveEvaluationFraction, double circleEvaluationFraction, double radiusFraction)
    {

    // Make a bspline known to be in xy plane ..
    bvector<DPoint3d> poles;
    // in xz plane, concave to the right ...
    poles.push_back (DPoint3d::From (3,-20, 0));
    poles.push_back (DPoint3d::From (1,-10, 0));
    poles.push_back (DPoint3d::From (0,  0, 0));
    poles.push_back (DPoint3d::From (1, 10, 0));
    poles.push_back (DPoint3d::From (3, 20, 0));
    
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false);
    Transform frenetFrame;
    curve->GetFrenetFrame (frenetFrame, curveEvaluationFraction);
    DVec3d xVec, yVec, zVec;
    DPoint3d center0;
    
    frenetFrame.GetOriginAndVectors (center0, xVec, yVec, zVec);
    double radius = 5.0;
    // put a circle around the frenet origin
    DEllipse3d arc = DEllipse3d::FromScaledVectors (
            center0,
            yVec, zVec,
            radius, radius,
            0.0, Angle::TwoPi ());
                
    DPoint3d arcPoint;                
    arc.FractionParameterToPoint (arcPoint, circleEvaluationFraction);
    
    DPoint3d center1 = DPoint3d::FromInterpolate (center0, -radiusFraction, arcPoint);
    arc.center = center1;
    ICurvePrimitivePtr primitiveA = ICurvePrimitive::CreateBsplineCurve (*curve);
    ICurvePrimitivePtr primitiveB = ICurvePrimitive::CreateArc (arc);
    CurveLocationDetail detailA, detailB;
    
    CurveCurve::ClosestApproach (detailA, detailB, primitiveA.get (), primitiveB.get ());
    Check::Near (center0, detailA.point, "MinDist point on curve");
   
    }
//static int s_noisy = 0;       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, MinDistBsplineToCircle)
    {

    Check::StartScope ("MinDistBsplineToCircle");
    
    testBsplineCircleProximity (0.5, 0.5, 0.9);
    testBsplineCircleProximity (0.5, 0.5, 0.1);
    testBsplineCircleProximity (0.5, 0.5, 0.0);     // hard case -- curve at dead center of circle.
    Check::EndScope ();
    }
    
//!<ul>
//!<li> Test if two curve vectors have matching structure and are geometrically related by relationship of their local transforms.
//!<li> This relies on the behavior of LOCAL_COORDINATE_SCALE_UnitAxesAtStart:
//!<ul>
//!<li> origin is at start of first primitive.
//!<li> frame X axs is tangent is along first primitive from that point
//!<li> frame Y axis is in plane of the tangent and the first subsequent tangent that is not parallel to X
//!<ul>
//!<li> Skew and scaling is not detected.
//!<ul>
bool TestSameStructureAndGeometryWithTransform (
CurveVectorCR dataA,    //!< [in] first geometry
CurveVectorCR dataB,    //!< [in] second geometry
TransformR transformAToB,   //! [out] transform that relocates dataB on top of dataA
TransformR transformBToA    //! [out] transform that relocateds dataA on top of dataB
)
    {
    transformBToA = Transform::FromIdentity ();
    if (!dataA.IsSameStructure (dataB))
        return false;
    Transform worldToLocalA, localToWorldA, worldToLocalB, localToWorldB;
    DRange3d localRangeA, localRangeB;
    auto dataA1 = dataA.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtStart, localToWorldA, worldToLocalA, localRangeA);
    auto dataB1 = dataB.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtStart, localToWorldB, worldToLocalB, localRangeB);
    if (dataA1->IsSameStructureAndGeometry (*dataB1))
        {
        transformAToB = localToWorldB * worldToLocalA;
        transformBToA = localToWorldA * worldToLocalB;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, IsSameStructureAndGeometryWithTransform)
    {
    bvector<CurveVectorPtr> curveSets;
    SampleGeometryCreator::AddMultiPrimitiveXYOpenPaths (curveSets);
    curveSets.push_back (SampleGeometryCreator::CircleInRectangle ());
    curveSets.push_back (SquareWavePolygon (4));
    curveSets.push_back (SquareWavePolygon (6));
    Transform placementA = Transform::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), Angle::DegreesToRadians (45)), DPoint3d::From (3,1,-2));
    Transform placementB = Transform::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (3,-1,2), Angle::DegreesToRadians (14)), DPoint3d::From (20,10,9));
    Transform inversePlacementA, inversePlacementB;
    inversePlacementA.InverseOf (placementA);
    inversePlacementB.InverseOf (placementB);
    Transform transformAToB, transformBToA;
    for (size_t i = 0; i < curveSets.size (); i++)
        {
        Check::NamedScope scopeI ("curveI", i);
        auto curveI0 = curveSets[i];
        for (size_t j = i; j < curveSets.size (); j++)
            {
            Check::NamedScope scopeI ("curveJ", j);
            Check::Bool (i == j, TestSameStructureAndGeometryWithTransform (*curveSets[i], *curveSets[j], transformAToB, transformBToA), "TestSameStructureWithTransform trivial cases");
            }
        }
    size_t n = 0;
    for (auto &curves : curveSets)
        {
        Check::NamedScope scopeI ("PlacedCurve", n++);
        auto curveA = curves->Clone (placementA);
        auto curveB = curves->Clone (placementB);
        if (Check::True (TestSameStructureAndGeometryWithTransform (*curveA, *curveB, transformAToB, transformBToA), "same geometry transformed"))
            {
            Check::Near (placementA * inversePlacementB, transformBToA,"B to A");
            Check::Near (placementB * inversePlacementA, transformAToB,"A to B");
            }
        }
    }


bool VerifyTangents(CurveVectorCR path)
    {
    bool stat = true;
    for (size_t i = 0; i + 1 < path.size(); i++)
        {
        auto rayA = path.at(i)->FractionToPointAndUnitTangent (1.0).Value();
        auto rayB = path.at(i+1)->FractionToPointAndUnitTangent(0.0).Value();
        if (!Check::Near(rayA.origin, rayB.origin, "C0 within path")
            || !Check::Near(rayA.direction, rayB.direction, "C1 within path"))
            {
            stat = false;
            }
        }
    return stat;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ConstructPlanarMinimunRadiusCurve)
    {
    static int s_noisy = 0;
    DPoint3d pointA = DPoint3d::From (-1,0.5,0);
    DPoint3d pointB = DPoint3d::From (9.1,-1.0,0);
    double bigStep = 200.0;
    double radiusA = 2.0;
    double manyDegrees[] = {15, 45, 75,
        105, 135, 165,
        195, 225, 255,
        285, 315, 345
        };
    double degrees0[] = {15,45};
    double degrees1[] = {-15, -45};
    for (double radiusB : {2.0, 1.0, 3.0})
        {
        for (double degreesA : manyDegrees)
            {
            auto radiansA = Angle::DegreesToRadians (degreesA);
            SaveAndRestoreCheckTransform shifterA(bigStep, 0, 0);
            for (double degreesB : manyDegrees)
                {
                SaveAndRestoreCheckTransform shifterA(0, bigStep, 0);
                auto radiansB = Angle::DegreesToRadians(degreesB);
                DVec3d tangentA = DVec3d::From (cos(radiansA), sin(radiansA));
                DVec3d tangentB = DVec3d::From (cos(radiansB), sin(radiansB));
                CurveVectorPtr shortestCurves = nullptr;
                double minLength = 1.0e20;
                for (DPoint2d signs : {
                    DPoint2d::From ( 1, 1),
                    DPoint2d::From ( 1,-1),
                    DPoint2d::From (-1, 1),
                    DPoint2d::From (-1,-1),
                    })
                    {
                    SaveAndRestoreCheckTransform shifterA(0, 10, 0);
                    auto c = ConstructTransition_PointTangentRadius_PointTangentRadius::ConstructOnXYPlane(
                            pointA, tangentA, signs.x * radiusA,
                            pointB, tangentB, signs.y * radiusB);
                    if (s_noisy > 0)
                        {
                        Check::SaveTransformed (c);
                        Check::SaveTransformedMarker (pointA);
                        Check::SaveTransformedMarker (pointB);
                        Check::SaveTransformed (DSegment3d::From (pointA, pointA + tangentA));
                        Check::SaveTransformed(DSegment3d::From(pointB, pointB + tangentB));
                        }
                    double length = c->Length ();
                    if (!VerifyTangents(*c))
                        if (s_noisy == 0)       // don't rewrite if already noisy.
                            Check::SaveTransformed(c);
                    if (length < minLength)
                        {
                        minLength = length;
                        shortestCurves = c;
                        }
                    if (s_noisy > 0)
                        {
                        DPoint3d pointA1, pointB1;
                        DVec3d unitA1, unitB1;
                        c->GetStartEnd (pointA1, pointB1, unitA1, unitB1);
                        Check::SaveTransformed (DSegment3d::From (pointA1, pointA1 + 2.0 * unitA1));
                        Check::SaveTransformed(DSegment3d::From(pointB1, pointB1 + 2.0 * unitB1));
                        if (unitA1.DotProduct (tangentA) < 0)
                            Check::SaveTransformedMarker (pointA1, -1.0);
                        if (unitB1.DotProduct(tangentB) < 0)
                            Check::SaveTransformedMarker(pointB1, -1.0);
                        }
                    }
                Check::Shift(0, 20, 0);
                Check::SaveTransformed(shortestCurves);
                }
            }

        Check::Shift (3.0 * bigStep, 0, 0);
        }
    Check::ClearGeometry("CurveVector.ConstructPlanarMinimunRadiusCurve");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ConstructSpaceMinimunRadiusCurve)
    {
    static int s_noisy = 0;
    DPoint3d pointA = DPoint3d::From(-1, 0.5, 0);
    DPoint3d pointB = DPoint3d::From(9.1, -1.0, 1);
    double bigStep = 200.0;
    double radiusA = 2.0;
    double radiusB = 1.0;
    double manyDegrees[] = { 15, 45, 75,
        105, 135, 165,
        195, 225, 255,
        285, 315, 345
        };
    double degrees0[] = { 15,45 };
    double degrees1[] = { -15, -45 };
    for (double degreesA : manyDegrees)
        {
        auto radiansA = Angle::DegreesToRadians(degreesA);
        SaveAndRestoreCheckTransform shifterA(bigStep, 0, 0);
        for (double degreesB : manyDegrees)
            {
            SaveAndRestoreCheckTransform shifterA(0, bigStep, 0);
            auto radiansB = Angle::DegreesToRadians(degreesB);
            DVec3d tangentA = DVec3d::From(cos(radiansA), sin(radiansA), 0);
            DVec3d tangentB = DVec3d::From(cos(radiansB), sin(radiansB), -2);
            CurveVectorPtr shortestCA = nullptr;
            double minLengthA = 1.0e20;
            CurveVectorPtr shortestCB = nullptr;
            double minLengthB = 1.0e20;
            for (DPoint2d signs : {
                DPoint2d::From(1, 1),
                    DPoint2d::From(1, -1),
                    DPoint2d::From(-1, 1),
                    DPoint2d::From(-1, -1),
                })
                {
                SaveAndRestoreCheckTransform shifterA(0, 10, 0);
                auto cA = ConstructTransition_PointTangentRadius_PointTangentRadius::ConstructOnPlaneFavoringFirstInput(
                    pointA, tangentA, signs.x * radiusA,
                    pointB, tangentB, signs.y * radiusB);
                auto cB = ConstructTransition_PointTangentRadius_PointTangentRadius::ConstructOnPlaneFavoringFirstInput(
                    pointB, -1.0 * tangentB, signs.y * radiusB,
                    pointA, -1.0 * tangentA, signs.x * radiusA);
                cB->ReverseCurvesInPlace();
                if (s_noisy > 0)
                    {
                    Check::SaveTransformedMarker(pointA);
                    Check::SaveTransformedMarker(pointB);
                    Check::SaveTransformed(DSegment3d::From(pointA, pointA + tangentA));
                    Check::SaveTransformed(DSegment3d::From(pointB, pointB + tangentB));
                    }
                double lengthA = cA->Length();
                if (!VerifyTangents(*cA))
                    Check::SaveTransformed(cA);
                if (lengthA < minLengthA)
                    {
                    minLengthA = lengthA;
                    shortestCA = cA;
                    }

                double lengthB = cB->Length();
                if (!VerifyTangents(*cB))
                    Check::SaveTransformed(cB);
                if (lengthB < minLengthB)
                    {
                    minLengthB = lengthB;
                    shortestCB = cB;
                    }


                if (s_noisy > 0)
                    {
                    Check::SaveTransformed (cA);
                    Check::SaveTransformed(cB);
                    DPoint3d pointA1, pointB1;
                    DVec3d unitA1, unitB1;
                    cA->GetStartEnd(pointA1, pointB1, unitA1, unitB1);
                    Check::SaveTransformed(DSegment3d::From(pointA1, pointA1 + 2.0 * unitA1));
                    Check::SaveTransformed(DSegment3d::From(pointB1, pointB1 + 2.0 * unitB1));
                    if (unitA1.DotProduct(tangentA) < 0)
                        Check::SaveTransformedMarker(pointA1, -1.0);
                    if (unitB1.DotProduct(tangentB) < 0)
                        Check::SaveTransformedMarker(pointB1, -1.0);
                    }
                }
                Check::Shift(0, 20, 0);
                Check::SaveTransformed(shortestCA);
                Check::SaveTransformed(shortestCB);
                auto cAB = ConstructTransition_PointTangentRadius_PointTangentRadius::ConstructMiddleCurveBlend (*shortestCA, *shortestCB, 17);
                Check::SaveTransformed (cAB);
            }
        }
    Check::ClearGeometry("CurveVector.ConstructSpatialMinimunRadiusCurve");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, SingleStepMinimumRadiusCurve)
    {
    DPoint3d pointA = DPoint3d::From(-1, 0.5, 0);
    DPoint3d pointB = DPoint3d::From(9.1, 3.0, 1);
    double bigStep = 20.0;
    double radius = 2.0;
    DVec3d tangentA = DVec3d::From (2,1,1);
    for (DVec3d tangentB :
        {
        DVec3d::From (4,3,0),
        DVec3d::From(2, 0, 0),
        DVec3d::From(0,5,-1),
        DVec3d::From(1, -3, 1),
        DVec3d::From(3, 0, -1)
        })
        {
        SaveAndRestoreCheckTransform shifterA(0, bigStep, 0);
        auto curve = ConstructTransition_PointTangentRadius_PointTangentRadius::ConstructFull3DBlend(
                pointA, tangentA, radius,
                pointB, tangentB, radius);
        Check::SaveTransformed(DSegment3d::From(pointA, pointA + tangentA));
        Check::SaveTransformed(DSegment3d::From(pointB, pointB + tangentB));
        Check::SaveTransformed(curve);
        }
    Check::ClearGeometry("CurveVector.SingleStepMinimumRadiusCurve");
    }
