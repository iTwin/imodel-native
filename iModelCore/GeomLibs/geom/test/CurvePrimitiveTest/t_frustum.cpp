/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include "frustumClipper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(EllipticIntegralIterations,Test1)
    {
    testEllipticIntegralIteration_xLTy_div4 (1,2,3, 20);
    testEllipticIntegralIteration_xLTy_div4 (2,2.1,2.2, 20);
    }
#endif
// compute planes between successive points (incuding cyclic closure)
// omit any points too close to form a plane.
void computeNormalizedPlanes(bvector<DPoint3d> const &points, bvector<DPlane3d> &planes)
    {
    planes.clear();
    size_t n = points.size();
    for (size_t i = 0; i < n; i++)
        {
        size_t j = (i + 1) % n;
        DVec3d vectorIJ = points[i] - points[j];
        DVec3d perp = DVec3d::FromCCWPerpendicularXY(vectorIJ);
        auto unitPerp = perp.ValidatedNormalize();
        if (unitPerp.IsValid())
            {
            planes.push_back(DPlane3d::FromOriginAndNormal(points[i], unitPerp));
            }
        }
    }
void CheckHull(bvector<DPoint3d> const &xyzHull, bvector<DPoint3d> &xyz)
    {
    bvector<DPlane3d> hullPlanes;
    computeNormalizedPlanes(xyzHull, hullPlanes);
    DRange1d altitudeRange;
    size_t i0, i1;
    for (auto &plane : hullPlanes)
        {
        auto range = plane.EvaluateRange(xyz, i0, i1);
        altitudeRange.Extend(range);
        }
    double tolerance = 1.0e-12;
    Check::LessThanOrEqual(altitudeRange.high, tolerance, "point range wrt all convex hull planes");
    }
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
    DPoint3d center = DPoint3d::From(0.1, 5.0);
    for (size_t i = 0; i < num1; i++, theta1 += dTheta1)
        {
        xyz.push_back(DPoint3d::From(center.x + r1 * cos(theta1), center.y + r1 * sin(theta1), 0.0));
        for (size_t k = 0; k < num2; k++, theta2 += dTheta2)
            xyz.push_back(DPoint3d::From(center.x + r2 * cos(theta2), center.y + r2 * sin(theta2), 0.0));
        }
    // The convex hull must contain all num1 points at r1.
    xyzHull.resize(xyz.size());
    int numOut;
    bsiDPoint3dArray_convexHullXY(&xyzHull[0], &numOut, &xyz[0], (int)xyz.size());
    Check::Int((int)num1, numOut, "Convex hull point count");
    for (size_t i = 0; i < (size_t)numOut; i++)
        {
        Check::Near(r1, xyzHull[i].Distance(center), "Hull point at radius r1");
        }
    Check::SaveTransformedMarkers(xyz, 0.02);
    xyzHull.resize((size_t)numOut);
    Check::SaveTransformed(xyzHull);
    Check::ClearGeometry("ConvexHull.Test1");
    }
// build an array of angles.
// If sweep is nonzero, throw out any NOT within a window of size (sweep) centered on angles in anglesToKeep.
void doPolarCircles(DVec3dCR shift, bvector<double> radii, double sweep, size_t numPop)
    {
    bvector<DPoint3d> xyz;
    double dTheta = 0.25;
    bvector<double> allAngles;
    for (double theta = -Angle::Pi(); theta < 6.5; theta += dTheta, dTheta *= 1.002)
        allAngles.push_back(theta);

    bvector<double> problemAngles;
    // a little tricky to pick out problem angles ...
    bvector<double> anglesToKeep{ Angle::DegreesToRadians(-35.464) , Angle::DegreesToRadians(170.0) };

    if (sweep == 0.0)
        {
        problemAngles = allAngles;
        }
    else
        {
        problemAngles.push_back(Angle::PiOver2());
        for (auto theta : allAngles)
            {
            for (double thetaB : anglesToKeep)
                {
                double thetaA = thetaB - 0.5 * sweep;
                if (Angle::InSweepAllowPeriodShift(theta, thetaA, sweep)
                    || Angle::InSweepAllowPeriodShift(theta + Angle::Pi(), thetaA, sweep))
                    {
                    problemAngles.push_back(theta);
                    problemAngles.push_back(-theta);
                    break;
                    }
                }
            }
        problemAngles.push_back(Angle::Pi());
        }
    double rMax = 0.0;
    for (double r : radii)
        {
        rMax = DoubleOps::Max(r, rMax);
        for (double theta : problemAngles)
            {
            xyz.push_back(DPoint3d::From(r * cos(theta), r * sin(theta)));
            xyz.push_back(DPoint3d::From(-r * cos(theta), -r * sin(theta)));
            }
        }
    for (auto &point : xyz)
        point.Add(shift);
    for (size_t i = 0; i < numPop; i++)
        xyz.pop_back();

    bvector<DPoint3d> xyzHull;
    DPoint3dOps::ConvexHullXY(xyz, xyzHull);
    bvector<DPoint3d> xyzHullB (2 * xyz.size ());
    int numB;
    bsiDPoint3dArray_convexHullXY (xyzHullB.data (), &numB, xyz.data (), (int)xyz.size ());
    if (Check::Int (numB, (int)xyzHull.size (), "wrapper count"))
        {
        for (size_t i = 0; i < xyzHull.size (); i++)
            Check::Near (xyzHull[i], xyzHullB[i]);
        }
    Check::SaveTransformedMarkers(xyz, 0.02);
    Check::SaveTransformed(xyzHull);
    CheckHull(xyzHull, xyz);
    }
TEST(ConvexHull, PolarCircles)
    {
    Check::Shift(30, 0, 0);
    double rMax = 3.7;
    bvector<double> radiiA{ 0.2, 0.5, 0.6, 0.8, 1.5, 1.0, 2.3, 2.7, 3.5, rMax };
    bvector<double> radiiB{ 0.2, 1.0, 3.5, rMax };

    bvector<DVec3d> shifts{
            DVec3d::From(0,0,0),
            DVec3d::From(1024, 1024),
            DVec3d::From(3.324234827980e6, 1.2312312e6)
        };
    for (uint32_t i = 0; i < shifts.size(); i++)
        {
        auto shift = shifts[i];
        Check::SetTransform(Transform::FromIdentity());
        for (double sweep : {0.10, 0.30, 0.50, 0.0})
            {
            SaveAndRestoreCheckTransform s(0, 12, 0);
            doPolarCircles(shift, radiiA, sweep, 0);
            Check::Shift(20, 0, 0);
            doPolarCircles(shift, radiiB, sweep, 1);
            Check::Shift(20, 0, 0);
            doPolarCircles(shift, radiiB, sweep, 5);
            Check::Shift(20, 0, 0);
            }
        char filename[1024];
        snprintf(filename, sizeof(filename), "ConvexHull.PolarCircles%d", i);
        Check::ClearGeometry(filename);
        }
    }

TEST(ConvexHull, GridTest)
    {
    // fully symmetric grid tickles sort logic

    for (int gridCount : {5, 4, 3, 2})
        {
        SaveAndRestoreCheckTransform shifter(0, 4.0 * gridCount, 0);
        for (size_t numTrim : {0, 1, 17})
            {
            bvector<DPoint3d> xyz;
            bvector<DPoint3d> xyzHull;

            for (double x = -gridCount; x <= gridCount; x++)
                {
                for (double y = -gridCount; y <= gridCount; y++)
                    {
                    xyz.push_back(DPoint3d::From(x, y, 0));
                    }
                }
            if (numTrim > xyz.size() / 2)
                numTrim = xyz.size() / 2;
            for (size_t i = 0; i < numTrim; i++)
                xyz.pop_back();
            for (auto &xyz1 : xyz)
                Check::SaveTransformedMarker(xyz1, 0.02);
            DPoint3dOps::ConvexHullXY (xyz, xyzHull);
            CheckHull(xyzHull, xyz);
            Check::SaveTransformed(xyzHull);
            Check::Shift(4 * gridCount, 0);
            }
        }
    Check::ClearGeometry("ConvexHull.GridTest");
    }
