/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/doc/build/bsivudoc/examples/splitconvexparts.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Vu.h>

#define MAX_VERTICES 5000

Public StatusInt    vu_splitToConvexParts
(
DPoint3d*                       pLoopPoints,
int                             numLoopPoints,
void*                           userDataP,
VuTaggedDPoint3dArrayFunction   polygonFunc
)
    {
    static double s_relTol = 1.0e-5;
    static double s_graphAbsTol = 1.0e-10;
    static double s_graphRelTol = 1.0e-10;

    VuSetP  graphP = vu_newVuSet (0);
    if (!graphP)
        return ERROR;

    vu_setTol (graphP, s_graphAbsTol, s_graphRelTol);

    int i0, i1, numThisLoop;
    for (i0 = 0 ; i0 < numLoopPoints ;)
        {
        for (i1 = i0; i1 < numLoopPoints && pLoopPoints[i1].x != DISCONNECT; i1++);
        numThisLoop = i1 - i0;
        if (numThisLoop > 2)
            vu_loopFromDPoint3dArrayXYTol (graphP, pLoopPoints + i0, numThisLoop, 0.0, s_relTol);
        i0 = i1 + 1;
        }

    vu_mergeLoops (graphP);
    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries (graphP, true);
    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_removeEdgesToExpandConvexInteriorFaces (graphP);

    VuArrayP    faceArrayP = vu_grabArray (graphP);
    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    VuP         faceP;
    DPoint3d    facePoints[MAX_VERTICES + 1];
    int         faceFlag[MAX_VERTICES + 1];
    for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &faceP);)
        {
        numThisLoop = 0;
        VU_FACE_LOOP (P, faceP)
            {
            vu_getDPoint3d (&facePoints[numThisLoop], P);
            faceFlag[numThisLoop] = vu_getMask (P, VU_BOUNDARY_EDGE) != 0;
            numThisLoop++;
            }
        END_VU_FACE_LOOP (P, faceP)

        facePoints[numThisLoop] = facePoints[0];
        faceFlag[numThisLoop] = 0;
        numThisLoop++;

        polygonFunc (facePoints, faceFlag, numThisLoop, userDataP);
        }

    vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);
    return SUCCESS;
    }
