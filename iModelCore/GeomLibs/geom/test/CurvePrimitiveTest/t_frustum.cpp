/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include "frustumClipper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FrustumClip, Test1)
    {
    DPoint3d frustumPointsA[8];
    DRange3d rangeA;
    DRange3d rangeB;
    double a0 = 1.0;
    double a1 = 3.0;
    rangeA.low = DPoint3d::From (a0, a0, a0);
    rangeA.high = DPoint3d::From (a1, a1, a1);
    rangeA.Get8Corners (frustumPointsA);

    double b0 = 0.5;
    double b1 = 2.0;
    rangeB.low = DPoint3d::From (b0, b0, b0);
    rangeB.high = DPoint3d::From (b1, b1, b1);
    
    DPoint3d intersections[12][9];
    int intersectionCounts[12];
    int numIntersectionFace;
    DPoint3d rangeBCorners[8];
    rangeB.Get8Corners (rangeBCorners);
    LexicalFrustumIntersection (intersections, intersectionCounts, &numIntersectionFace, frustumPointsA, rangeBCorners);
    Check::Int (6, numIntersectionFace, "range intersection");
    double c0 = a0 > b0 ? a0 : b0;  //larger
    double c1 = a1 < b1 ? a1 : b1;  // smaller
    double expectedArea = (c1 - c0) * (c1 - c0);
    double area;
    DPoint3d origin;
    DVec3d normal;
    for (int i = 0; i < numIntersectionFace; i++)
        {
        area = bsiPolygon_polygonNormalAndArea (&normal, &origin, intersections[i], intersectionCounts[i]);
        Check::Near (expectedArea, area, "inetersection area");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConvexHull, Test1)
    {
    bvector<DPoint3d> xyz;
    bvector<DPoint3d> xyzHull;
    double r1 = 1.0;
    double r2 = 0.5;
    double dTheta1 = 0.3;
    double dTheta2 = 0.83;
    size_t num1 = 20;
    size_t num2 = 3;
    double theta2 = 0.0;
    double theta1 = 0.0;
    DPoint3d center = DPoint3d::From (0.1, 5.0);
    for (size_t i = 0; i < num1; i++, theta1 += dTheta1)
        {
        xyz.push_back (DPoint3d::From (center.x + r1 * cos(theta1), center.y + r1 * sin (theta1), 0.0));
        for (size_t k = 0; k < num2; k++, theta2 += dTheta2)
            xyz.push_back (DPoint3d::From (center.x + r2 * cos(theta2), center.y + r2 * sin (theta2), 0.0));
        }
    // The convex hull must contain all num1 points at r1.
    xyzHull.resize (xyz.size ());
    int numOut;
    bsiDPoint3dArray_convexHullXY (&xyzHull[0], &numOut, &xyz[0], (int)xyz.size ());
    Check::Int ((int)num1, numOut, "Convex hull point count");
    for (size_t i = 0; i < (size_t)numOut; i++)
        {
        Check::Near (r1, xyzHull[i].Distance (center), "Hull point at radius r1");
        }
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TestEllipticIntegralAveragingStep
static double EllipticAngle (double x, double y, double z)
    {
    if (x < y)
        {
        return acos (x/y) / sqrt (y - x);
        }
    else if (x == y)
        {
        return 1/ sqrt (y);
        }
    else
        {
        return acosh (sqrt (x/y))/ sqrt (x-y);
        }
    }


// ASSUME x < y . .
void testEllipticIntegralIteration_xLTy_div4 (double x, double y, double z, int numSteps)
    {
    GEOMAPI_PRINTF ("  div4 elliptic iteration from %.6g,%.6g,%.6g (max %d)\n", x, y, z, numSteps);
    double x0 = x, y0 = y, z0 = z, q0, q;
    q0 = EllipticAngle (x, y, z);
    double a = 0.25;
    for (int i = 0; i < numSteps; i++, x0 = x, y0 = y, z0 = z, q0 = q)
        {
        double lambda = sqrt (x * y) + sqrt (y * z) + sqrt (x * z);
        x = (x + lambda) * a;
        y = (y + lambda) * a;
        z = (z + lambda) * a;
        q = EllipticAngle (x, y, z);
        GEOMAPI_PRINTF ("%3d (xyz %.17g, %.17g, %.17g) (delta %.2le, %.2le, %.2le) (q %.17g) (dq %.2le)\n", i, x, y, z, x - x0, y - y0, z - z0, q, q - q0);
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(EllipticIntegralIterations,Test1)
    {
    testEllipticIntegralIteration_xLTy_div4 (1,2,3, 20);
    testEllipticIntegralIteration_xLTy_div4 (2,2.1,2.2, 20);
    }
#endif