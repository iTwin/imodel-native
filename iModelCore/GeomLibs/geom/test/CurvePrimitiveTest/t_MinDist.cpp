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
TEST(CurveCurve, MinDistBsplineToCircle)
    {

    Check::StartScope ("MinDistBsplineToCircle");
    
    testBsplineCircleProximity (0.5, 0.5, 0.9);
    testBsplineCircleProximity (0.5, 0.5, 0.1);
    testBsplineCircleProximity (0.5, 0.5, 0.0);     // hard case -- curve at dead center of circle.
    Check::EndScope ();
    }
    
