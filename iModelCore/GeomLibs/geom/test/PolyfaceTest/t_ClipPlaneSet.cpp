/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_ClipPlaneSet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

TEST(ClipPlaneSet,SweptPolygon)
    {
    ConvexClipPlaneSet convexSetA, convexSetB, convexSet0;
    bvector<DPoint3d> triangle {
        DPoint3d::From (1,1,1),
        DPoint3d::From (3,1,2),
        DPoint3d::From (2,2,1)
        };
    auto sweepDirection = DVec3d::From (0,0,1);
    convexSetA.ReloadSweptConvexPolygon (triangle, sweepDirection, 1);
    convexSetB.ReloadSweptConvexPolygon (triangle, sweepDirection, -1);
    convexSet0.ReloadSweptConvexPolygon (triangle, sweepDirection, 0);
    double tolerance = 1.0e-10;
    bvector<double> values  {-0.5, 0.3, 0.5, 0.8, 1.1};
    for (double u : values)
        {
        for (double v : values)
            {
            double w = 1.0 - u - v;
            bool inside = DoubleOps::IsIn01 (u)
                && DoubleOps::IsIn01 (v)
                && DoubleOps::IsIn01 (w);
            DPoint3d planePoint = DPoint3d::FromSumOf (triangle[0], u, triangle[1], v, triangle[2], w);
            DPoint3d abovePoint = planePoint + sweepDirection;
            DPoint3d belowPoint = planePoint - sweepDirection;

            Check::Bool (inside, convexSetA.IsPointOnOrInside (abovePoint, tolerance));
            Check::False (convexSetA.IsPointOnOrInside (belowPoint, tolerance));
            Check::Bool (inside, convexSetA.IsPointOnOrInside (planePoint, tolerance));

            Check::Bool (inside, convexSetB.IsPointOnOrInside (belowPoint, tolerance));
            Check::False (convexSetB.IsPointOnOrInside (abovePoint, tolerance));
            Check::Bool (inside, convexSetB.IsPointOnOrInside (planePoint, tolerance));

            Check::Bool (inside, convexSet0.IsPointOnOrInside (abovePoint, tolerance));
            Check::Bool (inside, convexSet0.IsPointOnOrInside (belowPoint, tolerance));
            Check::Bool (inside, convexSet0.IsPointOnOrInside (planePoint, tolerance));

            }
        }
    }
void testConvexClipXY (double x0, double y0, double ux, double uy, bvector<DPoint3d> &xyz)
    {
    DPlane3d plane0 = DPlane3d::FromOriginAndNormal (x0, y0, 0.0, ux, uy, 0.0);
    DPlane3d plane1 = DPlane3d::FromOriginAndNormal (x0, y0, 0.0, -ux, -uy, 0.0);
    ClipPlane clip0 (plane0);
    ClipPlane clip1 (plane1);
    bvector<DPoint3d> xyz0 = xyz, xyz1 = xyz;
    DVec3d normal = PolygonOps::AreaNormal (xyz);
    double area = normal.Magnitude ();
    bvector<DPoint3d> work;
    clip0.ConvexPolygonClipInPlace (xyz0, work);
    clip1.ConvexPolygonClipInPlace (xyz1, work);
    DVec3d normal0 = PolygonOps::AreaNormal (xyz0);
    DVec3d normal1 = PolygonOps::AreaNormal (xyz1);
    double area0 = normal0.Magnitude ();
    double area1 = normal1.Magnitude ();
    Check::Near (area, area0 + area1);
    }

TEST(ClipPlane,ConvexPolygonClip)
    {
    for (auto &polygon :
        bvector<bvector<DPoint3d>>
            {
            bvector<DPoint3d>
                {
                DPoint3d::From (0,0,0),
                DPoint3d::From (1,0,0),
                DPoint3d::From (1,1,0),
                DPoint3d::From (0,1,0)
                },
            bvector<DPoint3d>
                {
                DPoint3d::From (0.2,0,0),
                DPoint3d::From (1-0.3,0),
                DPoint3d::From (1,1.2,0),
                }
            })
        {
        testConvexClipXY (0,0.5,   0,1  , polygon);
        testConvexClipXY (1,0.5,   0,1  , polygon);
        testConvexClipXY (0,0.5,   1,0  , polygon);
        testConvexClipXY (1,0.5,   1,0  , polygon);
        testConvexClipXY (0.5, 0.6, 0.3, 0.2, polygon);
        testConvexClipXY (0,2,   0,1  , polygon);
        testConvexClipXY (0,-3,   0,1  , polygon);

        testConvexClipXY (1,1, 1,1, polygon);
        testConvexClipXY (1,1, -1,2, polygon);
        }

    }

TEST(ClipPlaneSet,Undercut1)
    {
    printf (" enter undercut\n");
    auto roadSurface = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0),
            3,1, true
            );
    roadSurface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto bridge = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  1,1,0.5,    5,0,1,   0,2,0),
            1,1, true
            );
    bridge->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    printf (" road points %d\n", (int)roadSurface->Point ().size ());
    printf (" bridge points %d\n", (int)bridge->Point ().size());
    PolyfaceHeaderPtr undercut;
    PolyfaceHeader::ComputeUndercut (*roadSurface, nullptr, *bridge, nullptr, undercut);

    Check::SaveTransformed (*roadSurface);
    Check::SaveTransformed (*bridge);

    if (Check::True (undercut.IsValid ()))
        {
        printf (" undercut points %d\n", (int)undercut->Point ().size());
        Check::SaveTransformed (*undercut);
        }
    Check::ClearGeometry ("ClipPlaneSet.Undercut1");
    }

TEST(ClipPlaneSet,Undercut2)
    {
    printf (" enter undercut\n");
    auto dtm = PolyfaceWithSinusoidalGrid (20, 20,
                    0.0, 0.1, 
                    0.9, 0.15,
                    true);
    dtm->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto bridge = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  1,1,0,    5,0,0,   0,2,0.2),
            3,3, true
            );
    bridge->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    printf (" dtm points %d\n", (int)dtm->Point ().size ());
    printf (" bridge points %d\n", (int)bridge->Point ().size());
    PolyfaceHeaderPtr undercut;
    PolyfaceHeader::ComputeUndercut (*dtm, nullptr, *bridge, nullptr, undercut);

    Check::SaveTransformed (*dtm);
    Check::SaveTransformed (*bridge);

    if (Check::True (undercut.IsValid ()))
        {
        printf (" undercut points %d\n", (int)undercut->Point ().size());
        Check::SaveTransformed (*undercut);
        }
    Check::ClearGeometry ("ClipPlaneSet.Undercut2");
    }

