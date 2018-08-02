/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_section.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
    

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SweepLinestring, PointSelect)
    {
    TestLargeMesh (20,20, 1, 1.0, DrapeAction::FastDrapeWithPick);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SweepLinestring, Test3)
    {
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (1,1,20));
    linestring.push_back (DPoint3d::From (3,2,20));
    double edgeLength = 6.0;
    CheckDrape (6,3, edgeLength, edgeLength, linestring, LengthAllowDisconnects (linestring), "6x3 facets grid fully contained drape");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SweepLinestring, Test4)
    {
    bvector<DPoint3d> linestring;
    linestring.push_back (DPoint3d::From (5,5,20));
    linestring.push_back (DPoint3d::From (20,5,20));
    double edgeLength = 10.0;
    CheckDrape (1,1, edgeLength, edgeLength, linestring, 5.0, "1x1 facets grid partially contained drape");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
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

PolyfaceHeaderPtr createMesh (bvector<DPoint3d> &points, bvector<int> &indices, DPoint3dCR origin)
    {
    auto mesh = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
    auto transform = Transform::From (-origin.x, -origin.y, -origin.z);
    mesh->Transform (transform);
    return mesh;
    }
// A mesh from Stephane Poulin
// murky connectivity -- maybe some slivers
PolyfaceHeaderPtr CreatePoulinMeshA (DPoint3dCR origin)
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
    return createMesh (points, indices, origin );
    }


// A mesh from Stephane Poulin
// murky connectivity -- maybe some slivers
PolyfaceHeaderPtr CreatePoulinMeshB (DPoint3dCR origin)
    {
      bvector<DPoint3d> points{
        DPoint3d::From (689271.67534698988,4626205.0004895609,61.492208604956232),
        DPoint3d::From (689277.94141279440,4626143.9964871407,46.160966214851868),
        DPoint3d::From (689286.52290225599,4626206.5255658627,60.745352673233050),
        DPoint3d::From (689292.94810360693,4626143.9722810714,45.024751837497952),
        DPoint3d::From (689301.37045752187,4626208.0506421635,59.998496741514771),
        DPoint3d::From (689307.87166515447,4626144.7573895007,44.091930619085574),
        DPoint3d::From (689316.21801278810,4626209.5757184653,59.251640809764702),
        DPoint3d::From (689322.74844485451,4626145.9979479723,43.273570988545998),
        DPoint3d::From (689331.06556805398,4626211.1007947670,58.504784878075675),
        DPoint3d::From (689337.63721632678,4626147.1217591679,42.425870975382296),
        DPoint3d::From (689345.91312332009,4626212.6258710679,57.757928946330509),
        DPoint3d::From (689352.54459480988,4626148.0644196728,41.532645012132157),
        DPoint3d::From (689360.76067858597,4626214.1509473687,57.011073014612229),
        DPoint3d::From (689367.63594393362,4626147.216012910,40.189296347319498),
        DPoint3d::From (689375.60823385185,4626215.6760236714,56.264217082918307),
        DPoint3d::From (689382.47137838008,4626148.8590928484,39.472096541136636),
        DPoint3d::From (689390.45578911784,4626217.2010999713,55.517361151163506),
        DPoint3d::From (689397.18980241241,4626151.6413410818,39.041187216587304),
        DPoint3d::From (689405.30334438395,4626218.7261762731,54.770505219440331),
        DPoint3d::From (689411.76673878450,4626155.8010586686,38.956457089490257),
        DPoint3d::From (689420.15089965006,4626220.2512525730,54.023649287709688),
        DPoint3d::From (689426.28930539452,4626160.4900993090,39.004753971941639),
        DPoint3d::From (689434.99845491571,4626221.7763288757,53.276793356028129),
        DPoint3d::From (689440.71749095165,4626166.0979975164,39.283973867254346),
        DPoint3d::From (689449.84601018217,4626223.3014051756,52.529937424248601),
        DPoint3d::From (689455.24135649786,4626170.7743922174,39.329092631312506),
        DPoint3d::From (689464.69356544805,4626224.8264814774,51.783081492537789),
        DPoint3d::From (689469.77692916919,4626175.3368108580,39.345567460965597),
        DPoint3d::From (689479.54112071404,4626226.3515577791,51.036225560836399),
        DPoint3d::From (689484.27371372492,4626180.2768556438,39.456945538886657),
        DPoint3d::From (689494.38867598015,4626227.876634080,50.289369629091226),
        DPoint3d::From (689498.85779589624,4626184.3670044830,39.354731716858986),
        DPoint3d::From (689509.23623124580,4626229.4017103808,49.542513697385310),
        DPoint3d::From (689513.45714925113,4626188.3084789580,39.215153744817535),
        DPoint3d::From (689524.08378651179,4626230.9267866835,48.795657765669596),
        DPoint3d::From (689528.04704285169,4626192.3420499666,39.098721044976429),
        DPoint3d::From (689538.93134177790,4626232.4518629843,48.048801833938953),
        DPoint3d::From (689542.60914739477,4626196.6461645160,39.050280091333107),
        DPoint3d::From (689553.77889704402,4626233.9769392852,47.301945902193786),
        DPoint3d::From (689557.19706155802,4626200.6990065593,38.938690498751555),
        DPoint3d::From (689568.62645231013,4626235.5020155869,46.555089970485135),
        DPoint3d::From (689572.02698808396,4626202.3957094811,38.234966971771087),
        DPoint3d::From (689583.47400757612,4626237.0270918887,45.808234038754698),
        DPoint3d::From (689586.93456946849,4626203.3363946220,37.341244569609543),
        DPoint3d::From (689598.3215628420,4626238.5521681886,44.815849109369296),
        DPoint3d::From (689601.64979089284,4626206.1498222155,36.672642099680345),
        DPoint3d::From (689613.16911810811,4626240.0772444913,43.650446666403511),
        DPoint3d::From (689616.17229887773,4626210.8394336104,36.302535910133415),
        DPoint3d::From (689628.01667337422,4626241.6023207912,42.485044223413361),
        DPoint3d::From (689630.65088037844,4626215.9566963250,36.039905062956755),
        DPoint3d::From (689642.86422864033,4626243.1273970939,41.319641780440307),
        DPoint3d::From (689645.24922618025,4626219.9079799606,35.484245772064668),
        DPoint3d::From (689657.71178390586,4626244.6524733938,40.154239337482146),
        DPoint3d::From (689659.90808802517,4626223.2701027049,34.780521303964441),
        DPoint3d::From (689672.55933917209,4626246.1775496956,38.988837675298136),
        DPoint3d::From (689674.49052413006,4626227.3762770090,34.263789196955919)
        };

      bvector<int> indices {
        3,1,-2,0,
        2,4,-3,0,
        5,3,-4,0,
        4,6,-5,0,
        7,5,-6,0,
        6,8,-7,0,
        9,7,-8,0,
        8,10,-9,0,
        11,9,-10,0,
        10,12,-11,0,
        13,11,-12,0,
        12,14,-13,0,
        13,14,-16,0,
        16,15,-13,0,
        15,16,-18,0,
        18,17,-15,0,
        17,18,-20,0,
        20,19,-17,0,
        19,20,-22,0,
        22,21,-19,0,
        21,22,-24,0,
        24,23,-21,0,
        23,24,-26,0,
        26,25,-23,0,
        25,26,-28,0,
        28,27,-25,0,
        27,28,-30,0,
        30,29,-27,0,
        29,30,-32,0,
        32,31,-29,0,
        31,32,-34,0,
        34,33,-31,0,
        33,34,-36,0,
        36,35,-33,0,
        35,36,-38,0,
        38,37,-35,0,
        37,38,-40,0,
        40,39,-37,0,
        39,40,-42,0,
        42,41,-39,0,
        43,41,-42,0,
        42,44,-43,0,
        43,44,-46,0,
        46,45,-43,0,
        45,46,-48,0,
        48,47,-45,0,
        47,48,-50,0,
        50,49,-47,0,
        49,50,-52,0,
        52,51,-49,0,
        51,52,-54,0,
        54,53,-51,0,
        53,54,-56,0,
        56,55,-53,0
        };
    return createMesh (points, indices, origin );
    }


// A mesh from Stephane Poulin.
// murky connectivity -- maybe some slivers
PolyfaceHeaderPtr CreatePoulinMeshC (DPoint3dCR origin)
    {        bvector<DPoint3d> points{
        DPoint3d::From (689276.85120336222,4626115.0876722513,62.872957797183588),
        DPoint3d::From (689271.30354028917,4626179.4876577742,46.713334699677091),
        DPoint3d::From (689291.39772159746,4626116.3407654222,60.946158483480126),
        DPoint3d::From (689286.07709366141,4626178.1052160710,45.447859295835336),
        DPoint3d::From (689305.94423983281,4626117.5938585931,59.249684197898716),
        DPoint3d::From (689300.94712333078,4626175.6028303299,44.693731864801698),
        DPoint3d::From (689320.49075806804,4626118.8469517631,57.952835870508359),
        DPoint3d::From (689315.69197617169,4626174.5535584223,43.974606522534266),
        DPoint3d::From (689335.03727630351,4626120.1000449350,57.018986056857962),
        DPoint3d::From (689330.34494545334,4626174.5709157977,43.350834802059609),
        DPoint3d::From (689349.58379453851,4626121.3531381069,56.150478580336525),
        DPoint3d::From (689345.03952085157,4626174.1052890290,42.913598640875591),
        DPoint3d::From (689364.13031277398,4626122.6062312759,55.281971103754316),
        DPoint3d::From (689359.62515603262,4626174.9042935632,42.159033755219944),
        DPoint3d::From (689378.67683100956,4626123.8593244478,54.413463627196577),
        DPoint3d::From (689374.18214950489,4626176.0357850669,41.321039283996306),
        DPoint3d::From (689393.22334924492,4626125.1124176187,53.544956150667339),
        DPoint3d::From (689388.65578584268,4626178.1349269422,40.240236291238489),
        DPoint3d::From (689407.76986747992,4626126.3655107915,52.676448674160426),
        DPoint3d::From (689403.26961151592,4626178.6066824654,39.567786652340061),
        DPoint3d::From (689422.31638571573,4626127.6186039606,51.807941197571495),
        DPoint3d::From (689418.05873890151,4626177.0434499942,39.405968168165117),
        DPoint3d::From (689436.86290395109,4626128.8716971315,50.939433721020478),
        DPoint3d::From (689432.92291477276,4626174.6090180008,39.462756191893185),
        DPoint3d::From (689451.40942218632,4626130.1247903034,50.070926244491240),
        DPoint3d::From (689447.76717120328,4626172.4058206212,39.461521466210058),
        DPoint3d::From (689465.95594042155,4626131.3778834734,49.202418767931348),
        DPoint3d::From (689462.54163034051,4626171.0128643513,39.256976276678678),
        DPoint3d::From (689480.50245865714,4626132.6309766453,48.333911291359094),
        DPoint3d::From (689477.34175980522,4626169.3219143702,39.127205421971325),
        DPoint3d::From (689495.04897689249,4626133.8840698162,47.465403814822594),
        DPoint3d::From (689492.07938039291,4626168.3565979926,38.815354315619004),
        DPoint3d::From (689509.59549512784,4626135.1371629871,46.596896338293362),
        DPoint3d::From (689506.78628556477,4626167.7478411831,38.414033185168478),
        DPoint3d::From (689524.14201336284,4626136.3902561590,45.728388861756329),
        DPoint3d::From (689521.52221958560,4626166.8021032978,38.097269328321296),
        DPoint3d::From (689538.68853159831,4626137.6433493290,44.859881385204240),
        DPoint3d::From (689536.27988912899,4626165.6040488379,37.843818230408118),
        DPoint3d::From (689553.23504983366,4626138.8964425009,43.991373908660478),
        DPoint3d::From (689551.15932085738,4626162.9925194066,37.945044584542735),
        DPoint3d::From (689567.78156806924,4626140.1495356718,43.122866432110001),
        DPoint3d::From (689565.94500494131,4626161.4692584593,37.773196210882894),
        DPoint3d::From (689582.32808630471,4626141.4026288409,42.254358955527792),
        DPoint3d::From (689580.71079894039,4626160.1768913483,37.543410598116836),
        DPoint3d::From (689596.87460453983,4626142.6557220127,41.385851479012544),
        DPoint3d::From (689595.45245584438,4626159.1647194447,37.243316756006870),
        DPoint3d::From (689611.42112277541,4626143.9088151837,40.517344002447544),
        DPoint3d::From (689610.15744600131,4626158.5781930592,36.836417436747325),
        DPoint3d::From (689625.96764101065,4626145.1619083565,39.648836525904862),
        DPoint3d::From (689624.88282211637,4626157.7550165122,36.488899768572786),
        DPoint3d::From (689640.51415924612,4626146.4150015265,38.847494304617719),
        DPoint3d::From (689639.30971045396,4626160.3968320321,35.339091168565631),
        DPoint3d::From (689655.06067748135,4626147.6680946983,38.310347824329511),
        DPoint3d::From (689653.59288297186,4626164.7069710549,34.034852764883034),
        DPoint3d::From (689669.60719571670,4626148.9211878693,37.773201344018453),
        DPoint3d::From (689668.23257188522,4626164.8784934385,33.769100387083881),
        DPoint3d::From (689684.15371395205,4626150.1742810402,37.236054863737515),
        DPoint3d::From (689682.76242688531,4626166.3250215910,33.183416063893539)
        };

    bvector<int> indices {
        -1,4,2,0,
        -4,1,3,0,
        -3,6,4,0,
        -6,3,5,0,
        -5,8,6,0,
        -8,5,7,0,
        -7,10,8,0,
        -10,7,9,0,
        -9,12,10,0,
        -12,9,11,0,
        -11,14,12,0,
        -14,11,13,0,
        -13,16,14,0,
        -16,13,15,0,
        -17,16,15,0,
        -16,17,18,0,
        -17,20,18,0,
        -20,17,19,0,
        -19,22,20,0,
        -22,19,21,0,
        -21,24,22,0,
        -24,21,23,0,
        -23,26,24,0,
        -26,23,25,0,
        -25,28,26,0,
        -28,25,27,0,
        -27,30,28,0,
        -30,27,29,0,
        -29,32,30,0,
        -32,29,31,0,
        -31,34,32,0,
        -34,31,33,0,
        -33,36,34,0,
        -36,33,35,0,
        -35,38,36,0,
        -38,35,37,0,
        -37,40,38,0,
        -40,37,39,0,
        -39,42,40,0,
        -42,39,41,0,
        -41,44,42,0,
        -44,41,43,0,
        -43,46,44,0,
        -46,43,45,0,
        -45,48,46,0,
        -48,45,47,0,
        -47,50,48,0,
        -50,47,49,0,
        -51,50,49,0,
        -50,51,52,0,
        -53,52,51,0,
        -52,53,54,0,
        -53,56,54,0,
        -56,53,55,0,
        -57,56,55,0,
        -56,57,58,0
        };
    return createMesh (points, indices, origin );
    }

// A mesh from Stephane Poulin.
// murky connectivity -- maybe some slivers
PolyfaceHeaderPtr CreatePoulinMeshD (DPoint3dCR origin)
    {
    bvector<DPoint3d> points{
        DPoint3d::From (689277.93261092796,4626102.5341644669,62.842957797182002),
        DPoint3d::From (689284.84289138461,4626022.3162500896,42.714206968121289),
        DPoint3d::From (689292.47912916285,4626103.7872576388,60.916158483492516),
        DPoint3d::From (689299.20458931837,4626025.7148276903,41.325765224743279),
        DPoint3d::From (689307.02564739855,4626105.0403508097,59.219684197898736),
        DPoint3d::From (689313.57058130309,4626029.0635578427,40.155140494611317),
        DPoint3d::From (689321.57216563378,4626106.2934439806,57.922835870515641),
        DPoint3d::From (689328.01960975653,4626031.4483600706,39.142267259354199),
        DPoint3d::From (689336.11868386914,4626107.5465371506,56.988986056849114),
        DPoint3d::From (689342.64663684380,4626031.7668671263,37.973905601647445),
        DPoint3d::From (689350.66520210472,4626108.7996303225,56.120478580299704),
        DPoint3d::From (689357.27080547519,4626032.1185565302,36.879212590653189),
        DPoint3d::From (689365.21172034007,4626110.0527234944,55.251971103770472),
        DPoint3d::From (689371.70306653285,4626034.6980028888,36.343521455961564),
        DPoint3d::From (689379.75823857530,4626111.3058166634,54.383463627187723),
        DPoint3d::From (689386.05144367716,4626038.2512147250,36.052173281818270),
        DPoint3d::From (689394.30475681089,4626112.5589098353,53.514956150651770),
        DPoint3d::From (689400.31397740578,4626042.8009389080,36.010875845883390),
        DPoint3d::From (689408.85127504612,4626113.8120030072,52.646448674123612),
        DPoint3d::From (689414.64732219395,4626046.5286536822,35.763314974886356),
        DPoint3d::From (689423.39779328147,4626115.0650961772,51.777941197571515),
        DPoint3d::From (689429.00030527962,4626050.0283975080,35.458550293141798),
        DPoint3d::From (689437.94431151671,4626116.3181893481,50.909433721011624),
        DPoint3d::From (689443.29061746027,4626054.2556558372,35.336337829041625),
        DPoint3d::From (689452.49082975206,4626117.5712825190,50.040926244482392),
        DPoint3d::From (689457.56124424422,4626058.7114318730,35.271466371510151),
        DPoint3d::From (689467.03734798741,4626118.8243756909,49.172418767938638),
        DPoint3d::From (689471.85351183556,4626062.9159908369,35.143558048981006),
        DPoint3d::From (689481.58386622276,4626120.0774688618,48.303911291387621),
        DPoint3d::From (689486.19864133245,4626066.5069029545,34.861669850763441),
        DPoint3d::From (689496.13038445811,4626121.3305620318,47.435403814821008),
        DPoint3d::From (689500.48523146298,4626070.7773686564,34.750299234361876),
        DPoint3d::From (689510.67690269346,4626122.5836552037,46.566896338284515),
        DPoint3d::From (689514.85863171378,4626074.0401000371,34.386061996082653),
        DPoint3d::From (689525.22342092870,4626123.8367483756,45.698388861763611),
        DPoint3d::From (689529.12802744668,4626078.5101667261,34.324776433408459),
        DPoint3d::From (689539.76993916417,4626125.0898415465,44.829881385204260),
        DPoint3d::From (689543.28429117939,4626084.2935249778,34.593029712679012),
        DPoint3d::From (689554.31645739952,4626126.3429347165,43.961373908637647),
        DPoint3d::From (689557.47825878591,4626089.6391982380,34.751456500317218),
        DPoint3d::From (689568.86297563487,4626127.5960278884,43.092866432108416),
        DPoint3d::From (689571.63846370555,4626095.3768052645,35.008229613509542),
        DPoint3d::From (689583.40949387022,4626128.8491210593,42.224358955564654),
        DPoint3d::From (689585.79868639703,4626101.1142059937,35.264950961255479),
        DPoint3d::From (689597.95601210569,4626130.1022142284,41.355851478982444),
        DPoint3d::From (689599.97040236334,4626106.7181871673,35.488193889538621),
        DPoint3d::From (689612.50253034127,4626131.3553074002,40.487344002424706),
        DPoint3d::From (689614.16177196614,4626112.0940193152,35.654188322393701),
        DPoint3d::From (689627.04904857627,4626132.6084005721,39.618836525917793),
        DPoint3d::From (689628.32243675739,4626117.8262879318,35.909621890146965),
        DPoint3d::From (689641.59556681162,4626133.8614937440,38.817494304654581),
        DPoint3d::From (689643.04881973390,4626116.9914232437,34.584357003523778),
        DPoint3d::From (689656.14208504732,4626135.1145869140,38.280347824313942),
        DPoint3d::From (689657.84541817266,4626115.3414631365,33.318759322381055),
        DPoint3d::From (689670.68860328244,4626136.3676800849,37.743201344016867),
        DPoint3d::From (689672.36991646746,4626116.8501745397,32.845754072276726),
        DPoint3d::From (689685.23512151767,4626137.6207732568,37.206054863735929),
        DPoint3d::From (689686.84643139574,4626118.9159003263,32.512518146584640)
        };

    bvector<int> indices {
        1,2,-4,0,        4,3,-1,0,        3,4,-6,0,        6,5,-3,0,        5,6,-8,0,        8,7,-5,0,        9,7,-8,0,        8,10,-9,0,
        11,9,-10,0,        10,12,-11,0,        11,12,-14,0,        14,13,-11,0,        13,14,-16,0,        16,15,-13,0,        15,16,-18,0,
        18,17,-15,0,        17,18,-20,0,        20,19,-17,0,        19,20,-22,0,        22,21,-19,0,        21,22,-24,0,        24,23,-21,0,
        23,24,-26,0,        26,25,-23,0,        25,26,-28,0,        28,27,-25,0,        27,28,-30,0,        30,29,-27,0,        29,30,-32,0,
        32,31,-29,0,        31,32,-34,0,        34,33,-31,0,        33,34,-36,0,        36,35,-33,0,        35,36,-38,0,        38,37,-35,0,
        37,38,-40,0,        40,39,-37,0,        39,40,-42,0,        42,41,-39,0,        41,42,-44,0,        44,43,-41,0,        43,44,-46,0,
        46,45,-43,0,        45,46,-48,0,        48,47,-45,0,        47,48,-50,0,        50,49,-47,0,        51,49,-50,0,        50,52,-51,0,
        53,51,-52,0,        52,54,-53,0,        53,54,-56,0,        56,55,-53,0,        55,56,-58,0,        58,57,-55,0        };
    return createMesh (points, indices, origin );
    }
// ASSUME mesh is indexed, terminated triangles.
// excise all faces except the range i0 <= i < i1
bool trimIndicesFromIndexedTriangles (PolyfaceHeaderPtr &mesh, size_t i0, size_t i1)
    {
    size_t k0 = 4 * i0; // beginning of first facet
    size_t k1 = 4 * i1; // tail of last facet
    bvector<int> &indices = mesh->PointIndex ();
    if (k1 <= k0 ||  k1 > indices.size ())
        return false;
    for (size_t k = k0; k < k1; k++)
        indices[k - k0] = indices[k];
    indices.resize (k1-k0);
    mesh->Compress ();
    mesh->MarkAllEdgesVisible ();
    return true;
    }
void SaveTransformed (bvector<PolyfaceHeaderPtr> &meshes, double xShiftFactor = 0.0, bool markAndSaveBoundary = false)
    {
    DRange3d range;
    range.Init ();
    for (auto &mesh : meshes)
        {
        if (mesh.IsValid ())
            {
            Check::SaveTransformed (*mesh);
            range.Extend (mesh->PointRange ());
            }
        }
    if (markAndSaveBoundary)
        {
        double dY = 2.0 * range.YLength ();
        Check::Shift (0, dY, 0);
        for (auto &mesh : meshes)
            {
            if (mesh.IsValid ())
                {
                //printf (" precompress %d %d\n", (int)mesh->Point().size (), (int)mesh->PointIndex().size ());
                mesh->Compress ();
                Check::SaveTransformed (mesh);
                Check::Shift (0, dY, 0);
                //printf ("postcompress %d %d\n", (int)mesh->Point().size (), (int)mesh->PointIndex().size ());
                mesh->MarkTopologicalBoundariesVisible (false);
                Check::SaveTransformed (mesh);
                Check::Shift (0, dY, 0);
                size_t numOpen, numClosed;
                auto boundary = mesh->ExtractBoundaryStrings (numOpen, numClosed);
                if (boundary.IsValid () && boundary->size () > 0)
                    Check::SaveTransformed (*boundary);
                Check::Shift (0, -2.0 * dY, 0);
                }
            }
        Check::Shift (0, -dY, 0);
        }
    if (xShiftFactor != 0.0)
        Check::Shift (xShiftFactor * range.XLength (), 0, 0);
    }
void exerciseCleanup (bvector<PolyfaceHeaderPtr> &source)
    {
    auto range = source[0]->PointRange ();
    range.Extend (source[1]->Point ());
    auto dX = range.XLength ();
    // unused - auto dY = 2.0 * range.YLength ();
    //SaveAndRestoreCheckTransform shifter (0,dY,0);
    Check::Shift (dX, 0, 0);
    bvector<PolyfaceHeaderPtr> dest;
    DVec3d vectorToEye = DVec3d::From(0.0, 0.0, 1.0);
    Transform localToWorld = Transform::FromIdentity();
    Transform worldToLocal = Transform::FromIdentity();
    SaveTransformed (source);
    PolyfaceHeader::VisibleParts (source, vectorToEye, dest, nullptr, localToWorld, worldToLocal);
    Check::Shift (dX, 0, 0);
    SaveTransformed (dest, 1.0, true);
#ifdef extraOutput
    Check::Shift (dX, 0, 0);
    for (auto &mesh : dest)
        {
        mesh->Compress ();
        Check::SaveTransformed (mesh);
        }
    Check::Shift (dX, 0, 0);

    for (auto &mesh : dest)
        {
        mesh->Compress ();
        mesh->MarkTopologicalBoundariesVisible (false);
        size_t numOpen, numClosed;
        auto boundary = mesh->ExtractBoundaryStrings (numOpen, numClosed);
        Check::SaveTransformed (*boundary);
        if (numOpen + numClosed != 1)
            {
            PolyfaceHeaderPtr mesh1;
            mesh->Compress ();
            PolyfaceHeader::HealVerticalPanels (*mesh, false, false, mesh1, true);
            if (mesh1.IsValid ())
                {
                Check::Shift (0, dY, 0);
                mesh1->MarkTopologicalBoundariesVisible (false);
                Check::SaveTransformed (mesh1);
                Check::Shift (0, -dY, 0);
                }
            }
        }
#endif
    Check::Shift (2.0 * dX, 0, 0);
    }

// test the full set of 4 meshes ...
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CleanupVisiblesA)
    {
    DPoint3d origin = DPoint3d::From (0,0,0);
    bvector<PolyfaceHeaderPtr> meshes;
    meshes.push_back (CreatePoulinMeshA (origin));
    meshes.push_back (CreatePoulinMeshB (origin));
    meshes.push_back (CreatePoulinMeshC (origin));
    meshes.push_back (CreatePoulinMeshD (origin));
#ifdef ScaleDown
    double s = 1.0e-3;
    auto scale = Transform::FromMatrixAndFixedPoint (RotMatrix::FromScale (s), origin);
    for (auto &mesh : meshes)
        mesh->Transform (scale);
#endif
    exerciseCleanup (meshes);

    Check::ClearGeometry ("Polyface.CleanupVisiblesA");

    }
// isolate the problem area
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CleanupVisiblesB)
    {
    DPoint3d origin = DPoint3d::From (0,0,0);
    size_t iStart = 3;
    size_t numI = 1;
    size_t jStart = 3;
    size_t numJ = 1;
    // there are two meshes 
    // problems appear "early" in both meshes
    // grab combinations of 2 facets from each 
    for (size_t i0 = iStart; i0 < iStart + numI; i0++)
        {
        for (size_t j0 = jStart; j0 < jStart + numJ; j0++)
            {
            for (size_t numITriangle = 2; numITriangle < 3; numITriangle++)
                {
                for (size_t numJTriangle = 2; numJTriangle < 3; numJTriangle++)
                    {
                    bvector<PolyfaceHeaderPtr> meshes;
                    meshes.push_back (CreatePoulinMeshB (origin));
                    meshes.push_back (CreatePoulinMeshC (origin));
                    trimIndicesFromIndexedTriangles (meshes[0], i0, i0 + numITriangle);    
                    trimIndicesFromIndexedTriangles (meshes[1], j0, j0 + numJTriangle);
                    exerciseCleanup (meshes);
                    }
                }
            }
        }
    Check::ClearGeometry ("Polyface.CleanupVisiblesB");
    }
// unused - static int s_noisyCleanup = false;


void doUndercut (PolyfaceHeaderPtr &meshA, PolyfaceHeaderPtr &meshB)
    {
    auto range = meshA->PointRange ();
    range.Extend (meshB->Point ());
    auto dX = 1.5 * range.XLength ();
    auto dY = range.YLength ();
//    auto dZ = 1.5 * range.ZLength ();
//    double zMax = range.high.z + 0.1 * dZ;
    Check::SaveTransformed (DSegment3d::From (range.low, range.low + DVec3d::From (dX, 0, 0)));


    PolyfaceHeaderPtr meshAOverB, meshBUnderA;
    PolyfaceHeader::ComputeOverAndUnderXY (*meshA, nullptr, *meshB, nullptr, meshAOverB, meshBUnderA);

    Check::SaveTransformed (meshA);
    Check::SaveTransformed (meshB);
    Check::Shift (dX, 0, 0);
    Check::SaveTransformed (meshAOverB);
    Check::SaveTransformed (meshBUnderA);
    Check::Shift (0, dY, 0);
    if (Check::True (meshBUnderA.IsValid ()))
        {
        PolyfaceHeaderPtr meshBHidden, meshBVisible;
        PolyfaceHeader::ComputePunchXYByPlaneSets (*meshBUnderA, *meshB, &meshBHidden, &meshBVisible);
        Check::SaveTransformed (meshBHidden);
        Check::SaveTransformed (meshBVisible);
        }
    Check::Shift (2.0 * dX, -dY, 0);
    }
// test a single pair
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CleanupByUndercutAndPunchA)
    {
    DPoint3d origin = DPoint3d::From (0,0,0);
    auto meshB = CreatePoulinMeshB (origin);
    auto meshC = CreatePoulinMeshC (origin);

    doUndercut (meshB, meshC);
    doUndercut (meshC, meshB);

    Check::ClearGeometry ("Polyface.CleanupByUndercutAndPunchA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CleanupByUndercutAndPunchMany)
    {
    DPoint3d origin = DPoint3d::From (0,0,0);
    bvector<PolyfaceHeaderPtr> allMesh, visibleParts;
    
    allMesh.push_back (CreatePoulinMeshA (origin));
    allMesh.push_back (CreatePoulinMeshB (origin));
    allMesh.push_back (CreatePoulinMeshC (origin));
    allMesh.push_back (CreatePoulinMeshD (origin));
    SaveTransformed (allMesh, true);
    PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
    SaveTransformed (visibleParts, 1.2, true);

    // make rotated copies for a messier sequence ..
    auto range = allMesh.front ()->PointRange ();
    range.Extend (allMesh.back ()->PointRange ());
    DPoint3d rotationCenter = range.LocalToGlobal (0.5, 0,0);
    auto rotate = Transform::FromMatrixAndFixedPoint (
        RotMatrix::FromAxisAndRotationAngle (2, 0.56),
        rotationCenter);
    size_t num0 = allMesh.size ();
    for (size_t i = 0; i < num0; i++)
        {
        allMesh.push_back (allMesh[i]->Clone ());
        allMesh.back()->Transform (rotate);
        }
    PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
    SaveTransformed (visibleParts, 1.2, true);
    Check::ClearGeometry ("Polyface.CleanupByUndercutAndPunchMany");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityA)
    {
// 1
  bvector<DPoint3d> points1{ 
    DPoint3d::From (687745.24628939060,4628331.8363190368,36.652027632865682), 
    DPoint3d::From (687760.36752856954,4628361.5381623916,43.846792983534229), 
    DPoint3d::From (687760.20898026542,4628328.9707649322,35.704847135857555), 
    DPoint3d::From (687775.34381491505,4628361.4652531333,43.597781970460026), 
    DPoint3d::From (687775.16577092919,4628324.8932489594,34.454672579566953), 
    DPoint3d::From (687790.32010126056,4628361.3923438741,43.348770957340776), 
    DPoint3d::From (687790.11437440885,4628319.1340039419,32.784060780857857)
    };

  bvector<int> indices1 {
     1,3,-2,0,
     4,2,-3,0,
     3,5,-4,0,
     6,4,-5,0,
     5,7,-6,0
    };

//3


  bvector<DPoint3d> points3{
    DPoint3d::From (687790.32010126056,4628361.3923438741,43.348770957340776), 
    DPoint3d::From (687790.11437440885,4628319.1340039419,32.784060780857857), 
    DPoint3d::From (687805.29638760595,4628361.3194346149,43.099759944223194), 
    DPoint3d::From (687805.07790107594,4628316.4401299022,31.879800807703337), 
    DPoint3d::From (687820.27267395146,4628361.2465253556,42.850748931148992), 
    DPoint3d::From (687820.04391743848,4628314.2576641906,31.103394431650546), 
    DPoint3d::From (687835.24896029697,4628361.1736160973,42.601737917846535), 
    DPoint3d::From (687835.01353934931,4628312.8158138227,30.512144085745369), 
    DPoint3d::From (687850.22524664237,4628361.1007068399,42.352726904589119), 
    DPoint3d::From (687849.98879983032,4628312.5321817584,30.210451746826006), 
    DPoint3d::From (687865.20153298776,4628361.0277975798,42.103715891654751)
    };

  bvector<int> indices3 {
     3,1,-2,0,
    2, 4,-3,0,
    5,3, -4,0,
    4,6,-5, 0,
    7,5,-6,0
    };

//4

  bvector<DPoint3d> points4{  
    DPoint3d::From (687762.44792489847,4628315.5641302364,39.670177809029227), 
    DPoint3d::From (687762.84361512598,4628333.8682841547,35.093070224001082), 
    DPoint3d::From (687777.44245050137,4628315.2399859298,39.299826872879393), 
    DPoint3d::From (687777.90394909563,4628336.5883557433,33.961487508610183), 
    DPoint3d::From (687792.43697610416,4628314.9158416269,38.929475936724444), 
    DPoint3d::From (687793.01376984117,4628341.597626050,32.257471406774144)
    };

  bvector<int> indices4 {
     -3,2,1,0,
    -2, 3,4,0,
    -5,4, 3,0,
    -4,5,6,0
    };

//6

  bvector<DPoint3d> points6{
    DPoint3d::From (687792.43697610416,4628314.9158416269,38.929475936724444), 
    DPoint3d::From (687793.01376984117,4628341.597626050,32.257471406774144), 
    DPoint3d::From (687807.43150170683,4628314.5916973222,38.559125000565956), 
    DPoint3d::From (687808.05389933405,4628343.3830628404,31.359601980809096), 
    DPoint3d::From (687822.42602730950,4628314.2675530165,38.188774064448523), 
    DPoint3d::From (687823.07241255429,4628344.1685568560,30.711776653119280), 
    DPoint3d::From (687837.42055291240,4628313.9434087127,37.818423128289325), 
    DPoint3d::From (687838.08650191897,4628344.7494086502,30.115123833274936)
    };

  bvector<int> indices6 {
     -3,2,1,0,
    -2, 3,4,0,
    -5,4, 3,0,
    -4,5,6, 0,
    -7,6,5,0
    };


    bvector<PolyfaceHeaderPtr> allMesh, visibleParts, visibleParts2;
    DPoint3d origin = DPoint3d::From (0,0,0);
    allMesh.push_back (createMesh (points1, indices1, origin));
    allMesh.push_back (createMesh (points3, indices3, origin));
    allMesh.push_back (createMesh (points4, indices4, origin));
    allMesh.push_back (createMesh (points6, indices6, origin));
    SaveTransformed (visibleParts, 1.2, true);
    PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
    SaveTransformed (visibleParts, 1.2, true);
    PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (visibleParts, visibleParts2);
    SaveTransformed (visibleParts2, 1.2, true);
    Check::ClearGeometry ("Polyface.MultiMeshVisibilityA");
    }

void RunSelectiveVisibility (double dX, bvector<PolyfaceHeaderPtr> &allMesh, bvector<size_t> indices, size_t numFacetsAtEachEnd)
    {
    bvector<PolyfaceHeaderPtr> activeMesh, visibleParts;
    for( auto i : indices)
        {
        if (numFacetsAtEachEnd == 0)
            activeMesh.push_back (allMesh[i]);
        else
            {
            auto numFacets = allMesh[i]->GetNumFacet ();
            IFacetOptionsPtr options = IFacetOptions::Create ();
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            auto visitor = PolyfaceVisitor::Attach (*allMesh[i]);
            size_t facetIndex = 0;
            bvector<ptrdiff_t> readIndices;
            for (visitor->Reset (); visitor->AdvanceToNextFace (); facetIndex++)
                {
                if (facetIndex < numFacetsAtEachEnd || facetIndex + numFacetsAtEachEnd >= numFacets)
                    readIndices.push_back (visitor->GetReadIndex ());
                }
            bvector<PolyfaceHeaderPtr> result;
            allMesh[i]->CopyPartitions (readIndices, result);
            activeMesh.push_back (result[0]);
            }
        }
    PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (activeMesh, visibleParts);
    Check::Shift (dX, 0, 0);
    SaveTransformed (visibleParts, 0.0, true);
    }

bool DGNJSFileToGeometry (BeFileName &filename, bvector<IGeometryPtr> &geometry);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityB)
    {
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    BeFileName outputPath;
    BeTest::GetHost().GetOutputRoot(outputPath);
    printf (" DocumnetRoot %ls\n", dataPath.c_str ());
    printf (" OutputPath %ls\n", outputPath.c_str ());
    dataPath.AppendToPath (L"GeomLibsTestData");
    dataPath.AppendToPath (L"Polyface");
    dataPath.AppendToPath (L"MultiMeshVisibilityB");
    bvector<IGeometryPtr> g0;
    bvector<WCharCP> filenames {
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_0.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_1.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_2.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_3.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_4.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_5.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_6.dgnjs",
        L"BeforeMultiMeshVisiblePartsXYByPlaneSets_7.dgnjs"
        };
    bvector<PolyfaceHeaderPtr> allMesh;
    auto range = DRange3d::NullRange ();
    static double s_scale = 1.0 / 1024.0;
    for (auto &filename : filenames)
        {
        bvector<IGeometryPtr> geometry;
        auto fullPath = dataPath;
        fullPath.AppendToPath (filename);
        if (DGNJSFileToGeometry (fullPath, geometry))
            {
            for (auto g : geometry)
                {
                auto mesh = g->GetAsPolyfaceHeader ();
                if (mesh.IsValid ())
                    {
                    for (auto &xyz : mesh->Point ())
                        xyz.Scale (s_scale);
                    allMesh.push_back (mesh);
                    range.Extend (mesh->Point ());
                    }
                }
            }
        }
    if (allMesh.size () > 0)
        {
        SaveTransformed (allMesh, 0.0, true);
        double dX = 1.5 * range.XLength ();
        RunSelectiveVisibility (dX, allMesh, bvector<size_t> {1,3,4,6}, 0);
        RunSelectiveVisibility (dX, allMesh, bvector<size_t> {3,4,6}, 4);
        RunSelectiveVisibility (dX, allMesh, bvector<size_t> {3,4,6}, 3);
        RunSelectiveVisibility (dX, allMesh, bvector<size_t> {3,4,6}, 2);

        Check::ClearGeometry ("Polyface.MultiMeshVisibilityB");
        }
    }

struct MeshAreaCounts : SignCounter
{
void AddToCounts (PolyfaceHeaderPtr &mesh)
    {
    auto visitor = PolyfaceVisitor::Attach (*mesh);
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        auto area = PolygonOps::AreaXY (visitor->Point ());
        Announce (area);
        }
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityC)
    {
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);

    bvector<PolyfaceHeaderPtr> allMesh;
    auto range = DRange3d::NullRange ();
    static double s_scale = 1.0 / 1024.0;

    dataPath.AppendToPath (L"GeomLibsTestData");
    dataPath.AppendToPath (L"Polyface");
    dataPath.AppendToPath (L"MultiMeshVisibilityC");
    bvector<IGeometryPtr> g0;

    for (auto fileNumber : bvector<uint32_t>{9,18})
        {
        bvector<IGeometryPtr> geometry;
        auto fullPath = dataPath;
        WString filename;
        filename.Sprintf (L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_%d.dgnjs", fileNumber);
        fullPath.AppendToPath (filename.c_str ());

        if (!DGNJSFileToGeometry (fullPath, geometry))
            break;
        static int s_fixAreas = 1;
        for (auto g : geometry)
            {
            auto mesh = g->GetAsPolyfaceHeader ();
            if (mesh.IsValid ())
                {
                MeshAreaCounts areas;
                areas.AddToCounts (mesh);
                if (s_fixAreas && areas.NumPositive () == 0 && areas.NumNegative () > 0)
                    mesh->ReverseIndicesAllFaces ();
                for (auto &xyz : mesh->Point ())
                    xyz.Scale (s_scale);
                allMesh.push_back (mesh);
                range.Extend (mesh->Point ());
                }
            }
        }
    if (allMesh.size () > 0)
        {
        SaveTransformed (allMesh, 0.0, true);
        double dX = 1.5 * range.XLength ();
        bvector<PolyfaceHeaderPtr> visibleParts;
        PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
        Check::Shift (dX, 0, 0);
        SaveTransformed (visibleParts, 0.0, true);

        Check::ClearGeometry ("Polyface.MultiMeshVisibilityC");
        }
    }


bool ReadDgnjsGeometry (bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC)
    {
    static double s_scale = 10000.0;
    s_scale = 1.0;	
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    dataPath.AppendToPath (L"GeomLibsTestData");
    if (nameA)
        dataPath.AppendToPath (nameA);
    if (nameB)
        dataPath.AppendToPath (nameB);
    if (nameC)
        dataPath.AppendToPath (nameC);

    bool stat = DGNJSFileToGeometry (dataPath, geometry);
    if (!Check::True (stat, "Read geometry from DGNJS file\n"))
        {
        printf ("   File %ls\n", dataPath.c_str ());
        return false;
        }
    if (!Check::True (geometry.size () >= minGeometry, "Insufficient geometry count"))
        return false;
    auto scale = Transform::FromFixedPointAndScaleFactors (DPoint3d::FromZero (), 1.0 / s_scale, 1.0 / s_scale, 1.0 / s_scale);
    for (auto &g : geometry)
        {
        g->TryTransformInPlace (scale);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityD)
    {
    bvector<IGeometryPtr> allGeometry;
    bvector<PolyfaceHeaderPtr> allMesh;
    if (ReadDgnjsGeometry (allGeometry, 2, L"Polyface", L"MultiMeshVisibilityD", L"TwoMeshForMultiMeshVisibility.dgnjs"))
        {
        DRange3d range;
        range.Init ();
        for (auto &g : allGeometry)
            {
            PolyfaceHeaderPtr mesh = g->GetAsPolyfaceHeader ();
            if (g.IsValid ())
                {
                allMesh.push_back (mesh);
                range.Extend (mesh->Point ());
                }
            }
        double dX = 1.5 * range.XLength ();
        Check::SaveTransformed (allMesh[0]);
        Check::SaveTransformed (allMesh[1]);
        //SaveTransformed (allMesh, 0.0, true);

        bvector<PolyfaceHeaderPtr> visibleParts;
        PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
        Check::Shift (dX, 0, 0);
        //SaveTransformed (visibleParts, 0.0, true);

        PolyfaceHeaderPtr visibleFacets2, hiddenFacets2;
        //PolyfaceHeader::MeshHidesMeshXYByPlaneSets (allMesh[0], allMesh[1], visibleFacets2, hiddenFacets2);
        PolyfaceHeaderPtr meshAOverB, meshBUnderA;
        PolyfaceHeaderPtr meshBVisible, meshBHidden;
        PolyfaceHeader::ComputeOverAndUnderXY (*allMesh[0], nullptr, *allMesh[1], nullptr, meshAOverB, meshBUnderA);
        Check::SaveTransformed (meshAOverB);
        Check::SaveTransformed (meshBUnderA);
        Check::Shift (dX, 0, 0);
        if (meshBUnderA.IsValid ())
            {
            PolyfaceHeader::ComputePunchXYByPlaneSets (*meshBUnderA, *allMesh[1], &meshBHidden, &meshBVisible);
            Check::SaveTransformed (meshBHidden);
            Check::SaveTransformed (meshBVisible);
            }

        Check::Shift (dX, 0, 0);
        Check::SaveTransformed (visibleFacets2);

        Check::ClearGeometry ("Polyface.MultiMeshVisibilityD");
        Check::SaveTransformed (meshBUnderA);
        Check::SaveTransformed (allMesh[1]);
        Check::ClearGeometry ("Polyface.MultiMeshVisibilityDPunchInputs");

        }
    }

bool LoadAndRunMMV (wchar_t const * dataDirectory, bvector<WCharCP> filenames, char const *outputName,
bool oneCall = true     // true ==> one call to MultiMeshVisiblePartsXYByPlaneSets.
                        // false==> emulate the loop of clips, output each step.
)
    {
    bvector<IGeometryPtr> allGeometry;
    bvector<PolyfaceHeaderPtr> allMesh;
    DRange3d range;
    range.Init ();
    for (auto filename : filenames)
        {
        if (ReadDgnjsGeometry (allGeometry, 1, L"Polyface", dataDirectory, filename))
            {
            for (auto &g : allGeometry)
                {
                PolyfaceHeaderPtr mesh = g->GetAsPolyfaceHeader ();
                if (g.IsValid ())
                    {
                    allMesh.push_back (mesh);
                    range.Extend (mesh->Point ());
                    Check::SaveTransformed (mesh);
                    }
                }
            }
        }
    if (allMesh.size () > 1)
        {
        double dX = range.XLength ();
        double dY = range.YLength ();
        if (oneCall)
            {
            bvector<PolyfaceHeaderPtr> visibleParts;
            PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
            Check::Shift (dX, 0, 0);
            SaveTransformed (visibleParts, 0.0, true);
            }
        else
            {
            // These braces are to define scope for the shifter.
                {
                SaveAndRestoreCheckTransform shifter (dX,0,0);
                for (size_t i = 0; i < allMesh.size (); i++)
                    {
                    Check::Shift (0, dY, 0);
                    Check::SaveTransformed (allMesh[i]);
                    }
                Check::Shift (dX, 0, 0);
                }
            for (size_t i = 0; i < allMesh.size (); i++)
                {
                PolyfaceHeaderPtr visibleI = allMesh[i]->Clone ();
                SaveAndRestoreCheckTransform shifter (dX,0,0);
                Check::SaveTransformed (visibleI);
                for (size_t j = 0; j < allMesh.size () && visibleI.IsValid (); j++)
                    {
                    Check::Shift (0, dY, 0);
                    if (j == i)
                        continue;
                    PolyfaceHeaderPtr meshAOverB, meshBUnderA;
                    PolyfaceHeaderPtr meshBVisible, meshBHidden;
                    PolyfaceHeader::ComputeOverAndUnderXY (*allMesh[j], nullptr, *visibleI, nullptr, meshAOverB, meshBUnderA);
                    Check::Shift (0, dY,0);
                    if (meshAOverB.IsValid ())
                        Check::SaveTransformed (meshAOverB);
                    if (meshBUnderA.IsValid ())
                        Check::SaveTransformed (meshBUnderA);
                    if (meshBUnderA.IsValid ())
                        {
                        meshBUnderA->Triangulate();
                        PolyfaceHeader::ComputePunchXYByPlaneSets (*meshBUnderA, *visibleI, &meshBHidden, &meshBVisible);
                        if (meshBVisible.IsValid () || meshBHidden.IsValid ())
                            {
                            Check::Shift (0, dY,0);
                            if (meshBHidden.IsValid ())
                                Check::SaveTransformed (meshBHidden);
                            visibleI = meshBVisible;
                            }
                        }
                    Check::Shift (0, dY,0);
                    Check::SaveTransformed (visibleI);
                    }
                }
            }
        Check::ClearGeometry (outputName);
        return true;
        }
    return false;
    }
#ifdef CompileLongMultiMeshTests
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityE)
    {
    bvector<IGeometryPtr> allGeometry;
    bvector<PolyfaceHeaderPtr> allMesh;
    DRange3d range;
    range.Init ();

    for (WCharCP filename : bvector<WCharCP> {
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_0.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_1.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_3.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_5.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_7.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_9.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_11.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_13.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_15.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_17.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_19.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_21.dgnjs",

            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_22.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_24.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_26.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_28.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_30.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_32.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_34.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_36.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_38.dgnjs",
            //L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_40.dgnjs"
            })
        {
        if (ReadDgnjsGeometry (allGeometry, 1, L"Polyface", L"MultiMeshVisibilityE", filename))
            {
            for (auto &g : allGeometry)
                {
                PolyfaceHeaderPtr mesh = g->GetAsPolyfaceHeader ();
                if (g.IsValid ())
                    {
                    allMesh.push_back (mesh);
                    range.Extend (mesh->Point ());
                    Check::SaveTransformed (mesh);
                    }
                }
            }
        }
    if (allMesh.size () > 1)
        {
        double dX = range.XLength ();
        bvector<PolyfaceHeaderPtr> visibleParts;
        PolyfaceHeader::MultiMeshVisiblePartsXYByPlaneSets (allMesh, visibleParts);
        Check::Shift (dX, 0, 0);
        SaveTransformed (visibleParts, 0.0, true);
        Check::ClearGeometry ("Polyface.MultiMeshVisibilityE");
        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityF)
    {
    LoadAndRunMMV (
        L"MultiMeshVisibilityF",
        bvector<WCharCP> {
            L"1road.dgnjs",
            L"1gradeA.dgnjs",
            L"1gradeB.dgnjs"
            },
        "Polyface.MultiMeshVisibilityF",
        false
        );
    }


#ifdef CompileLongMultiMeshTests
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MultiMeshVisibilityCS4)
    {
    LoadAndRunMMV (
        L"MultiMeshVisibilityCS4",
        bvector<WCharCP> {
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_0.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_1.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_10.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_11.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_12.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_13.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_14.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_15.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_16.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_17.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_18.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_19.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_2.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_20.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_21.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_22.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_23.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_24.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_25.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_26.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_27.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_28.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_29.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_3.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_30.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_31.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_32.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_33.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_34.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_35.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_36.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_37.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_38.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_39.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_4.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_40.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_41.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_5.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_6.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_7.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_8.dgnjs",
            L"RunOnSelection_BeforeMultiMeshVisiblePartsXYByPlaneSets_9.dgnjs"
            },
        "Polyface.MultiMeshVisibilityCS4",
        true
        );
    }
#endif
