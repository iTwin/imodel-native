/*----------------------------------------------------------------------+
|
|     $Source: vu/src/vuconvex.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_graphAbsTol = 1.0e-10;
static double s_graphRelTol = 1.0e-10;
#define MAX_VERTICES 5000

/*---------------------------------------------------------------------------------**//**
* @description Split a polygon into smaller convex polygons.
* @remarks Polygon may have disconnects (points with coordinate values = DISCONNECT) to separate multiple loops (e.g., holes).
* @param pLoopPoints    IN  polygon points, with optional disconnects
* @param numLoopPoints  IN  number of polygon points
* @param userDataP      IN  user data pointer passed into callback
* @param polygonFunc    IN  callback function to announce convex polygons
* @return SUCCESS unless graph allocation failed
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      02/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt vu_splitToConvexParts
(
DPoint3d                            *pLoopPoints,
int                                 numLoopPoints,
void                                *userDataP,
VuTaggedDPoint3dArrayFunction       polygonFunc
)
    {
    StatusInt status = SUCCESS;
    VuSetP      graphP;
    VuArrayP    faceArrayP;
    VuP         faceP;
    int         i0, i1;
    int         trapMask = 1;

    static double s_relTol = 1.0e-13;

    bvector<DPoint3d> facePoints;
    bvector<int> faceFlag;
    facePoints.reserve (10);
    faceFlag.reserve (10);

    graphP = vu_newVuSet (0);

    vu_setTol (graphP, s_graphAbsTol, s_graphRelTol);

    for ( i0 = 0 ; i0 < numLoopPoints ;)
        {
        for (i1 = i0; i1 < numLoopPoints && pLoopPoints[i1].x != DISCONNECT;)
            {
            i1++;
            }
        int numThisLoop = i1 - i0;
        if (numThisLoop > 2)
            vu_loopFromDPoint3dArrayXYTol (graphP, pLoopPoints + i0, numThisLoop, 0.0, s_relTol);
        i0 = i1 + 1;
        }

    vu_postGraphToTrapFunc (graphP, "polyfill", numLoopPoints, (trapMask = trapMask << 1));

    vu_mergeOrUnionLoops (graphP, VUUNION_PARITY);

    vu_regularizeGraph (graphP);
    vu_postGraphToTrapFunc (graphP, "vu_regularizeGraph", numLoopPoints, (trapMask = trapMask << 1));
    //vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_parityFloodFromNegativeAreaFaces (graphP, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_postGraphToTrapFunc (graphP, "vu_parityFloodFromNegativeAreaFaces", numLoopPoints, (trapMask = trapMask << 1));

    faceArrayP = vu_grabArray (graphP);

    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_postGraphToTrapFunc (graphP, "vu_triangulateMonotoneInteriorFaces", numLoopPoints, (trapMask = trapMask << 1));
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_postGraphToTrapFunc (graphP, "vu_flipTrianglesToImproveQuadraticAspectRatio", numLoopPoints, (trapMask = trapMask << 1));
    vu_removeEdgesToExpandConvexInteriorFaces (graphP);
    vu_postGraphToTrapFunc (graphP, "vu_removeEdgesToExpandConvexInteriorFaces", numLoopPoints, (trapMask = trapMask << 1));
    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    vu_arrayOpen (faceArrayP);

    for (;vu_arrayRead (faceArrayP, &faceP);)
        {
        facePoints.clear ();
        faceFlag.clear ();
        VU_FACE_LOOP (P, faceP)
            {
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, P);
            facePoints.push_back (xyz);
            faceFlag.push_back (vu_getMask (P, VU_BOUNDARY_EDGE) ? 1 : 0);
            }
        END_VU_FACE_LOOP (P, faceP)
        DPoint3d xyz0 = facePoints[0];
        int flag0 = faceFlag[0];
        facePoints.push_back (xyz0);
        faceFlag.push_back (flag0);
        polygonFunc (facePoints.data (), faceFlag.data (), (int)facePoints.size (), userDataP);
        }

    if (faceArrayP)
        vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);

    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
