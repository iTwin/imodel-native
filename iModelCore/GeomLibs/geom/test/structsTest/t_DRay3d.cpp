/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdlib.h>
#include <chrono>

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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, IntersectCircleXY)
    {
    testRayCircleIntersectionXY (0,0, 1, 0, Angle::PiOver2 ());
    testRayCircleIntersectionXY (2, 3, 5, 0, 0.3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d,IntersectHyperbolicParaboloid)
    {
    Check::StartScope ("HyperbolicParaboloid");
    TestIntersectHyperbolicParboloidA (0.43,0.51);
    TestIntersectHyperbolicParboloidA (0.43,0.51, 1.02, 0.94, 0.92, 0.981);
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, DotVector)
    {
    DRay3d ray0 = DRay3d::FromOriginAndTarget(DPoint3d::From(2, 2, 2), DPoint3d::From(9, 9, 9));
    DVec3d vec = DVec3d::From(DPoint3d::From(8, 8, 8));
    double dotProduct = ray0.DirectionDotVector(vec);
    double dotProductExpected = ray0.direction.DotProduct(vec);
    Check::Near(dotProduct, dotProductExpected);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
/** Return a random number between -100 and 100 */
int getRandomNumber()
    {
    return rand() % 200 - 100;
    }
TEST(DRay3d, IntersectTriangle)
    {
    DPoint3d origin;
    DVec3d direction;
    DRay3d ray;
    DPoint3d trianglePoints[3];
    DPoint3d intersection;
    DPoint3d expectedIntersection;
    DVec3d rotatedDirection;
    DRay3d rotatedRay;
    DPoint3d rotatedTrianglePoints[3];
    DPoint3d rotatedIntersectionPoint;
    DPoint3d rotatedOriginalIntersectionPoint;
    RotMatrix rotationMatrix;
    double angle = getRandomNumber();
    DVec3d rotationAxis = DVec3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
    if (0 != rotationAxis.Magnitude())
        {
        rotationMatrix.InitIdentity();
        }
    else
        {
        rotationMatrix = RotMatrix::FromVectorAndRotationAngle(rotationAxis, angle);
        }
    origin = DPoint3d::From(3, 3, -5);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    trianglePoints[0] = DPoint3d::From(2, 0, 0);
    trianglePoints[1] = DPoint3d::From(10, 0, 0);
    trianglePoints[2] = DPoint3d::From(2, 10, 0);
    expectedIntersection = DPoint3d::From(3, 3, 0);
    if (bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints))
    {
        // ray intersects triangle at a triangle point
        Check::Near(intersection, expectedIntersection, "ray intersects triangle at a point.");
        rotationMatrix.Multiply(rotatedDirection, direction);
        rotatedRay = DRay3d::FromOriginAndVector(origin, rotatedDirection);
        rotationMatrix.Multiply(rotatedTrianglePoints[0], trianglePoints[0]);
        rotationMatrix.Multiply(rotatedTrianglePoints[1], trianglePoints[1]);
        rotationMatrix.Multiply(rotatedTrianglePoints[2], trianglePoints[2]);

        if (bsiDRay3d_intersectTriangleFast(&rotatedRay, &rotatedIntersectionPoint, rotatedTrianglePoints))
            {
            rotationMatrix.Multiply(rotatedOriginalIntersectionPoint, intersection);
            // rotating original intersection points gives rotated intersection points
            Check::Near(
                rotatedIntersectionPoint,
                rotatedOriginalIntersectionPoint,
                "rotating original intersection points gives rotated intersection points."
            );
            }
        else
            {
            Check::Fail("rotating original intersection points gives rotated intersection points.");
            }
        }
    else
        {
        Check::Fail("ray intersects triangle at a point.");
        }
    origin = DPoint3d::From(2, 0, -2);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    expectedIntersection = DPoint3d::From(2, 0, 0);
    if (bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints))
        {
        // ray intersects triangle at a triangle vertex
        Check::Near(intersection, expectedIntersection, "ray intersects triangle at a triangle vertex.");
        rotationMatrix.Multiply(rotatedDirection, direction);
        rotatedRay = DRay3d::FromOriginAndVector(origin, rotatedDirection);
        rotationMatrix.Multiply(rotatedTrianglePoints[0], trianglePoints[0]);
        rotationMatrix.Multiply(rotatedTrianglePoints[1], trianglePoints[1]);
        rotationMatrix.Multiply(rotatedTrianglePoints[2], trianglePoints[2]);

        if (bsiDRay3d_intersectTriangleFast(&rotatedRay, &rotatedIntersectionPoint, rotatedTrianglePoints))
            {
            rotationMatrix.Multiply(rotatedOriginalIntersectionPoint, intersection);
            // rotating original intersection points gives rotated intersection points
            Check::Near(
                rotatedIntersectionPoint,
                rotatedOriginalIntersectionPoint,
                "rotating original intersection points gives rotated intersection points."
                );
            }
        else
            {
            Check::Fail("rotating original intersection points gives rotated intersection points.");
            }
        }
    else
        {
        Check::Fail("ray intersects triangle at a triangle vertex.");
        }
    origin = DPoint3d::From(5, 0, -2);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    expectedIntersection = DPoint3d::From(5, 0, 0);
    if (bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints))
        {
        // ray intersects triangle at a triangle edge
        Check::Near(intersection, expectedIntersection, "ray intersects triangle at a triangle edge.");
        rotationMatrix.Multiply(rotatedDirection, direction);
        rotatedRay = DRay3d::FromOriginAndVector(origin, rotatedDirection);

        if (bsiDRay3d_intersectTriangleFast(&rotatedRay, &rotatedIntersectionPoint, rotatedTrianglePoints))
            {
            rotationMatrix.Multiply(rotatedOriginalIntersectionPoint, intersection);
            // rotating original intersection points gives rotated intersection points
            Check::Near(
                rotatedIntersectionPoint,
                rotatedOriginalIntersectionPoint,
                "rotating original intersection points gives rotated intersection points."
            );
            }
        else
            {
            Check::Fail("rotating original intersection points gives rotated intersection points.");
            }
        }
    else
        {
        Check::Fail("ray intersects triangle at a triangle edge.");
        }
    origin = DPoint3d::From(5, 0, -2);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    trianglePoints[0] = DPoint3d::From(2, 0, 0);
    trianglePoints[1] = DPoint3d::From(2, 0, 0);
    trianglePoints[2] = DPoint3d::From(2, 10, 0);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when we have a degenerate triangle with two equal vertexes."
    );
    origin = DPoint3d::From(5, 0, -2);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    trianglePoints[0] = DPoint3d::From(2, 0, 0);
    trianglePoints[1] = DPoint3d::From(2, 0, 0);
    trianglePoints[2] = DPoint3d::From(2, 0, 0);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when we have a degenerate triangle with three equal vertexes."
    );
    origin = DPoint3d::From(0, 0, 0);
    direction = DVec3d::From(1, 1, 0);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    trianglePoints[0] = DPoint3d::From(2, 0, 0);
    trianglePoints[1] = DPoint3d::From(10, 0, 0);
    trianglePoints[2] = DPoint3d::From(2, 10, 0);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when ray and triangle are co-planer."
    );
    origin = DPoint3d::From(0, 0, 1);
    direction = DVec3d::From(1, 1, 0);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when ray and triangle are parallel."
    );
    origin = DPoint3d::From(0, 0, 0);
    direction = DVec3d::From(0, 0, 0);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when ray direction is (0,0,0)."
    );
    origin = DPoint3d::From(11, 0, 0);
    direction = DVec3d::From(1, 1, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "expect no intersection when ray intersects triangle at ray origin."
    );
    origin = DPoint3d::From(5, 5, 5);
    direction = DVec3d::From(0, 0, 1);
    ray = DRay3d::FromOriginAndVector(origin, direction);
    Check::False(
        bsiDRay3d_intersectTriangleFast(&ray, &intersection, trianglePoints),
        "ray intersects triangle behind the ray origin."
    );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d, IntersectTriangleAccuracyAndPerformance)
    {
    if (!Check::GetEnableLongTests())
        return;
    const int N = 1000; // N*N is the number of times we shoot a random ray to a random triangle
    bool exitTheTest = false;
    DPoint3d origin;
    DVec3d direction;
    DRay3d ray;
    DPoint3d trianglePoints[3];
    DPoint3d intersectionPointSlow;
    DPoint3d intersectionPointFast;
    DPoint3d barycentric;
    double rayParameter;
    bool slowRet = false;
    bool fastRet = false;
    int hits = 0;
    long long timeBySlowFunction = 0;
    long long timeByFastFunction = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop;
    DVec3d rotatedDirection;
    DRay3d rotatedRay;
    DPoint3d rotatedTrianglePoints[3];
    DPoint3d rotatedIntersectionPoint;
    DPoint3d rotatedOriginalIntersectionPoint;
    RotMatrix rotationMatrix;
    double angle = getRandomNumber();
    DVec3d rotationAxis = DVec3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
    if (0 != rotationAxis.Magnitude())
        {
        rotationMatrix.InitIdentity();
        }
    else
        {
        rotationMatrix = RotMatrix::FromVectorAndRotationAngle(rotationAxis, angle);
        }
    for (int i = 0; i < N && !exitTheTest; i++)
        for (int j = 0; j < N && !exitTheTest; j++)
        {
            origin = DPoint3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
            direction = DVec3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
            ray = DRay3d::FromOriginAndVector(origin, direction);
            trianglePoints[0] = DPoint3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
            trianglePoints[1] = DPoint3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
            trianglePoints[2] = DPoint3d::From(getRandomNumber(), getRandomNumber(), getRandomNumber());
            // shoot ray at triangle using bsiDRay3d_intersectTriangle
            start = std::chrono::high_resolution_clock::now();
            slowRet = bsiDRay3d_intersectTriangle(
                &ray, &intersectionPointSlow, &barycentric, &rayParameter, trianglePoints
            );
            stop = std::chrono::high_resolution_clock::now();
            timeBySlowFunction = timeBySlowFunction + std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
            // shoot ray at triangle using bsiDRay3d_intersectTriangleFast
            start = std::chrono::high_resolution_clock::now();
            fastRet = bsiDRay3d_intersectTriangleFast(&ray, &intersectionPointFast, trianglePoints);
            stop = std::chrono::high_resolution_clock::now();
            timeByFastFunction = timeByFastFunction + std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
            // if ray hits the triangle via bsiDRay3d_intersectTriangle
            if (slowRet && rayParameter > 0
                && barycentric.x >= -Angle::TinyAngle() && barycentric.x <= 1 + Angle::TinyAngle()
                && barycentric.y >= -Angle::TinyAngle() && barycentric.y <= 1 + Angle::TinyAngle()
                && barycentric.z >= -Angle::TinyAngle() && barycentric.z <= 1 + Angle::TinyAngle())
                {
                if (fastRet) // if ray hits the triangle via bsiDRay3d_intersectTriangleFast
                    {
                    Check::Near(
                        intersectionPointSlow,
                        intersectionPointFast,
                        "intersection points calculated by slow and fast functions are equal."
                    );
                    hits++;
                    rotationMatrix.Multiply(rotatedDirection, direction);
                    rotatedRay = DRay3d::FromOriginAndVector(origin, rotatedDirection);
                    rotationMatrix.Multiply(rotatedTrianglePoints[0], trianglePoints[0]);
                    rotationMatrix.Multiply(rotatedTrianglePoints[1], trianglePoints[1]);
                    rotationMatrix.Multiply(rotatedTrianglePoints[2], trianglePoints[2]);
                    // shoot rotated ray at rotated triangle
                    if (bsiDRay3d_intersectTriangleFast(&rotatedRay, &rotatedIntersectionPoint, rotatedTrianglePoints))
                        {
                        rotationMatrix.Multiply(rotatedOriginalIntersectionPoint, intersectionPointFast);
                        Check::Near(
                            rotatedOriginalIntersectionPoint,
                            rotatedIntersectionPoint,
                            "rotating original intersection points gives rotated intersection points."
                        );
                        }
                    else
                        {
                        Check::Fail("rotated ray hits rotated triangle while original ray did not hit original triangle.");
                        exitTheTest = true;
                        }
                    }
                else
                    {
                    Check::Fail("slow function reported ray intersection while fast function did not.");
                    Check::Fail("Please fix me: reproduce with below inputs and investigate.");
                    Check::Print(ray, "ray");
                    Check::Print(trianglePoints[0], "trianglePoints[0]");
                    Check::Print(trianglePoints[1], "trianglePoints[1]");
                    Check::Print(trianglePoints[2], "trianglePoints[2]");
                    exitTheTest = true;
                    }
                }
            else
                if (fastRet)
                    {
                    Check::Fail("fast function reported ray intersection while slow function did not.");
                    Check::Fail("Please fix me: reproduce with below inputs and investigate.");
                    Check::Print(ray, "ray");
                    Check::Print(trianglePoints[0], "trianglePoints[0]");
                    Check::Print(trianglePoints[1], "trianglePoints[1]");
                    Check::Print(trianglePoints[2], "trianglePoints[2]");
                    exitTheTest = true;
                    }
            }
    if (!exitTheTest)
        {
        printf("%i intersections happened out of %i shoots \n", hits, N * N);
        printf("Calls to bsiDRay3d_intersectTriangle     took %llu nanoseconds \n", timeBySlowFunction);
        printf("Calls to bsiDRay3d_intersectTriangleFast took %llu nanoseconds \n", timeByFastFunction);
        }
    }

TEST(DRay3d, IntersectRayOriginOnTriangle)
    {
    const int numSamples = 6;
    DPoint3d origin[numSamples] = {
        DPoint3d::From(-2, 40, 82), DPoint3d::From(2, 70, 0), DPoint3d::From(-32, 6, -43),
        DPoint3d::From(13, -59, 2), DPoint3d::From(47, 0, -85), DPoint3d::From(-61, -79, 48),
    };
    DVec3d direction[numSamples] = {
        DVec3d::From(-5, 41, -2), DVec3d::From(31, 90, 66), DVec3d::From(-93, -82, -12),
        DVec3d::From(-6, 0, 23), DVec3d::From(-38, -74, 51), DVec3d::From(61, 61, 64),
    };
    DPoint3d triangles[numSamples][3] = {
        {DPoint3d::From(9, -26, 93), DPoint3d::From(75, 36, 93), DPoint3d::From(-87, 92, 63)},
        {DPoint3d::From(-17, 39, 5), DPoint3d::From(73, 84, -30), DPoint3d::From(-55, 94, 28)},
        {DPoint3d::From(-58, -28, -82), DPoint3d::From(-20, 4, -23), DPoint3d::From(-50, 9, -73)},
        {DPoint3d::From(56, -29, -90), DPoint3d::From(6, -1, 89), DPoint3d::From(13, -59, 2)},
        {DPoint3d::From(64, -39, -84), DPoint3d::From(-50, -100, 17), DPoint3d::From(-47, -10, -16)},
        {DPoint3d::From(57, 10, -92), DPoint3d::From(-10, -28, -85), DPoint3d::From(-42, -60, 79)},
    };
    DRay3d ray[numSamples]{};
    bool slowRet = false;
    bool fastRet = false;
    DPoint3d intersectionPointSlow;
    DPoint3d intersectionPointFast;
    DPoint3d barycentric;
    double rayParameter;

    for (int i = 0; i < numSamples; i++)
        {
        ray[i] = DRay3d::FromOriginAndVector(origin[i], direction[i]);
        slowRet = bsiDRay3d_intersectTriangle(&ray[i], &intersectionPointSlow, &barycentric, &rayParameter, triangles[i]);
        fastRet = bsiDRay3d_intersectTriangleFast(&ray[i], &intersectionPointFast, triangles[i]);

        Check::True(slowRet);
        Check::True(fastRet);
        Check::Near(
            intersectionPointSlow,
            intersectionPointFast,
            "intersection points calculated by slow and fast functions are equal."
        );
        }
    }
