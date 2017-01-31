/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_section.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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