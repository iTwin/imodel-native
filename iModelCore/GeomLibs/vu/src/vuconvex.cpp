/*----------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_graphAbsTol = 1.0e-10;
static double s_graphRelTol = 1.0e-10;
#define MAX_VERTICES 5000

/*---------------------------------------------------------------------------------**//**
* @description Split a polygon into smaller convex polygons.
* (With multiple input args -- called with one-or-the-other)
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_splitToConvexParts
(
bvector<DPoint3d>  const *singleLoop,
bvector<bvector<DPoint3d>>  const *loops,
uint32_t numWrap,
bvector<bvector<DPoint3d>>  &convexFaces,
bvector<bvector<bool>> *isBoundary
)
    {
    VuSetP      graphP;
    VuArrayP    faceArrayP;
    VuP         faceP;

    static double s_relTol = 1.0e-13;
    convexFaces.clear ();
    if (nullptr != isBoundary)
        isBoundary->clear ();

    graphP = vu_newVuSet (0);

    vu_setTol (graphP, s_graphAbsTol, s_graphRelTol);
    if (nullptr != loops)
        for (auto &loop : *loops)
            {
            vu_loopFromDPoint3dArrayXYTol (graphP, loop.data (), (int)loop. size (), 0.0, s_relTol);
            }
    if (nullptr != singleLoop)
        vu_loopFromDPoint3dArrayXYTol (graphP, singleLoop->data (), (int)singleLoop->size (), 0.0, s_relTol);

    vu_mergeOrUnionLoops (graphP, VUUNION_PARITY);

    vu_regularizeGraph (graphP);
    //vu_markAlternatingExteriorBoundaries(graphP,true);
    vu_parityFloodFromNegativeAreaFaces (graphP, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);

    faceArrayP = vu_grabArray (graphP);

    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_removeEdgesToExpandConvexInteriorFaces (graphP);
    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    vu_arrayOpen (faceArrayP);

    for (;vu_arrayRead (faceArrayP, &faceP);)
        {
        convexFaces.push_back (bvector<DPoint3d>());
        if (nullptr != isBoundary)
            isBoundary->push_back (bvector<bool>());
        VU_FACE_LOOP (P, faceP)
            {
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, P);
            convexFaces.back ().push_back (xyz);
            if (nullptr != isBoundary)
                isBoundary->back ().push_back (
                        vu_getMask (P, VU_BOUNDARY_EDGE) ? true : false
                        );
            }
        END_VU_FACE_LOOP (P, faceP)

        for (uint32_t i = 0; i < numWrap; i++)
            {
            DPoint3d xyz = convexFaces.back ()[i];
            convexFaces.back ().push_back (xyz);
            if (nullptr != isBoundary)
                {
                bool value = isBoundary->back ()[i];
                isBoundary->back ().push_back (value);
                }
            }
        }

    if (faceArrayP)
        vu_returnArray (graphP, faceArrayP);
    vu_freeVuSet (graphP);

    }

Public GEOMDLLIMPEXP void vu_splitToConvexParts
(
bvector<bvector<DPoint3d>>  const &loops,
uint32_t numWrap,
bvector<bvector<DPoint3d>>  &convexFaces,
bvector<bvector<bool>> *isBoundary
)
    {
    return vu_splitToConvexParts (nullptr, &loops, numWrap, convexFaces, isBoundary);
    }
Public GEOMDLLIMPEXP void vu_splitToConvexParts
(
bvector<DPoint3d>  const &loop,
uint32_t numWrap,
bvector<bvector<DPoint3d>>  &convexFaces,
bvector<bvector<bool>> *isBoundary
)
    {
    return vu_splitToConvexParts (&loop, nullptr, numWrap, convexFaces, isBoundary);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
