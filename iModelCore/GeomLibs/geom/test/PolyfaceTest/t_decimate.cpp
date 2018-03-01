/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_decimate.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
// ASSUME points are in quad grid layout 
void testDecimate (int select, bvector<DPoint3d> &points, int vertsPerRow, double shortEdge, size_t numCollapse, bool print = false)
    {
    if (select != 0)
        return;
    Check::StartScope ("Decimate ShortEdge", shortEdge);
    DRange3d range = DRange3d::From (points);
    SaveAndRestoreCheckTransform shifter (1.5 * range.XLength (), 0, 0);
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh->Param ().SetActive (true);
    if (print)
        printf ("\nEdgeCollapse method %d\n", select);
        // simple push to point and param arrays.
    for (size_t i = 0; i < points.size (); i++)
        {
        mesh->Point ().push_back (points[i]);
        mesh->Param ().push_back( DPoint2d::From(points[i]));
        }
    for (size_t i0 = 0; i0 + vertsPerRow  <= points.size (); i0 += vertsPerRow)
        {

        if (i0 > 0)
            {
            for (size_t i = 0; i + 1 < vertsPerRow; i++)    // i is index of a quad moving left to right along this row.
                {
                // ZERO based point indices of quad in this row and below
                //    j0---j1
                //     |\  |
                //     |  \|
                //    k0---k1
                size_t j0 = i0 + i;           size_t j1 = j0 + 1;
                size_t k0 = j0 - vertsPerRow; size_t k1 = k0 + 1;
                // add one to make one-based
                mesh->PointIndex ().push_back ((int)(k0 + 1));
                mesh->PointIndex ().push_back ((int)(k1 + 1));
                mesh->PointIndex ().push_back ((int)(j0 + 1));
                mesh->PointIndex ().push_back ((int)0);

                mesh->PointIndex ().push_back ((int)(k1 + 1));
                mesh->PointIndex ().push_back ((int)(j1 + 1));
                mesh->PointIndex ().push_back ((int)(j0 + 1));
                mesh->PointIndex ().push_back ((int)0);
                }
            }
        }
    Check::SaveTransformed (*mesh);
    Check::Shift (0, 1.5 * range.YLength (), 0);
    size_t numA = 0;
    if (select == 1)
        {
#ifdef CompileDecimateByEdgeCollapseWithBoundaryControl
        numA = mesh->DecimateByEdgeCollapseWithBoundaryControl (shortEdge, 0.0);
#endif
        }
    else

        numA = mesh->DecimateByEdgeCollapse (shortEdge, 0.0);
    Check::SaveTransformed (*mesh);
    Check::Size (numCollapse, numA, "decimate count");
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,DecimateByEdgeCollapseWithBoundaryControl)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),     // Close to corner
        DPoint3d::From (3,0,0),

        DPoint3d::From (0,2,0),
        DPoint3d::From (1,2,0),     // Close to mid-edge
        DPoint3d::From (3,2,0),

        DPoint3d::From (0,4,0),
        DPoint3d::From (0.5,3.5,0),       // Closer, with sharp angle that will get pinned early.
        DPoint3d::From (3,4,0),
        };
    static bool s_print = false;
    // blind edge collapse -- expect to collapse edges from 00--10, 02--10, (0,4)--(0.5,3.5)
    testDecimate (0, points, 3, 1.2, 3, s_print);
    testDecimate (0, points, 3, 0.3, 0, s_print);

    // with boundary protect expect to preserve the upper boundar beause if sharp edge?
    testDecimate (1, points, 3, 1.2, 2, s_print);
    testDecimate (1, points, 3, 0.3, 0, s_print);

    Check::ClearGeometry ("Polyface.DecimateByEdgeCollapseWithBuondaryControl");
    }
#ifdef TestCPPCtorDtorScopes
struct ScopeTester
{
int m_id;
ScopeTester (int id) : m_id (id){printf ("( %d", id);}
~ScopeTester (){printf ("  %d)", m_id);}
};
void testScopes ()
    {
    ScopeTester a(0);
    ScopeTester b(1);
    {
    ScopeTester c0(100);
    ScopeTester c1(101);
    }
    ScopeTester d(2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPP,ScopeTester)
    {
    printf ("BEGINSCOPES\n");
    testScopes ();
    printf ("ENDSCOPES\n");
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SortForLongEdgeRemovalA)
    {
    bvector<DPoint3d> points
        {
        // a sqaure
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (1,1,0),
        // a tall rectangle stacked
        DPoint3d::From (0,4,0),
        DPoint3d::From (1,4,0),
        // a square above that
        DPoint3d::From (0,5,0),
        DPoint3d::From (1,5,0),
        };
//   7---8
//   | / |
//   5---6
//   |  /|
//   | / |
//   |/  |
//   3---4
//   | / |
//   1---2
    bvector<int> indices
        {
        1,2,4,0,
        1,4,3,0,
        3,4,6,0,
        3,6,5,0,
        5,6,8,0,
        5,8,7,0
        };
    static bool s_print = false;
    auto polyface = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
    bvector<size_t> smallToLargeReadIndices;
    polyface->ConstructOrderingForLongEdgeRemoval (smallToLargeReadIndices);
    Check::SaveTransformed (*polyface);
    polyface->ExcavateFacetsWithLongBoundaryEdges (1.5);
    Check::Shift (0,10,0);
    Check::SaveTransformed (*polyface);
    Check::ClearGeometry ("Polyface.SortForLongEdgeRemovalA");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SortForLongEdgeRemovalB)
    {
    bvector<DPoint3d> points;
    bvector<int> indices;
//   D1--D2-----D3--D4
//   | / |   /  | / |
//   C1--C2-----C3--C4
//   |  /|    / |  /|
//   | / |   /  | / |
//   |/  |  /   |/  |
//   B1--B2-----B3--B4
//   | / |   /  | / |
//   A1--A2-----A3--A4
    DVec3d bilinearShift = DVec3d::From (3, 8, 0);
// x and y grid values to create isolated smallish triangles in the corners
    bvector<double> xx {0,0.5,1,3,4, 6,7,8,11};
    bvector<double> yy {0,1,4,5};
    for (auto y : yy)
        for (auto x : xx)
            {
            double u = x / xx.back ();
            double v = y / yy.back ();
            points.push_back (DPoint3d::From (x,y) + u * v * bilinearShift);
            }
    size_t nx = xx.size ();
    // triangles moving across the rows ...
    for (size_t iy = 0; iy + 1 < yy.size (); iy++)
        {
        int leftPointIndex = (int)(1 + iy * nx);    // ONE BASED point index at left of row
        for (size_t ix = 0; ix + 1 < xx.size (); ix++)
            {
            int i00 = leftPointIndex + (int)ix;
            int i10 = i00 + 1;
            int i01 = i00 + (int)nx;
            int i11 = i01 + 1;
            indices.push_back (i00); indices.push_back (i10); indices.push_back (i11); indices.push_back (0);
            indices.push_back (i00); indices.push_back (i11); indices.push_back (i01); indices.push_back (0);
            }
        }
    auto polyface = PolyfaceHeader::CreateIndexedMesh (0, points, indices);
    bvector<size_t> smallToLargeReadIndices;
    polyface->ConstructOrderingForLongEdgeRemoval (smallToLargeReadIndices);
    Check::SaveTransformed (*polyface);
    for (auto maxEdgeLength : {4.0, 3.0, 2.0, 1.1, 0.75})
        {
        auto mesh1 = polyface->Clone ();
        Check::Shift (0,20,0);
        if (mesh1->ExcavateFacetsWithLongBoundaryEdges (maxEdgeLength))
            Check::SaveTransformed (*mesh1);

        }
    Check::ClearGeometry ("Polyface.SortForLongEdgeRemovalB");
    }
