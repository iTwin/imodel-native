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
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  10/17
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