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
    if (select != 0)
        return;
    Check::StartScope ("Decimate ShortEdge", shortEdge);
    DRange3d range = DRange3d::From (points);
    SaveAndRestoreCheckTransform shifter (1.5 * range.XLength (), 0, 0);
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateQuadGrid (3);
    if (print)
        printf ("\nEdgeCollapse method %d\n", select);
    for (DPoint3d xyz : points)
        mesh->Point ().push_back (xyz);
    mesh->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
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
    testDecimate (0, points, 1.2, 3, s_print);
    testDecimate (0, points, 0.3, 0, s_print);

    // with boundary protect expect to preserve the upper boundar beause if sharp edge?
    testDecimate (1, points, 1.2, 2, s_print);
    testDecimate (1, points, 0.3, 0, s_print);

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