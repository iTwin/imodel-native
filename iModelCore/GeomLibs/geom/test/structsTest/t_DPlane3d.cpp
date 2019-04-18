/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPlane3d, InitFromArray_FirstPointFarthest)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (100,0,0));  // put first point way out there to confirm fix to off-by-one index!!!
    points.push_back (DPoint3d::From (100,0,0));  // repeat it so 3-point prefix is bad.
    points.push_back (DPoint3d::From (0,2,0));
    points.push_back (DPoint3d::From (2,3,0));
    points.push_back (DPoint3d::From (5,-1,0));
    points.push_back (DPoint3d::From (2,1,0));
    
    DPlane3d planeA, planeB, planeC;
    Check::True (planeA.InitFromArray (&points[0], (int)points.size ()));

    Check::False (planeB.InitFromArray (&points[0], 3));

    Transform rotation = Transform::FromPlaneNormalToLine (
          DPoint3d::From (1,2,3),
          DPoint3d::From (4,2,9),
          0,
          false
          );
    bvector<DPoint3d> pointsC;
    rotation.Multiply (pointsC, points);
    Check::True (planeC.InitFromArray (&pointsC[0], (int)pointsC.size ()));
    DPlane3d planeC1 = planeA;
    rotation.Multiply (planeC1);
    Check::Parallel (planeC.normal, planeC1.normal);

    }



void buildFractions
(
double a0,
double a1,
double u0,
double u1,    // assume u1 > u0
int numU01,
bvector<double> &splitFractions
)
    {
    splitFractions.clear ();
    double f0 = 0.0, f1 = 1.0, df;
    double da = a1 - a0;
    if (fabs (da) < 1.0e-8)
        return;
        
    // f0,f1 are start and end "a" coordinates corrected for direction.
    if (a0 >= a1)
        {
        f0 = u1;
        f1 = u0;
        }
    else if (a0 < a1)
        {
        f0 = u0;
        f1 = u1;
        } 
    for (int i = 0; i < numU01; i++)
        {
        df = (f1 - f0) / (numU01 - 1);
        double a = f0 + i * df;
        if ((a - a0) * (a - a1) <= 0.0)
            splitFractions.push_back ((a-a0) / da);    
        }
    }

void CheckFractions (double a0, double a1, double u0, double u1, int n, bvector<double> &fraction1, bvector<double> &fraction2)
    {
    buildFractions (a0, a1, u0, u1, n, fraction1);
    buildFractions (a1, a0, u0, u1, n, fraction2);
    if (Check::Size (fraction1.size (), fraction2.size (), "reverse counts match"))
        {
        for (size_t i = 0; i < fraction1.size (); i++)
            {
            Check::Near (fraction1[i], 1.0 - fraction2[fraction2.size () - i - 1], "match fraction");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BuildFractions, Test1)
    {
    bvector<double> fraction1;
    bvector<double> fraction2;
    for (int n = 2; n < 5; n++)
        {
        CheckFractions (0, 5, 2, 4, n, fraction1, fraction2);
        Check::Size (n, fraction1.size (), "Complete interval cover");
        Check::Size (n, fraction1.size (), "Complete interval cover");
        CheckFractions (5,15,0,10, n, fraction1, fraction2);
        CheckFractions (-5,8,0,10, n, fraction1, fraction2);
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPlane3d,Intersect)
    {
    auto planeA = DPlane3d::FromOriginAndNormal (1,2,3, 2,1,9);
    auto planeB = DPlane3d::FromOriginAndNormal (-2,1,2, 1,3,0.4);
    auto planeC = DPlane3d::FromOriginAndNormal (1,-2,5, 0.2, -1,9);
    auto point0 = DPlane3d::Intersect3Planes (planeA, planeB, planeC);
    if (Check::True (point0.IsValid (), "Intersect3Planes"))
        {
        Check::Near (0.0, planeA.Evaluate (point0.Value ()), "confirm intersection on plane");
        Check::Near (0.0, planeB.Evaluate (point0.Value ()), "confirm intersection on plane");
        Check::Near (0.0, planeC.Evaluate (point0.Value ()), "confirm intersection on plane");
        }
    Check::False (DPlane3d::Intersect3Planes (planeA, planeB, planeA).IsValid (),
            "confirm intesrection failure with repeated plane");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPlane3d,IntersectByDistance)
    {
    auto normalA = DVec3d::From (1,2,3);
    auto normalB = DVec3d::From (-2,1,4);
    auto normalC = DVec3d::From (0.1,0.4, 2);
    normalA.Normalize ();
    normalB.Normalize ();
    normalC.Normalize ();
    double distanceA = 2.0;
    double distanceB = -0.3;
    double distanceC = 4.1234;
    auto result = DPlane3d::Intersect3Planes (normalA, distanceA, normalB, distanceB, normalC, distanceC);
    if (Check::True (result.IsValid ()))
        {
        DPoint3d xyz = result.Value ();
        Check::Near (distanceA, xyz.DotProduct (normalA), "confirm intersection on plane");
        Check::Near (distanceB, xyz.DotProduct (normalB), "confirm intersection on plane");
        Check::Near (distanceC, xyz.DotProduct (normalC), "confirm intersection on plane");

        DPlane3d planeA = DPlane3d::FromNormalAndDistance (normalA, distanceA);
        DPlane3d planeB = DPlane3d::FromNormalAndDistance (normalB, distanceB);
        DPlane3d planeC = DPlane3d::FromNormalAndDistance (normalC, distanceC);
        auto result1 = DPlane3d::Intersect3Planes (planeA, planeB, planeC);
        Check::Near (result.Value (), result1.Value (), "compare arg list variants");
        }

#ifdef TEST_LINEAR_COMBINATION
    // Construct  normal that is coplanar with the normals of A and B
    DVec3d normalAB = DVec3d::FromSumOf (normalA, 1.8, normalB, 2.3);
    normalAB.Normalize ();
    // This should be detected as singular (3 planes, but the intersect in an infinite line rather than a point)
    // But there is numerical fuzz and it comes back true.
    // Should change the RotMatrix solver to have a better tolerance.  But not now ...
    Check::False (
        DPlane3d::Intersect3Planes (normalA, distanceA, normalB, distanceB, normalAB, distanceC).IsValid (),
        "confirm no intersection case");
#endif

    // on windows, this generated an exact 0 determinant which (correctly) triggered a failure in RotMatrix::Solve
    // on mobiles, the determinant was fuzz and it seemed to succeed.
    // But now the ::Solve test is more subtle so the mobile (correctly) fails
    Check::False (DPlane3d::Intersect3Planes (normalA, distanceA, normalB, distanceB, normalA, distanceC).IsValid (),
            "confirm intesrection failure with repeated plane");

    // confirm things close to the perturbation that (experimentally) failed on windows:
    double epsilon = 1.0e-12;   // should pass.
    DVec3d normalA1 = normalA;
    normalA1.x += epsilon;
    Check::True(DPlane3d::Intersect3Planes (normalA, distanceA, normalB, distanceB, normalA1, distanceC).IsValid (),
            "confirm intesrection ok with big fuzz");
    normalA1 = normalA;
    normalA1.x += epsilon / 10.0;
    Check::False (DPlane3d::Intersect3Planes (normalA, distanceA, normalB, distanceB, normalA1, distanceC).IsValid (),
            "confirm intesrection failure with fuzz");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, ParallelPerpendicular)
    {
    DPlane3d plane1;
    double pnt1[]  = { 6, 3, 1 };
    DPoint3d origin1 = DPoint3d::FromArray(pnt1);
    double nrm1[] = { 1, 0, 1 };
    DVec3d normal1 = DVec3d::FromArray(nrm1);
    plane1.InitFromOriginAndNormal(origin1, normal1);
    DPlane3d plane2;
    DPoint3d origin2 = DPoint3d::From(8, 3, 7);
    double nrm2[] = { 1, 0, 1 };
    DVec3d normal2 = DVec3d::FromArray(nrm2);
    plane2.InitFromOriginAndNormal(origin2, normal2);
    Check::Parallel(normal1, normal2);  //if planes are parallel

    nrm2[2] = -1;
    normal2 = DVec3d::FromArray(nrm2);
    plane2.InitFromOriginAndNormal(origin2, normal2);
    Check::Perpendicular(normal1, normal2); //if planes are perpendicular
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, PointVector_NormalPerpendicular) 
    {
    DPlane3d plane1;
    double pnt1[]  = { 6, 3, 1 };
    DPoint3d origin1 = DPoint3d::FromArray(pnt1);
    DVec3d vecP1 = DVec3d::From(origin1);
    double nrm1[] = { 1, 0, 0 };
    DVec3d normal = DVec3d::FromArray(nrm1);
    plane1.InitFromOriginAndNormal(origin1, normal);

    double pnt2[]  = { 6, 0, 4 };
    DPoint3d origin2 = DPoint3d::FromArray(pnt2);
    DVec3d vecP2 = DVec3d::From(origin2);
    vecP2.Subtract(vecP1);
    Check::Perpendicular(vecP2, normal);  //vector between two points on a plane is perpendicular to normal
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, DistanceFromOrigin)
    {
    //If the normal vector is normalized (unit length), then the constant term of the plane equation, d becomes the distance from the origin.
    double nrm1[] = { 1, 2, 2 };
    DVec3d normal = DVec3d::FromArray(nrm1);
    double distance = 6;
    DPlane3d plane = DPlane3d::FromNormalAndDistance(normal, distance);
    DPoint4d pointReceived;
    plane.GetDPoint4d(pointReceived);
    DPoint3d pnt3d;
    pnt3d.XyzOf(pointReceived);
    normal.Normalize();
    Check::Near(pnt3d, normal);

    double nrm2[3];
    nrm2[0] = nrm1[0] * 2;
    nrm2[1] = nrm1[1] * 2;
    nrm2[2] = nrm1[2] * 2;
    DPoint3d originExpected = DPoint3d::FromArray(nrm2);
    Check::Near(originExpected, plane.origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, PlaneFrom3Points) 
    {
    DPoint3d origin = DPoint3d::From(1, 2, 3);
    DPoint3d point1 = DPoint3d::From(12, 11, 9);
    DPoint3d point2 = DPoint3d::From(4, 6, 9);
    DPlane3d plane1 = DPlane3d::From3Points(origin, point1, point2);
    
    DVec3d vecA = DVec3d::FromStartEnd(origin, point1);
    DVec3d vecB = DVec3d::FromStartEnd(origin, point2);
    DVec3d normal = DVec3d::FromCrossProduct(vecA, vecB);

    DPoint4d planeEquation;
    plane1.GetDPoint4d(planeEquation);
    Check::Near(normal.x, planeEquation.x);
    Check::Near(normal.y, planeEquation.y);
    Check::Near(normal.z, planeEquation.z);
    Check::Near(15, planeEquation.w);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, PointProjectionOnPlane)
    {
    //random point projection on plane
    double nrm1[] = { 1, 0, 0 };
    DVec3d normal = DVec3d::FromArray(nrm1);
    double distance = 6;
    DPlane3d plane = DPlane3d::FromNormalAndDistance(normal, distance);

    DPoint3d pntToProject = DPoint3d::From(10, 10, 10);
    DPoint3d projection;
    plane.ProjectPoint(projection, pntToProject);

    DVec3d vec3d = DVec3d::FromStartEnd(plane.origin, pntToProject);
    double intm = vec3d.DotProduct(normal) / normal.MagnitudeSquared();
    normal.Scale(intm);
    pntToProject.Subtract(normal);

    Check::Near(pntToProject, projection);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, ZeroPlane)
    {
    DPlane3d plane = DPlane3d::FromNormalAndDistance(DVec3d::From(0, 0, 0), 0);
    Check::True(plane.IsZero());
    Check::False(plane.Normalize());
    DPlane3d planeNonZero = DPlane3d::FromNormalAndDistance(DVec3d::From(1, 2, 1), 6);
    Check::False(planeNonZero.IsZero());
    Check::True(plane.Normalize());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, VectorProjectionOnNormal)
    {
    DPlane3d plane = DPlane3d::FromNormalAndDistance(DVec3d::From(0, 1, 0), 6);
    DVec3d vector = DVec3d::From(7, 8, 7);
    double projection = plane.EvaluateVector(vector);
    double projectionExpected = plane.normal.DotProduct(vector);
    Check::Near(projectionExpected, projection);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, TrueDistance) 
    {
    DVec3d planeNormal = DVec3d::From(1, 0, 0);
    DPlane3d plane = DPlane3d::FromNormalAndDistance(planeNormal, 5);
    DPoint3d pointUnderTest = DPoint3d::From(7, 8, 8);
    double distance = plane.Evaluate(pointUnderTest);
    DVec3d vecPnt = DVec3d::FromStartEnd(plane.origin, pointUnderTest);
    double distanceExpected = vecPnt.DotProduct(plane.normal);
    printf("%f    %f   \n", distance, distanceExpected);
    Check::Near(distance, distanceExpected);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, TrueDistanceFromRange)
    {
    DVec3d planeNormal = DVec3d::From(1, 0, 0);
    DPlane3d plane = DPlane3d::FromNormalAndDistance(planeNormal, 5);
    DPoint3d pnts[] = { DPoint3d::From(9,9,9),
                        DPoint3d::From(2,3,2),
                        DPoint3d::From(9,8,8),
                        DPoint3d::From(7,9,3) };
    size_t maxIndex, minIndex;
    plane.EvaluateRange(pnts, 4, minIndex, maxIndex);

    double maxValue = plane.EvaluateMaxAbs(pnts, 4);

    double maxValExpected = plane.Evaluate(pnts[maxIndex]);
    Check::Near(maxValue, maxValExpected);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, TrueDistanceFromRange2)
    {
    DVec3d planeNormal = DVec3d::From(1, 0, 0);
    DPlane3d plane = DPlane3d::FromNormalAndDistance(planeNormal, 5);
    bvector<DPoint3d> pnts= { DPoint3d::From(9,9,9),
        DPoint3d::From(2,3,2),
        DPoint3d::From(9,8,8),
        DPoint3d::From(7,9,3) };
    size_t maxIndex, minIndex;
    plane.EvaluateRange(pnts, minIndex, maxIndex);

    double maxValue = plane.EvaluateMaxAbs(pnts);

    double maxValExpected = plane.Evaluate(pnts[maxIndex]);
    Check::Near(maxValue, maxValExpected);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPlane3d, Plane3Points)
    {
    DPlane3d plane0 = DPlane3d::From3Points(DPoint3d::From(1, 1, 1), DPoint3d::From(-1, 1, 0), DPoint3d::From(2, 0, 3));
    DPlane3d plane1 = DPlane3d::FromOriginAndNormal(DPoint3d::From(1, 1, 1), DVec3d::From(-1, 3, 2));
    
    Check::Near(plane0.normal, plane1.normal);
    Check::Near(plane0.origin, plane1.origin);
    }
bool CheckNear (DPlane3dCR planeA, DPlane3dCR planeB)
    {
    return Check::Near (planeA.origin, planeB.origin)
        && Check::Near (planeA.normal, planeB.normal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPlane3d, Init)
    {
    for (DPoint4d coffA : {
        DPoint4d::From (2,3,5,7),
            DPoint4d::From (0,0,3,4),
            DPoint4d::From (1,0,0,3),
            DPoint4d::From (0,1,0,7)
            })
        {
        DPlane3d planeFromDPoint4d;
        planeFromDPoint4d.Init (coffA);
        DPlane3d planeFromScalars;
        planeFromScalars.Init (coffA.x, coffA.y, coffA.z, -coffA.w);
        CheckNear (planeFromDPoint4d, planeFromScalars);
        DPoint4d coffB;
        planeFromScalars.GetDPoint4d (coffB);

        DPoint4d coffB1, coffA1;
        coffB1.InitWithNormalizedWeight (coffB);
        coffA1.InitWithNormalizedWeight (coffA);
        Check::Near (coffA1, coffB1);
        for (DPoint3d xyz :
            {
            DPoint3d::From (1,2,3),
            DPoint3d::From (9,2,3),
            DPoint3d::From (1,3,3),
            DPoint3d::From (1,2,7),
            DPoint3d::From (-3,2,1)})
            {
            Check::Near (planeFromDPoint4d.Evaluate (xyz), planeFromScalars.Evaluate (xyz));
            }
        }


    }
