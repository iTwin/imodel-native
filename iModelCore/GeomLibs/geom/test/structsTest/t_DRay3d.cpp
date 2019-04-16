/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"


void rayIntersectPlaneTest (DPoint3d planeOrigin, DVec3d planeU, DVec3d planeV, DPoint3d rayOrigin, double u, double v)
    {
    DPoint3d targetPoint = DPoint3d::FromSumOf (planeOrigin, planeU, u, planeV, v);
    DPlane3d plane;
    plane.origin = planeOrigin;
    plane.normal = DVec3d::FromCrossProduct (planeU, planeV);
    DRay3d ray1 = DRay3d::FromOriginAndTarget (rayOrigin, targetPoint);  // This ray intersects the target at fraction 1.
    DPoint3d intersectionPoint;
    double intersectionParameter;
    for (double fraction = 0.25; fraction < 10.0; fraction *= 2.0)
        {
        DRay3d ray = ray1;
        ray.direction.Scale (1.0 / fraction);
        if (Check::True (ray.Intersect (intersectionPoint, intersectionParameter, plane)))
            {
            Check::Near (intersectionParameter, fraction, "intersection fraction");
            Check::Near (targetPoint, intersectionPoint, "intersection point");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DRay3d, IntersectPlane)
    {
    rayIntersectPlaneTest (DPoint3d::From (0,0,0), DVec3d::From (1,0,0), DVec3d::From (0,1,0), DPoint3d::From (1,2,3), 2,3);
    rayIntersectPlaneTest (DPoint3d::From (1,2,3), DVec3d::From (1,0.4,0.2), DVec3d::From (0.2,-1.2,0.5), DPoint3d::From (1.2,2.9,3.0), 2.2, 3.4);
    }

void testClosestApproach (DRay3dCR rayA, DRay3dCR rayB)
    {
    Check::StartScope ("ClosestApproachUnboundedRayUnboundedRay");
    Check::StartScope ("rayA", rayA);
    Check::StartScope ("rayB", rayB);
    double fA, fB;
    DPoint3d xyzA, xyzB;
    if (rayA.direction.IsParallelTo (rayB.direction))
        {
        Check::False (DRay3d::ClosestApproachUnboundedRayUnboundedRay (fA, fB, xyzA, xyzB, rayA, rayB), "Detect parallel");
        }
    else if (Check::True(DRay3d::ClosestApproachUnboundedRayUnboundedRay (fA, fB, xyzA, xyzB, rayA, rayB), "Detect nonparallel"))
        {
        Check::Near (xyzA, rayA.FractionParameterToPoint (fA), "Point::Parameter on A");
        Check::Near (xyzB, rayB.FractionParameterToPoint (fB), "Point::Parameter on B");
        DVec3d chord = DVec3d::FromStartEnd (xyzA, xyzB);
        Check::Perpendicular (chord, rayA.direction, "Chord perp A");
        Check::Perpendicular (chord, rayB.direction, "Chord perp B");
        DRay3d rayC = DRay3d::FromOriginAndVector (xyzA, rayA.direction);
        DRay3d rayD = DRay3d::FromOriginAndVector (xyzB, rayB.direction);
        double fC, fD;
        DPoint3d xyzC, xyzD;
        Check::True (DRay3d::ClosestApproachUnboundedRayUnboundedRay (fC, fD, xyzC, xyzD, rayC, rayD), "Detect nonparallel");
        Check::Near (xyzA, xyzC, "closest approach");
        Check::Near (xyzB, xyzD, "closest approach");
        Check::Near (0.0, fC, "fC");
        Check::Near (0.0, fD, "fD");
        }
    Check::EndScope ();
    Check::EndScope ();
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DRay3d, ClosestApproach)
    {
    testClosestApproach (DRay3d::FromOriginAndVector (DPoint3d::From (1,2,3), DVec3d::From (0,1,0)),
            DRay3d::FromOriginAndVector (DPoint3d::From (4,0,0), DVec3d::From (1,0,0)));
    testClosestApproach (DRay3d::FromOriginAndVector (DPoint3d::From (1,2,3), DVec3d::From (0.4,1.1,0.4)),
            DRay3d::FromOriginAndVector (DPoint3d::From (4,0,0), DVec3d::From (1,0,0)));
    testClosestApproach (DRay3d::FromOriginAndVector (DPoint3d::From (1,2,3), DVec3d::From (5,1,-2)),
            DRay3d::FromOriginAndVector (DPoint3d::From (4,2,1), DVec3d::From (1.2,0.1,0.5)));
    }

void testRayCircleIntersectionXY (double cx, double cy, double r, double theta0, double theta1)
    {
    DPoint3d center = DPoint3d::From (cx, cy);
    Check::StartScope ("testRayCircleIntersectionXY");
    Check::StartScope ("r", r);
    Check::StartScope ("center", center);    
    DPoint2d xy0C = DPoint2d::From (cx + cos (theta0) * r, cy + sin (theta0) * r);
    DPoint2d xy1C = DPoint2d::From (cx + cos (theta1) * r, cy + sin (theta1) * r);
    DRay3d ray = DRay3d::FromOriginAndTarget (xy0C, xy1C);
    double params[2];
    DPoint3d points[2];
    if (Check::Int (2, ray.IntersectCircleXY (points, params, center, r), "numIntersection"))
        {
        Check::Near (params[0], 0.0, "param0");
        Check::Near (params[1], 1.0, "param1");
        }
    Check::EndScope ();
    Check::EndScope ();
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, IntersectCircleXY)
    {
    testRayCircleIntersectionXY (0,0, 1, 0, Angle::PiOver2 ());
    testRayCircleIntersectionXY (2, 3, 5, 0, 0.3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DRay3d, IntersectPlanes)
    {
    DPlane3d planeA = DPlane3d::FromOriginAndNormal (1,2,3, 0.4, 0.2, 1.2);
    DPlane3d planeB = DPlane3d::FromOriginAndNormal (5,2,0.1, -0.3, 0.2, 0.29);
    DRay3d intersection;
    Check::False (intersection.InitFromPlanePlaneIntersection (planeA, planeA), "Degenerate planeplane intersect");
    if (Check::True(intersection.InitFromPlanePlaneIntersection (planeA, planeB), "planeplane intersect"))
        {
        for (double f = -2.0; f < 3.0; f += 0.5)
            {
            DPoint3d point = intersection.FractionParameterToPoint (f);
            Check::Near (0.0, planeA.Evaluate (point), "point on plane A");
            Check::Near (0.0, planeB.Evaluate (point), "point on plane B");
            }
        }
    
    }

void TestTransverseIntersection (DTriangle3dCR triangle, DRay3dCR ray)
    {
    DPoint3d uvw, xyz;
    double lambda;
    if (Check::True (triangle.TransverseIntersection (ray, xyz, uvw, lambda)))
        {
        DPoint3d xyz1 = triangle.EvaluateBarycentric (uvw);
        DPoint3d xyz2 = ray.FractionParameterToPoint (lambda);
        Check::Near (xyz1, xyz2, "RayTriangle intersection");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DTriangle3d,TransverseIntersection)
    {
    TestTransverseIntersection (
        DTriangle3d (
            DPoint3d::From (0,0,0),
            DPoint3d::From (1,0,0),
            DPoint3d::From (0,1,0)
            ),
        DRay3d::FromOriginAndVector (DPoint3d::From (0.5, 0.5, 2.0), DVec3d::From (0,0,1))
        );
    TestTransverseIntersection (
        DTriangle3d (
            DPoint3d::From (3,4,3),
            DPoint3d::From (1,-3,1),
            DPoint3d::From (9,2,4)
            ),
        DRay3d::FromOriginAndVector (DPoint3d::From (0.3, 0.5, 2.1),    DVec3d::From (2,3,4))
        );
    }
struct DHyperbolicParaboloid4d
{
DPoint4d xyzw00;
DPoint4d xyzw10;
DPoint4d xyzw01;
DPoint4d xyzw11;

DHyperbolicParaboloid4d (
        DPoint3dCR xyz00, double w00,
        DPoint3dCR xyz10, double w10,
        DPoint3dCR xyz01, double w01,
        DPoint3dCR xyz11, double w11
        ) :
        xyzw00(DPoint4d::From (xyz00, w00)),
        xyzw10(DPoint4d::From (xyz10, w10)),
        xyzw01(DPoint4d::From (xyz01, w01)),
        xyzw11(DPoint4d::From (xyz11, w11))
    {
    }

DPoint4d Evaluate (double u, double v)
    {
    return DPoint4d::FromInterpolate (
        DPoint4d::FromInterpolate (xyzw00, u, xyzw10),
        v,
        DPoint4d::FromInterpolate (xyzw01, u, xyzw11)
        );
    }
int Intersect (DRay3dCR ray, DPoint3d* points, double *rayParams, DPoint2d *patchParams)
    {
    DPoint3d xyz00, xyz10, xyz01, xyz11;
    xyzw00.GetXYZ (xyz00);
    xyzw10.GetXYZ (xyz10);
    xyzw01.GetXYZ (xyz01);
    xyzw11.GetXYZ (xyz11);
    return ray.IntersectHyperbolicParaboloid
        (
        points, rayParams, patchParams,
        xyz00, xyzw00.w,
        xyz10, xyzw10.w,
        xyz01, xyzw01.w,
        xyz11, xyzw11.w
        );
    }
};
        
bool Find (double value, double *data, int n, double tol = 1.0e-10)
    {
    for (int i = 0; i < n; i++)
        if (fabs (value - data[i]) <= tol)
            return true;
    return false;
    }
void TestIntersectHyperbolicParboloidA (double u, double v, double w00 = 1.0, double w10 = 1.0, double w01 = 1.0, double w11 = 1.0)
    {
    DHyperbolicParaboloid4d patch (
        DPoint3d::From (1,0.32,1.1), w00,
        DPoint3d::From (3,1.92,1.02), w10,
        DPoint3d::From (0.92,4.1,2.04), w01,
        DPoint3d::From (3,4,2), w11
        );
    DPoint4d xyzw = patch.Evaluate (u,v);
    DRay3d ray;
    xyzw.GetProjectedXYZ (ray.origin);
    ray.direction = DVec3d::From (0.001,0.83,1);
    double f = 0.5;
    ray.origin -= f * ray.direction;
    DPoint2d patchParams[2];
    double rayParams[2];
    DPoint3d points[2];
    int n = patch.Intersect (ray, points, rayParams, patchParams);
    Check::True (Find (f, rayParams, n), "patch root");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d,IntersectHyperbolicParaboloid)
    {
    Check::StartScope ("HyperbolicParaboloid");
    TestIntersectHyperbolicParboloidA (0.43,0.51);
    TestIntersectHyperbolicParboloidA (0.43,0.51, 1.02, 0.94, 0.92, 0.981);
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, Evaluate)
    {
    DSegment3d seg = DSegment3d::From(DPoint3d::From(2, 3, 2), DPoint3d::From(8, 9, 8));
    DRay3d ray = DRay3d::From(seg);
    DPoint3d origin, target;
    ray.EvaluateEndPoints(origin, target);

    Check::Near(seg.point[0], origin);
    Check::Near(seg.point[1], target);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, DotVector)
    {
    DRay3d ray0 = DRay3d::FromOriginAndTarget(DPoint3d::From(2, 2, 2), DPoint3d::From(9, 9, 9)); 
    DVec3d vec = DVec3d::From(DPoint3d::From(8, 8, 8));
    double dotProduct = ray0.DirectionDotVector(vec);
    double dotProductExpected = ray0.direction.DotProduct(vec);
    Check::Near(dotProduct, dotProductExpected);
    }