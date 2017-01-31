/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/facetout.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/**
* Collect specified edges in GraphicsPointArray form.  Directly sequenced edges
* are chained (using vertex id to identify chaining opportunities)
* but no other searches are performed to enhance chaining.
* @param pFacetHeader   => source mesh
* @param pGraphicsPointHeader       <= collected edges
* @param pEdgeArray     => edges to collect
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointEdges
(
    const MTGFacets    *pFacetHeader,
    GraphicsPointArray              *pGraphicsPointHeader,
const EmbeddedIntArray  *pEdgeArray
)
    {
    int i;
    MTGNodeId  tailNodeId, headNodeId;
    int         tailVertexId, headVertexId, prevVertexId;
    int vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    DPoint3d tailCoordinates, headCoordinates;


    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);

    prevVertexId = -1;
    for (i = 0; jmdlEmbeddedIntArray_getInt (pEdgeArray, &tailNodeId, i); i++)
        {
        headNodeId = jmdlMTGGraph_getFSucc (pGraph, tailNodeId);

        if (    jmdlMTGGraph_getLabel (pGraph, &tailVertexId, tailNodeId, vertexLabelOffset)
            &&  jmdlMTGGraph_getLabel (pGraph, &headVertexId, headNodeId, vertexLabelOffset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &tailCoordinates, tailVertexId)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &headCoordinates, headVertexId)
            )
            {

            if (tailVertexId != prevVertexId)
                {
                jmdlGraphicsPointArray_markBreak (pGraphicsPointHeader);
                jmdlGraphicsPointArray_addDPoint3d (pGraphicsPointHeader, &tailCoordinates);
                }

            jmdlGraphicsPointArray_addDPoint3d (pGraphicsPointHeader, &headCoordinates);
            prevVertexId = headVertexId;
            }
        }

    jmdlGraphicsPointArray_markBreak (pGraphicsPointHeader);
    }


/**
* Collect faces as major-break-delimited loops.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectInteriorFacesToGPA
(
    const MTGFacets    *pFacets,
    GraphicsPointArray  *pGPA
)
    {
    EmbeddedIntArray *pFaceArray = jmdlEmbeddedIntArray_grab ();
    MTGNodeId seedNodeId;
    int i;
    const MTGGraph * pGraph = (&pFacets->graphHdr);
    DPoint3d xyz, xyz0;

    jmdlMTGGraph_collectAndNumberFaceLoops (pGraph, pFaceArray, NULL);
    for (i = 0; jmdlEmbeddedIntArray_getInt (pFaceArray, &seedNodeId, i); i++)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, MTG_EXTERIOR_MASK))
            {
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
                {
                jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, currNodeId);
                if (currNodeId == seedNodeId)
                    xyz0 = xyz;
                jmdlGraphicsPointArray_addDPoint3d (pGPA, &xyz);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
            jmdlGraphicsPointArray_addDPoint3d (pGPA, &xyz);
            jmdlGraphicsPointArray_markBreak (pGPA);
            jmdlGraphicsPointArray_markMajorBreak (pGPA);
            }
        }
    jmdlEmbeddedIntArray_drop (pFaceArray);
    }


/**
*
* Extract masked edges from facets to GraphicsPointArray.   Attempts to keep
* edges within a face in the same polyline.
*
* @param pFacetHeader    => source mesh
* @param pGraphicsPointHeader <= collected edges
* @param searchMask => search mask
* @param maskOn => target value of mask
*                    true collects masked edges.
*                    false collects unmasked edges.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointMaskedEdges
(
const MTGFacets    *pFacetHeader,
      GraphicsPointArray        *pGraphicsPointHeader,
      MTGMask       searchMask,
      bool          maskOn
)
    {
    MTGNodeId currNodeId, lastNodeId = MTG_NULL_NODEID;
    int vertexId;
    // We need a non-const graph pointer so we can use mask bits.
    MTGGraph * pGraph = (MTGGraph *)(&pFacetHeader->graphHdr);

    MTGMask visitMask;
    MTGMask bothMasks;
    MTGMask nodeMask;
    MTGMask nodeSearchMask;
    DPoint3d xyz;
    int vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);

    visitMask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    bothMasks = visitMask | searchMask;
    int chainLength;

    MTGARRAY_SET_LOOP (tailNodeId, pGraph)
        {
        for (currNodeId = tailNodeId, chainLength = 0;; currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId))
            {
            nodeMask = jmdlMTGGraph_getMask (pGraph, currNodeId, bothMasks);
            nodeSearchMask = searchMask & nodeMask;
            jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
            if (     (nodeMask & visitMask)
                ||  !((maskOn && nodeSearchMask) || (!maskOn && !nodeSearchMask))
                || !jmdlMTGGraph_getLabel (pGraph, &vertexId, currNodeId, vertexLabelOffset)
                || SUCCESS != jmdlVArrayDPoint3d_getDPoint3d (pVertexArray, &xyz, vertexId)
                )
                break;
            jmdlGraphicsPointArray_addDPoint3d (pGraphicsPointHeader, &xyz);
            lastNodeId = currNodeId;
            chainLength++;
            }

        if (chainLength > 0)
            {
            MTGNodeId finalNodeId = jmdlMTGGraph_getFSucc (pGraph, lastNodeId);
            if (
                   jmdlMTGGraph_getLabel (pGraph, &vertexId, finalNodeId, vertexLabelOffset)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &xyz, vertexId)
               )
                {
                jmdlGraphicsPointArray_addDPoint3d (pGraphicsPointHeader, &xyz);
                }
            jmdlGraphicsPointArray_markBreak (pGraphicsPointHeader);
            }
        }
    MTGARRAY_END_SET_LOOP (tailNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, bothMasks);
    }


/**
* Output selected edges from a mesh.  Each edge is identified by
* its start node.   Edges marked exterior are skipped.
* @param pFacetHeader    => mesh to emit
* @param pEdgeStartArray => start nodes of selected edges.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectGraphicsPointEdgeStrokes
(
const MTGFacets    *pFacetHeader,
      GraphicsPointArray        *pGraphicsPointHeader,
      EmbeddedIntArray    *pEdgeStartArray
)
    {
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pPointArray = (&pFacetHeader->vertexArrayHdr);
    int offset = pFacetHeader->vertexLabelOffset;
    int vertexIndex[2];
    DPoint3d vertexCoordinates[2];
    MTGNodeId startNodeId;
    MTGNodeId endNodeId;
    int nNode = jmdlEmbeddedIntArray_getCount (pEdgeStartArray);
    int i;


    for (i = 0; i < nNode; i++)
        {
        jmdlVArrayInt_getInt (pEdgeStartArray, &startNodeId, i);
        endNodeId = jmdlMTGGraph_getEdgeMate (pGraph, startNodeId);
        if (   (    !jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK)
                ||  !jmdlMTGGraph_getMask (pGraph, endNodeId, MTG_EXTERIOR_MASK)
               )
            && jmdlMTGGraph_getLabel (pGraph, &vertexIndex[0], startNodeId, offset)
            && jmdlMTGGraph_getLabel (pGraph, &vertexIndex[1], endNodeId, offset)
            && 2 == jmdlVArrayDPoint3d_getIndexedArray (pPointArray, vertexCoordinates, 2, vertexIndex, 2)
            )
            {
            jmdlGraphicsPointArray_addDPoint3dArray (pGraphicsPointHeader, vertexCoordinates, 2);
            jmdlGraphicsPointArray_markBreak (pGraphicsPointHeader);
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
