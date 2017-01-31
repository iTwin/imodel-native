/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_facetmemory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* @dllName mtg */


/**
* Compress this instance by deleting obsolete/unused nodes and the corresponding
* coordinates/normals, and by dropping the last numLabelsToDrop labels of each
* remaining node.
* <P>
* The facets coordinate array is compressed if vertex data is present;
* otherwise, its allocated memory is released.
* <P>
* If a bool    parameter is false then the corresponding facets array(s) will
* be ignored, allowing for external custom compression by the caller.  Note that
* after this is done, any preexisting parallelism to the facets coordinate array
* will have been lost.
* <P>
* If a bool    parameter is true then the normal mode (IVertexFormat) of this
* instance determines the action on the corresponding facets array(s):
* <UL>
* <LI> <EM>No vertex data:</EM> release allocated memory
* <LI> <EM>Vertex coordinate data only:</EM>
*      <UL>
*      <LI> compressNormalArray is true: release allocated memory of normal
*           array
*      <LI> compressParamArrays is true: compress param arrays parallel to
*           vertex array
*      </UL>
* <LI> <EM>Normal per vertex:</EM> compress parallel to vertex array
* <LI> <EM>Separate normals:</EM>
*      <UL>
*      <LI> compressNormalArray is true: compress normal array independently
*      <LI> compressParamArrays is true: if compressNormalArray is also true,
*           then compress param arrays parallel to normal array; otherwise,
*           ignore param arrays
*      </UL>
* </UL>
* Warning: external references to or undropped labels that store MTG node IDs
* will no longer be valid.
*
* @param    pFacets             <=> on output: compressed facets
* @param    obsoleteNodeMask     => mask of obsoleted nodes
* @param    numLabelsToDrop      => #labels to drop from end of each label block
* @param    compressNormalArray  => compress or release normalArray iff true
* @param    compressParamArrays  => compress paramArrays iff true
* @return false iff error (i.e., too many labels dropped for normal mode)
* @see MTGGraph#compress
* @see MTGFacets.IVertexFormat
* @bsimethod                                    DavidAssaf      2/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_compress
(
MTGFacets       *pFacets,
MTGMask         obsoleteNodeMask,
int             numLabelsToDrop,
bool            compressNormalArray,
bool            compressParamArrays
)
    {
    MTGGraph            *pGraph;        /* graph header */
    EmbeddedIntArray            *pVLabelMapHdr; /* maps old vertex label to new */
    EmbeddedIntArray            *pNLabelMapHdr; /* maps old normal label to new */
    EmbeddedDPoint3dArray     *pVArrayHdr;    /* new vertex array header */
    EmbeddedDPoint3dArray     *pNArrayHdr;    /* new normal array header */
    EmbeddedDPoint3dArray     *pP1ArrayHdr;   /* new param1 array header */
    EmbeddedDPoint3dArray     *pP2ArrayHdr;   /* new param2 array header */
    EmbeddedDPoint3dArray     *pOldVHdr;      /* old vertex array header */
    EmbeddedDPoint3dArray     *pOldNHdr;      /* old normal array header */
    EmbeddedDPoint3dArray     *pOldP1Hdr;     /* old param1 array header */
    EmbeddedDPoint3dArray     *pOldP2Hdr;     /* old param2 array header */
    DPoint3d            point;          /* element of vert/norm/param arrays */
    int                 *pVLabelMapBase;    /* base of vertex label map */
    int                 *pNLabelMapBase;    /* base of normal label map */
    int                 reservedLabels; /* # labels reserved for coords/normals */
    int                 numNodes;       /* # (active) MTG nodes */
    int                 numVerts;       /* # compressed vertices */
    int                 numOldVerts;    /* # old vertices */
    int                 numNorms;       /* # compressed normals */
    int                 numOldNorms;    /* # old normals */
    int                 vertexOffset;   /* offset to vertex label */
    int                 normalOffset;   /* offset to normal label */
    int                 label;          /* old index into a facets array */
    int                 index;          /* new index of point in a facets array */
    bool                noNormals = false;  /* distinguish unitary reservedLabels */
    int                 i;

    /* ptr parameter error trap */
    if (!pFacets)
        return false;
    else
        {
        pGraph = &pFacets->graphHdr;
        vertexOffset = pFacets->vertexLabelOffset;
        normalOffset = pFacets->normalLabelOffset;
        pOldVHdr = &pFacets->vertexArrayHdr;
        pOldNHdr = &pFacets->normalArrayHdr;
        pOldP1Hdr = &pFacets->param1ArrayHdr;
        pOldP2Hdr = &pFacets->param2ArrayHdr;
        }

    /* how many labels are reserved by normalMode? */
    switch (pFacets->normalMode)
        {
        case MTG_Facets_NoData:
            reservedLabels = 0;
            break;
        case MTG_Facets_VertexOnly:
            noNormals = true;
            reservedLabels = 1;
            break;
        case MTG_Facets_NormalPerVertex:
            reservedLabels = 1;
            break;
        case MTG_Facets_SeparateNormals:
            reservedLabels = 2;
            break;
        default:
            return false;   /* normalMode invalid */
            break;
        }

    /* quantity error trap */
    if  (
        numLabelsToDrop < 0 ||
        numLabelsToDrop + reservedLabels > jmdlMTGGraph_getLabelCount (pGraph)
        )
        return false;

    /* compress the MTG node list, preserving vertex (and normal) labels */
    if (!jmdlMTGGraph_compress (pGraph, obsoleteNodeMask, numLabelsToDrop))
        return false;

    /* ------ At this point, all MTG nodes are active! ------ */

    /* nothing left to do if no vertices/normals */
    if (!reservedLabels)
        {
        jmdlVArrayDPoint3d_releaseMem (pOldVHdr);
        if (compressNormalArray)
            jmdlVArrayDPoint3d_releaseMem (pOldNHdr);
        if (compressParamArrays)
            {
            jmdlVArrayDPoint3d_releaseMem (pOldP1Hdr);
            jmdlVArrayDPoint3d_releaseMem (pOldP2Hdr);
            }
        return true;
        }

    /* init vertex label map large enough to hold all vertices */
    numNodes = jmdlMTGGraph_getNodeCount (pGraph);
    numOldVerts = jmdlEmbeddedDPoint3dArray_getCount (pOldVHdr);
    pVLabelMapHdr = jmdlEmbeddedIntArray_grab ();
    jmdlVArrayInt_setConstant (pVLabelMapHdr, MTG_NULL_NODEID, numOldVerts);
    pVLabelMapBase = jmdlEmbeddedIntArray_getPtr (pVLabelMapHdr, 0);

    /* repoint/count vertex labels and fill vertex label map */
    for (i = numVerts = 0; i < numNodes; i++)
        {
        jmdlMTGGraph_getLabel (pGraph, &label, i, vertexOffset);
        if ((index = pVLabelMapBase[label]) == MTG_NULL_NODEID)
            index = pVLabelMapBase[label] = numVerts++;
        jmdlMTGGraph_setLabel (pGraph, i, vertexOffset, index);
        }

    /* only create normal label map if compressing 'em separately */
    if (reservedLabels == 2 && compressNormalArray)
        {
        /* init normal label map large enough to hold all normals */
        numOldNorms = jmdlEmbeddedDPoint3dArray_getCount (pOldNHdr);
        pNLabelMapHdr = jmdlEmbeddedIntArray_grab ();
        jmdlVArrayInt_setConstant (pNLabelMapHdr, MTG_NULL_NODEID, numOldNorms);
        pNLabelMapBase = jmdlEmbeddedIntArray_getPtr (pNLabelMapHdr, 0);

        /* repoint/count normal labels and fill normal label map */
        for (i = numNorms = 0; i < numNodes; i++)
            {
            jmdlMTGGraph_getLabel (pGraph, &label, i, normalOffset);
            if ((index = pNLabelMapBase[label]) == MTG_NULL_NODEID)
                index = pNLabelMapBase[label] = numNorms++;
            jmdlMTGGraph_setLabel (pGraph, i, normalOffset, index);
            }
        }

    /* init new vertex array of just the right size */
    pVArrayHdr = jmdlVArrayDPoint3d_new ();
    jmdlVArrayDPoint3d_setExactBufferSize (pVArrayHdr, numVerts);

    /* init new normal array of just the right size */
    if (!noNormals && compressNormalArray)
        {
        if (reservedLabels == 1)
            numNorms = numVerts;    /* norm || vert if normalPerVertex */

        pNArrayHdr = jmdlVArrayDPoint3d_new ();
        jmdlVArrayDPoint3d_setExactBufferSize (pNArrayHdr, numNorms);
        }

    /*
    Init new param arrays of just the right size if compressing param arrays,
    but only if the parallel array (vertex/normal) is also being compressed.
    */
    if (compressParamArrays)
        {
        if (reservedLabels == 1)
            {
            /* param arrays || vertex array, which is always compressed */
            pP1ArrayHdr = jmdlVArrayDPoint3d_new ();
            pP2ArrayHdr = jmdlVArrayDPoint3d_new ();
            jmdlVArrayDPoint3d_setExactBufferSize (pP1ArrayHdr, numVerts);
            jmdlVArrayDPoint3d_setExactBufferSize (pP2ArrayHdr, numVerts);
            }

        else if (compressNormalArray)
            {
            /* param arrays || normal array, which is not always compressed */
            pP1ArrayHdr = jmdlVArrayDPoint3d_new ();
            pP2ArrayHdr = jmdlVArrayDPoint3d_new ();
            jmdlVArrayDPoint3d_setExactBufferSize (pP1ArrayHdr, numNorms);
            jmdlVArrayDPoint3d_setExactBufferSize (pP2ArrayHdr, numNorms);
            }
        }

    /* fill new vertex array and other parallel array(s) */
    for (i = 0; i < numOldVerts; i++)
        if ((index = pVLabelMapBase[i]) != MTG_NULL_NODEID)
            {
            /* set vertex in new location */
            jmdlVArrayDPoint3d_getDPoint3d (pOldVHdr, &point, i);
            jmdlVArrayDPoint3d_setDPoint3d (pVArrayHdr, &point, index);

            if (reservedLabels == 1)
                {
                if (!noNormals && compressNormalArray)
                    {
                    /* set normal in same location */
                    jmdlVArrayDPoint3d_getDPoint3d (pOldNHdr, &point, i);
                    jmdlVArrayDPoint3d_setDPoint3d (pNArrayHdr, &point, index);
                    }

                if (compressParamArrays)
                    {
                    /* set param1 in same location */
                    jmdlVArrayDPoint3d_getDPoint3d (pOldP1Hdr, &point, i);
                    jmdlVArrayDPoint3d_setDPoint3d (pP1ArrayHdr, &point, index);

                    /* set param2 in same location */
                    jmdlVArrayDPoint3d_getDPoint3d (pOldP2Hdr, &point, i);
                    jmdlVArrayDPoint3d_setDPoint3d (pP2ArrayHdr, &point, index);
                    }
                }
            }

    /* fill new normal array separately, and param arrays, if parallel */
    if (reservedLabels == 2 && compressNormalArray)
        {
        for (i = 0; i < numOldNorms; i++)
            if ((index = pNLabelMapBase[i]) != MTG_NULL_NODEID)
                {
                /* set normal in new location */
                jmdlVArrayDPoint3d_getDPoint3d (pOldNHdr, &point, i);
                jmdlVArrayDPoint3d_setDPoint3d (pNArrayHdr, &point, index);

                if (compressParamArrays)
                    {
                    /* set param1 in same location */
                    jmdlVArrayDPoint3d_getDPoint3d (pOldP1Hdr, &point, i);
                    jmdlVArrayDPoint3d_setDPoint3d (pP1ArrayHdr, &point, index);

                    /* set param2 in same location */
                    jmdlVArrayDPoint3d_getDPoint3d (pOldP2Hdr, &point, i);
                    jmdlVArrayDPoint3d_setDPoint3d (pP2ArrayHdr, &point, index);
                    }
                }
        }

    /* replace param arrays */
    if (compressParamArrays && (reservedLabels == 1 || compressNormalArray))
        {
        jmdlVArrayDPoint3d_releaseMem (pOldP2Hdr);
        jmdlVArrayDPoint3d_releaseMem (pOldP1Hdr);
        pFacets->param2ArrayHdr = *pP2ArrayHdr;
        pFacets->param1ArrayHdr = *pP1ArrayHdr;
        }

    /* replace/release normal array */
    if (compressNormalArray)
        {
        jmdlVArrayDPoint3d_releaseMem (pOldNHdr);
        if (!noNormals)
            pFacets->normalArrayHdr = *pNArrayHdr;
        }

    /* replace vertex array */
    jmdlVArrayDPoint3d_releaseMem (pOldVHdr);
    pFacets->vertexArrayHdr = *pVArrayHdr;

    /* drop the label maps */
    if (reservedLabels == 2 && compressNormalArray)
        jmdlEmbeddedIntArray_drop (pNLabelMapHdr);
    jmdlEmbeddedIntArray_drop (pVLabelMapHdr);

    return true;
    }END_BENTLEY_GEOMETRY_NAMESPACE
