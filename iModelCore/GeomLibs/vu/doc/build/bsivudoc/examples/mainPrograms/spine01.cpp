/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/doc/build/bsivudoc/examples/mainPrograms/spine01.cpp $
|
|  $Copyright: (c) 2008 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Test program to demonstrate extracting a polygon "spine" using vu.
#include <stdio.h>
#include <vuspinecontext.h>

static void Test (void)
{
    STDVectorDPoint3d A;
    PushBack (A, 0,0,0);
    PushBack (A, 0,0,0);
    PushBack (A, 10,0,0);
    PushBack (A, 10,10,0);
    PushBack (A, 9,10,0);
    PushBack (A, 9,1,0);
    PushBack (A, 0,1,0);
    PushBack (A, 0,0,0);

    PrintDisconnectChainsAsKeyin (A);
    //
    //
    double minSplitAngle = 0.3;
    //
    //
    double minDiagonalAngle = 1.0;
    VuSpineContext sc;
    sc.InsertEdges (A, true);
    sc.TriangulateForSpine (true, minSplitAngle);
    sc.MarkBoxes (true, minDiagonalAngle);


    STDVectorDPoint3d xyzOut;
    sc.GetSpineEdges (xyzOut);
    PrintDisconnectChainsAsKeyin (xyzOut);

    printf ("SELECT\n");
}//Test

void main ()
    {
    Test ();
    }