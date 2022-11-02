/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Bentley/BeConsole.h>

// replicated a single polygon numX * numY * numZ times at unit spacing.
PolyfaceHeaderPtr CreateSpaceBlockMesh
(
bvector<DPoint3d> &polygon,
size_t numX,
size_t numY,
size_t numZ,
DVec3d originShift,
double step
)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &meshPoints = mesh->Point ();
    bvector<int> &meshIndices = mesh->PointIndex ();
    DPoint3d xyz1;
    for (size_t k = 0; k < numZ; k++)
        {
        for (size_t j = 0; j < numY; j++)
            {
            for (size_t i = 0; i < numX; i++)
                {
                for (DPoint3d xyz : polygon)
                    {
                    xyz1.SumOf (xyz, originShift);
                    xyz1.x += i * step;
                    xyz1.y += j * step;
                    xyz1.z += k * step;
                    size_t index = meshPoints.size ();
                    meshPoints.push_back (xyz1);
                    meshIndices.push_back ((int)index + 1);    // 1 based indexing !!!
                    }
                meshIndices.push_back (0);
                }
            }
        }
    return mesh;
    }


struct HitCounter : DRange3dPairRecursionHandler
{
BoolCounter m_II;
BoolCounter m_IL;
BoolCounter m_LI;
BoolCounter m_LL;
double m_distance;

HitCounter (double distance) : m_II (), m_IL (), m_LL (), m_LI () , m_distance (distance) {}

bool TestOverlap (DRange3dCR rangeA, DRange3dCR rangeB)
    {
    return rangeA.IntersectsWith (rangeB, m_distance, 3);
    }

virtual bool TestInteriorInteriorPair (DRange3dCR rangeA, DRange3dCR rangeB) override 
  {
  return m_II.Count (TestOverlap (rangeA, rangeB));
  }

virtual bool TestInteriorLeafPair (DRange3dCR rangeA, DRange3dCR rangeB, size_t indexB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

virtual bool TestLeafInteriorPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB) override
  {
  return m_IL.Count (TestOverlap (rangeA, rangeB));
  }

virtual void TestLeafLeafPair (DRange3dCR rangeA, size_t indexA, DRange3dCR rangeB, size_t indexB) override
    {
    m_LL.Count (rangeA.IntersectsWith (rangeB, m_distance, 3));
    }

virtual bool StillSearching () override
    {
    return true;
    }
};

#ifdef DOES_NOT_LINK
void TestRangeTree (size_t numX, size_t numY, size_t numZ)
    {
    bvector<DPoint3d> points;
    double a = 0.9;
    double b = 0.8;
    double c = 0.5;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (a,0,0));
    points.push_back (DPoint3d::From (a,b,c));
    points.push_back (DPoint3d::From (0,b,c));
    PolyfaceHeaderPtr mesh1 = CreateSpaceBlockMesh (points, numX, numY, numZ, DVec3d::From (0,0,0), 1.0);
    // singleton completely outside
    PolyfaceHeaderPtr mesh2 = CreateSpaceBlockMesh (points, 1, 1, 1, DVec3d::From ((double)numX, (double)numY, (double)numZ), 1.0);
    // singleton with one hit.
    PolyfaceHeaderPtr mesh3 = CreateSpaceBlockMesh (points, 1, 1, 1, DVec3d::From ((double)numX - 2.0, (double)numY - 2.0, (double)numZ - 2.0), 1.0);

    PolyfaceHeaderPtr mesh4 = CreateSpaceBlockMesh (points, 2,2,2, DVec3d::From (0.3, 0.4, 0.2), 2.7);

    PolyfaceRangeTreePtr rangeTree1 = PolyfaceRangeTree::CreateForPolyface (*mesh1);
    PolyfaceRangeTreePtr rangeTree2 = PolyfaceRangeTree::CreateForPolyface (*mesh2);
    PolyfaceRangeTreePtr rangeTree3 = PolyfaceRangeTree::CreateForPolyface (*mesh3);
    PolyfaceRangeTreePtr rangeTree4 = PolyfaceRangeTree::CreateForPolyface (*mesh4);


    double tol = 1.0e-8;
    HitCounter counter12 (tol);
    HitCounter counter13 (tol);
    HitCounter counter23 (tol);

    HitCounter counter11 (tol);
    HitCounter counter22 (tol);
    HitCounter counter33 (tol);

    HitCounter counter44 (tol);

    XYZRangeTreeMultiSearch searcher;
    searcher.RunSearch (rangeTree2.get ()->GetXYZRangeTree (), rangeTree3.get ()->GetXYZRangeTree (), counter23);

    searcher.RunSearch (rangeTree2.get ()->GetXYZRangeTree (), rangeTree2.get ()->GetXYZRangeTree (), counter22);
    searcher.RunSearch (rangeTree3.get ()->GetXYZRangeTree (), rangeTree3.get ()->GetXYZRangeTree (), counter33);

    searcher.RunSearch (rangeTree1.get ()->GetXYZRangeTree (), rangeTree2.get ()->GetXYZRangeTree (), counter12);
    searcher.RunSearch (rangeTree1.get ()->GetXYZRangeTree (), rangeTree3.get ()->GetXYZRangeTree (), counter13);

    searcher.RunSearch (rangeTree1.get ()->GetXYZRangeTree (), rangeTree1.get ()->GetXYZRangeTree (), counter11);

    searcher.RunSearch (rangeTree1.get ()->GetXYZRangeTree (), rangeTree4.get ()->GetXYZRangeTree (), counter44);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PFRangeTree,CountRangeHits)
    {
    TestRangeTree (4,4,4);
    TestRangeTree (8,8,8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PFRangeTree,CollectClash)
    {
    bvector<DPoint3d> small;
    double a = 0.9;
    //double b = 0.8;
    double c = 0.5;
    // small rectangles parallel to xz plane
    small.push_back (DPoint3d::From (0,0,0));
    small.push_back (DPoint3d::From (a,0,0));
    small.push_back (DPoint3d::From (a,0,c));
    small.push_back (DPoint3d::From (0,0,c));
    // big lattice of these.
    size_t numX = 3;
    size_t numY = 2;
    size_t numZ = 8;
    PolyfaceHeaderPtr mesh1 = CreateSpaceBlockMesh (small, numX, numY, numZ, DVec3d::From (0,0,0), 1.0);

    // Big rectangle parallel to xz
    bvector<DPoint3d> large;
    double d = 100.0;
    double e = 100.0;
    double f = 3.1;     // place it so an entire layer has hits.
    // small rectangles parallel to xz plane
    large.push_back (DPoint3d::From (0,0,f));
    large.push_back (DPoint3d::From (d,0,f));
    large.push_back (DPoint3d::From (d,e,f));
    large.push_back (DPoint3d::From (0,e,f));
    PolyfaceHeaderPtr mesh2 = CreateSpaceBlockMesh (large, 1,1,1, DVec3d::From (0,0,0), 1.0);


    PolyfaceRangeTreePtr rangeTree1 = PolyfaceRangeTree::CreateForPolyface (*mesh1);
    PolyfaceRangeTreePtr rangeTree2 = PolyfaceRangeTree::CreateForPolyface (*mesh2);
    bvector<std::pair<size_t, size_t>> hits;
    XYZRangeTreeMultiSearch searcher;
    PolyfaceRangeTree::CollectClashPairs (*mesh1, *rangeTree1, *mesh2, *rangeTree2, 0.0, hits, 10000, searcher);
    Check::Size (numX * numY, hits.size (), "hits");    
    }
#ifdef COMPILE_bvRangeTree

void TestPolyfaceRangeTree01 (size_t numX, size_t numY, size_t numZ)
    {
    bvector<DPoint3d> small;
    double a = 0.9;
    //double b = 0.8;
    double c = 0.5;
    // small rectangles parallel to xz plane
    small.push_back (DPoint3d::From (0,0,0));
    small.push_back (DPoint3d::From (a,0,0));
    small.push_back (DPoint3d::From (a,0,c));
    small.push_back (DPoint3d::From (0,0,c));

    PolyfaceHeaderPtr mesh1 = CreateSpaceBlockMesh (small, numX, numY, numZ, DVec3d::From (0,0,0), 1.0);

    PolyfaceRangeTreePtr rangeTree0 = PolyfaceRangeTree::CreateForPolyface (*mesh1);
    PolyfaceRangeTree01Ptr rangeTree1 = PolyfaceRangeTree01::CreateForPolyface (*mesh1);
    PolyfaceIndexedHeapRangeTreePtr rangeTree2 = PolyfaceIndexedHeapRangeTree::CreateForPolyface (*mesh1);
    bvector<size_t> hit0, hit1, hit2;
    DRange3d range = DRange3d::From (DPoint3d::From (1,1,1));
    
    for (double expansion = 0.001; expansion < 3.0; expansion *= 50)
        {
        rangeTree0->CollectInRange (hit0, range, expansion);
        rangeTree1->CollectInRange (hit1, range, expansion);
        rangeTree2->CollectInRange (hit2, range, expansion);
        BeConsole::Printf ("(expansion %lg) (hit0 %d) (hit1 %d) (hit2 %d)\n", expansion,
                        hit0.size (), hit1.size (), hit2.size ());
        std::sort (hit0.begin (), hit0.end ());
        std::sort (hit1.begin (), hit1.end ());
        std::sort (hit2.begin (), hit2.end ());
        if (Check::Size (hit0.size (), hit1.size (), "tree hit counts")
           && Check::Size (hit0.size (), hit2.size (), "tree hit counts"))
            {
            for (size_t i = 0; i <hit0.size (); i+= 17)
                Check::Size (hit0[i], hit1[i], "tree search");
            }

        if (Check::Size (hit0.size (), hit2.size (), "tree hit2 counts")
           && Check::Size (hit0.size (), hit2.size (), "tree hit2 counts"))
            {
            for (size_t i = 0; i <hit0.size (); i+= 17)
                Check::Size (hit0[i], hit2[i], "tree search 2");
            }


        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceRangeTree01,CollectInRange)
    {
    TestPolyfaceRangeTree01 (3,2,8);
    TestPolyfaceRangeTree01 (13,21,82);

    }    
#endif
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,Cube)
    {
    PolyfaceHeader::OffsetOptions offsetOptions;

    auto box0 = ISolidPrimitive::CreateDgnBox (
       DgnBoxDetail::InitFromCenterAndSize (
            DPoint3d::From (5,5,5),
            DPoint3d::From (1,2,3),
            true
            ));
    auto line = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 1,1,0));
    auto options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
    meshBuilder->AddSolidPrimitive (*box0);
    auto mesh0 = meshBuilder->GetClientMeshPtr ();
    Check::SaveTransformed (*line);
    Check::SaveTransformed (*mesh0);

    auto mesh1 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25);
    if (mesh1.IsValid ())
        {
        Check::Shift (10,0,0);
        Check::SaveTransformed (*line);
        Check::SaveTransformed (*mesh1);
        }
    Check::ClearGeometry ("PolyfaceOffset.Cube");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,OpenMesh1)
    {
    PolyfaceHeader::OffsetOptions offsetOptions;

    auto line = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 1,1,0));
    auto options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
    bvector<DPoint3d> points
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (10,5,0),
        DPoint3d::From (5,5,0),
        DPoint3d::From (5,10,0),
        DPoint3d::From (0,10,0)
        };
    meshBuilder->AddTriangulation (points);
    auto mesh0 = meshBuilder->GetClientMeshPtr ();
    Check::SaveTransformed (*line);
    Check::SaveTransformed (*mesh0);

    auto mesh1 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25);
    if (mesh1.IsValid ())
        {
        Check::Shift (10,0,0);
        Check::SaveTransformed (*line);
        Check::SaveTransformed (*mesh1);
        }
    Check::ClearGeometry ("PolyfaceOffset.OpenMesh1");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,OpenMesh2)
    {
    PolyfaceHeader::OffsetOptions offsetOptions;
    TransformShifter shifter (20,0,0, 0,20,0, false);
    double sz = -1.0;
    DPoint3d pointB = DPoint3d::From (10,0, sz * 8);
    DPoint3d pointC = DPoint3d::From (4,0, sz * 8);
    DPoint3d pointD = DPoint3d::From (4,0, sz * 12);

    auto arc = ICurvePrimitive::CreateArc
        (
        DEllipse3d::FromPointsOnArc
            (
            DPoint3d::From (10,0,sz * 0),
            DPoint3d::From (12,0,sz * 4),
            pointB
            )
        );
    auto path = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    path->push_back (arc);
    path->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    path->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointD)));

    auto options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
    auto solid = ISolidPrimitive::CreateDgnRotationalSweep
                        (
                        DgnRotationalSweepDetail (path,
                            DPoint3d::From (0,0,sz * -1),
                            DVec3d::From (1,0,0),
                            0.5 * Angle::Pi (),
                            false
                        ));

    meshBuilder->AddSolidPrimitive (*solid);
    auto mesh0 = meshBuilder->GetClientMeshPtr ();

    Check::SaveTransformed (*mesh0);

    auto mesh1 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25);
    if (mesh1.IsValid ())
        {
        shifter.DoShift0 ();
        Check::SaveTransformed (*mesh1);

        shifter.DoShift0 ();
        auto mesh2 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25, false, true, false);
        Check::SaveTransformed (*mesh2);
        shifter.DoShift0 ();
        mesh2 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25, false, false, true);
        Check::SaveTransformed (*mesh2);
        shifter.DoShift0 ();
        mesh2 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25, true, false, false);
        Check::SaveTransformed (*mesh2);
        }

    // Make sure "zero shift" works . . .
    shifter.DoGridShift (0.0, 2.0, true);
    auto mesh3 = mesh0->ComputeOffset (offsetOptions, 1.0, 0.0, true, true, true);
    if (mesh3.IsValid ())
        {
        shifter.DoShift0 ();
        Check::SaveTransformed (*mesh3);
        }    
    Check::ClearGeometry ("PolyfaceOffset.OpenMesh2");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,CrinklePointA)
    {
    static double s_zScale = 1.0;
    PolyfaceHeader::OffsetOptions offsetOptions;

    auto options = IFacetOptions::Create ();

    // central point and 4 outer points of 20x20 square around the origin
    bvector<DPoint3d> points
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (10,-10,0),
        DPoint3d::From (10,10,0),
        DPoint3d::From (-10,10,0),
        DPoint3d::From (-10,-10,0),
        };
    // create variations with shifted z at fringe points.
    for (auto shift : bvector<DVec3d> {
            DVec3d::From (0,0,0),
            DVec3d::From (1,0,0),
            DVec3d::From(1,1,0),
            DVec3d::From (1,0,-1),
            DVec3d::From (1,0,1),
            DVec3d::From (2,0,2),
            DVec3d::From (4,-1,4)
            })
        {
        IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
        SaveAndRestoreCheckTransform shifter (40,0,0);
        DPoint3d point1 = points[1];  point1.z += s_zScale * shift.x;
        DPoint3d point2 = points[2];  point2.z += s_zScale * shift.y;
        DPoint3d point3 = points[3];  point3.z += s_zScale * shift.z;
        meshBuilder->AddTriangulation (bvector<DPoint3d> {points[0],point1, point2});
        meshBuilder->AddTriangulation (bvector<DPoint3d> {points[0],point2, point3});
        meshBuilder->AddTriangulation (bvector<DPoint3d> {points[0],point3, points[4]});
        meshBuilder->AddTriangulation (bvector<DPoint3d> {points[0],points[4], point1});
        auto mesh0 = meshBuilder->GetClientMeshPtr ();
        Check::SaveTransformed (*mesh0);

        auto mesh1 = mesh0->ComputeOffset (offsetOptions, 1.0, -0.25);
        if (mesh1.IsValid ())
            {
            Check::Shift (0,30,0);
            Check::SaveTransformed (*mesh1);
            }
        }
    Check::ClearGeometry ("PolyfaceOffset.CrinklePointA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,CrinklePointB)
    {
    // unused - static double s_zScale = 1.0;
    PolyfaceHeader::OffsetOptions offsetOptions;

    auto options = IFacetOptions::Create ();
    double positiveOffset = 2.0;
    for (double centerOffset : {0.0, 2.0, -2.0})
        {
        SaveAndRestoreCheckTransform shifter1 (0, 400, 0);
        for (uint32_t totalEdgePoint : {5, 9, 11, 13})
            {
            SaveAndRestoreCheckTransform shifter2 (200, 0);
            double radians = Angle::TwoPi () / totalEdgePoint;
            double aX = 10.0;
            double aY = 8.0;
            DPoint3d origin = DPoint3d::From (0,0,centerOffset);
            // create variations with shifted z at fringe points.
            double aZ = 3.0;
            for (double phaseFactor : {3.5, 6.2})
                {
                SaveAndRestoreCheckTransform shifter3 (0, 30, 0);
                bvector<DPoint3d> edgePoint;

                // make point coordinates around an ellipse, with z varying on a (faster) sine wave
                for (size_t i = 0; i < totalEdgePoint; i++)
                    {
                    double theta = i * radians;
                    double beta = phaseFactor * radians * i;
                    edgePoint.push_back (DPoint3d::From (aX * cos(theta), aY * sin (theta), aZ * sin (beta)));
                    }
                DPoint3d edgePoint0 = edgePoint[0];
                edgePoint.push_back (edgePoint0);     // replicate for simple indexing
                // Make meshes with various leading subsets of the egde points.
                for (size_t numEdgePoint : bvector<size_t>{totalEdgePoint, 3, 4, 6, 7})
                    {
                    if (numEdgePoint <= totalEdgePoint)
                        {
                        SaveAndRestoreCheckTransform shifter (25,0,0);
                        bvector<DPoint3d> vertexPolygon;
                        IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
                        for (size_t i = 0; i < numEdgePoint; i++)
                            {
                            meshBuilder->AddTriangulation (bvector<DPoint3d> {origin, edgePoint[i], edgePoint[i+1]});
                            auto unitNormal = DVec3d::FromNormalizedCrossProductToPoints(origin, edgePoint[i], edgePoint[i+1]);
                            vertexPolygon.push_back (origin + unitNormal * positiveOffset);
                            }
                        for (auto xyz : vertexPolygon)
                            Check::SaveTransformed (DSegment3d::From (origin, DPoint3d::FromInterpolate (origin, 0.8, xyz)));
                        auto v0 = vertexPolygon[0];
                        vertexPolygon.push_back (v0);
                        Check::SaveTransformed (vertexPolygon);
                        auto mesh0 = meshBuilder->GetClientMeshPtr ();
                        Check::SaveTransformed (*mesh0);

                        auto mesh1 = mesh0->ComputeOffset (offsetOptions, positiveOffset, -0.25, true, false, false);
                        if (mesh1.IsValid ())
                            {
                            Check::Shift (0,0,15);
                            Check::SaveTransformed (*mesh0);
                            Check::SaveTransformed (*mesh1);
                            }
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("PolyfaceOffset.CrinklePointB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,CrinklePointC)
    {
    // unused - static double s_zScale = 1.0;
    PolyfaceHeader::OffsetOptions offsetOptions;

    auto options = IFacetOptions::Create ();
    double positiveOffset = 2.0;
    for (double centerZ : {0.0, 1.0, -1.0})
        {
        SaveAndRestoreCheckTransform shifter1 (0, 50, 0);
        for (uint32_t numEdgePoint : {3, 4, 7})
            {
            SaveAndRestoreCheckTransform shifter2 (100, 0);
            double radians = Angle::TwoPi () / numEdgePoint;
            double aX = 10.0;
            double aY = 10.0;
            DPoint3d origin = DPoint3d::From (0,0,centerZ);
            // create variations with shifted z at fringe points.
            // unused - double aZ = 3.0;
            bvector<DPoint3d> edgePoint;

            // make point coordinates around an ellipse, with z varying on a (faster) sine wave
            for (size_t i = 0; i < numEdgePoint; i++)
                {
                double theta = i * radians;
                edgePoint.push_back (DPoint3d::From (aX * cos(theta), aY * sin (theta), 0.0));
                }
            DPoint3d edgePoint0 = edgePoint[0];
            edgePoint.push_back (edgePoint0);     // replicate for simple indexing
            bvector<DPoint3d> vertexPolygon;
            IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
            for (size_t i = 0; i < numEdgePoint; i++)
                {
                bvector<DPoint3d> triangle {origin, edgePoint[i], edgePoint[i+1]};
                auto unitNormal = DVec3d::FromNormalizedCrossProductToPoints(origin, edgePoint[i], edgePoint[i+1]);
                meshBuilder->AddTriangulation (triangle);
                bvector<DPoint3d> offsetTriangle = triangle;
                offsetTriangle.push_back (triangle.front ());
                for (auto i1 = 0; i1 < offsetTriangle.size (); i1++)
                    offsetTriangle[i1] = offsetTriangle[i1] + positiveOffset * unitNormal;
                Check::SaveTransformed (offsetTriangle);
                }
            auto mesh0 = meshBuilder->GetClientMeshPtr ();
            Check::SaveTransformed (*mesh0);

            auto mesh1 = mesh0->ComputeOffset (offsetOptions, positiveOffset, -0.25, true, false, false);
            if (mesh1.IsValid ())
                {

                Check::Shift (3.0 * aX, 0, 0);
                Check::SaveTransformed (*mesh0);
                Check::SaveTransformed (*mesh1);
                }
            }
        }
    Check::ClearGeometry ("PolyfaceOffset.CrinklePointC");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceOffset,Torus)
    {
    PolyfaceHeader::OffsetOptions offsetOptions;
    double r0 = 8.0;
    double r1 = 2.0;
    auto solid = ISolidPrimitive::CreateDgnTorusPipe (
       DgnTorusPipeDetail
            (
            DPoint3d::From (0,0,0),
            DVec3d::From (1,0,0),
            DVec3d::From (0,1,0),
            r0, r1,
            Angle::DegreesToRadians (180.0),
            true
            ));

    auto options = IFacetOptions::Create ();
    options->SetAngleTolerance (Angle::DegreesToRadians (15.0));
    IPolyfaceConstructionPtr meshBuilder = IPolyfaceConstruction::Create (*options);
    meshBuilder->AddSolidPrimitive (*solid);
    auto mesh0 = meshBuilder->GetClientMeshPtr ();
    Check::SaveTransformed (*mesh0);
    TransformShifter shifter (25,0,0, 0,20,0);
    for (double offset : bvector<double>{0.1, 0.3})
        {
        shifter.DoShift1 ();
        for (Angle theta : bvector<Angle>
                {
                Angle::FromDegrees (10.0),
                Angle::FromDegrees (20.0),
                Angle::FromDegrees (30.0)
                })
            {
            auto meshA = mesh0->Clone ();
            shifter.DoShift0 ();
            shifter.DoShift0 ();
            //static double s_accumulatedAngleFactor = 3.0;
            auto meshB = meshA->ComputeOffset (offsetOptions, offset, 0.0, true, false, false);
            // save meshA AFTER so it shows the intermediate normal computations
            Check::SaveTransformed (*meshA);
            if (meshB.IsValid ())
                Check::SaveTransformed (*meshB);
            }
        }
    Check::ClearGeometry ("PolyfaceOffset.Torus");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SimpleCreate)
    {
    //     4  3  6
    //     1  2  5
    bvector<DPoint3d> points
        {
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (2,1,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (3,0,1),
        DPoint3d::From (3,1,1)
        };
    // Cover the two unit squares with various index structures.
    bvector<int> index0 {1,2,3,4,0, 6,3,2,5, 0};

    bvector<int> index1 {1,2,3,4,0,    6,3,2,0,     2,5,6,0};

    bvector<int> index3 {1,2,3,   3,4,1,  2,5,6,  2,6,3};

    bvector<int> index4 {1,2,3,4,    2,5,6,3};

    bvector<PolyfaceHeaderPtr> meshes;
    meshes.push_back (PolyfaceHeader::CreateIndexedMesh (0, points, index0));
    meshes.push_back (PolyfaceHeader::CreateIndexedMesh (0, points, index1));
    meshes.push_back (PolyfaceHeader::CreateIndexedMesh (3, points, index3));
    meshes.push_back (PolyfaceHeader::CreateIndexedMesh (4, points, index4));
    DPoint3d origin = DPoint3d::From (0,0,0);
    bvector<DMatrix4d> moments;
    for (auto & mesh : meshes)
        {
        DMatrix4d matrix;
        mesh->SumFacetSecondAreaMomentProducts (origin, matrix);
        moments.push_back (matrix);
        double area = mesh->SumFacetAreas ();
        Check::Near (area, matrix.coff[3][3], "Area via direct or moment call.");
        }
    for (size_t i = 1; i < moments.size (); i++)
        {
        double d0, d1, d2, d3;
        moments[0].MaxAbsDiff (moments[1], d0, d1, d2, d3);
        double d = d0 + d1 + d2 + d3;
        Check::Near (1.0, 1.0 + d, "AreaMoment match");
        }
    }
void TestGridCutFill (DPoint3dDVec3dDVec3d frameA, DPoint3dDVec3dDVec3d frameB, char const * title)
    {
    auto dtm = UnitGridPolyface (frameA, 7,5, false);
    Check::PrintIndent (0);
    Check::Print ("TestGridCutFill");
    Check::Print (title);
    PrintPolyfaceSummary (*dtm, "DTM", stdout);
    for (int ny = 1; ny < 4; ny++)
        {
        Check::PrintIndent (0);
        Check::PrintIndent (1);
        Check::Print ((int)(ny+1), "NX");
        Check::Print ((int)(ny), "NY");
        auto road = UnitGridPolyface (frameB, ny + 1, ny, false);
        PrintPolyfaceSummary (*road, "road", stdout);
        bvector<PolyfaceHeaderPtr> cut, fill;
        PolyfaceQuery::ComputeCutAndFill (*dtm, *road, cut, fill);
        for (auto &c : cut)
            PrintPolyfaceSummary (*c, "cut", stdout);
        for (auto &f : fill)
            PrintPolyfaceSummary (*f, "fill", stdout);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CutFill0)
    {
    DPoint3dDVec3dDVec3d frameA (0,0,0,   4,0,0,   0,4,0);
    DPoint3dDVec3dDVec3d frameB (1,2,1,   1,0,0,   0,1,0);
    auto dtm = UnitGridPolyface (frameA, 1,1, false);
    auto road = UnitGridPolyface (frameB, 1, 1, false);
    PrintPolyfaceSummary (*dtm, "dtm", stdout);
    PrintPolyfaceSummary (*road, "road", stdout);
    bvector<PolyfaceHeaderPtr> cut, fill;
    PolyfaceQuery::ComputeCutAndFill (*dtm, *road, cut, fill);
    for (auto &c : cut)
        {
        PrintPolyfaceSummary (*c, "cut", stdout);
        PrintPolyface (*c, "cut", stdout, 30);
        }
    for (auto &f : fill)
        {
        PrintPolyfaceSummary (*f, "fill", stdout);
        PrintPolyface (*f, "fill", stdout, 30);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(Polyface,CutFillA)
    {
    DPoint3dDVec3dDVec3d frameA (0,0,0,   2,0,0,   0,2,0);
    DPoint3dDVec3dDVec3d frameBpos (1,1,1,   2,0,0,   0,2,0);
    DPoint3dDVec3dDVec3d frameBneg (1,1,-1,   2,0,0,   0,2,0);
    DPoint3dDVec3dDVec3d frameB1(1,1,1,   2,0,0,   0,2,1);
    DPoint3dDVec3dDVec3d frameB2(1,1,1,   2,0,0,   0,2,-1);
    TestGridCutFill (frameA, frameBpos, "Flat0*Flat1");
    TestGridCutFill (frameA, frameBneg, "Flat0*Flatneg1");
    TestGridCutFill (frameA, frameB1, "Flat0*Slope1UP");
    TestGridCutFill (frameA, frameB2, "Flat0*Slope1DOWN");

    }

void CollectDrape (PolyfaceHeaderPtr  &mesh, double x0, double x1, double y, double z = 1.0)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (x0,y, z), DPoint3d::From (x1,y, z)
        };
    DVec3d viewVector = DVec3d::From (0,0.1, -1); // view direction tips to make top view expose result.
    auto cv = mesh->DrapeLinestring (points, viewVector);
    points.push_back (points.back () + viewVector);
    Check::SaveTransformed (points);
    if (cv.IsValid ())
        Check::SaveTransformed (*cv);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,DrapeLinestring)
    {
    double b = 10.0;
    double c = 300.0;
    DPoint3dDVec3dDVec3d frameA (0,0,0,   b,0,0,   c,c,0);  // slant so the facet is much smaller than its range box
    auto facet = UnitGridPolyface (frameA, 1, 1, false);
    facet->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    Check::SaveTransformed (*facet);
    Check::Shift (0,0,0.1);     // keep the lines strictly above the facet.
    double y = 1.0;
    bvector<double> allX {-5, -1, 0, 3, 6, 10, 12, 15};
    double dy = 1.0;
    double y0 = 100.0;  // facet right edge is x=y
    for (auto x0 : allX)
        {
        y0 += 20.0;
        y = y0;
        for (double x1 : allX)
            {
            CollectDrape (facet, y + x0, y + x1, y);
            y += dy;
            }
        }
    Check::ClearGeometry ("Polyface.DrapeLinestring");
    }