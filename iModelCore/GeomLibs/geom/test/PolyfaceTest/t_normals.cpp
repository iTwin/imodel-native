/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_normals.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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






void ExerciseCutFill (PolyfaceHeaderPtr dtm, PolyfaceHeaderPtr road, char const *message)
    {
    MeshAnnotationVector messages (false);
    PolyfaceHeaderPtr cutMesh, fillMesh;
    double cutVolume0, fillVolume0;
    PolyfaceQuery::ComputeSingleSheetCutFillVolumes (*dtm, *road, cutVolume0, fillVolume0, messages);
    PolyfaceQuery::ComputeSingleSheetCutFillMeshes (*dtm, *road, cutMesh, fillMesh, messages);
    auto cutVolume1 = cutMesh->ValidatedVolume ();
    auto fillVolume1 = fillMesh->ValidatedVolume ();
    Check::Near (cutVolume1.Value (), fabs (cutVolume0), "Cut Volume from mesh vs direct");
    Check::Near (fillVolume1.Value (), fillVolume0, "Fill Volume from mesh vs direct");
    bvector<PolyfaceHeaderPtr> polyfaceA, polyfaceB, fill2, cut2;
    polyfaceA.push_back (dtm);
    polyfaceB.push_back (road);
    PolyfaceQuery::ComputeCutAndFill (polyfaceA, polyfaceB, fill2, cut2);
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
        auto v = m1->ValidatedVolume ();
        cutVolume2 += v;
        if (!v.IsValid ())
            cutVolume2 += v;
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
        printf (" new method volumes from sweep %g %g \n", fillVolume0, cutVolume0);
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

            ExerciseCutFill (dtm, road, "Flat Road And DTM");
            }
        }
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

        ExerciseCutFill (dtm, road, "CutFill with road slope");
        }
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

        ExerciseCutFill (dtm, road, "CutFill with single touch");
        }
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
        ExerciseCutFill (dtm, road, "CutFill with single touch");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FastCutFill,SinusoidPlane)
    {
    double z0 = 0.5;
    double z1 = 0.5;
    for (size_t numX = 2, numY = 2; numX < 8; numX++, numY++)
        {
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
        ExerciseCutFill (dtm, road, "Sinusoid, Road");
        }
    }
