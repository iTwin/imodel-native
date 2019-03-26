/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_normals.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>


static int s_noisy = 0;

void AppendOneBasedTriangleIndices (bvector<int> &indices, int i0, int i1, int i2)
    {
    indices.push_back (i0);
    indices.push_back (i1);
    indices.push_back (i2);
    indices.push_back (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,AverageNormalsWithDegenerateTriangles)
    {
    double zz = 1.0;
    bvector<DPoint3d> points
        {
        DPoint3d::From ( 0,0,0),        // (1) The pole point
        DPoint3d::From (1,0,zz),         // (2) right side of unit box
        DPoint3d::From (1,0.5,zz),       // (3) halfway up
        DPoint3d::From (0.5, 1,zz),      // (4) haflway on top
        DPoint3d::From (0,1,zz)          // (5) back to y axis
        };

    auto mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    for (auto xyz : points)
        mesh->Point().push_back (xyz);

    bvector<int> &indices = mesh->PointIndex ();
    AppendOneBasedTriangleIndices (indices, 1,2,3);
    AppendOneBasedTriangleIndices (indices, 1,3,1);  // Degenerate !!!
    AppendOneBasedTriangleIndices (indices, 1,3,4);
    AppendOneBasedTriangleIndices (indices, 1,4,1);  // Degenerate !!!
    AppendOneBasedTriangleIndices (indices, 1,4,5);

    Check::True (mesh->BuildApproximateNormals (0.2, 0.4, true), "BuildAproximateNormals");
    PrintPolyface (*mesh, "AverageNormals", stdout, 100, false);
    }


ValidatedDouble GetVolume (PolyfaceHeaderPtr &mesh)
    {
    if (mesh.IsValid ())
        {
        auto volume = mesh->ValidatedVolume ();
        if (volume.IsValid ())
            return volume;
        DPoint3d centroid;
        double volume2;
        RotMatrix axes;
        DVec3d momentxyz;

        if (mesh->ComputePrincipalMomentsAllowMissingSideFacets (volume2, centroid, axes, momentxyz, false))
            {
            return ValidatedDouble (volume2, true);
            }
            
        }
    return ValidatedDouble (0, false);
    }



void ExerciseSingleSheetCutFill (PolyfaceHeaderPtr dtm, PolyfaceHeaderPtr road, char const *message)
    {
    MeshAnnotationVector messages (false);
    PolyfaceHeaderPtr cutMesh, fillMesh;
    PolyfaceHeader::ComputeSingleSheetCutFill (*dtm, *road, DVec3d::From (0,0,1), cutMesh, fillMesh);
    Check::SaveTransformed (*dtm);
    Check::SaveTransformed (*road);
    auto range = dtm->PointRange ();
    Check::Shift (0, 1.5 * range.YLength (), 0);
    if (cutMesh.IsValid ())
        Check::SaveTransformed (*cutMesh);
    if (fillMesh.IsValid ())
        Check::SaveTransformed (*fillMesh);


    auto cutVolume1 = GetVolume (cutMesh);
    auto fillVolume1 = GetVolume (fillMesh);

    bvector<PolyfaceHeaderPtr> polyfaceA, polyfaceB, fill2, cut2;
    polyfaceA.push_back (dtm);
    polyfaceB.push_back (road);
    PolyfaceQuery::ComputeCutAndFill (polyfaceA, polyfaceB, fill2, cut2);
    Check::Shift (0,10,0);

    for (auto &a : fill2)
        Check::SaveTransformed (*a);
    for (auto &a : cut2)
        Check::SaveTransformed (*a);

    double fillVolume2 = 0, cutVolume2 = 0;
    int errors = 0;
    for (auto &m : fill2)
        {
        PolyfaceHeaderPtr m1;
        PolyfaceQuery::HealVerticalPanels (*m, true, false, m1);
        auto v = m1->ValidatedVolume ();
        fillVolume2 += v;
        if (!v.IsValid ())
            fillVolume2 += v;
        }
    for (auto &m : cut2)
        {
        PolyfaceHeaderPtr m1;
        PolyfaceQuery::HealVerticalPanels (*m, true, false, m1);
        if (m1.IsValid ())
            {
            auto v = m1->ValidatedVolume ();
            cutVolume2 += v;
            if (!v.IsValid ())
                cutVolume2 += v;
            }
        }
    
    if (s_noisy > 100)
        {
        printf ("\n\n **** %s\n", message);
        PrintPolyface (*dtm, "dtm", stdout, 1000, false);
        PrintPolyface (*road, "road", stdout, 1000, false);
        if (cutMesh.IsValid ())
            PrintPolyface (*cutMesh, "cut", stdout, 1000, false);
        if (fillMesh.IsValid ())
            PrintPolyface (*fillMesh, "fill", stdout, 1000, false);
        printf (" new method volumes from mesh  %g %g \n", fillVolume1.Value (), cutVolume1.Value ());
        printf (" old method volumes            %g %g (closure errors %d)\n", cutVolume2, fillVolume2, errors);

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,NoCrossings)
    {
    double dtmSide = 2.5;
    double roadSide = 1.0;
    // unused - double zRoad = 1.0;
    double zDtm = 0.5;
    for (int numAdd : bvector<int>{0,1, 2,5})
        {
        for (double zRoad : bvector<double> {1.0, 0.25})
            {
            SaveAndRestoreCheckTransform shifter (30.0, 0, 0);
            int roadNumX = 1 + numAdd;
            int roadNumY = 1 + numAdd / 2;
            auto road = UnitGridPolyface (
                    DPoint3dDVec3dDVec3d (
                                DPoint3d::From (2,1,zRoad),
                                DVec3d::From (roadSide,0,0),
                                DVec3d::From (0,roadSide,0)
                                ),
                                    roadNumX , roadNumY, false);
            DRange3d roadRange = road->PointRange ();
            int dtmNumX = 1 + (int)((roadRange.XLength () + dtmSide) / dtmSide);
            int dtmNumY = 1 + (int)((roadRange.YLength () + dtmSide) / dtmSide);

            auto dtm = UnitGridPolyface (
                    DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,zDtm),   DVec3d::From (dtmSide,0,0),   DVec3d::From (0,dtmSide,0)),
                                    dtmNumX, dtmNumY, false);

            ExerciseSingleSheetCutFill (dtm, road, "Flat Road And DTM");
            }
        }
    Check::ClearGeometry ("FastCutFill.NoCrossings");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,SlopedWithCrossings)
    {
    double dtmSide = 2.5;
    double roadSide = 1.0;
    // double roadSlope = -0.8;
    for (int numAdd : bvector<int>{0, 1})
        {
        SaveAndRestoreCheckTransform shifter (30.0, 0, 0);
        int roadNumX = 3 + numAdd;
        int roadNumY = 1 + numAdd;
        auto road = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (1,1,1),   DVec3d::From (roadSide,0,0),   DVec3d::From (0,roadSide,-0.8)),
                                roadNumX , roadNumY, false);
        DRange3d roadRange = road->PointRange ();
        int dtmNumX = 1 + (int)((roadRange.XLength () + dtmSide) / dtmSide);
        int dtmNumY = 1 + (int)((roadRange.YLength () + dtmSide) / dtmSide);

        auto dtm = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,0),   DVec3d::From (dtmSide,0,0),   DVec3d::From (0,dtmSide,0)),
                                dtmNumX, dtmNumY, false);

        ExerciseSingleSheetCutFill (dtm, road, "CutFill with road slope");
        }
    Check::ClearGeometry ("FastCutFill.SlopedWithCrossings");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,SingleVertexTouch)
    {
    double dtmSide = 5.0;
    int dtmNumX = 2;
    int dtmNumY = 1;

    for (double x1 : bvector<double>{dtmSide, dtmSide + 1})
        {
        SaveAndRestoreCheckTransform shifter (30.0, 0, 0);
        auto dtm = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,0),   DVec3d::From (dtmSide,0,0),   DVec3d::From (0,dtmSide,0)),
                                dtmNumX, dtmNumY, false);

        // single triangle with a point exactly "on" the dtm ...
        // (being sure that
        bvector<DPoint3d>points
            {
            DPoint3d::From (3,1,1),     // ABOVE
            DPoint3d::From (x1,2,0),     // ON
            DPoint3d::From (3,4,-1)     // BELOW
            };
        bvector<int> indices
            {
            1,2,3,0
            };
        auto road = PolyfaceHeader::CreateIndexedMesh (0, points, indices);

        ExerciseSingleSheetCutFill (dtm, road, "CutFill with single touch");
        }
    Check::ClearGeometry ("FastCutFill.SingleVertexTouch");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,NoEdgeContact)
    {
    double dtmSide = 5.0;
    int dtmNumX = 1;
    int dtmNumY = 1;

    for (double z1 : bvector<double>{1.0, -1.0})
        {
        SaveAndRestoreCheckTransform shifter (30.0, 0, 0);
    // 1x1 dtm (large face)
        auto dtm = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,0),   DVec3d::From (dtmSide,0,0),   DVec3d::From (0,dtmSide,0)),
                                dtmNumX, dtmNumY, false);

        // single triangle fully contained in the large dtm face
        bvector<DPoint3d>points
            {
            DPoint3d::From (1,1,1),
            DPoint3d::From (3,2,z1),
            DPoint3d::From (1,4,1)
            };
        bvector<int> indices
            {
            1,2,3,0
            };
        auto road = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
        ExerciseSingleSheetCutFill (dtm, road, "CutFill with single touch");
        }
    Check::ClearGeometry ("FastCutFill.NoEdgeContact");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,VerticalGaps)
    {
    double dtmSide = 4.0;
    int dtmNumX = 1;
    int dtmNumY = 1;

    for (double z1 : bvector<double>{0, 1.5, 3.0, 6.0})
        {
        SaveAndRestoreCheckTransform shifter (30.0, 0, 0);
    // 1x1 dtm (large face)
        auto dtm = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (0,0,0),   DVec3d::From (dtmSide,0,z1),   DVec3d::From (0,dtmSide,0)),
                                dtmNumX, dtmNumY, false);

        // two squares that share an edge when viewed from above, but there is a vertical gap at the edge.
        bvector<DPoint3d>points
            {
            DPoint3d::From (1,1,1),
            DPoint3d::From (2,1,1),
            DPoint3d::From (2,2,1),
            DPoint3d::From (1,2,1),
            DPoint3d::From (2,1,2),
            DPoint3d::From (3,1,2),
            DPoint3d::From (3,2,2),
            DPoint3d::From (2,2,2),
            };
        bvector<int> indices
            {
            1,2,3,4,0,
            5,6,7,8,0

            };
        auto road = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
        ExerciseSingleSheetCutFill (dtm, road, "CutFill with single touch");
        }
    Check::ClearGeometry ("FastCutFill.VerticalGaps");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,ManyVerticals)
    {
    double dtmSide = 6.0;
    int dtmNumX = 1;
    int dtmNumY = 1;
    auto options = IFacetOptions::CreateForSurfaces ();

    // We need a mesh with multiple vertical breaks.
    // Stroke a curve ..
        auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (
        bvector<DPoint3d> {            
            DPoint3d::From (1,1.1,1),
            DPoint3d::From (2,1.1,2),
            DPoint3d::From (3,2,1),
            DPoint3d::From (4,2,3),
            DPoint3d::From (5,1,1)
            },
            nullptr, nullptr,
            4, false, false);
    bvector<DPoint3d> strokes;
    bcurve->AddStrokes (strokes, 0, 0.15, 0.0);

    double z1 = 1.0;
    for (size_t n1 : {3, 6, 15})
        {
        if (n1 > strokes.size ())
            break;
        SaveAndRestoreCheckTransform shifter (30.0, 0, 0);

        auto dtm = UnitGridPolyface (
                DPoint3dDVec3dDVec3d (DPoint3d::From (0,-1.0),   DVec3d::From (dtmSide,0,z1),   DVec3d::From (0,dtmSide,0)),
                                dtmNumX, dtmNumY, false);
        bvector<DPoint3d> strokeA = strokes;
        strokeA.resize (n1);
        bvector<DPoint3d> strokeB = strokeA;
        for (auto &xyz : strokeB)
            xyz.z += 1.0;
        bvector<DPoint3d> strokeB1 = strokeB;
        bvector<DPoint3d> strokeA1 = strokeA;
        for (auto &xyz : strokeB1)
            {
            xyz.y = 0.5;
            xyz.z -= 0.25;
            }
        for (auto &xyz : strokeA1)
            xyz.y = 2.5;

        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
        bvector<DTriangle3d> triangles;
        PolylineOps::GreedyTriangulationBetweenLinestrings (strokeA, strokeA1, triangles);
        builder->AddTriangles (triangles);
        triangles.clear ();
        PolylineOps::GreedyTriangulationBetweenLinestrings (strokeB1, strokeB, triangles);
        builder->AddTriangles (triangles);
        auto road = builder->GetClientMeshPtr ();
        ExerciseSingleSheetCutFill (dtm, road, "CutFill with single touch");
        }
    Check::ClearGeometry ("FastCutFill.ManyVerticals");

    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,SinusoidPlane)
    {
    double z0 = 0.5;
    double z1 = 0.5;
    for (size_t numX = 2, numY = 2; numX < 25; numX = 2 * numX + 1, numY = 2 * numY + 2)
        {
        SaveAndRestoreCheckTransform shifter (50.0, 0, 0);
        auto bsurf = SurfaceWithSinusoidalControlPolygon (2, 2, numX, numY, 0.0, 0.3, 0.0, 0.5);
        bvector<DPoint3d> poles;
        bsurf->GetPoles (poles);
        auto dtm = PolyfaceHeader::CreateTriangleGrid ((int)bsurf->GetNumUPoles ());
        DPoint3dOps::Append (&dtm->Point (), &poles);
        DRange3d range = dtm->PointRange ();
        auto road = PolyfaceHeader::CreateTriangleGrid (2);

        dtm->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

        bvector<DPoint3d> roadPoints
            {
            range.LocalToGlobal (0.1, 0.2, z0),
            range.LocalToGlobal (0.8, 0.2, z0),
            range.LocalToGlobal (0.1, 0.9, z1),
            range.LocalToGlobal (0.8, 0.9, z1)
            };
        DPoint3dOps::Append (&road->Point (), &roadPoints);
        road->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
        ExerciseSingleSheetCutFill (dtm, road, "CutFill with single touch");
#ifdef TestIntersectionSegments
        bvector<DSegment3dSizeSize> segments;
        PolyfaceQuery::SearchIntersectionSegments (*dtm, *road, segments);
        if (s_noisy)
            Check::Print (segments, "dtm road segments");
        bvector<PolyfaceHeaderPtr> meshList {dtm, road};
        bvector<PolyfaceHeaderPtr> volumeList;
        PolyfaceQuery::MergeAndCollectVolumes (meshList, volumeList);
        for (auto &m : volumeList)
            {
            auto v = m->ValidatedVolume ();
            printf ("  volume %g\n", v.Value ());
            }
#endif
        }
    Check::ClearGeometry ("FastCutFill.SinusoidPlane");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,GetNumFacet)
    {
    for (auto triangulated: {false, true})
        {
        Check::NamedScope scope0 ("Triangulated ", triangulated ? 1 : 0);
        for (auto coordinateOnly : {false, true})
            {
            Check::NamedScope scope1 ("CoordinateOnly", coordinateOnly ? 1 : 0);
            auto pf3 = UnitGridPolyface (DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0), 3,1, triangulated, coordinateOnly);
            size_t maxPerFacetGri;
            size_t numFacetGri = pf3->GetNumFacet (maxPerFacetGri);
            pf3->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
            size_t maxPerFacetIndexe;
            size_t numFacetIndexe = pf3->GetNumFacet (maxPerFacetIndexe);
            Check::Size (maxPerFacetGri, maxPerFacetIndexe, "MaxPerFace grid, indexed");
            Check::Size (numFacetGri, numFacetIndexe, "numFacet grid, indexed");
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SharedNormals)
    {
    // an xz plane arc to rotate around z:
    auto arc = DEllipse3d::From (
                0,0,0,
                4,0,0,
                0,0,5,
                0, Angle::DegreesToRadians (70.0));
    auto prim = ICurvePrimitive::CreateArc (arc);
    auto cv    = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv->Add (prim);
    auto rotated = ISolidPrimitive::CreateDgnRotationalSweep (
            DgnRotationalSweepDetail (
                cv,
                DPoint3d::From (0,0,0),
                DVec3d::From (0,0,1),
                Angle::DegreesToRadians (45),
                false));
    Check::SaveTransformed (*rotated);
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetNormalsRequired (true);
    options->SetParamsRequired (true);
    options->SetMaxPerFace (4);
    options->SetAngleTolerance (Angle::DegreesToRadians (14.0));
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddSolidPrimitive (*rotated);
    Check::Shift (0,10,0);
    Check::SaveTransformed (builder->GetClientMeshR ());
    Check::ClearGeometry ("Polyface.SharedNormals");
    }