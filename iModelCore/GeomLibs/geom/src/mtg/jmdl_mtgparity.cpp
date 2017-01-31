/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgparity.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/

typedef struct _paritySearchContext
    {
    MTGGraph                *pGraph;            /* containing graph */
    EmbeddedDPoint3dArray           *pCoordinates;      /* xyz coordinates of points. */
    EmbeddedIntArray                *pStack;            /* stack of unexplored nodes */
    int                     vertexLabelOffset;  /* offset to access coordinate data */
    MTGMask                 visitMask;          /* mask to test and apply for visited nodes */
    MTGMask                 parityChangeMask;   /* mask for edges where parity changes. */
    MTGMask                 exteriorMask;       /* mask to set on exterior edges */
    } ParityContext;


/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGParity_initContext
(
ParityContext   *pContext,
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         parityChangeMask,
MTGMask         exteriorMask
)
    {
    pContext->pGraph = pGraph;
    pContext->vertexLabelOffset = vertexLabelOffset;
    pContext->pCoordinates = pCoordinates;
    pContext->parityChangeMask  = parityChangeMask;
    pContext->exteriorMask      = exteriorMask;
    pContext->visitMask = jmdlMTGGraph_grabMask (pContext->pGraph);
    pContext->pStack    = jmdlEmbeddedIntArray_grab ();
    jmdlMTGGraph_clearMaskInSet (pContext->pGraph,
                    pContext->visitMask | pContext->exteriorMask);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGParity_closeContext
(
ParityContext   *pContext
)
    {
    jmdlEmbeddedIntArray_drop (pContext->pStack);
    jmdlMTGGraph_dropMask (pContext->pGraph, pContext->visitMask);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGParity_getNodeXY
(
ParityContext   *pContext,
DPoint2d        *pXY,
MTGNodeId       nodeId
)
    {
    int vertexIndex;
    DPoint3d xyz;
    xyz.Zero ();
    bool    boolstat =
           jmdlMTGGraph_getLabel
                        (
                        pContext->pGraph,
                        &vertexIndex,
                        nodeId,
                        pContext->vertexLabelOffset
                        )
        && jmdlEmbeddedDPoint3dArray_getDPoint3d
                        (
                        pContext->pCoordinates,
                        &xyz,
                        vertexIndex
                        );

    if (boolstat)
        {
        pXY->x = xyz.x;
        pXY->y = xyz.y;
        }

    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       jmdlMTGParity_faceProperties
(
ParityContext   *pContext,
double          *pArea,
double          *pSumLengthSquared,
MTGNodeId       startNodeId
)
    {
    DPoint2d vector0, vector1, vector01;
    DPoint2d basePoint, point0, point1;
    MTGNodeId nodeId0, nodeId1;
    double area, sumL2;

    jmdlMTGParity_getNodeXY (pContext, &basePoint, startNodeId);
    nodeId0 = jmdlMTGGraph_getFSucc (pContext->pGraph, startNodeId);
    jmdlMTGParity_getNodeXY (pContext, &point0, nodeId0);
    vector0.DifferenceOf (point0, basePoint);

    for (   area = 0.0, sumL2 = 0.0;
            startNodeId !=
                (nodeId1 = jmdlMTGGraph_getFSucc (pContext->pGraph, nodeId0));
            nodeId0 = nodeId1,
            vector0 = vector1,
            point0 = point1
            )
        {
        jmdlMTGParity_getNodeXY (pContext, &point1, nodeId1);
        vector1.DifferenceOf (point1, basePoint);
        vector01.DifferenceOf (point1, point0);

        area += vector0.CrossProduct (vector01);
        sumL2 += vector01.DotProduct (vector01);
        }
    if (pArea)
        *pArea = area;
    if (pSumLengthSquared)
        *pSumLengthSquared = sumL2;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static int     jmdlMTGParity_faceOrientation
(
ParityContext   *pContext,
MTGNodeId       startNodeId
)
    {
    double area, L2;
    int orientation = 0;
    static double s_relTol = 1.0e-10;

    if (jmdlMTGParity_faceProperties (pContext, &area, &L2, startNodeId))
        {
        if (fabs (area) > s_relTol * orientation)
            orientation = area > 0.0 ? 1 : -1;
        }

    return orientation;
    }

/*---------------------------------------------------------------------------------**//**
* Seed and search a component.
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGParity_searchComponent
(
ParityContext   *pContext,
MTGNodeId       seedNodeId,
bool            currStateIsExterior
)
    {
    MTGNodeId startNodeId;
    MTGNodeId farNodeId;
    MTGGraph *pGraph = pContext->pGraph;
    EmbeddedIntArray  *pStack = pContext->pStack;
    int     faceState;
    MTGMask faceMask;
    MTGMask queryMask = pContext->visitMask | pContext->parityChangeMask;
    MTGMask nearMask, farMask;
    bool    changeParity;
    /* seed the search ... */
    jmdlVArrayInt_add2Int (pStack, seedNodeId, currStateIsExterior);

    while (   boolean_jmdlVArrayInt_pop (pStack, &faceState)
           && boolean_jmdlVArrayInt_pop (pStack, &startNodeId))
        {
        if (!jmdlMTGGraph_getMask (pGraph, startNodeId, pContext->visitMask))
            {
            faceMask = pContext->visitMask;
            if (faceState)
                faceMask |= pContext->exteriorMask;
            jmdlMTGGraph_setMaskAroundFace (pGraph, startNodeId, faceMask);
            MTGARRAY_FACE_LOOP (nearNodeId, pGraph, startNodeId)
                {
                farNodeId = jmdlMTGGraph_getEdgeMate (pGraph, nearNodeId);
                farMask = jmdlMTGGraph_getMask (pGraph, farNodeId, queryMask);
                nearMask = jmdlMTGGraph_getMask (pGraph, nearNodeId, pContext->parityChangeMask);
                if (!(farMask  & pContext->visitMask))
                    {
                    changeParity = 0 != (nearMask | (farMask & pContext->parityChangeMask));
                    jmdlVArrayInt_add2Int (pStack, farNodeId, changeParity ? !faceState : faceState);
                    }
                }
            MTGARRAY_END_FACE_LOOP (nearNodeId, pGraph, startNodeId)
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGParity_searchAll
(
ParityContext   *pContext
)
    {
    MTGARRAY_SET_LOOP (seedNodeId, pContext->pGraph)
        {
        if (!jmdlMTGGraph_getMask (pContext->pGraph, seedNodeId, pContext->visitMask)
            && jmdlMTGParity_faceOrientation (pContext, seedNodeId) < 0)
            {
            jmdlMTGParity_searchComponent (pContext, seedNodeId, true);
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pContxt->pGraph)
    }




/*---------------------------------------------------------------------------------**//**
* Use UV area to identify true exterior faces; start recursive traversal
* from each negative area face to set exterior masks according to parity
* rules as boundary edges are crossed.
* @bsimethod                                                    EarlinLutz      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGParity_markFaceParityFromNegativeAreaFaces
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         parityChangeMask,
MTGMask         exteriorMask
)
    {
    ParityContext context;
    jmdlMTGParity_initContext (&context,
                            pGraph,
                            vertexLabelOffset,
                            pCoordinates,
                            parityChangeMask,
                            exteriorMask
                            );
#ifdef COMPILE_PRINTF
    if (s_noisy)
        jmdlMTGGraph_printFaceLoops (pGraph);
#endif
    jmdlMTGParity_searchAll (&context);

#ifdef COMPILE_PRINTF
    if (s_noisy)
        jmdlMTGGraph_printFaceLoops (pGraph);
#endif
    jmdlMTGParity_closeContext (&context);
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
