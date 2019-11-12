/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Vu/vuApi.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

void printFaceLoops (VuSetP graph, char *message)
    {
    VuMask visited = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visited);
    printf ("<FaceLoops");
    if (message)
        printf (" state=\"%s\"", message);
    printf (">\n");
    VU_SET_LOOP (faceSeed, graph)
        {
        if (!vu_getMask (faceSeed, visited))
            {
            vu_setMaskAroundFace (faceSeed, visited);
            printf ("<face>");
            VU_FACE_LOOP (vertex, faceSeed)
                {
                printf ("%3d ", vu_getIndex (vertex));
                }
            END_VU_FACE_LOOP (vertex, faceSeed)
            printf ("</face>\n");
            }
        }
    END_VU_SET_LOOP (faceSeed, graph)
    printf ("</FaceLoops>\n");
    }
void PrintFacetIndices (bvector<int> &index)
    {
    for (size_t i = 0; i < index.size (); i++)
        {
        if (index[i] == 0)
            printf ("\n");
        else
            printf (" %d", index[i]);
        }
    }

void Print (bvector<DPoint3d> &xyz, char *pTitle = NULL)
    {
    if (NULL != pTitle)
        printf (" %s\n", pTitle);
    for (size_t i = 0; i < xyz.size (); i++)
        {
        printf ("%4d %.15le,%.15le,%.15le\n", i, xyz[i].x, xyz[i].y, xyz[i].z);
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    bvector<DPoint3d> xyz;
    bvector<DPoint3d>xyzOut;
    bvector<int> index;

    DPoint3dOps::Append (&xyz, 0,0);
    DPoint3dOps::Append (&xyz, 1,0);
    DPoint3dOps::Append (&xyz, 1,1);
    DPoint3dOps::Append (&xyz, 0,1);
    DPoint3dOps::FixupAndTriangulateLoopsXY (&index, NULL, NULL, &xyz, 0.0, 3, true, true);
    Print (xyz,"Unit Square");
    PrintFacetIndices (index);

    DPoint3dOps::AppendDisconnect (&xyz);
    DPoint3dOps::Append (&xyz, -1,-1,0);
    DPoint3dOps::Append (&xyz, 3,1,0);
    DPoint3dOps::Append (&xyz, 3,2,0);
    DPoint3dOps::Append (&xyz, -1,3,0);
    DPoint3dOps::AppendDisconnect (&xyz);

    DPoint3dOps::FixupAndTriangulateLoopsXY (&index, NULL, &xyzOut, &xyz, 0.0, 3, true, true);
    Print (xyz,"HoleIn");
    PrintFacetIndices (index);
    Print (xyzOut, "HoleOut");

    return getExitStatus();
    }
