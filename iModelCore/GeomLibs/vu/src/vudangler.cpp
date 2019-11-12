/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static int s_numSmallFace = 0;
static int s_numDangler = 0;

/*----------------------------------------------------------------------+
* Test if a vertex has a single unmasked edge.  If so return it; otherwise
*   return null.
+----------------------------------------------------------------------*/
static VuP findSingletonUnmasked
(
VuP             pSeed,
VuMask          mask
)
    {
    VuP pResult = NULL;
    //int n = 0;
    VU_VERTEX_LOOP (pCurr, pSeed)
        {
        if (!vu_getMask( pCurr, mask))
            {
            if (pResult)
                return NULL;
            pResult = pCurr;
            }
        }
    END_VU_VERTEX_LOOP (pCurr, pSeed)
    return pResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void setDoubleMaskAroundVertex
(
VuP pSeed,
VuMask mask
)
    {
    VuP pMate;
    VU_VERTEX_LOOP (pCurr, pSeed)
        {
        pMate = vu_edgeMate (pCurr);
        vu_setMask (pCurr, mask);
        vu_setMask (pMate, mask);
        }
    END_VU_VERTEX_LOOP (pCurr, pSeed)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void spreadDanglerMask
(
VuP             pSeed,
VuMask          combinedMask,
VuMask          danglerMask
)
    {
    VuP pDangler;
    while (pSeed)
        {
        pDangler = findSingletonUnmasked (pSeed, combinedMask);
        if (!pDangler)
            return;
        setDoubleMaskAroundVertex (pSeed, danglerMask);
        pSeed = vu_fsucc (pDangler);
        s_numDangler++;
        }
    }

/*---------------------------------------------------------------------*//**
* @description Set a mask on edges that dangle into faces.
* @param pGraph         IN OUT  vu graph
* @param nullFaceMask   IN      optional mask preset on faces that are to be ignored in determining if an edge dangles
*                               (e.g., as set by ~mvu_markSmallFaces with numEdge = 2)
* @param danglerMask    IN      mask to set on both sides of dangling edges
* @group "VU Edges"
* @see vu_deleteDanglingEdges, vu_markSmallFaces
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markDanglingEdges
(
VuSetP  pGraph,
VuMask  nullFaceMask,
VuMask  danglerMask
)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    VuMask combinedMask = danglerMask | nullFaceMask;
    vu_clearMaskInSet (pGraph, danglerMask | visitMask);
    VU_SET_LOOP (pCandidate, pGraph)
        {
        if (!vu_getMask (pCandidate, visitMask))
            {
            vu_setMaskAroundVertex (pCandidate, visitMask);
            spreadDanglerMask (pCandidate, combinedMask, danglerMask);
            }
        }
    END_VU_SET_LOOP (pCandidate, pGraph)

    vu_returnMask (pGraph, visitMask);
    }

/*---------------------------------------------------------------------*//**
* @description Set a mask on faces with numEdge or fewer edges.
* @param pGraph     IN OUT  vu graph
* @param numEdge    IN      maximum number of edges for a marked face
* @param mask       IN      mask to set
* @group "VU Edges"
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markSmallFaces
(
VuSetP  pGraph,
int     numEdge,
VuMask  mask
)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask | mask);
    VU_SET_LOOP (pCandidate, pGraph)
        {
        if (!vu_getMask (pCandidate, visitMask))
            {
            vu_setMaskAroundFace (pCandidate, visitMask);
            if (vu_countEdgesAroundFace (pCandidate) <= numEdge)
                {
                vu_setMaskAroundFace (pCandidate, mask);
                s_numSmallFace++;
            }
            }
        }
    END_VU_SET_LOOP (pCandidate, pGraph)

    vu_returnMask (pGraph, visitMask);
    }

/*---------------------------------------------------------------------*//**
* @description Delete all edges that dangle into faces of the graph.
* @param pGraph     IN OUT  vu graph
* @group "VU Edges"
* @see vu_markDanglingEdges
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_deleteDanglingEdges
(
VuSetP  pGraph
)
    {
    VuMask nullFaceMask = vu_grabMask (pGraph);
    VuMask danglerMask  = vu_grabMask (pGraph);
    s_numSmallFace = 0;
    s_numDangler = 0;
#ifdef DoPrint
    if (s_noisy > 1)
        vu_printFaceLabels (pGraph, "before danglers..");
#endif

    vu_markSmallFaces (pGraph, 2, nullFaceMask);
    vu_markDanglingEdges (pGraph, nullFaceMask, danglerMask);
    vu_freeMarkedEdges (pGraph, danglerMask);
#ifdef DoPrint
    if (s_noisy)
        printf  (" (numSmallFace %d) (numDangler %d)\n",
                            s_numSmallFace,
                            s_numDangler);
    if (s_noisy > 1)
        vu_printFaceLabels (pGraph, "after danglers..");
#endif
    vu_returnMask (pGraph, nullFaceMask);
    vu_returnMask (pGraph, danglerMask);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
