/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"





bvector<DPoint3d> Quadrant1Rectangle (double a, double b)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (a,0,0));
    points.push_back (DPoint3d::From (a,b,0));
    points.push_back (DPoint3d::From (0,b,0));
    return points;
    }


void CheckPlaneIntersections (PlanePolygonSSICode code, bvector<DPoint3d> const &points, DPlane3dCR plane, size_t n0, size_t n1, char const * name)
    {
    Check::StartScope (name);
    static double touchTolerance = 1.0e-5;
    bvector<CurveLocationDetail> trueCrossings;
    bvector<CurveLocationDetail> touchData;
    DRange1d altitudeRange;
    if (Check::Int ((int)code,
         (int)PolygonOps::PlaneIntersectionPoints (points, plane, touchTolerance, trueCrossings, &touchData, altitudeRange),
         "Transverse Crossings"))
         {
         Check::Size (n0, trueCrossings.size (), "crossing count");
         Check::Size (n1, touchData.size (), "touch count");
         }
    Check::EndScope ();
    }

DPlane3d MakePlane (DPoint3dCR pointA, DPoint3dCR pointB, DVec3dCR vectorC)
    {
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d normal = DVec3d::FromNormalizedCrossProduct (vectorC, vectorAB);
    return DPlane3d::FromOriginAndNormal (pointA, normal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonOps,PlaneIntersection)
    {
    double a = 3;
    double b = 2;
    double trueArea = a * b;
    bvector<DPoint3d> xyRectangle = Quadrant1Rectangle (3,2);
    size_t n = xyRectangle.size ();
    DVec3d areaNormal = PolygonOps::AreaNormal (xyRectangle);
    Check::Near (trueArea, areaNormal.Magnitude (), "areaNormal");
    DPoint3d centroid;
    DVec3d unitNormal;
    double area;
    Check::True (PolygonOps::CentroidNormalAndArea (xyRectangle, centroid, unitNormal, area), "compute area normal");
    Check::Near (trueArea, area, "area B");

    for (size_t i = 0; i < n; i++)
        {
        size_t i1 = (i + 1) % n;
        size_t i2 = (i1 + 1) % n;

        DPoint3d xyzA = DPoint3d::FromInterpolate (xyRectangle[i],   0.2349,  xyRectangle[i1]);
        DPoint3d xyzB = DPoint3d::FromInterpolate (xyRectangle[i1],  0.67,    xyRectangle[i2]);
        DPoint3d xyzD = DPoint3d::FromInterpolate (xyRectangle[i1], -0.1,     xyRectangle[i2]); // OUTSIDE
        DPoint3d xyzE = DPoint3d::FromInterpolate (xyRectangle[i],   1.1,     xyRectangle[i1]); // OUTSIDE
        // a plane through A and B surely has an edge cut and no vertex hits ...
        DPlane3d plane0 = MakePlane (xyzA, xyzB, areaNormal);
        CheckPlaneIntersections (PlanePolygonSSICode::Transverse, xyRectangle, plane0, 2, 0, "Edge to Edge");

        // A plane through B and xyRectangle[i] surely has an edge cut and 1 vertex hit ...
        DPlane3d plane1 = MakePlane (xyRectangle[i], xyzB, areaNormal);
        CheckPlaneIntersections (PlanePolygonSSICode::Transverse, xyRectangle, plane1, 2, 1, "Edge to vertex");

        // A plane through D and xyRectangle[i] surely has no edge cuts and a vertex hit ..
        DPlane3d plane3 = MakePlane (xyRectangle[i], xyzD, areaNormal);
        CheckPlaneIntersections (PlanePolygonSSICode::Transverse, xyRectangle, plane3, 0, 1, "Vertex to outside");

        // A plane through xyRectangle[i] and xyRectangle[i1] no edge cuts and two vertex hits
        DPlane3d plane4 = MakePlane (xyRectangle[i], xyRectangle[i1], areaNormal);
        CheckPlaneIntersections (PlanePolygonSSICode::Transverse, xyRectangle, plane4, 0, 2, "Along edge");

        // A plane through xyRectangle[i] and xyRectangle[i1] no edge cuts and two vertex hits
        DPlane3d plane5 = MakePlane (xyzD, xyzE, areaNormal);
        CheckPlaneIntersections (PlanePolygonSSICode::Transverse, xyRectangle, plane5, 0, 0, "Outside");

        }
    }

bool CheckClosestPoint (DSegment3dCR segment, bool extend0, bool extend1, double fA)
    {
    char message[1024];
    sprintf (message, "ExtendedSegment projetion %d %d (f %g)", extend0 ? 1 : 0, extend1 ? 1 : 0, fA);
    Check::StartScope (message);
    bvector<DPoint3d> points;
    // make a polyline that matches the segment, with an added midpoint...
    points.push_back (segment.point[0]);
    points.push_back (DPoint3d::FromInterpolate (segment.point[0], 0.5, segment.point[1]));
    points.push_back (segment.point[1]);

    DPoint3d pointA;
    // A is true evaluation of the segment.
    segment.FractionParameterToPoint (pointA, fA);

    DPoint3d pointB, pointC;
    double fB, fC, fCEdge;
    size_t edgeIndex, numEdge;
    segment.ProjectPointBounded (pointB, fB, pointA, extend0, extend1);
    PolylineOps::ClosestPoint (points, false, pointA, fC, pointC, edgeIndex, numEdge, fCEdge, extend0, extend1);
    bool  stat = Check::Near (pointB, pointC, "near point");
    stat &= Check::Near (fB, fC, "fraction");
    if (DoubleOps::IsIn01 (fA)
        || (extend0 && fA <= 0.0)
        || (extend1 && fA >= 1.0)
        )
        Check::Near (fA, fB, "fraction");

    ICurvePrimitivePtr prim = ICurvePrimitive::CreateLine (segment);
    CurveLocationDetail detail;
    prim->ClosestPointBounded (pointA, detail, extend0, extend1);
    Check::Near (pointB, detail.point, "curvePrimLine point");
    Check::Near (fB, detail.fraction, "curvePrimLine fraction");

    ICurvePrimitivePtr primE = ICurvePrimitive::CreateLineString (points);
    prim->ClosestPointBounded (pointA, detail, extend0, extend1);
    Check::Near (pointB, detail.point, "curvePrimLineString point");
    Check::Near (fB, detail.fraction, "curvePrimLineString fraction");

    Check::EndScope ();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonOps,Extend01Test)
    {
    DSegment3d segment = DSegment3d::From (1,2,3, 7,11,13);
    // 2 colinear, equal segments so global fractions match segment fractions:
    for (double fA = -2.5; fA < 3.0; fA += 0.25)
        {
        CheckClosestPoint (segment, false, false, fA);
        CheckClosestPoint (segment, false, true, fA);
        CheckClosestPoint (segment, true, false, fA);
        CheckClosestPoint (segment, true, true, fA);
        }

    }
// ASSUME xy0, xy1 are points "on" the convex polygon ...
bool TestPointsOnConvexLine (bvector<DPoint2d> &points, DPoint2dCR xy0, DPoint2dCR xy1)
    {
    for (double f : {-1.0, -0.5, 0.1, 0.6, 1.1, 2.0})
        {
        DPoint2d xy = DPoint2d::FromInterpolate (xy0, f, xy1);
        bool expected = fabs (f - 0.5) <= 0.5;
        if (expected != PolygonOps::IsPointInConvexPolygon (xy, points.data (), (int)points.size (), 0))
            return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonOps,FastConvexInuOut)
    {
    for (size_t n : {4,5,6,12,22})
        {
        bvector<DPoint2d> points;
        SampleGeometryCreator::StrokeUnitCircle (points, n);
        for (size_t i = 1; i + 2 < n; i++)
            {
            DPoint2d xy = DPoint2d::FromInterpolate(points[i], 0.5, points[i+1]);
            if (!Check::True (TestPointsOnConvexLine (points, points[0], xy)))
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonOps,InOutXYTriangle)
    {
    DTriangle3d triangle1 (
        DPoint3d::From (1,2,4),
        DPoint3d::From (4,3,5),
        DPoint3d::From (-1, 7,2));

    DTriangle3d triangle2 = triangle1;
    std::swap (triangle2.point[1], triangle2.point[2]);
    bvector<DPoint3d> points;
    size_t numError = 0;
    // these point coordinates avoid exact edge hits ..
    UnitGridPoints (points, 12, 12, -0.09, -0.09, 0.10, 0.10);
    for (auto &triangle : {triangle1, triangle2})
        for (auto uv : points)
            {
            DPoint3d xyz = triangle.Evaluate (uv.x, uv.y);
            if (DTriangle3d::IsBarycentricInteriorUV (uv.x, uv.y)
                != PolygonOps::IsPointInOrOnXYTriangle (xyz, 
                            triangle.point[0], triangle.point[1], triangle.point[2]))
                numError ++;
            }
    Check::Size (0, numError, "InOutXYTriangle");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolygonOps,InOutXYPolygon)
    {
    bvector<DPoint2d> polygon;
    bvector<DPoint3d> polygon3d;
    bvector<DPoint3d> points;
    auto transform = Transform::From (
                RotMatrix::FromAxisAndRotationAngle (2, 0.1),
                DPoint3d::From (0,0,0));

    for (int numTooth : {1,3})
        {
        SquareWavePolygon (polygon, numTooth, 0, 1, 1, 0, 1, true, -1 );
        SquareWavePolygon (polygon3d, numTooth, 0, 1, 1, 0, 1, true, -1 );
        bvector<DPoint2d> skewPolygon;
        transform.Multiply (skewPolygon, polygon);
        // the polygon and point coordinates will have exact integer hits and alignment with edges
        UnitGridPoints (points, 6 * numTooth + 1, 7, -0.5, -1.5, 0.5, 0.5);
        double tol = 1.0e-10;
        size_t numError = 0;
        size_t numErrorXYZ = 0;
        for (auto uv : points)
            {
            DPoint3d skewPoint = transform * uv;
            auto parity = PolygonOps::PointPolygonParity (DPoint2d::From (uv), polygon, tol);
            auto parity2 = bsiGeom_XYPolygonParity (&uv, polygon3d.data (), (int)polygon3d.size (), tol);
            auto parity1 = PolygonOps::PointPolygonParity (DPoint2d::From (skewPoint), skewPolygon, tol);
            if (parity != parity1)
                numError++;
            if (parity != parity2)
                numErrorXYZ++;
            }
        Check::Size (0, numError, "points in polygon versus rotated");
        Check::Size (0, numErrorXYZ, "points in polygon XY versus XYZ");
        }
    }

int countNegativePoints(DRay3d const &ray, DPoint3d const points[], int numPoints) {
    int n = 0;
    for (int i = 0; i < numPoints; i++) {
        if (points[i].DotDifference (ray.origin, ray.direction) < 0.0)
            n++;
        }
    return n;
    }

// If hasPointInside is false, all output is shifted by yShiftForFalse
// (always) save the triangle edges
// (always) run clip triangle and save the clip.
void SaveClassifiedTriangleResult(bool hasPointInside, DRange3d const & range, DPoint3d const &pointA, DPoint3d const &pointB, DPoint3d const &pointC,
double yShiftForFalse)
    {
    double yShift;
    if (hasPointInside)
        yShift = 0.0;
    else
        yShift = yShiftForFalse;

    Check::Shift(0, yShift, 0);
    bvector<DPoint3d> points{ pointA, pointB, pointC, pointA };
    bvector<DPoint3d> workPoints;
    Check::SaveTransformed(points);
    points.pop_back();
    PolygonOps::ClipConvexPolygonToRange(range, points, workPoints);
    if (points.size() > 0)
        {
        Check::SaveTransformed(points, true);
        }
    Check::Shift(0, -yShift, 0);

    }
// ** shift by xShift ( persistent)
// ** output range edges
// ** output range edges again with temporary yShift
void SaveRangeEdgesWithShifts(double xShift, DRange3dCR range, double yShift)
    {
    DPoint3d corners[8];
    Check::Shift (xShift, 0, 0);
    range.Get8Corners(corners);
    Check::SaveTransformedEdges(range);
    Check::Shift(0, yShift, 0);
    Check::SaveTransformedEdges(range);
    Check::Shift(0, -yShift, 0);
    }
int doTests(DRange3d const & range, DPoint3d const &pointA, DPoint3d const &pointB, DPoint3d const &pointC0, DPoint3d const &pointC1, int numPoints) {
    int numErrors = 0;
    SaveRangeEdgesWithShifts(20, range, 10);
    for (int i = 0; i <= numPoints; i++) {
        auto pointC = DPoint3d::FromInterpolate(pointC0, i / (double)numPoints, pointC1);
        auto classifyBySeparatorPlanes = PolygonOps::TriangleIntersectsRangeBySeparatorPlanes(range, pointA, pointB, pointC);
        auto classifyByClip = PolygonOps::TriangleIntersectsRangeByPlaneClip(range, pointA, pointB, pointC);
        SaveClassifiedTriangleResult (classifyBySeparatorPlanes, range, pointA, pointB, pointC, 10.0);
        if (classifyByClip != classifyBySeparatorPlanes)
            {
            classifyByClip = PolygonOps::TriangleIntersectsRangeByPlaneClip(range, pointA, pointB, pointC);
            numErrors++;
            }
        }
    return numErrors;
    }

TEST(PolygonOps,ClassifyTriangleRange)
    {
    static int s_noisy = 1;
    auto unitRange = DRange3d::From (0, 0, 0, 1, 1, 1);
    Check::Int (0, doTests(unitRange,
        DPoint3d::From(2, 0, 0), DPoint3d::From(2, 1, 0),
        DPoint3d::From(3, 0, 0.5), DPoint3d::From(1, 0, 5),
        4), "ClassifyTriangleRange: easy all `out` cases");
    Check::Int(0, doTests(unitRange,
        DPoint3d::From(0.5, 0.5, 0.5), DPoint3d::From(2, 1, 0),
        DPoint3d::From(3, 0, 0.5), DPoint3d::From(1, 0, 5),
        4), "ClassifyTriangleRange: vertex in range cases");
    Check::Int(0, doTests(unitRange,
        DPoint3d::From(0.9, -0.1, -0.1), DPoint3d::From(1.1, 0.1, -0.1),
        DPoint3d::From(0.5, 0.5, 0.5), DPoint3d::From(0.5, 0.5, 3),
        4), "ClassifyTriangleRange: near-corner cases A");

    Check::Int(0, doTests(unitRange,
        unitRange.LocalToGlobal(0.8, -0.1, -0.1),
        unitRange.LocalToGlobal(1.1, 0.2, -0.1),
        unitRange.LocalToGlobal(1, 0, 0.2), unitRange.LocalToGlobal(1.1, -0.2, 2.0),
        6), "ClassifyTriangleRange: near-corner cases B");


    Check::Int(0, doTests(unitRange,
        DPoint3d::From(2, 0, 0),
        DPoint3d::From(2, 2, 2),
        DPoint3d::From(3, 1, 1),
        DPoint3d::From(1.1, 0.5, 0.5),
        4), "ClassifyTriangleRange: fringe trim cases");
    int num0 = 0;
    int num1 = 0;
    for (double c1u : {1.02, 0.5, 0.02, -0.5})
        {
        for (auto range : {
//            DRange3d::From(0,0,0,1,1,1),
            DRange3d::From(0,0,0,1,2,3),
            DRange3d::From(1,2,3, 10,9,8) })
            {
            if (s_noisy > 2)
                SaveRangeEdgesWithShifts(20, range, 10);
            auto pointA0 = range.LocalToGlobal(0.8, -0.1, -0.1);
            auto pointA1 = range.LocalToGlobal(0.8, -0.1, 1.1);
            auto pointB0 = range.LocalToGlobal(1.1, 0.15, -0.1);
            auto pointB1 = range.LocalToGlobal(1.1, 0.15, 1.1);
            auto pointC0 = range.LocalToGlobal(0.95, -0.1, -0.2);
            auto pointC1 = range.LocalToGlobal(c1u, -0.05, 1.2);
            int nI = 4;
            int nJ = 4;
            int nK = 6;
            for (int i = 0; i <= nI; i++)
                {
                auto pointA = DPoint3d::FromInterpolate(pointA0, i / (double)nI, pointA1);
                for (int j = 0; j <= nJ; j++)
                    {
                    auto pointB = DPoint3d::FromInterpolate(pointB0, j / (double)nJ, pointB1);
                    for (int k = 0; k <= nK; k++)
                        {
                        auto pointC = DPoint3d::FromInterpolate(pointC0, k / (double)nK, pointC1);
                        auto classifyBySeparatorPlanes = PolygonOps::TriangleIntersectsRangeBySeparatorPlanes(range, pointA, pointB, pointC);
                        auto classifyByClip = PolygonOps::TriangleIntersectsRangeByPlaneClip(range, pointA, pointB, pointC);
                        bvector<DPoint3d> points = { pointA, pointB, pointC };
                        bvector<DPoint3d> workPoints;
                        PolygonOps::ClipConvexPolygonToRange(range, points, workPoints);
                        if (s_noisy > 2)
                            SaveClassifiedTriangleResult(classifyBySeparatorPlanes, range, pointA, pointB, pointC, 10.0);
                        if (!Check::Bool (points.size() != 0, classifyBySeparatorPlanes, "ClassifyByPlanes vs ClassifyBySeperatorPlanes"))
                            {
                            bvector<DPoint3d> points1 = { pointA, pointB, pointC };
                            PolygonOps::ClipConvexPolygonToRange(range, points1, workPoints);
                            }
                        if (!Check::Bool (classifyBySeparatorPlanes, classifyByClip, "ClassifyByPlanes vs ClassifyByClip"))
                            {
                            classifyBySeparatorPlanes = PolygonOps::TriangleIntersectsRangeBySeparatorPlanes(range, pointA, pointB, pointC);
                            classifyByClip = PolygonOps::TriangleIntersectsRangeByPlaneClip(range, pointA, pointB, pointC);
                            }
                        else
                            {
                            if (classifyByClip == 0)
                                num0++;
                            else
                                num1++;
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry("PolygonOps.TriangleRange");
    }
