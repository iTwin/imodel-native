//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Vu/vuApi.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>

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

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    VuSetP graph = vu_newVuSet (0);
    VuP nodeA0 = NULL;
    VuP nodeA1 = NULL;
    VuP chain0 = NULL;
    VuP chain1 = NULL;
    for (int i = 0; i < 4; i++)
        {
        vu_join (graph, chain1, NULL, &nodeA0, &nodeA1);
        chain1 = nodeA1;
        if (NULL == chain0)
            chain0 = nodeA0;
        }
    printFaceLoops (graph, "Raw chain");
    vu_vertexTwist (graph, chain0, chain1);
    printFaceLoops (graph, "Loop");
    vu_freeVuSet (graph);
    return getExitStatus();
    }
