/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Mtg/MtgApi.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>


int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    MTGGraphP graph = jmdlMTGGraph_newGraph ();

    MTGNodeId nodeA0 = MTG_NULL_NODEID;
    MTGNodeId nodeA1 = MTG_NULL_NODEID;
    MTGNodeId chain0 = MTG_NULL_NODEID;
    MTGNodeId chain1 = MTG_NULL_NODEID;

    for (int i = 0; i < 4; i++)
        {
        jmdlMTGGraph_join (graph, &nodeA0, &nodeA1, chain1, MTG_NULL_NODEID, MTG_NULL_MASK, MTG_NULL_MASK);
        chain1 = nodeA1;
        if (MTG_NULL_NODEID == chain0)
            chain0 = nodeA0;
        }
    printf ("PRE TWIST\n");
    jmdlMTGGraph_printFaceLoops (graph);
    jmdlMTGGraph_vertexTwist (graph, chain0, chain1);

    printf ("POST TWIST\n");
    jmdlMTGGraph_printFaceLoops (graph);
    jmdlMTGGraph_freeGraph (graph);
    return getExitStatus();
    }
