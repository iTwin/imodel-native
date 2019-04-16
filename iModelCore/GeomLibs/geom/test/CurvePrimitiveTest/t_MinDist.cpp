/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

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