/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <msgeomstructs.h>
#include <vuPolygonClassifier.h>


static void Test (void)
{
    STDVectorDPoint3d A, B;

    PushBack (A, 10000, 3029.99,   0);
    PushBack (A, 10000, 10000,     0);
    PushBack (A, 2008.98, 10000,   0);
    PushBack (A, 2008.98, 3980.55, 0);
    PushBack (A, 0, 4000,          0);
    PushBack (A, 0, 3029.99,       0);
    PushBack (A, 10000, 3029.99,   0);


    PushBack (B, 4000, 2000, 0);
    PushBack (B, 4000, 4000, 0);
    PushBack (B, 10000, 4000, 0);
    PushBack (B, 10000, 6000, 0);
    PushBack (B, 2008.98, 6000, 0);
    PushBack (B, 2008.98, 3980.55,  0);
    PushBack (B, 0, 4000, 0);
    PushBack (B, 0, 2000, 0);
    PushBack (B, 4000, 2000, 0);
    //
    //
    //
    VuPolygonClassifier vu;
    vu.mFaceMode = FaceMode_Triangulate;
    PrintDisconnectChainsAsKeyin (A);
    PrintDisconnectChainsAsKeyin (B);
    vu.ClassifyAMinusB (A, B);

    //
    //
    //

    STDVectorDPoint3d xyz;
    PrintDisconnectChainsAsKeyin (xyz);
    for (;vu.GetFace (xyz);)
        {
        PrintDisconnectChainsAsKeyin (xyz);
        }

   printf ("PLACE SHAPE\n");
}//Test
void main ()
    {
    Test ();
    }