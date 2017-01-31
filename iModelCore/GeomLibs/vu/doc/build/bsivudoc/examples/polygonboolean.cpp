/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/doc/build/bsivudoc/examples/polygonboolean.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Vu.h>

Public void     myPolygonBoolean
(
bvector<DPoint3d>&  xyzOut,             // input and output are structured as packed point arrays...
bvector<int>&       loopCountsOut,      // ... with parallel count arrays indicating loop sizes
DPoint3d*               xyzIn,              // input polygon vertices; size = sum of entries in loopCountsIn
int*                    loopCountsIn,       // size of each polygon; first/last points do not have to be equal
int                     numLoops,           // number of polygons to merge
bool                    closeOutputLoops,   // whether to duplicate first point at end of each output loop
int                     opCode              // 0 for union of all loops, 1 for intersection of all loops
)
    {
    VuStackOpFuncP  vuOpFunc;
    VuSetP          pGraph = vu_newVuSet (0);
    VuArrayP        pFaceArray;
    VuP             pFace;
    DPoint3d        xyz0, xyz;
    int             iLoop, numThisLoop, xyzStart;

    if (opCode == 0)
        vuOpFunc = vu_orLoops;
    else if (opCode == 1)
        vuOpFunc = vu_andLoops;
    else
        return;

    // Build the VU graph
    for (iLoop = xyzStart = numThisLoop = 0; iLoop < numLoops; iLoop++, xyzStart += numThisLoop)
        {
        numThisLoop = loopCountsIn[iLoop];

        if (numThisLoop > 2)
            {
            // Save the earlier graph
            if (iLoop > 0)
                vu_stackPush (pGraph);

            // Add this loop to the graph
            vu_makeLoopFromArray3d (pGraph, xyzIn + xyzStart, numThisLoop, true, true);

            // Resolve loop criss-cross(es) using parity rules
            vu_mergeLoops (pGraph);
            vu_regularizeGraph (pGraph);
            vu_markAlternatingExteriorBoundaries (pGraph, true);

            // Merge with prior loop(s)
            if (iLoop > 0)
                vu_stackPopWithOperation (pGraph, vuOpFunc, NULL);
            }
        }

    // Collect a node from each resultant loop
    pFaceArray = vu_grabArray (pGraph);
    vu_collectInteriorFaceLoops (pFaceArray, pGraph);

    xyzOut.clear();
    loopCountsOut.clear();

    // Output each loop
    for (vu_arrayOpen (pFaceArray); vu_arrayRead (pFaceArray, &pFace);)
        {
        numThisLoop = 0;
        vu_getDPoint3d (&xyz0, pFace);

        VU_FACE_LOOP (pNode, pFace)
            {
            vu_getDPoint3d (&xyz, pNode);
            xyzOut.push_back (xyz);
            numThisLoop++;
            }
        END_VU_FACE_LOOP (pNode, pFace)

        // Replicate first point
        if (closeOutputLoops)
            {
            numThisLoop++;
            xyzOut.push_back (xyz0);
            }

        loopCountsOut.push_back (numThisLoop);
        }

    vu_returnArray (pGraph, pFaceArray);
    vu_freeVuSet (pGraph);
    }
