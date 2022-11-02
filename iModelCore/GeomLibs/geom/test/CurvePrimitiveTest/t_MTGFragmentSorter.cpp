/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Mtg/MtgApi.h>
//static int s_noisy = 0;
void Print (Utf8CP title, bvector<int> &data)
    {
    if (data.size () > 0)
        {
        GEOMAPI_PRINTF ("%s\n", title);
        for (int x : data)
            {
            GEOMAPI_PRINTF (" %d", x);
            if (x == 0)
              GEOMAPI_PRINTF ("\n");
            }
        if (data.back () != 0)
            GEOMAPI_PRINTF ("\n");
        }
    }

void TestStepGraph (int numSteps, int minDiagonal, int maxDiagonal)
    {
    MTGFragmentSorterP sorter = bsiMTGFragmentSorter_new ();
    double x0 = 0.0;
    double y0 = 0.0;
    double dx = 1.0;
    double dy = 2.0;
    // Create segments that join to form steps, but not in usual order . .
    DPoint3d xyz0, xyz1, xyz2;
    int numFragment = 0;
    for (int i = 0; i < numSteps; i++, x0 += dx, y0 += dy)
        {
        xyz0.Init (x0, y0);
        xyz1.Init (x0 + dx, y0);
        xyz2.Init (x0 + dx, y0 + dy);
        
        bsiMTGFragmentSorter_addFragment (sorter, numFragment++, &xyz2, &xyz1);
        bsiMTGFragmentSorter_addFragment (sorter, numFragment++, &xyz0, &xyz1);

        if (i >= minDiagonal && i <= maxDiagonal)
            {
            DPoint3d xyz3 = DPoint3d::FromInterpolate (xyz2, 0.5, xyz0);
            bsiMTGFragmentSorter_addFragment (sorter, numFragment++, &xyz2, &xyz3);
            bsiMTGFragmentSorter_addFragment (sorter, numFragment++, &xyz3, &xyz0);
            }
        }
    bvector<int> loops;
    bvector<int> chains;        
    bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains (sorter, &loops, &chains);
    if (Check::PrintDeepStructs ())
        {
        GEOMAPI_PRINTF ("\n Fragment assembly (numSteps %d) (diagonals %d %d)\n", numSteps, minDiagonal, maxDiagonal);
        Print ("Loops", loops);
        Print ("Chains", chains);
        }
    bsiMTGFragmentSorter_free (sorter);        

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MTGFragmentSorter,HelloWorld)
    {
    TestStepGraph (1, 0,-1);
    TestStepGraph (1, 0,0);
    TestStepGraph (2, 0,0);
    TestStepGraph (4, 1,3);
    }
