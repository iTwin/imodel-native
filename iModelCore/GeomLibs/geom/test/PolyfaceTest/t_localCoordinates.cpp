/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Vu/VuApi.h>
static int s_noisy = 0;
void CopyCyclic (bvector<DPoint3d> &dest, bvector<DPoint3d> const & source, size_t baseIndex, bool close)
    {
    size_t n = source.size ();
    dest.clear ();
    for (size_t i = 0; i < n; i++)
        {
        dest.push_back (source[(baseIndex + i) % n]);
        }
    if (close)
        dest.push_back (source[baseIndex]);
    }

void TestLCS (bvector<DPoint3d> const &points, LocalCoordinateSelect select)
    {
    Transform worldToLocal, localToWorld;
    if (Check::True(PolygonOps::CoordinateFrame (&points[0], points.size (),
                localToWorld, worldToLocal, select), "Polygon LCS"))
        {
        Transform product = Transform::FromProduct (localToWorld, worldToLocal);
        Check::True (product.IsIdentity (), "LCS identity");
        DRange3d localRange;
        localRange.Init ();
        localRange.Extend (worldToLocal, &points[0], (int)points.size ());
        DVec3d direction0 = DVec3d::FromStartEnd (points[0], points[1]);
        DVec3d localX, localY, localZ;
        DPoint3d origin;
        localToWorld.GetOriginAndVectors (origin, localX, localY, localZ);
        Check::True (direction0.IsParallelTo (localX));
        DPoint3d localDiagonal = DVec3d::FromStartEnd (localRange.low, localRange.high);
        if (select == LOCAL_COORDINATE_SCALE_UnitAxesAtStart)
            {
            Check::Near (1.0, localX.Magnitude (), "UnitX");
            Check::Near (1.0, localY.Magnitude (), "UnitY");
            Check::Near (points[0], origin, "Origin");
            }
        else if (select == LOCAL_COORDINATE_SCALE_01RangeBothAxes)
            {
            Check::Near (1.0, localDiagonal.x, "X local size");
            Check::Near (1.0, localDiagonal.y, "Y local size");
            Check::Near (0.0, localRange.low.x, "localOrigin x0");
            Check::Near (0.0, localRange.low.y, "localOrigin y0");
            }
        else if (select == LOCAL_COORDINATE_SCALE_01RangeLargerAxis)
            {
            Check::Near (1.0, DoubleOps::Max (localDiagonal.x, localDiagonal.y), "larger local size");
            Check::Near (0.0, localRange.low.x, "localOrigin x0");
            Check::Near (0.0, localRange.low.y, "localOrigin y0");
            }
        else if (select == LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft)
            {
            Check::Near (1.0, localX.Magnitude (), "UnitX");
            Check::Near (1.0, localY.Magnitude (), "UnitY");
            Check::Near (0.0, localRange.low.x, "localOrigin x0");
            Check::Near (0.0, localRange.low.y, "localOrigin y0");
            }
        }
        
    DPoint3d interiorPoint;
    if (Check::Int (SUCCESS, vu_anyInteriorPointInPolygon (&interiorPoint, &points[0], (int)points.size ()), "Find interior point"))
        {
        CurveVectorPtr cv = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
        Check::Int (CurveVector::INOUT_In, cv->PointInOnOutXY (interiorPoint),
                        "interiorpoint confirmed");
        }
    }

// ASSUME no disconnects or self intersections.
void TestSimplePolygonTriangulation (bvector<DPoint3d> &points)
    {
    bvector<int> indices;
    int numInputVertex = (int)points.size ();
    Check::Int (SUCCESS, vu_triangulateSpacePolygon (&indices,
                    &points[0], numInputVertex,
                    0.0, 3, true), "triangulation");
    int numResultTriangle = (int)indices.size () / 4;
    Check::Int (numInputVertex - 2, numResultTriangle, "Simple triangulation count");
    
    }

void AddPolygonA (bvector<DPoint3d> &points, double ax, double ay, bool addClosure = false)
    {
    points.clear ();
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (ax,0,0));
    points.push_back (DPoint3d::From (ax,ay,0));
    points.push_back (DPoint3d::From (0,ay,0));
    if (addClosure)
        points.push_back (DPoint3d::From (0,0,0));
    }

void AddPolygonB (bvector<DPoint3d> &polygon, bool addClosure = false)
    {
//
//
//  2-----------1
//  |           |
//  |  5--6-----0
//  |    \
//  3-----4
//
    polygon.clear ();
    polygon.push_back (DPoint3d::From (3,1,0));
    polygon.push_back (DPoint3d::From (3,2,0));
    polygon.push_back (DPoint3d::From (0,2,0));
    polygon.push_back (DPoint3d::From (0,0,0));
    polygon.push_back (DPoint3d::From (1,0,0));
    polygon.push_back (DPoint3d::From (0.5,1,0));
    polygon.push_back (DPoint3d::From (1,1,0));
    if (addClosure)
        polygon.push_back (DPoint3d::From (3,1,0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LocalCoordinateScale, Test0)
    {
    bvector<DPoint3d> polygon, testPolygon;
    AddPolygonB (polygon);
    for (size_t i = 0; i < polygon.size (); i++)
        {
        CopyCyclic (testPolygon, polygon, i, true);
        TestLCS (testPolygon, LOCAL_COORDINATE_SCALE_UnitAxesAtStart);
        TestLCS (testPolygon, LOCAL_COORDINATE_SCALE_01RangeBothAxes);
        TestLCS (testPolygon, LOCAL_COORDINATE_SCALE_01RangeLargerAxis);
        TestLCS (testPolygon, LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft);
        }
    TestSimplePolygonTriangulation (polygon);
    }


void TestSweepToSolid (bvector<DPoint3d> &points, DVec3dCR sweepVector, bool triangulate)
    {
    PolyfaceHeaderPtr header = PolyfaceHeader::CreateVariableSizeIndexed ();
    DVec3d areaNormal = PolygonOps::AreaNormal (points);
    double area = areaNormal.Normalize ();
    double expectedVolume = fabs (area * areaNormal.DotProduct (sweepVector));
    header->AddPolygon (points);
    if (triangulate)
        header->Triangulate ();
    size_t numEdges = points.size ();
    if (Check::True (header->SweepToSolid (sweepVector, false), "Sweep to solid"))
        {
        if (Check::True (header->IsClosedByEdgePairing (), "swept mesh is closed"))
            {
            if (Check::Size (2 * numEdges, header->Point ().size (), "sweep doubles point count"))
        {
        double volume = header->SumTetrahedralVolumes (points[0]);
        Check::Near (expectedVolume, volume, "volume");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SweepToSolid, Test0)
    {
    bvector<DPoint3d> points;
    double a = 10.0;
    double b = 15.0;
    double c = 8.0;
    DVec3d sweep0 = DVec3d::From (0,0, c);
    for (int i = 0; i < 2; i++)
        {
        double s = 1.0 - 2 * i;
        DVec3d sweep1;
        AddPolygonA (points, a, b);
        sweep1.Scale (sweep0, s);
        TestSweepToSolid (points, sweep1, false);
        TestSweepToSolid (points, sweep1, true);

        AddPolygonB (points);
        TestSweepToSolid (points, sweep1, false);
        TestSweepToSolid (points, sweep1, true);
        }
    }
void CheckMatchedCubeLengths (bvector<DPoint3d> const &xyz,
size_t i0, size_t i1,
size_t k0, size_t k1
)
    {
    double a = xyz[i0].Distance (xyz[i1]);
    Check::Near (a, xyz[k0].Distance(xyz[k1]), "edge lengths", xyz[i0].Magnitude ());   // force larger tolerance than usual
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SlabSystem,Test0)
    {
    // This is a slab, measuring a few centimeters along edges.
    // It is located somewhere on pluto.
    bvector<DPoint3d> gistPoints
        {
        DPoint3d::From (1959594.0968472199, 1360918.9904422471, 350.07819693377763),
        DPoint3d::From (1959594.1208769109, 1360918.9525495467, 350.10003864012816),
        DPoint3d::From (1959594.1126007438, 1360918.9473012078, 350.10003864012816),
        DPoint3d::From (1959594.0885710530, 1360918.9851939082, 350.07819693377763),
        DPoint3d::From (1959594.0909363297, 1360918.9997631989, 350.10087066142205),
        DPoint3d::From (1959594.0826601628, 1360918.9945148600, 350.10087066142205),
        DPoint3d::From (1959594.1149660205, 1360918.9618704983, 350.12271236777258),
        DPoint3d::From (1959594.1066898536, 1360918.9566221593, 350.12271236777258),
        };

    // verify expected axis length matches ..
    CheckMatchedCubeLengths (gistPoints, 0, 4, 3, 5);
    CheckMatchedCubeLengths (gistPoints, 0, 3, 4, 5);
    CheckMatchedCubeLengths (gistPoints, 1, 6, 2, 7);


    bvector<int> gistIndices
        {
        1, 2, -3, 0,
        3, 4, -1, 0,
        5, 1, -4, 0,
        4, 6, -5, 0,
        7, 5, -6, 0,
        6, 8, -7, 0,
        7, 8, -3, 0,
        3, 2, -7, 0,
        7, 2, -1, 0,
        1, 5, -7, 0,
        8, 6, -4, 0,
        4, 3, -8, 0,
        };

    bvector<DPoint3d> midPoints {
            DPoint3d::FromInterpolate (gistPoints[0], 0.5, gistPoints[7]),
            DPoint3d::FromInterpolate (gistPoints[1], 0.5, gistPoints[5]),
            DPoint3d::FromInterpolate (gistPoints[2], 0.5, gistPoints[4]),
            DPoint3d::FromInterpolate (gistPoints[3], 0.5, gistPoints[6])
            };

    bvector<DPoint3d> allOrigins;
    allOrigins.push_back (DPoint3d::From (0,0,0));
    for (auto xyz : gistPoints) allOrigins.push_back (xyz);
    for (auto xyz : midPoints) allOrigins.push_back (xyz);

    DRange3d gistRange = DRange3d::From (gistPoints);
    DRange3d midRange = DRange3d::From (midPoints);
    if (s_noisy)
        {
        Check::Print(midRange.DiagonalDistance (), "Diagonal range size for nominally equal midpoints");
        Check::Print(midRange.DiagonalDistance () / gistRange.DiagonalDistance (), "Diagonal range ratio for nominally equal midpoints");
        }
    // The principal axes for a slab are parallel to edges.
    // But their order and sense have multiple possibilities
    // This construction 
    auto xEdge = DVec3d::FromStartEnd (gistPoints[1], gistPoints[0]).ValidatedNormalize ();
    auto yEdge = DVec3d::FromStartEnd (gistPoints[3], gistPoints[0]).ValidatedNormalize ();
    auto zEdge = DVec3d::FromStartEnd (gistPoints[0], gistPoints[4]).ValidatedNormalize ();
    auto axesFromPoints = RotMatrix::FromColumnVectors (yEdge, xEdge, zEdge);
    Check::Print (axesFromPoints, "Direct axes");

    for (auto origin : allOrigins)
       {
        auto mesh = PolyfaceHeader::CreateIndexedMesh (0, gistPoints, gistIndices);
        //UNUSED auto transform = Transform::From (-origin.x, -origin.y, -origin.z);
        DPoint3d centroid;
        DVec3d   moments;
        RotMatrix axesFromMesh;
        double area;
        mesh->ComputePrincipalAreaMoments (area, centroid, axesFromMesh, moments);
        double axisDiff = axesFromMesh.MaxDiff (axesFromPoints);
        if (s_noisy)
            {
            Check::PrintIndent (0);
            Check::Print (origin, "origin");
            Check::Print (moments, "moments");
            Check::Print (axesFromMesh, "axes");
            Check::PrintIndent (4);
            Check::Print (axesFromMesh.MaxDiff (axesFromPoints), "Axes diff");
            }
        Check::Near (axisDiff, 0.0, "AxisDiff", 10000.0);    // accept 9 digits
       }
     }
