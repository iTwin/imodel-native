/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_decimate.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

void testDecimate (int select, bvector<DPoint3d> &points, double shortEdge, size_t numCollapse, bool print = false)
    {
    Check::StartScope ("Decimate ShortEdge", shortEdge);
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateQuadGrid (3);
    if (print)
        printf ("\nEdgeCollapse method %d\n", select);
    for (DPoint3d xyz : points)
        mesh->Point ().push_back (xyz);
    mesh->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    if (print)
        PrintPolyface (*mesh, "PreDecimate", stdout, 100, false);

    size_t numA = 0;
    if (select == 1)
        numA = mesh->DecimateByEdgeCollapseWithBoundaryControl (shortEdge, 0.0);
    else
        numA = mesh->DecimateByEdgeCollapse (shortEdge, 0.0);
    if (print)
        PrintPolyface (*mesh, "PostDecimate", stdout, 100, false);
    Check::Size (numCollapse, numA, "decimate count");
    Check::EndScope ();
    }

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
    testDecimate (0, points, 1.2, 3, s_print);
    testDecimate (0, points, 0.3, 0, s_print);
    // with boundary protect expect to preserve the upper boundar beause if sharp edge?
    testDecimate (1, points, 1.2, 2, s_print);
    testDecimate (1, points, 0.3, 0, s_print);
    }
