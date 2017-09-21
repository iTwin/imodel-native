/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_section.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

#include "SampleGeometry.h"


static PolyfaceHeaderPtr Mesh_TriangulatedBsurf (bvector<bvector<DPoint2d>> &uvTrim, bool outerLoopActive)
    {
    MSBsplineSurfacePtr surface = SurfaceWithSinusoidalControlPolygon 
            (4, 4, 8, 4,  0.0, 0.23, 1.0, 0.67);
    for (size_t i = 0; i < uvTrim.size (); i++)
        {
        surface->AddTrimBoundary (uvTrim[i]);
        }
    surface->SetOuterBoundaryActive (outerLoopActive);
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetNormalsRequired (false);
    options->SetMaxPerFace (3);
    options->SetParamsRequired (false);
    options->SetEdgeChainsRequired (false);
    options->SetParamMode (FACET_PARAM_01BothAxes);
    options->SetMaxEdgeLength (0.2);
    options->SetAngleTolerance (0.15);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->Add (*surface);
    return builder->GetClientMeshPtr ();
    }

void CheckTopology (CurveVectorPtr curves, bool expectRegion, int expectedLinestringCount)
    {
    if (!curves.IsValid ())
        return;
    Check::StartScope ("TopologyProperites");
    Check::Bool (expectRegion, curves->IsAnyRegionType (), "Region");
    if (expectedLinestringCount > 0)
        Check::Int (expectedLinestringCount, (int)curves->CountPrimitivesBelow (), "Primitives");
    Check::EndScope();
    }

// Recusively find all linestrings with location details.  Confirm coordinate evaluations.
void VerifyFacetEdgeLocations (PolyfaceHeaderR mesh, CurveVectorCP curve)
    {
    if (NULL == curve)
        return;
    for (size_t i = 0; i < curve->size (); i++)
        {
        ICurvePrimitiveCP prim = curve->at (i).get ();
        if (NULL != prim)
            {
            bvector <DPoint3d> const *points = prim->GetLineStringCP ();
            FacetEdgeLocationDetailVectorPtr edgeData = prim->GetFacetEdgeLocationDetailVectorPtr ();
            if (edgeData.IsValid () && edgeData->size () == points->size ())
                {
                FacetEdgeLocationDetail data;
                for (size_t i = 0; edgeData->TryGet (i, data); i++)
                    {
                    DPoint3d xyz;
                    if (Check::True (mesh.TryEvaluateEdge (data, xyz), "EvaluateEdge"))
                        Check::Near (points->at(i), xyz, "Edge XYZ");
                    }
                }
            }
        }
    }
// Send two meshes with different triangulation but same surface.
// Verify that section properties match.  Optionally check length.
void VerifySectionMatch (PolyfaceHeaderR meshA, PolyfaceHeaderR meshB, DPlane3dCR plane1,
    bool skipSmallRange,
    bool expectRegion,
    int expectedLinestringCount,
    double expectedLength,
    int noiseLevel)
    {
    DPlane3d plane = plane1;
    plane.normal.Normalize ();
    bvector<DPoint3d> &pointA = meshA.Point ();
    CurveVectorPtr sectionQ = meshA.PlaneSlice (plane, expectRegion, true);
    VerifyFacetEdgeLocations (meshA, sectionQ.get ());
    CurveVectorPtr sectionT = meshB.PlaneSlice (plane, expectRegion, true);
    VerifyFacetEdgeLocations (meshB, sectionT.get ());

    if (noiseLevel > 0)
        {
        Check::PrintStructure (sectionQ.get (), "SectionQ");
        Check::PrintStructure (sectionT.get (), "SectionT");
        }

    DRange3d rangeA = DPoint3dOps::Range (&meshA.Point ());
    DRange3d rangeB = DPoint3dOps::Range (&meshB.Point ());
    Check::Near (rangeA, rangeB, "Paired mesh range");
    
    DRange1d altitudeRange = DRange1d::FromAltitudes (pointA, plane);
    double tol = 1.0e-10 * rangeA.LargestCoordinate ();
    if (skipSmallRange)
        {
        // Do nothing if range overlap is small ...
        if (altitudeRange.Length () <= tol)
            return;
        if (altitudeRange.Low () > -tol)
            return;
        if (altitudeRange.High () < tol)
            return;
        }

    Check::StartScope ("SectionTopology", plane);        
    CheckTopology (sectionQ, expectRegion, expectedLinestringCount);
    CheckTopology (sectionT, expectRegion, expectedLinestringCount);
    Check::EndScope ();

    Check::StartScope ("SectionMatch", plane);        
    DMatrix4d productQ, productT;
    if (  Check::True (sectionQ.IsValid (), "Q section")
       && Check::True (sectionT.IsValid (), "T section")
       && Check::True (sectionQ->ComputeSecondMomentWireProducts (productQ), "Quad Section WireMoments")
       && Check::True (sectionT->ComputeSecondMomentWireProducts (productT), "Triangle Section WireMoments")
       )
        {
        Check::Near (productQ, productT, "Section WireMoments");
        if (expectedLength > 0.0)
            Check::Near (expectedLength, productQ.coff[3][3], "section length");
        }
    Check::EndScope ();
    }

TEST(TrivialSection, Rectangular)
    {
    for (int numX = 1; numX < 10; numX += 4)
        {
        for (int numY = 1; numY < 20; numY += 7)
            {
            double xSize = (double)numX;
            double ySize = (double)numY;
            int maxPlaneIndex = 4;
            double originYStep = ySize / (double)maxPlaneIndex;
            PolyfaceHeaderPtr meshQ = Mesh_XYGrid (numX, numY, xSize, ySize, false);
            PolyfaceHeaderPtr meshT = Mesh_XYGrid (numX, numY, xSize, ySize, true);

            for (int planeIndex = 0; planeIndex <= maxPlaneIndex; planeIndex++)
                {
                DPoint3d origin = DPoint3d::From (0.0, planeIndex * originYStep, 0.0);
                DPlane3d plane = DPlane3d::FromOriginAndNormal (origin, DVec3d::From (0,1,0));
                VerifySectionMatch (*meshQ, *meshT, plane, false, false, 1, xSize, 0);
                DPlane3d plane1 = DPlane3d::FromOriginAndNormal (origin, DVec3d::From (1,2,3));
                VerifySectionMatch (*meshQ, *meshT, plane1, true, false, 1, 0.0, 0);
                }
            }
        }
    }
    

    
TEST(TrivialSection, Cube)
    {
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    double ax = 2.0;
    builder->AddSweptNGon (4, ax, -1.0, 1.0, true, true);
    PolyfaceHeaderR meshQ = builder->GetClientMeshR ();
    PolyfaceHeaderPtr meshTPtr = PolyfaceHeader::CreateVariableSizeIndexed  ();
    PolyfaceHeaderR meshT = *meshTPtr;
    meshQ.CopyTo (meshT);
    meshT.Triangulate ();
    DPlane3d plane0 = DPlane3d::FromOriginAndNormal (DPoint3d::From (0,0,0), DVec3d::From(0,0,1));
    DPlane3d plane1 = DPlane3d::FromOriginAndNormal (DPoint3d::From (0,0,0), DVec3d::From(0,0.5,1));
    VerifySectionMatch (meshQ, meshT, plane0, true, true, 1, 4.0 * ax * sqrt (2.0), 1);
    VerifySectionMatch (meshQ, meshT, plane1, true, true, 1, 0.0, 1);
    
    }




#include <Bentley/BeTimeUtilities.h>

enum class DrapeAction
  {
  ClassicDrape,
  FastDrape,
  FastDrapeWithPick
  };
void TestLargeMesh (int numX, int numY, int numCall, double globalScale, DrapeAction action)
    {
    static bool s_noisy = false;
    double maxX = 20.0 * globalScale;
    double maxY = 250.0 * globalScale;
    PolyfaceHeaderPtr mesh = Mesh_XYGrid (numX, numY, maxX, maxY, true);
    DSegment3d segment = DSegment3d::From (0.5, 0.5, 100, 10.5, 1.5, 200);
    bvector<DPoint3d> linestring;
    linestring.push_back (segment.point[0]);
    linestring.push_back (segment.point[1]);

    uint64_t time0 = 0, time1 = 0;
    if (action == DrapeAction::FastDrape)
        {
        bvector<DrapeSegment> drapeSegments;
        time0 = BeTimeUtilities::QueryMillisecondsCounter ();
        PolyfaceSearchContext context (mesh, true, true, false);
        time1 = BeTimeUtilities::QueryMillisecondsCounter ();
        if (s_noisy)
            printf ("  range construction %d\n", (int)(time1 - time0));
        time0 = BeTimeUtilities::QueryMillisecondsCounter ();
        for (int i = 0; i < numCall; i++)
            {
            context.DoDrapeXY (segment, drapeSegments);
            }
        time1 = BeTimeUtilities::QueryMillisecondsCounter ();
        }
    else if (action == DrapeAction::ClassicDrape)
        {
        time0 = BeTimeUtilities::QueryMillisecondsCounter ();
        for (int i = 0; i < numCall; i++)
            {
            bvector<DPoint3d> xyzOut;
            bvector<int> lineIndex;
            bvector<int> meshIndex;
            mesh->SweepLinestringToMesh (xyzOut, lineIndex, meshIndex, linestring, DVec3d::From (0,0,1));
            time1 = BeTimeUtilities::QueryMillisecondsCounter ();
            }
        }
    else if (action == DrapeAction::FastDrapeWithPick)
        {
        bvector<DPoint3dSizeSize> pickPoints;
        bvector<DrapeSegment> drapeSegments;
        time0 = BeTimeUtilities::QueryMillisecondsCounter ();
        PolyfaceSearchContext context (mesh, true, true, false);
        time1 = BeTimeUtilities::QueryMillisecondsCounter ();
        if (s_noisy)
            printf ("  range construction %d\n", (int)(time1 - time0));
        size_t errors = 0;
        time0 = BeTimeUtilities::QueryMillisecondsCounter ();
        for (int i = 0; i < numCall; i++)
            {
            context.DoDrapeXY (segment, drapeSegments);
            DPoint3d xyz;
            for (DrapeSegment const &segment : drapeSegments)
                {
                // We expect a pick a the middle of a draped segment to produce exactly one point pick with matching readIndex ...
                segment.m_segment.FractionParameterToPoint (xyz, 0.5);
                context.SelectFacetsXY (xyz, 0, pickPoints);
                if (pickPoints.size () != 1
                    || pickPoints[0].GetTagB () != segment.m_facetReadIndex )
                    {
                    errors ++;
                    }
                }
            }
        time1 = BeTimeUtilities::QueryMillisecondsCounter ();
        Check::Size (0, errors, "Error count in mid-segment pick");
        }
        if (s_noisy)
            printf (" %s (Grid size %d %d) (scale %g) (numSearch %d) (searchTime %g)\n",
                    action == DrapeAction::FastDrape ? "FastDrape "
                    : action == DrapeAction::ClassicDrape ? "FULLSEARCH"
                    : action == DrapeAction::FastDrapeWithPick ? "DrapeAndPick"
                    : "Unknown DrapeAction",
                    numX, numY, globalScale, numCall, (double)(time1 - time0));
#ifdef showSections
    printf ("\n(ProjectedLinestrings");
        ptrdiff_t lastDisconnect = -1;
        for (ptrdiff_t i = 0; i < (ptrdiff_t)xyzOut.size (); i++)
            {
            if (i == lastDisconnect + 1)
                printf ("\n(ProjectedLinestring");
            if (xyzOut[i].IsDisconnect ())
                printf ("\n)");
            else
                printf ("\n   (%g, %g, %g) (L %d M %d)", xyzOut[i].x, xyzOut[i].y, xyzOut[i].z, lineIndex[i], meshIndex[i]);
            }
    printf ("\n)\n");
#endif
    }

TEST(DoDrapeXY,ExtendBothEnds)
    {
    //static bool s_noisy = false;
    double maxX = 1.0;
    double maxY = 1.0;
    for (int numBlock = 1; numBlock <= 8; numBlock *= 2)
      {
      PolyfaceHeaderPtr mesh = Mesh_XYGrid(numBlock, numBlock, maxX, maxY, true);
      DSegment3d segment = DSegment3d::From(-0.5, 0.4, 100, 10.5, 0.4, 100);
      PolyfaceSearchContext context(mesh, true, true, false);
      bvector<DrapeSegment> drapeSegments;
      context.DoDrapeXY(segment, drapeSegments);
      }
    }

TEST(SweepLinestring, LargeMesh)
    {
    TestLargeMesh (20,20, 1, 1.0, DrapeAction::ClassicDrape);
    TestLargeMesh (20,20, 1, 1.0, DrapeAction::FastDrape);

    static int s_numCallLongTest = 5;
    static int s_numBlockLongTest = 5;
    int numA = s_numBlockLongTest;
    TestLargeMesh (numA, numA, s_numCallLongTest, 1.0, DrapeAction::ClassicDrape);
    TestLargeMesh (numA, numA, s_numCallLongTest, 10.0, DrapeAction::ClassicDrape);
    TestLargeMesh (numA, numA, s_numCallLongTest, 1.0, DrapeAction::FastDrape);
    TestLargeMesh (numA, numA, s_numCallLongTest, 10.0, DrapeAction::FastDrape);
    int numB = 4 * numA;
    TestLargeMesh (numB, numB, s_numCallLongTest, 1.0, DrapeAction::ClassicDrape);
    TestLargeMesh (numB, numB, s_numCallLongTest, 10.0, DrapeAction::ClassicDrape);
    TestLargeMesh (numB, numB, s_numCallLongTest, 1.0, DrapeAction::FastDrape);
    TestLargeMesh (numB, numB, s_numCallLongTest, 10.0, DrapeAction::FastDrape);
    }

TEST(SweepLinestring, PointSelect)
    {
    TestLargeMesh (20,20, 1, 1.0, DrapeAction::FastDrapeWithPick);
    }



TEST(SweepLinestring, Test6)
    {
    int numX = 4;
    int numY = 2;
    PolyfaceHeaderPtr mesh = Mesh_XYGrid (4, 2, (double)numX, (double)numY, true);
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (-1,0.5, 1.0));
    linestring.push_back (DPoint3d::From (2,0.5, 1.0));
    linestring.push_back (DPoint3d::From (6,1.5, 1.0));
    linestring.push_back (DPoint3d::From (6, 0.25, 1.0));
    linestring.push_back (DPoint3d::From (2, 0.25, 1.0));
    bvector<DPoint3d> xyzOut;
    bvector<int> lineIndex;
    bvector<int> meshIndex;
    mesh->SweepLinestringToMesh (xyzOut, lineIndex, meshIndex, linestring, DVec3d::From (0,0,1));
#ifdef showSections

    printf ("\n(ProjectedLinestrings");
        ptrdiff_t lastDisconnect = -1;
        for (ptrdiff_t i = 0; i < (ptrdiff_t)xyzOut.size (); i++)
            {
            if (i == lastDisconnect + 1)
                printf ("\n(ProjectedLinestring");
            if (xyzOut[i].IsDisconnect ())
                printf ("\n)");
            else
                printf ("\n   (%g, %g, %g) (L %d M %d)", xyzOut[i].x, xyzOut[i].y, xyzOut[i].z, lineIndex[i], meshIndex[i]);
            }
    printf ("\n)\n");
#endif
    }

double LengthAllowDisconnects (bvector<DPoint3d> const &xyz)
    {
    double a = 0.0;
    for (size_t i = 0; i + 1 < xyz.size (); i++)
        {
        if (!xyz[i].IsDisconnect () && !xyz[i+1].IsDisconnect ())
            a += xyz[i].Distance (xyz[i+1]);
        }
    return a;
    }
// Check length of linestring (with disconnects allowed) versus curve vector
void CheckLength (bvector<DPoint3d> &xyz, CurveVectorCR curve, char const*name)
    {
    double a = LengthAllowDisconnects (xyz);
    double b = curve.Length ();
    Check::Near (a, b, name);
    }

void CheckDrape (int iMax, int jMax, double edgeLengthX, double edgeLengthY, bvector<DPoint3d> &linestring, double expectedLength, char const* name)
    {
    Check::StartScope (name);

    bvector<DPoint3d> xyzOut;
    bvector<int> lineIndex;
    bvector<int> meshIndex;


    PolyfaceHeaderPtr mesh = Mesh_XYGrid (iMax, jMax, edgeLengthX, edgeLengthY, false);
    mesh->SweepLinestringToMesh (xyzOut, lineIndex, meshIndex, linestring, DVec3d::From (0,0,1));
    DVec3d direction = DVec3d::From (0,0,-1);
    CurveVectorPtr curve = mesh->DrapeLinestring (linestring, direction);
    Check::Near (expectedLength, LengthAllowDisconnects (xyzOut), "SweepLinestringToMesh length");
    if (Check::True (curve.IsValid (), "Expected curve vector"))
      Check::Near (expectedLength, curve->Length (), "DrapeLinestring length");
    Check::EndScope ();
    }

TEST(SweepLinestring, Test2)
    {
    bvector<DPoint3d> linestring;
    double x0 = 1;
    double x1 = 5;
    double y = 1;
    double edgeLength = 10.0;
    linestring.push_back (DPoint3d::From (x0, y, 0));
    linestring.push_back (DPoint3d::From (x1, y, 0));
    CheckDrape (1, 1, edgeLength, edgeLength, linestring, fabs (x1 -x0), "large single Facet Fully contained drape");
    }


TEST(SweepLinestring, Test3)
    {
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (1,1,20));
    linestring.push_back (DPoint3d::From (3,2,20));
    double edgeLength = 6.0;
    CheckDrape (6,3, edgeLength, edgeLength, linestring, LengthAllowDisconnects (linestring), "6x3 facets grid fully contained drape");
    }


TEST(SweepLinestring, Test4)
    {
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (5,5,20));
    linestring.push_back (DPoint3d::From (20,5,20));
    double edgeLength = 10.0;
    CheckDrape (1,1, edgeLength, edgeLength, linestring, 5.0, "1x1 facets grid partially contained drape");
    }

TEST(SweepLinestring, Test4a)
    {
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (-5,5,20));
    linestring.push_back (DPoint3d::From (20,5,20));
    double edgeLength = 10.0;
    CheckDrape (1,1, edgeLength, edgeLength, linestring, 10.0, "1x1 facets grid, drape extends at both ends");
    }


void TestLargeMeshXYRangeConstruction (int numX, int numY, double globalScale)
    {
    double maxX = 20.0 * globalScale;
    double maxY = 25.0 * globalScale;
    PolyfaceHeaderPtr mesh = Mesh_XYGrid (numX, numY, maxX, maxY, true);
    PolyfaceIndexedHeapRangeTreePtr tree = PolyfaceIndexedHeapRangeTree::CreateForPolyfaceXYSort (*mesh);
    }

TEST(RangeTreeConstruction,XYGrid)
    {
    TestLargeMeshXYRangeConstruction (7,5, 1.0);
    }

void LoadCutout (bvector<DPoint2d> &xyz)
    {
    xyz.push_back (DPoint2d::From (0.4, 0.1));
    xyz.push_back (DPoint2d::From (0.7, 0.3));
    xyz.push_back (DPoint2d::From (0.75, 0.85));
    xyz.push_back (DPoint2d::From (0.60, 0.80));
    xyz.push_back (DPoint2d::From (0.42, 0.5));
    DPoint2d origin = xyz.front ();
    xyz.push_back (origin);
    }

void LoadOuter (int numX, int numY, bvector<DPoint2d> &xyz)
    {
    DPoint2d corners[5] =
      {
        {0,0},
        {1,0},
        {1,1},
        {0,1},
        {0,0}
      };
    int numOnEdge[4] = {numX, numY, numX, numY};
    for (int edge = 0; edge < 4; edge++)
        {
        xyz.push_back (corners[edge]);
        for (int i = 1; i < numOnEdge[edge]; i++)
            {
            double f = i / numOnEdge[edge];
            DPoint2d uv;
            uv.Interpolate (corners[edge], f, corners[edge+1]);
            xyz.push_back (uv);
            }
        }
      xyz.push_back (corners[0]);
    }
TEST(RangeTreeConstruction,Bsurf1)
    {

    bvector<bvector<DPoint2d>> uvTrim;
    uvTrim.push_back (bvector<DPoint2d> ());
    LoadCutout (uvTrim.back ());
    uvTrim.push_back (bvector<DPoint2d> ());
    LoadOuter (8, 10, uvTrim.back ());
    for (double theta = 0.0; theta < 3.0; theta += 0.45)
        {
        PolyfaceHeaderPtr mesh = Mesh_TriangulatedBsurf (uvTrim, false);
        Transform rotateZ = Transform::FromAxisAndRotationAngle (DRay3d::FromOriginAndVector (
                DPoint3d::From (0,0,0), DVec3d::From (0,0,1)), theta);
        mesh->Transform (rotateZ);
        PolyfaceIndexedHeapRangeTreePtr tree = PolyfaceIndexedHeapRangeTree::CreateForPolyfaceXYSort (*mesh);
        }
    }

void Print (bvector<DrapeSegment> const &data, char const *name)
    {
    GEOMAPI_PRINTF("\n(DrapeSegments %s (size %d)\n", name, (int)data.size ());
    for (size_t i = 0; i < data.size (); i++)
        {
        DrapeSegment segment = data[i];
        GEOMAPI_PRINTF("\n   (seg %3d u %g %g  v %g %g ) (ri %d) (xyz (%g,%g,%g) (%g,%g,%g))",
                    (int)segment.m_drapeEdgeIndex,
                    segment.m_drapeEdgeFraction[0], segment.m_drapeEdgeFraction[1],
                    segment.m_drapeDistance[0], segment.m_drapeDistance[1],
                    (int)segment.m_facetReadIndex,
                    segment.m_segment.point[0].x, segment.m_segment.point[0].y, segment.m_segment.point[0].z,
                    segment.m_segment.point[1].x, segment.m_segment.point[1].y, segment.m_segment.point[1].z
                    );
        if (i + 1 < data.size () && !data[i].AlmostEqualXYZEndToStart (data[i+1]))
            GEOMAPI_PRINTF (" (xyzBreak)");
        }
    GEOMAPI_PRINTF ("\n)\n");
    }


// drape a linestring onto a solid.
// sweep is in +Y direction (!!!)
// linestring is Z=1, starts at Y=-2, has X breaks at -5, 5, 105
void TestDrapeZ1 (bvector<DPoint3d> &basePoints)
    {
    static bool s_noisy = false;
    auto contour = CurveVector::CreateLinear (basePoints, CurveVector::BOUNDARY_TYPE_Open, false);
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    polyface->AddPolygon (basePoints);
    DVec3d meshSweepVector = DVec3d::From (0,0,2);
    polyface->SweepToSolid (meshSweepVector, false);


    bvector<DPoint3d> sweepPoints
        {
        DPoint3d::From ( -5, -2, 1),
        DPoint3d::From (  5, -2, 1),
        DPoint3d::From (105, -2, 1)
        };
 
    DVec3d sweepDirection = DVec3d::From (0,1,0);

    if (s_noisy)
        {
        Check::Print (basePoints, "SweptSolidBaseContour");
        Check::Print (sweepPoints, "LinestringToDrape");
        Check::Print (sweepDirection, "SweepDirection");
        }

    DPlane3d refPlane = DPlane3d::FromOriginAndNormal (sweepPoints[0], sweepDirection);
#ifdef TestOldSweepLinestringToMesh
    bvector<DPoint3d> xyzOut;
    bvector<int> indexA, indexB;
    polyface->SweepLinestringToMesh (xyzOut, indexA, indexB, sweepPoints, sweepDirection);
    if (s_noisy)
        Check::Print (xyzOut, "SweepLinestringToMesh");
#endif
    bvector<DrapeSegment> allDrape, frontDrape, backDrape;
    polyface->DrapeLinestring (sweepPoints, sweepDirection, allDrape);
    DrapeSegment::SelectMinMaxVisibleDrape (allDrape, frontDrape, backDrape, false);
    if (s_noisy)
        {
        Print (allDrape, "allDrape");
        Print (frontDrape, "frontDrape");
        Print (backDrape, "backDrape");
        }
    double area1 = DrapeSegment::SumSignedSweptAreaToPlaneWithUnitNormal (backDrape, refPlane)
                    - DrapeSegment::SumSignedSweptAreaToPlaneWithUnitNormal (frontDrape, refPlane);
    DVec3d areaNormal = PolygonOps::AreaNormal (basePoints);
    double area0 = areaNormal.Magnitude ();
    Check::Parallel (areaNormal, meshSweepVector);
    if (PolygonOps::IsConvex (basePoints))
        Check::Near (area0, area1, "Convex Polygon Area == drape fragment area");
    else
        Check::True (area1 > area0, "Convex Polygon Area < drape fragment area");

    bvector<DrapeSegment> frontDrape1, backDrape1;    
    bvector<bvector<DPoint3d>> chainA0, chainB0, chainA1, chainB1;
    DrapeSegment::SelectMinMaxVisibleDrape (allDrape, frontDrape1, backDrape1, true);

    DrapeSegment::ExtractLinestrings (frontDrape, chainA0);
    DrapeSegment::ExtractLinestrings (backDrape, chainB0);
    DrapeSegment::ExtractLinestrings (frontDrape1, chainA1);
    DrapeSegment::ExtractLinestrings (backDrape1, chainB1);
    Check::Near (PolylineOps::Length (chainA0), PolylineOps::Length (chainA1), "compressed chain length");
    Check::Near (PolylineOps::Length (chainB0), PolylineOps::Length (chainB1), "compressed chain length");
    }
TEST(SweepLinestring,Test1000)
    {
    bvector<DPoint3d> basePoints
        {
        DPoint3d::From ( 0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (20,1,0)
        };
    TestDrapeZ1 (basePoints);
    }

TEST(SweepLinestring,Test1010)
    {
    bvector<DPoint3d> basePoints
        {
        DPoint3d::From ( 0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,4,0),
        DPoint3d::From ( 8,4,0),
        DPoint3d::From ( 8,6,0),
        DPoint3d::From (12,7,0),
        DPoint3d::From ( 0,7,0),
        DPoint3d::From ( 0,0,0)
        };
    TestDrapeZ1 (basePoints);
    }


TEST(DrapePointsXY,Test1)
    {
    bvector<DPoint3d> basePoints
        {
        DPoint3d::From ( 0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,0,4),
        DPoint3d::From ( 5,0,4),
        DPoint3d::From ( 0,0,0)
        };


    auto contour = CurveVector::CreateLinear (basePoints, CurveVector::BOUNDARY_TYPE_Open, false);
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    polyface->AddPolygon (basePoints);
    DVec3d meshSweepVector = DVec3d::From (0,20,0);
    polyface->SweepToSolid (meshSweepVector, false);

    bvector<DPoint3d> spacePoints
        {
        DPoint3d::From (2,2,5),     // bottom, slant
        DPoint3d::From (-2,3,5),    // none
        DPoint3d::From (8,15,5),    // bottom, top
        };
    bvector<DPoint3dSizeSize> meshPoints;
    polyface->DrapePointsXY (spacePoints, meshPoints);
    Check::Size (4, meshPoints.size ());
    }

CurveVectorPtr LinestringWithDisconnectsToCurveVector (bvector<DPoint3d> &xyzIn)
    {
    CurveVectorPtr result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    bvector<DPoint3d> xyzA;
    for (size_t i = 0; i < xyzIn.size (); i++)
        {
        auto xyz = xyzIn[i];
        if (i + 1 == xyzIn.size () || xyz.IsDisconnect ())
            {
            result->push_back (ICurvePrimitive::CreateLineString (xyzA));
            xyzA.clear ();
            }
        else
            {
            xyzA.push_back (xyz);
            }
        }
    return result;
    }

void doDrapes (bvector<DPoint3d> &alignmentPts, PolyfaceHeaderPtr &polyface, double dz)
    {
    Check::SaveTransformed (*polyface);
    //Drape
    CurveVectorPtr drapedLine = polyface->DrapeLinestring (alignmentPts, DVec3d::From (0, 0, 1));
    Check::Shift (0, 0, dz);
    Check::SaveTransformed (*drapedLine);
    bvector<DPoint3d> xyzSweep;
    bvector<int> linestringIndexOut, meshIndexOut;
    polyface->SweepLinestringToMesh (xyzSweep, linestringIndexOut, meshIndexOut, alignmentPts, DVec3d::From (0,0,1));
    auto drape1 = LinestringWithDisconnectsToCurveVector (xyzSweep);
    Check::Shift (0, 0, dz);
    Check::SaveTransformed (*drape1);
    Check::Shift (0, 0, -2.0 *dz);
    }

TEST(Polyface,DrapeOnBoreMesh)
    {

    //Create the mesh and add it to the model
    bvector<DSegment3d> meshSeg;
    meshSeg.push_back (DSegment3d::From (2311150187.40772, 39702123577.6102, 248285.869362643, 2311150187.40772, 39702123577.6102, 339285.869362643));
    meshSeg.push_back (DSegment3d::From (2311019731.00631, 39701453382.6441, 267241.353238683, 2311019731.00631, 39701453382.6441, 298241.353238683));
    meshSeg.push_back (DSegment3d::From (2311057699.66045, 39700758834.6632, 271603.286001726, 2311057699.66045, 39700758834.6632, 287603.286001726));
    meshSeg.push_back (DSegment3d::From (2311234886.71311, 39700093997.3603, 328647.301397522, 2311234886.71311, 39700093997.3603, 419647.301397522));
    meshSeg.push_back (DSegment3d::From (2311551778.94191, 39699504167.3428, 283897.35217137, 2311551778.94191, 39699504167.3428, 322897.35217137));
    meshSeg.push_back (DSegment3d::From (2311991339.13025, 39698999572.877, 372605.336991057, 2311991339.13025, 39698999572.877, 387605.336991057));
    meshSeg.push_back (DSegment3d::From (2312560382.16476, 39698593851.6512, 272021.118775907, 2312560382.16476, 39698593851.6512, 303021.118775907));
    meshSeg.push_back (DSegment3d::From (2310101668.42025, 39700356888.1546, 285474.801437681, 2310101668.42025, 39700356888.1546, 376474.801437681));
    meshSeg.push_back (DSegment3d::From (2310307505.81267, 39700935515.7853, 280516.251665919, 2310307505.81267, 39700935515.7853, 296516.251665919));
    meshSeg.push_back (DSegment3d::From (2310662610.08442, 39701459557.8728, 256882.841588289, 2310662610.08442, 39701459557.8728, 295882.841588289));
    meshSeg.push_back (DSegment3d::From (2311169467.27834, 39701835446.6556, 333304.566415655, 2311169467.27834, 39701835446.6556, 348304.566415655));
    meshSeg.push_back (DSegment3d::From (2311721047.16584, 39702086039.1775, 255709.593856694, 2311721047.16584, 39702086039.1775, 346709.593856694));
    meshSeg.push_back (DSegment3d::From (2312357600.17114, 39702159128.663, 237657.37235973, 2312357600.17114, 39702159128.663, 268657.37235973));
    meshSeg.push_back (DSegment3d::From (2312879364.92958, 39702065156.4673, 273643.613620606, 2312879364.92958, 39702065156.4673, 312643.613620606));
    meshSeg.push_back (DSegment3d::From (2313515917.93488, 39701783239.8803, 268541.000141473, 2313515917.93488, 39701783239.8803, 284541.000141473));
    meshSeg.push_back (DSegment3d::From (2314037682.69332, 39701454337.1953, 258368.214720346, 2314037682.69332, 39701454337.1953, 274368.214720346));
    meshSeg.push_back (DSegment3d::From (2311388342.27038, 39700176176.826, 337704.205180269, 2311388342.27038, 39700176176.826, 428704.205180269));
    meshSeg.push_back (DSegment3d::From (2311668604.48348, 39700068780.0309, 257994.776974901, 2311668604.48348, 39700068780.0309, 362994.776974901));
    meshSeg.push_back (DSegment3d::From (2312002533.92889, 39699937517.2814, 339696.893964292, 2312002533.92889, 39699937517.2814, 378696.893964292));
    meshSeg.push_back (DSegment3d::From (2314010038.08042, 39705724540.9896, 236785.95431105, 2314010038.08042, 39705724540.9896, 275785.95431105));
    meshSeg.push_back (DSegment3d::From (2313302909.05651, 39705139173.7124, 342779.327748566, 2313302909.05651, 39705139173.7124, 357779.327748566));
    meshSeg.push_back (DSegment3d::From (2312144458.65416, 39704170881.83, 228609.682335931, 2312144458.65416, 39704170881.83, 259609.682335931));
    meshSeg.push_back (DSegment3d::From (2310336145.83099, 39702941080.4612, 258874.814857464, 2310336145.83099, 39702941080.4612, 274874.814857464));
    meshSeg.push_back (DSegment3d::From (2312921468.07037, 39703838694.104, 257676.084932655, 2312921468.07037, 39703838694.104, 273676.084932655));
    meshSeg.push_back (DSegment3d::From (2310225333.69265, 39699732288.3839, 329947.589792403, 2310225333.69265, 39699732288.3839, 345947.589792403));
    meshSeg.push_back (DSegment3d::From (2309641252.18263, 39698282059.2553, 299658.463837355, 2309641252.18263, 39698282059.2553, 390658.463837355));
    meshSeg.push_back (DSegment3d::From (2314422067.50543, 39700944420.1928, 248946.673071874, 2314422067.50543, 39700944420.1928, 339946.673071874));
    meshSeg.push_back (DSegment3d::From (2316174312.0355, 39700100256.9687, 350430.154155619, 2316174312.0355, 39700100256.9687, 365430.154155619));

    bvector<ptrdiff_t> topFacetReadIndex, bottomFacetReadIndex, sideFacetReadIndex;
    bool foldedSurfaces;
    PolyfaceHeaderPtr polyface = PolyfaceHeader::VolumeFromBoreData (meshSeg, foldedSurfaces, &topFacetReadIndex, &bottomFacetReadIndex, &sideFacetReadIndex);
    bvector<PolyfaceHeaderPtr> topFacets, bottomFacets;
    polyface->CopyPartitions (topFacetReadIndex, topFacets);
    polyface->CopyPartitions (bottomFacetReadIndex, bottomFacets);

    Check::Size (1, topFacets.size (), "Expect single top");
    Check::Size (1, bottomFacets.size (), "Expect single bottom");

    auto range = polyface->PointRange ();
    double dx = 2.0 * range.XLength ();
    //double dy = range.YLength ();
    double dz = 2.0 * range.ZLength ();

    //Create the line that will be draped and add it to the model
    bvector<DPoint3d> alignmentPts;
    alignmentPts.push_back (DPoint3d::From (2314010038.08042, 39705724540.9896, 0));
    alignmentPts.push_back (DPoint3d::From (2313302909.05651, 39705139173.7124, 0));
    alignmentPts.push_back (DPoint3d::From (2312144458.65416, 39704170881.83, 0));
    alignmentPts.push_back (DPoint3d::From (2311150187.40772, 39702123577.6102, 0));
    alignmentPts.push_back (DPoint3d::From (2311169467.27834, 39701835446.6556, 0));
    alignmentPts.push_back (DPoint3d::From (2311019731.00631, 39701453382.6441, 0));
    alignmentPts.push_back (DPoint3d::From (2311057699.66045, 39700758834.6632, 0));
    alignmentPts.push_back (DPoint3d::From (2311232009.00347, 39700098430.6205, 0));
    alignmentPts.push_back (DPoint3d::From (2311551778.94191, 39699504167.3428, 0));
    alignmentPts.push_back (DPoint3d::From (2311991339.13025, 39698999572.877, 0));
    alignmentPts.push_back (DPoint3d::From (2312560382.16476, 39698593851.6512, 0));

    Check::SaveTransformed (alignmentPts);
    Check::SaveTransformed (*polyface);

    Check::Shift (dx, 0, 0);
    doDrapes (alignmentPts, topFacets[0], dz);
    Check::Shift (dx, 0, 0);
    doDrapes (alignmentPts, bottomFacets[0], dz);
    
    Check::ClearGeometry("Polyface.DrapeOnBoreMesh");
   }

PolyfaceHeaderPtr createMesh (bvector<DPoint3d> &points, bvector<int> &indices, bool translateToOrigin)
    {
    auto mesh = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
    if (translateToOrigin)
        {
        auto range = mesh->PointRange ();
        auto transform = Transform::From (-range.low.x, -range.low.y, -range.low.z);
        mesh->Transform (transform);
        }
    return mesh;
    }
// A mesh from Samuel Vaillancourt.
// murky connectivity -- maybe some slivers
PolyfaceHeaderPtr CreateVaillancourtMeshA (bool translateToOrigin)
    {
      bvector<DPoint3d> points{
        DPoint3d::From (689270.38790355960,4626217.5345427534,61.522208604960881), 
        DPoint3d::From (689266.65602976852,4626253.8666280918,52.391397760544550), 
        DPoint3d::From (689285.23545882560,4626219.0596190561,60.775352673259697), 
        DPoint3d::From (689281.45473209012,4626255.8673178330,51.525012869324371), 
        DPoint3d::From (689300.08301409159,4626220.5846953560,60.028496741519426), 
        DPoint3d::From (689295.76103826333,4626262.6617867602,49.453877670513322), 
        DPoint3d::From (689314.93056935759,4626222.1097716577,59.281640809781720), 
        DPoint3d::From (689309.99485952558,4626270.1619408494,47.205392956919795), 
        DPoint3d::From (689329.77812462347,4626223.6348479586,58.534784878063434), 
        DPoint3d::From (689324.20140358969,4626277.9276552228,44.890168866400671), 
        DPoint3d::From (689344.62567988958,4626225.1599242603,57.787928946340259), 
        DPoint3d::From (689338.97264525783,4626280.1956913136,43.956595732915119), 
        DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616), 
        DPoint3d::From (689353.79856007674,4626281.9314506724,43.156791914418307), 
        DPoint3d::From (689374.32079042192,4626228.2100768629,56.294217082874077), 
        DPoint3d::From (689368.65624057618,4626283.3579516038,42.434709486652970), 
        DPoint3d::From (689389.16834568779,4626229.7351531647,55.547361151185058), 
        DPoint3d::From (689383.54941604857,4626284.4388871491,41.799472945355816), 
        DPoint3d::From (689404.01590095391,4626231.2602294656,54.800505219439884), 
        DPoint3d::From (689398.44779413729,4626285.4691720372,41.176965692296690), 
        DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605), 
        DPoint3d::From (689413.33683325257,4626286.5903775636,40.531608686976483), 
        DPoint3d::From (689433.71101148566,4626234.3103820691,53.306793356020421), 
        DPoint3d::From (689428.26555332320,4626287.3252646038,39.983339448423230), 
        DPoint3d::From (689448.55856675166,4626235.8354583690,52.559937424265620), 
        DPoint3d::From (689443.20398678584,4626287.9655857952,39.458836059308169), 
        DPoint3d::From (689463.40612201777,4626237.3605346698,51.813081492542437), 
        DPoint3d::From (689458.23937338160,4626287.6620086432,39.171548815288673), 
        DPoint3d::From (689478.25367728353,4626238.8856109716,51.066225560846149), 
        DPoint3d::From (689473.27868412528,4626287.3202274963,38.893862821380019), 
        DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611), 
        DPoint3d::From (689488.35472960479,4626286.6208111169,38.706056052832956), 
        DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809), 
        DPoint3d::From (689503.44174214290,4626285.8146236716,38.545082491830705), 
        DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626), 
        DPoint3d::From (689518.60331697576,4626284.2825264661,38.566541198647222), 
        DPoint3d::From (689537.64389834763,4626244.9859161768,48.078801833943608), 
        DPoint3d::From (689533.80614504009,4626282.3488036944,38.688934576162836), 
        DPoint3d::From (689552.49145361374,4626246.5109924776,47.331945902198441), 
        DPoint3d::From (689549.16862366872,4626278.8607845344,39.201946499251690), 
        DPoint3d::From (689567.33900887961,4626248.0360687794,46.585089970502153), 
        DPoint3d::From (689564.41069787939,4626276.5449764011,39.420363793888150), 
        DPoint3d::From (689582.18656414561,4626249.5611450812,45.838234038771709), 
        DPoint3d::From (689579.56279817107,4626275.1051196745,39.418641052227997), 
        DPoint3d::From (689597.03411941184,4626251.0862213830,44.845849109383579), 
        DPoint3d::From (689594.62108168309,4626274.5786269614,38.941846913063515), 
        DPoint3d::From (689611.88167467772,4626252.6112976829,43.680446666398531), 
        DPoint3d::From (689609.54102844978,4626275.3989275061,37.953565438342963), 
        DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278), 
        DPoint3d::From (689624.24674665683,4626278.3048747694,36.441128922382312), 
        DPoint3d::From (689641.57678520982,4626255.6614502864,41.349641780457318), 
        DPoint3d::From (689639.35461762163,4626277.2956177257,35.912643358020354), 
        DPoint3d::From (689656.42434047582,4626257.1865265863,40.184239337467176), 
        DPoint3d::From (689654.48587422201,4626276.0586871440,35.441375649370059), 
        DPoint3d::From (689671.27189574204,4626258.7116028881,39.018837675290428), 
        DPoint3d::From (689669.62181095406,4626274.7761926036,34.981559643700351)
        };

      bvector<int> indices {
         -3,2,1,0,
        -2, 3,4,0,
        -5,4, 3,0,
        -4,5,6, 0,
        -7,6,5,0,
         -6,7,8,0,
        -9, 8,7,0,
        -8,9, 10,0,
        -11,10,9, 0,
        -10,11,12,0,
         -13,12,11,0,
        -12, 13,14,0,
        -13,16, 14,0,
        -16,13,15, 0,
        -15,18,16,0,
         -18,15,17,0,
        -17, 20,18,0,
        -20,17, 19,0,
        -19,22,20, 0,
        -22,19,21,0,
         -21,24,22,0,
        -24, 21,23,0,
        -23,26, 24,0,
        -26,23,25, 0,
        -25,28,26,0,
         -28,25,27,0,
        -27, 30,28,0,
        -30,27, 29,0,
        -29,32,30, 0,
        -32,29,31,0,
         -31,34,32,0,
        -34, 31,33,0,
        -33,36, 34,0,
        -36,33,35, 0,
        -35,38,36,0,
         -38,35,37,0,
        -37, 40,38,0,
        -40,37, 39,0,
        -39,42,40, 0,
        -42,39,41,0,
         -41,44,42,0,
        -44, 41,43,0,
        -43,46, 44,0,
        -46,43,45, 0,
        -45,48,46,0,
         -48,45,47,0,
        -49, 48,47,0,
        -48,49, 50,0,
        -49,52,50, 0,
        -52,49,51,0,
         -51,54,52,0,
        -54, 51,53,0,
        -53,56, 54,0,
        -56,53,55,0
        };
    return createMesh (points, indices, translateToOrigin );
    }

// after visible part extraction?
PolyfaceHeaderPtr CreateVaillancourtMeshB (bool translateToOrigin)
    {
    
  bvector<DPoint3d> points{
    DPoint3d::From (689270.38790355960,4626217.5345427534,61.522208604960881),
    DPoint3d::From (689285.23545882560,4626219.0596190561,60.775352673259697),
    DPoint3d::From (689266.65602976852,4626253.8666280918,52.391397760544550),
    DPoint3d::From (689281.45473209012,4626255.8673178330,51.525012869324371),
    DPoint3d::From (689266.65602976852,4626253.8666280918,52.391397760544550),
    DPoint3d::From (689285.23545882560,4626219.0596190561,60.775352673259697),
    DPoint3d::From (689285.23545882560,4626219.0596190561,60.775352673259697),
    DPoint3d::From (689300.08301409159,4626220.5846953560,60.028496741519426),
    DPoint3d::From (689281.45473209012,4626255.8673178330,51.525012869324371),
    DPoint3d::From (689285.23545882560,4626219.0596190561,60.775352673259697),
    DPoint3d::From (689300.08301409159,4626220.5846953560,60.028496741519426),
    DPoint3d::From (689295.76104981813,4626262.6616742676,49.453905941652323),
    DPoint3d::From (689281.45473209012,4626255.8673178330,51.525012869324371),
    DPoint3d::From (689300.08301409159,4626220.5846953560,60.028496741519426),
    DPoint3d::From (689295.76104981813,4626262.6616742676,49.453905941652323),
    DPoint3d::From (689295.76103826333,4626262.6617867602,49.453877670513322),
    DPoint3d::From (689281.45473209012,4626255.8673178330,51.525012869324371),
    DPoint3d::From (689300.08301409159,4626220.5846953560,60.028496741519426),
    DPoint3d::From (689314.93056935759,4626222.1097716577,59.281640809781720),
    DPoint3d::From (689295.76103826333,4626262.6617867602,49.453877670513322),
    DPoint3d::From (689309.99485952558,4626270.1619408494,47.205392956919795),
    DPoint3d::From (689295.76103826333,4626262.6617867602,49.453877670513322),
    DPoint3d::From (689314.93056935759,4626222.1097716577,59.281640809781720),
    DPoint3d::From (689314.93056935759,4626222.1097716577,59.281640809781720),
    DPoint3d::From (689329.77812462347,4626223.6348479586,58.534784878063434),
    DPoint3d::From (689309.99485952558,4626270.1619408494,47.205392956919795),
    DPoint3d::From (689324.20140358969,4626277.9276552228,44.890168866400671),
    DPoint3d::From (689309.99485952558,4626270.1619408494,47.205392956919795),
    DPoint3d::From (689329.77812462347,4626223.6348479586,58.534784878063434),
    DPoint3d::From (689329.77812462347,4626223.6348479586,58.534784878063434),
    DPoint3d::From (689344.62567988958,4626225.1599242603,57.787928946340259),
    DPoint3d::From (689324.20140358969,4626277.9276552228,44.890168866400671),
    DPoint3d::From (689338.97264525783,4626280.1956913136,43.956595732915119),
    DPoint3d::From (689324.20140358969,4626277.9276552228,44.890168866400671),
    DPoint3d::From (689344.62567988958,4626225.1599242603,57.787928946340259),
    DPoint3d::From (689344.62567988958,4626225.1599242603,57.787928946340259),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689338.97264525783,4626280.1956913136,43.956595732915119),
    DPoint3d::From (689344.62567988958,4626225.1599242603,57.787928946340259),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689353.79856007674,4626281.9314506724,43.156791914418307),
    DPoint3d::From (689338.97264525783,4626280.1956913136,43.956595732915119),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689353.79856007674,4626281.9314506724,43.156791914418307),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689368.65624057618,4626283.3579516038,42.434709486652970),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689374.32079042192,4626228.2100768629,56.294217082874077),
    DPoint3d::From (689368.65624057618,4626283.3579516038,42.434709486652970),
    DPoint3d::From (689359.47323515569,4626226.6850005612,57.041073014609616),
    DPoint3d::From (689374.32079042192,4626228.2100768629,56.294217082874077),
    DPoint3d::From (689380.47320750635,4626265.6959503870,46.631054324528570),
    DPoint3d::From (689368.65624057618,4626283.3579516038,42.434709486652970),
    DPoint3d::From (689370.54442385805,4626264.9753266899,47.054545352060003),
    DPoint3d::From (689368.65624057618,4626283.3579516038,42.434709486652970),
    DPoint3d::From (689380.47320750635,4626265.6959503870,46.631054324528570),
    DPoint3d::From (689383.54941604857,4626284.4388871491,41.799472945355816),
    DPoint3d::From (689380.47320750635,4626265.6959503870,46.631054324528570),
    DPoint3d::From (689370.54442385805,4626264.9753266899,47.054545352060003),
    DPoint3d::From (689374.32082156639,4626228.2102666227,56.294168166446113),
    DPoint3d::From (689374.32082156639,4626228.2102666227,56.294168166446113),
    DPoint3d::From (689370.54442385805,4626264.9753266899,47.054545352060003),
    DPoint3d::From (689374.32079042192,4626228.2100768629,56.294217082874077),
    DPoint3d::From (689389.16834568779,4626229.7351531647,55.547361151185058),
    DPoint3d::From (689383.54941604857,4626284.4388871491,41.799472945355816),
    DPoint3d::From (689374.32079042192,4626228.2100768629,56.294217082874077),
    DPoint3d::From (689383.54941604857,4626284.4388871491,41.799472945355816),
    DPoint3d::From (689389.16834568779,4626229.7351531647,55.547361151185058),
    DPoint3d::From (689398.44779413729,4626285.4691720372,41.176965692296690),
    DPoint3d::From (689404.01590095391,4626231.2602294656,54.800505219439884),
    DPoint3d::From (689398.44779413729,4626285.4691720372,41.176965692296690),
    DPoint3d::From (689389.16834568779,4626229.7351531647,55.547361151185058),
    DPoint3d::From (689398.44779413729,4626285.4691720372,41.176965692296690),
    DPoint3d::From (689404.01590095391,4626231.2602294656,54.800505219439884),
    DPoint3d::From (689413.33683325257,4626286.5903775636,40.531608686976483),
    DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605),
    DPoint3d::From (689413.33683325257,4626286.5903775636,40.531608686976483),
    DPoint3d::From (689404.01590095391,4626231.2602294656,54.800505219439884),
    DPoint3d::From (689404.01590095391,4626231.2602294656,54.800505219439884),
    DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605),
    DPoint3d::From (689413.33683325257,4626286.5903775636,40.531608686976483),
    DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605),
    DPoint3d::From (689428.26555332320,4626287.3252646038,39.983339448423230),
    DPoint3d::From (689433.71101148566,4626234.3103820691,53.306793356020421),
    DPoint3d::From (689428.26555332320,4626287.3252646038,39.983339448423230),
    DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605),
    DPoint3d::From (689418.86345621978,4626232.7853057664,54.053649287721605),
    DPoint3d::From (689433.71101148566,4626234.3103820691,53.306793356020421),
    DPoint3d::From (689428.26555332320,4626287.3252646038,39.983339448423230),
    DPoint3d::From (689433.71101148566,4626234.3103820691,53.306793356020421),
    DPoint3d::From (689443.20398678584,4626287.9655857952,39.458836059308169),
    DPoint3d::From (689448.55856675166,4626235.8354583690,52.559937424265620),
    DPoint3d::From (689443.20398678584,4626287.9655857952,39.458836059308169),
    DPoint3d::From (689433.71101148566,4626234.3103820691,53.306793356020421),
    DPoint3d::From (689443.20398678584,4626287.9655857952,39.458836059308169),
    DPoint3d::From (689448.55856675166,4626235.8354583690,52.559937424265620),
    DPoint3d::From (689458.23937338160,4626287.6620086432,39.171548815288673),
    DPoint3d::From (689463.40612201777,4626237.3605346698,51.813081492542437),
    DPoint3d::From (689458.23937338160,4626287.6620086432,39.171548815288673),
    DPoint3d::From (689448.55856675166,4626235.8354583690,52.559937424265620),
    DPoint3d::From (689458.23937338160,4626287.6620086432,39.171548815288673),
    DPoint3d::From (689463.40612201777,4626237.3605346698,51.813081492542437),
    DPoint3d::From (689473.27868412528,4626287.3202274963,38.893862821380019),
    DPoint3d::From (689478.25367728353,4626238.8856109716,51.066225560846149),
    DPoint3d::From (689473.27868412528,4626287.3202274963,38.893862821380019),
    DPoint3d::From (689463.40612201777,4626237.3605346698,51.813081492542437),
    DPoint3d::From (689473.27868412528,4626287.3202274963,38.893862821380019),
    DPoint3d::From (689478.25367728353,4626238.8856109716,51.066225560846149),
    DPoint3d::From (689488.35472960479,4626286.6208111169,38.706056052832956),
    DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611),
    DPoint3d::From (689488.35472960479,4626286.6208111169,38.706056052832956),
    DPoint3d::From (689478.25367728353,4626238.8856109716,51.066225560846149),
    DPoint3d::From (689478.25367728353,4626238.8856109716,51.066225560846149),
    DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611),
    DPoint3d::From (689488.35472960479,4626286.6208111169,38.706056052832956),
    DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611),
    DPoint3d::From (689503.44174214290,4626285.8146236716,38.545082491830705),
    DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809),
    DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611),
    DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809),
    DPoint3d::From (689503.44174214290,4626285.8146236716,38.545082491830705),
    DPoint3d::From (689493.10123254976,4626240.4106872734,50.319369629110611),
    DPoint3d::From (689503.44174214290,4626285.8146236716,38.545082491830705),
    DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809),
    DPoint3d::From (689518.60331697576,4626284.2825264661,38.566541198647222),
    DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626),
    DPoint3d::From (689518.60331697576,4626284.2825264661,38.566541198647222),
    DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809),
    DPoint3d::From (689507.94878781575,4626241.9357635733,49.572513697355809),
    DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626),
    DPoint3d::From (689518.60331697576,4626284.2825264661,38.566541198647222),
    DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626),
    DPoint3d::From (689533.80614504009,4626282.3488036944,38.688934576162836),
    DPoint3d::From (689537.64388915710,4626244.9860056518,48.078779347511791),
    DPoint3d::From (689533.80614504009,4626282.3488036944,38.688934576162836),
    DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626),
    DPoint3d::From (689537.64388915710,4626244.9860056518,48.078779347511791),
    DPoint3d::From (689522.79634308175,4626243.4608398760,48.825657765654626),
    DPoint3d::From (689537.64389834763,4626244.9859161768,48.078801833943608),
    DPoint3d::From (689533.80614504009,4626282.3488036944,38.688934576162836),
    DPoint3d::From (689537.64389834763,4626244.9859161768,48.078801833943608),
    DPoint3d::From (689549.16862366872,4626278.8607845344,39.201946499251690),
    DPoint3d::From (689552.49145361374,4626246.5109924776,47.331945902198441),
    DPoint3d::From (689544.55873354024,4626265.3108371915,42.752688633128457),
    DPoint3d::From (689537.64389834763,4626244.9859161768,48.078801833943608),
    DPoint3d::From (689544.55873354024,4626265.3108371915,42.752688633128457),
    DPoint3d::From (689552.49145361374,4626246.5109924776,47.331945902198441),
    DPoint3d::From (689549.16862366872,4626278.8607845344,39.201946499251690),
    DPoint3d::From (689549.16862366872,4626278.8607845344,39.201946499251690),
    DPoint3d::From (689552.49145361374,4626246.5109924776,47.331945902198441),
    DPoint3d::From (689564.41069787939,4626276.5449764011,39.420363793888150),
    DPoint3d::From (689564.41069787939,4626276.5449764011,39.420363793888150),
    DPoint3d::From (689549.16862366872,4626278.8607845344,39.201946499251690),
    DPoint3d::From (689567.33900887961,4626248.0360687794,46.585089970502153),
    DPoint3d::From (689564.41069787939,4626276.5449764011,39.420363793888150),
    DPoint3d::From (689552.49145361374,4626246.5109924776,47.331945902198441),
    DPoint3d::From (689564.41069787939,4626276.5449764011,39.420363793888150),
    DPoint3d::From (689567.33900887961,4626248.0360687794,46.585089970502153),
    DPoint3d::From (689579.56279817107,4626275.1051196745,39.418641052227997),
    DPoint3d::From (689582.18656414561,4626249.5611450812,45.838234038771709),
    DPoint3d::From (689579.56279817107,4626275.1051196745,39.418641052227997),
    DPoint3d::From (689567.33900887961,4626248.0360687794,46.585089970502153),
    DPoint3d::From (689588.40382291435,4626262.0698860213,42.390040475917615),
    DPoint3d::From (689579.56279817107,4626275.1051196745,39.418641052227997),
    DPoint3d::From (689582.18656414561,4626249.5611450812,45.838234038771709),
    DPoint3d::From (689579.56279817107,4626275.1051196745,39.418641052227997),
    DPoint3d::From (689588.40382291435,4626262.0698860213,42.390040475917615),
    DPoint3d::From (689594.62108168309,4626274.5786269614,38.941846913063515),
    DPoint3d::From (689597.03411941184,4626251.0862213830,44.845849109383579),
    DPoint3d::From (689594.62108168309,4626274.5786269614,38.941846913063515),
    DPoint3d::From (689582.18656414561,4626249.5611450812,45.838234038771709),
    DPoint3d::From (689594.62108168309,4626274.5786269614,38.941846913063515),
    DPoint3d::From (689597.03411941184,4626251.0862213830,44.845849109383579),
    DPoint3d::From (689609.54102844978,4626275.3989275061,37.953565438342963),
    DPoint3d::From (689597.03411941184,4626251.0862213830,44.845849109383579),
    DPoint3d::From (689611.88167467772,4626252.6112976829,43.680446666398531),
    DPoint3d::From (689609.54102844978,4626275.3989275061,37.953565438342963),
    DPoint3d::From (689597.03411941184,4626251.0862213830,44.845849109383579),
    DPoint3d::From (689611.88167467772,4626252.6112976829,43.680446666398531),
    DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278),
    DPoint3d::From (689610.32124385913,4626267.8030508980,39.862525847694819),
    DPoint3d::From (689611.88167467772,4626252.6112976829,43.680446666398531),
    DPoint3d::From (689610.32124385913,4626267.8030508980,39.862525847694819),
    DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278),
    DPoint3d::From (689609.54102844978,4626275.3989275061,37.953565438342963),
    DPoint3d::From (689624.24674665683,4626278.3048747694,36.441128922382312),
    DPoint3d::From (689609.54102844978,4626275.3989275061,37.953565438342963),
    DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278),
    DPoint3d::From (689624.24674665683,4626278.3048747694,36.441128922382312),
    DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278),
    DPoint3d::From (689639.35461762163,4626277.2956177257,35.912643358020354),
    DPoint3d::From (689641.57678520982,4626255.6614502864,41.349641780457318),
    DPoint3d::From (689639.35461762163,4626277.2956177257,35.912643358020354),
    DPoint3d::From (689626.72922994394,4626254.1363739846,42.515044223425278),
    DPoint3d::From (689654.48587422201,4626276.0586871440,35.441375649370059),
    DPoint3d::From (689639.35461762163,4626277.2956177257,35.912643358020354),
    DPoint3d::From (689654.48587422201,4626276.0586871440,35.441375649370059),
    DPoint3d::From (689639.35461762163,4626277.2956177257,35.912643358020354),
    DPoint3d::From (689641.57678520982,4626255.6614502864,41.349641780457318),
    DPoint3d::From (689641.57678520982,4626255.6614502864,41.349641780457318),
    DPoint3d::From (689656.42434047582,4626257.1865265863,40.184239337467176),
    DPoint3d::From (689654.48587422201,4626276.0586871440,35.441375649370059),
    DPoint3d::From (689641.57678520982,4626255.6614502864,41.349641780457318),
    DPoint3d::From (689656.42434047582,4626257.1865265863,40.184239337467176),
    DPoint3d::From (689654.48587421828,4626276.0586871384,35.441375651049441),
    DPoint3d::From (689656.42434045894,4626257.1865265844,40.184239338792203),
    DPoint3d::From (689669.62181095406,4626274.7761926036,34.981559643700351),
    DPoint3d::From (689671.27189574204,4626258.7116028881,39.018837675290428),
    DPoint3d::From (689669.62181095406,4626274.7761926036,34.981559643700351),
    DPoint3d::From (689656.42434047582,4626257.1865265863,40.184239337467176)
    };

  bvector<int> indices {
    1,2,3,0,
    4,5,6,0,
    7,8,0,
    9,10,11,0,
    12,13,14,0,
    15,16,17,0,
    18,19,20,0,
    21,22,23,0,
    24,25,26,0,
    27,28,29,0,
    30,31,32,0,
    33,34,35,0,
    36,37,0,
    38,39,40,0,
    41,42,43,0,
    44,45,46,0,
    47,48,0,
    49,50,51,0,
    52,53,54,0,
    55,56,57,0,
    58,59,60,0,
    61,62,63,0,
    64,65,66,0,
    67,68,69,0,
    70,71,72,0,
    73,74,75,0,
    76,77,78,0,
    79,80,0,
    81,82,83,0,
    84,85,86,0,
    87,88,0,
    89,90,91,0,
    92,93,94,0,
    95,96,97,0,
    98,99,100,0,
    101,102,103,0,
    104,105,106,0,
    107,108,109,0,
    110,111,112,0,
    113,114,0,
    115,116,117,0,
    118,119,0,
    120,121,122,0,
    123,124,125,0,
    126,127,128,0,
    129,130,0,
    131,132,133,0,
    134,135,136,0,
    137,138,139,0,
    140,141,142,0,
    143,144,145,0,
    146,147,148,0,
    149,150,151,0,
    152,153,0,
    154,155,156,0,
    157,158,159,0,
    160,161,162,0,
    163,164,165,0,
    166,167,168,0,
    169,170,171,0,
    172,173,174,0,
    175,176,0,
    177,178,179,0,
    180,181,182,0,
    183,184,185,0,
    186,187,188,0,
    189,190,191,0,
    192,193,194,0,
    195,196,0,
    197,198,199,0,
    200,201,0,
    202,203,204,0,
    205,206,207,0,
    208,209,210,0
    };
    return createMesh (points, indices, translateToOrigin);
    }


void exerciseCleanup (PolyfaceHeaderPtr &mesh)
    {
    auto range = mesh->PointRange ();
    auto dX = range.XLength ();
    auto dY = range.YLength ();
    SaveAndRestoreCheckTransform shifter (0,dY,0);
    Check::SaveTransformed (*mesh);
    Check::Shift (dX, 0, 0);
    mesh->MarkTopologicalBoundariesVisible (false);
    Check::SaveTransformed (*mesh);
    Check::Shift (dX, 0, 0);

    mesh->Compress ();
    Check::SaveTransformed (*mesh);
    Check::Shift (dX, 0, 0);

    mesh->MarkTopologicalBoundariesVisible (false);
    Check::SaveTransformed (*mesh);
    Check::Shift (dX, 0, 0);
    }

TEST(Polyface,CleanupV)
    {
    bool translateToOrigin = false;
    auto meshA = CreateVaillancourtMeshA (translateToOrigin);
    exerciseCleanup (meshA);
    auto meshB = CreateVaillancourtMeshB (translateToOrigin);
    exerciseCleanup (meshB);
    Check::ClearGeometry ("Polyface.CleanupV");
    }
