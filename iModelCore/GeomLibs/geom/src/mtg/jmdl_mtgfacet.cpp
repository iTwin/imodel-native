/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


template<typename T>
static int PushedId (bvector<T> &data, T const &value)
    {
    size_t id = data.size ();
    data.push_back(value);
    return (int)id;
    }


/*----------------------------------------------------------------------+
|TITLE FacetMemoryManagement Facet Header Memory Management             |
| The MTGFacets structure is a header for the connectivity and          |
| coordinate data in a set of facets.                                   |
|                                                                       |
| Application software refers to the structure                          |
| via a pointer of type MTGFacets *.  This is declared const MTGFacets *|
| by query functions.                                                   |
|                                                                       |
| Facet headers that are to be retained for long term use (e.g. as part |
| of a persistent OMS_Facets object) are allocated and freed by         |
| omdlFacets_new and omdlFacets_free.                                   |
|                                                                       |
| When a facet header is only needed temporarily, it is more efficent   |
| to borrow and return a header from a cache accessed by                |
| omdlFacets_grab and omdlFacets_drop.                                  |
|                                                                       |
| omdlFacets_empty deletes the connectivity and coordinate data in a    |
| facet set but retains its memory for possible reuse when the facet    |
| set is repopulated.   omdlFacets_releaseMem empties the facet set     |
| and also the associated memory.                                       |
+----------------------------------------------------------------------*/

/**
* Allocate a facet header to the caller.  If possible, the header
* is taken from a cache of headers that were previously used and hence
* may have preallocated memory for connectivity and coordinate data.
* Use jmdlMTGFacets_drop to return the header to the cache.
* @param void
* @see
* @return pointer to a borrowed facet header
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacets * jmdlMTGFacets_grab
(
void
)
    {
    return jmdlMTGFacets_new ();
    }


/**
* Allocate a facet header from the system heap.
* @param void
* @see
* @return pointer to newly allocated and initialized facet header structure.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacetsP jmdlMTGFacets_new
(
void
)
    {
    MTGFacets * pFacetHeader = new MTGFacets ();
    return pFacetHeader;

    }
_MTGFacets::_MTGFacets ()
    {
    normalMode = MTG_Facets_NoData;
    vertexLabelOffset = -1;
    normalLabelOffset = -1;
    }

_MTGFacets::_MTGFacets (MTGFacets_NormalMode mode)
    {
    normalMode = MTG_Facets_NoData;
    vertexLabelOffset = -1;
    normalLabelOffset = -1;
    SetNormalMode (mode);
    }


_MTGFacets::~_MTGFacets ()
    {
    }

/**
* Initialize a facet header structure.   Prior contents ignored and destroyed.
* @param pFacetHeader <= initialized structure.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_init
(
MTGFacets *pFacetHeader
)
    {


    }



/**
* Return a facet header to the system heap.
* @param     => Header for facets to free
* @see
* @return MTGFacets *
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacetsP jmdlMTGFacets_free
(
MTGFacetsP      pFacetHeader
)
    {
    if (pFacetHeader)
        {
        jmdlMTGFacets_releaseMem (pFacetHeader);
        delete pFacetHeader;
        }
    return NULL;
    }


/**
* Return the associated memory (but not the facet header itself) to
* the sytem heap.
* @param pFacetHeader    => header for facets to free.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_releaseMem
(
MTGFacets *     pFacetHeader
)
    {
    if (pFacetHeader)
        {
        bvector<DPoint3d>().swap (pFacetHeader->vertexArrayHdr);
        bvector<DPoint3d>().swap (pFacetHeader->normalArrayHdr);
        bvector<DPoint3d>().swap (pFacetHeader->param1ArrayHdr);
        bvector<DPoint3d>().swap (pFacetHeader->param2ArrayHdr);
        jmdlMTGGraph_releaseMem ((&pFacetHeader->graphHdr));
        }
    }


/**
* Set all counts (nodes, vertices, normals, parameter values) in a
* facet set to 0, but retain associated memory for possible reuse
* if the facet set is repopulated.
* @param    pFacetHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_empty
(
MTGFacets *     pFacetHeader
)
    {
    pFacetHeader->vertexArrayHdr.clear ();
    pFacetHeader->normalArrayHdr.clear ();
    pFacetHeader->param1ArrayHdr.clear ();
    pFacetHeader->param2ArrayHdr.clear ();
    jmdlMTGGraph_emptyNodes ((&pFacetHeader->graphHdr), false);
    pFacetHeader->normalMode = MTG_Facets_NoData;
    }


/**
* Return a facet header to the facet cache.
* @param  pFacetHeader   => header to return
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_drop
(
MTGFacets *     pFacetHeader        // => header to return
)
    {
    jmdlMTGFacets_free (pFacetHeader);
    }


/**
* Call this immediately after grab, new, empty, or releaseMem to
* indicate the type of geometry data that will be stored with the
* facets.
* @param pFacetHeader    <=> Header whose normal mode is to be set.
* @param normalMode => Indicates how normal data is to be constructed.
* @param numVertex => approximate number of vertices.  May be zero.
* @param numNormal => approximate number of normals.  May be zero.
*                       Only applied if normals are separate from vertices.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_setNormalMode
(
MTGFacets *         pFacetHeader,
MTGFacets_NormalMode normalMode,
int                 numVertex,
int                 numNormal
)
    {
    pFacetHeader->SetNormalMode (normalMode, numVertex, numNormal);
    }
void _MTGFacets::SetNormalMode (MTGFacets_NormalMode _normalMode, int numVertex, int numNormal)
    {
    normalMode = _normalMode;
    switch (normalMode)
        {
        case MTG_Facets_VertexOnly:
            vertexLabelOffset = jmdlMTGGraph_defineLabel ((&graphHdr), -1, MTG_LabelMask_VertexProperty, -1);
            normalLabelOffset = -1;
            vertexArrayHdr.reserve (numVertex);
            break;
        case MTG_Facets_NormalPerVertex:
            vertexLabelOffset = jmdlMTGGraph_defineLabel ((&graphHdr), -1, MTG_LabelMask_VertexProperty, -1);
            normalLabelOffset = vertexLabelOffset;
            vertexArrayHdr.reserve (numVertex);
            normalArrayHdr.reserve (numVertex);
            break;
        case MTG_Facets_SeparateNormals:
            vertexLabelOffset = jmdlMTGGraph_defineLabel ((&graphHdr), -1, MTG_LabelMask_VertexProperty, -1);
            normalLabelOffset = jmdlMTGGraph_defineLabel ((&graphHdr), -2, MTG_LabelMask_SectorProperty, -1);
            vertexArrayHdr.reserve (numVertex);
            normalArrayHdr.reserve (numNormal);
            break;
        case MTG_Facets_NoData:
            normalLabelOffset = -1;
            vertexLabelOffset = -1;
            break;
        }
    }


/**
* Returns the underlying topology structure for the facets.
* @param pFacetHeader
* @see
* @return pointer to the connectivity graph for the facets.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGFacets_getGraph
(
MTGFacetsP pFacetHeader
)
    {
    return (&pFacetHeader->graphHdr);
    }


/**

* @param pFacetHeader
* @param    userTag =>
* @param    labelType =>
* @param    defaultValue =>
* @see
* @return label offset.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_defineLabel
(
MTGFacets *         pFacetHeader,
int                 userTag,
MTGLabelMask        labelType,
int                 defaultValue
)
    {
    return jmdlMTGGraph_defineLabel ((&pFacetHeader->graphHdr), userTag, labelType, defaultValue);
    }


/**

* @param pFacetHeader
* @param userTag =>
* @see
* @return label offset for the given tag.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_getLabelOffset
(
const MTGFacets*    pFacetHeader,
int                 userTag
)
    {
    return jmdlMTGGraph_getLabelOffset ((&pFacetHeader->graphHdr), userTag);
    }



/**
* Add a vertex to a facet set.  If the facet set has normalMode
* MTG_Facets_NormalPerVertex, also add the corresponding normal.
* (Otherwise ignore the normal data.)
* @param pFacetHeader    <=> Header whose normal mode is to be set.
*                           if normals are separate from vertices.
* @param pVertex    => vertex coordinates.
* @param pNormal    => normal coordinates.   Only used when facet set
*                             normal mode is MTG_Facets_NormalPerVertex.
* @see MTGFacets.IVertexFormat
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_addVertex
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal
)
    {
    int id = -1;
    static double normalScale = 1.0;

    if (pFacetHeader)
        {
        switch (pFacetHeader->normalMode)
            {
            case MTG_Facets_VertexOnly:
            case MTG_Facets_SeparateNormals:
                id = PushedId<DPoint3d> (pFacetHeader->vertexArrayHdr, *pVertex);
                break;
            case MTG_Facets_NormalPerVertex:
                id = PushedId<DPoint3d> (pFacetHeader->vertexArrayHdr, *pVertex);
                DPoint3d scaledNormal;
                scaledNormal.Scale (*pNormal, normalScale);
                pFacetHeader->normalArrayHdr.push_back (scaledNormal);
                break;
            case MTG_Facets_NoData:
                break;
            }
        }
    return id;
    }


/**
*
* Add to the normal array for a facet set.
* It is only valid to do this normalonly addition if the facet set
* is designated MTG_Facets_SeparateNormals.
*
* @param  pFacetHeader  <=> Header whose normal mode is to be set.
*                       if normals are separate from vertices.
* @param pNormal    => normal coordinates.
* @see MTGFacets.IVertexFormat
* @return id of the newly added normal.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_addNormal
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pNormal
)
    {
    int id = -1;
    if (pFacetHeader && pFacetHeader->normalMode == MTG_Facets_SeparateNormals)
        {
        id = PushedId (pFacetHeader->normalArrayHdr, *pNormal);
        }
    return id;
    }

/* METHOD(default,none,addIndexedEdge)
/**
*
* @param pFacetHeader   <=> facet set to receive new edge.
* @param nodeId0 => start node
* @param nodeId1 => end node
* @param mask0 => mask for start node
* @param mask1 => mask for end node
* @param vertexIndex0 => vertex index for start node
* @param vertexIndex1 => vertex index for end node
* @param normalIndex0 => normal index for start node. Ignored unless SeparateNormals facet type.
* @param normalIndex1 => normal index for end node. Ignored unless SeparateNormals facet type.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_addIndexedEdge
(
MTGFacets *         pFacets,
MTGNodeId           *pNodeId0,
MTGNodeId           *pNodeId1,
MTGNodeId           nodeId0,
MTGNodeId           nodeId1,
MTGMask             mask0,
MTGMask             mask1,
int                 vertexIndex0,
int                 vertexIndex1,
int                 normalIndex0,
int                 normalIndex1
)
    {
    MTGGraph *pGraph = &pFacets->graphHdr;
    bool    boolstat;
    int vertexLabelOffset = pFacets->vertexLabelOffset;
    int normalLabelOffset = pFacets->normalLabelOffset;
    /* Connect the topology */
    boolstat = jmdlMTGGraph_join (pGraph, pNodeId0, pNodeId1, nodeId0, nodeId1, mask0, mask1);

    /* Store the vertex index ... */
    if (boolstat)
        {
        jmdlMTGGraph_setLabel (pGraph, *pNodeId0, vertexLabelOffset, vertexIndex0);
        jmdlMTGGraph_setLabel (pGraph, *pNodeId1, vertexLabelOffset, vertexIndex1);
        boolstat = true;
        if (pFacets->normalMode == MTG_Facets_SeparateNormals)
            {
            jmdlMTGGraph_setLabel (pGraph, *pNodeId0, normalLabelOffset, normalIndex0);
            jmdlMTGGraph_setLabel (pGraph, *pNodeId1, normalLabelOffset, normalIndex1);
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------*//**
* Test an array of points to see if the first and last points are within
* a small tolerance of each other.  The tolerance is chosen as 10e-8 of the
* data range of the entire array of points.
* This test is used to eliminate duplicated first/last points in polygon coordinate
* arrays where the caller may have unneccesarily included a duplicate point.
* @param pXYZArray   => coordinate data.
* @param numXYZArray => number of vertices in loops.
*                       First, last are not doubled.
* @return true if the first and last points appear to be duplicated.
* @bsihdr                                       EarlinLutz      02/00
+---------------+---------------+---------------+---------------+------*/
static bool        jmdlMTGDPoint3d_isFinalPointDuplicate
(
const DPoint3d *pXYZArray,
int             numXYZ
)
    {
    double faceSize = bsiDPoint3d_getLargestCoordinateDifference (pXYZArray, numXYZ);
    double edgeSize;
    static double s_relTol = 1.0e-8;
    double tol = s_relTol * faceSize;
    DPoint3d delta;
    delta.DifferenceOf (pXYZArray[0], pXYZArray[numXYZ - 1]);
    edgeSize = delta.MaxAbs ();
    /* Filter out doubled up final point. */
    if (   numXYZ > 1
        && edgeSize < tol)
        {
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------*//**
* @param pFacetHeader   <=> facet set to receive new face.
* @param pVertex    => vertex array.
* @param pNormal    => normal array (NULL if not being used ..)
* @param numVertex  => number of vertices in loops.   First, last not doubled.
* @param labelTag   => tag for labels being attached.
* @param pLabel     => optional array of labels, indexed same as vertice.
* @param headLabel  => true if the label is to be at the head vertex, false for tail.
* @see
* @return id the node corresponding to vertex 0 of the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGFacets_addCoordinateFace
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal,
int                 numVertex,
int                 labelTag,
int                 *pLabel,
int                 headLabel
)
    {
    int i;
    MTGNodeId  baseNodeId, nodeId0, nodeId1;
    int vertexId, normalId;

    MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    int vertexLabel = pFacetHeader->vertexLabelOffset;
    int normalLabel = pFacetHeader->normalLabelOffset;

    int edgeLabelOffset = pLabel ? jmdlMTGGraph_getLabelOffset (pGraph, labelTag) : -1;

    baseNodeId = MTG_NULL_NODEID;

    if (jmdlMTGDPoint3d_isFinalPointDuplicate (pVertex, numVertex))
        numVertex--;

    if (pFacetHeader->normalMode == MTG_Facets_NormalPerVertex)
        {

        for (i = 0; i < numVertex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            if (i == 0)
                {
                jmdlMTGGraph_setMask (pGraph, nodeId1, MTG_PRIMARY_EDGE_MASK);
                jmdlMTGGraph_setMask (pGraph, nodeId0, MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK);
                }

            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i],
                            pNormal ? &pNormal[i] : NULL);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);
            baseNodeId = nodeId0;
            }
        }
    else if (pFacetHeader->normalMode == MTG_Facets_VertexOnly)
        {
        baseNodeId = MTG_NULL_NODEID;

        for (i = 0; i < numVertex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            if (i == 0)
                {
                jmdlMTGGraph_setMask (pGraph, nodeId1, MTG_PRIMARY_EDGE_MASK);
                jmdlMTGGraph_setMask (pGraph, nodeId0, MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK);
                // Split will copy this to subsequent edges.
                }
            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i], NULL);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);
            baseNodeId = nodeId0;
            }
        }
    else if (pFacetHeader->normalMode == MTG_Facets_SeparateNormals)
        {
        baseNodeId = MTG_NULL_NODEID;

        for (i = 0; i < numVertex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            if (i == 0)
                {
                jmdlMTGGraph_setMask (pGraph, nodeId1, MTG_PRIMARY_EDGE_MASK);
                jmdlMTGGraph_setMask (pGraph, nodeId0, MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK);
                }
            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i], NULL);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);

            if (pNormal)
                {
                normalId = jmdlMTGFacets_addNormal (pFacetHeader, &pNormal[i]);
                jmdlMTGGraph_setLabel   (pGraph, nodeId0, normalLabel, normalId);
                jmdlMTGGraph_setLabel   (pGraph, nodeId1, normalLabel, normalId);
                }

            baseNodeId = nodeId0;
            }
        }

    if (pLabel && edgeLabelOffset >= 0)
        {
        // baseNodeId is the LAST node in the loop.
        // Advance at least back to the zeroth node, one more
        // if label is indicated for the head.
        int firstLabelNode = jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
        if (headLabel)
            firstLabelNode = jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
        int counter = 0;

        MTGARRAY_FACE_LOOP      (node0Id, pGraph, firstLabelNode)
            {
            int node1Id = jmdlMTGGraph_getVSucc (pGraph, node0Id);
            jmdlMTGGraph_setLabel   (pGraph, node0Id, edgeLabelOffset, pLabel[counter]);
            jmdlMTGGraph_setLabel   (pGraph, node1Id, edgeLabelOffset, pLabel[counter]);
            counter++;
            }
        MTGARRAY_END_FACE_LOOP  (node0Id, pGraph, firstLabelNode)
        }
    return jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
    }


/**
* Add a face using addCoordinateFace, with no normals or other labels.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
* <P>
* Vertex coordinates are tested with the given tolerances to detect adjacent
* duplicates.
*
* @param pFacets    <=> facet set to receive new face
* @param partnerLabelOffset => label offset where the through id is installed
* @param pXYZ               => vertex array
* @param numXYZ             => number of vertices
* @param absTol             => absolute tolerance for identical point test.
* @param relTol             => relative tolerance for identical point test.
* @return id of the node corresponding to vertex 0 of the array or MTG_NULL_NODEID
*   if face is degenerate.
* @bsihdr                                       DavidAssaf      07/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addDoubleFaceTol
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const DPoint3d  *pXYZArray,
int             numXYZ,
double          absTol,
double          relTol
)
    {
    DPoint3d    *pXYZCC = (DPoint3d *) _alloca (numXYZ * sizeof (*pXYZCC));
    MTGNodeId   baseNodeId = MTG_NULL_NODEID, partnerNodeId;

    bsiPolygon_compressDuplicateVertices (pXYZCC, NULL, NULL, &numXYZ,
        pXYZArray, NULL, numXYZ, absTol, relTol);

    if (numXYZ < 3)
        return MTG_NULL_NODEID;

    baseNodeId = jmdlMTGFacets_addCoordinateFace (pFacets, pXYZCC, NULL, numXYZ, 0, NULL, 0);

    /* There are two face loops -- one on each side.  Each face loop has one node
        at each vertex.  These two nodes are connected in their vertex loop.
        Annotate the pairing in the partner label.
    */
    MTGARRAY_FACE_LOOP (currNodeId, &pFacets->graphHdr, baseNodeId)
        {
        partnerNodeId = jmdlMTGGraph_getVSucc (&pFacets->graphHdr, currNodeId);
        jmdlMTGGraph_setLabel (&pFacets->graphHdr, currNodeId,      partnerLabelOffset, partnerNodeId);
        jmdlMTGGraph_setLabel (&pFacets->graphHdr, partnerNodeId,   partnerLabelOffset, currNodeId);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, &pFacets->graphHdr, baseNodeId)

    return baseNodeId;
    }


/**
* Add a face using addCoordinateFace, with no normals or other labels.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
* <P>
* Vertex coordinates are tested with default tolerances to detect adjacent
* duplicates.
*
* @param pFacets            <=> facet set to receive new face
* @param partnerLabelOffset => label offset where the through id is installed
* @param pXYZ               => vertex array
* @param numXYZ             => number of vertices
* @return id of the node corresponding to vertex 0 of the array or MTG_NULL_NODEID
*   if face is degenerate.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addDoubleFace
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const DPoint3d  *pXYZArray,
int             numXYZ
)
    {
    return jmdlMTGFacets_addDoubleFaceTol (pFacets, partnerLabelOffset,
                pXYZArray, numXYZ, 0.0, 1.0e-8);
    }



/*---------------------------------------------------------------------*//**
* @param pFacetHeader   <=> facet set to receive new face.
* @param pVertex    => vertex array.
* @param pNormal    => normal array (NULL if not being used ..)
* @param numVertex  => number of vertices in loops.   First, last not doubled.
* @param labelTag   => tag for labels being attached.
* @param pLabel     => optional array of labels, indexed same as vertice.
* @param headLabel  => true if the label is to be at the head vertex, false for tail.
* @see
* @return id the node corresponding to vertex 0 of the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGFacets_addIndexedFace
(
MTGFacets *         pFacetHeader,
const int           *pXYZIndexArray,
const int           *pNormalIndexArray,
int                 numVertex,
int                 labelTag,
int                 *pLabel,
int                 headLabel
)
    {
    int i;
    MTGNodeId  baseNodeId, nodeId0, nodeId1;
    int vertexId, normalId;

    MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    int vertexLabel = pFacetHeader->vertexLabelOffset;
    int normalLabel = pFacetHeader->normalLabelOffset;

    int edgeLabelOffset = pLabel ? jmdlMTGGraph_getLabelOffset (pGraph, labelTag) : -1;

    baseNodeId = MTG_NULL_NODEID;
    // Ignore normal data if facets are not expecting them:
    if (pFacetHeader->normalMode != MTG_Facets_SeparateNormals)
        pNormalIndexArray = NULL;

    for (i = 0; i < numVertex; i++)
        {
        jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
        if (i == 0)
            {
            jmdlMTGGraph_setMask (pGraph, nodeId1, MTG_PRIMARY_EDGE_MASK);
            jmdlMTGGraph_setMask (pGraph, nodeId0, MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK);
            }
        vertexId = pXYZIndexArray[i];
        jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
        jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);

        if (pNormalIndexArray)
            {
            normalId = pNormalIndexArray[i];
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, normalLabel, normalId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, normalLabel, normalId);
            }
            baseNodeId = nodeId0;
        }

    if (pLabel && edgeLabelOffset >= 0)
        {
        // baseNodeId is the LAST node in the loop.
        // Advance at least back to the zeroth node, one more
        // if label is indicated for the head.
        int firstLabelNode = jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
        if (headLabel)
            firstLabelNode = jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
        int counter = 0;

        MTGARRAY_FACE_LOOP      (node0Id, pGraph, firstLabelNode)
            {
            int node1Id = jmdlMTGGraph_getVSucc (pGraph, node0Id);
            jmdlMTGGraph_setLabel   (pGraph, node0Id, edgeLabelOffset, pLabel[counter]);
            jmdlMTGGraph_setLabel   (pGraph, node1Id, edgeLabelOffset, pLabel[counter]);
            counter++;
            }
        MTGARRAY_END_FACE_LOOP  (node0Id, pGraph, firstLabelNode)
        }
    return jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
    }


/**
* Add a face, with no normals or other lables.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
*
* @param pFacets    <=> facet set to receive new face
* @param partnerLabelOffset => label offset where the through id is installed
* @param pIndexArray        => vertex index array.
* @param numXYZ             => number of vertices
* @return id of the node corresponding to vertex 0 of the array
* @bsihdr                                       DavidAssaf      01/06
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addIndexedDoubleFace
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const int       *pXYZIndexArray,
const int       *pNormalIndexArray,
int             numXYZ
)
    {
    MTGNodeId baseNodeId, partnerNodeId;
    if (numXYZ < 3)
        return MTG_NULL_NODEID;

    baseNodeId = jmdlMTGFacets_addIndexedFace (pFacets, pXYZIndexArray, pNormalIndexArray, numXYZ, 0, NULL, 0);

    /* There are two face loops -- one on each side.  Each face loop has one node
        at each vertex.  These two nodes are connected in their vertex loop.
        Annotate the pairing in the partner label.
    */
    MTGARRAY_FACE_LOOP (currNodeId, &pFacets->graphHdr, baseNodeId)
        {
        partnerNodeId = jmdlMTGGraph_getVSucc (&pFacets->graphHdr, currNodeId);
        jmdlMTGGraph_setLabel (&pFacets->graphHdr, currNodeId,      partnerLabelOffset, partnerNodeId);
        jmdlMTGGraph_setLabel (&pFacets->graphHdr, partnerNodeId,   partnerLabelOffset, currNodeId);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, &pFacets->graphHdr, baseNodeId)

    return baseNodeId;
    }


/**
* Add a chain (polyline) to the graph.
* The entire chain is left as a nonexterior.
* @param pFacetHeader    <=> facet set to receive new face
* @param pVertex => point array.
* @param pNormal => normal array
* @param numVertex => number of vertices
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_addCoordinateChain
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal,
int                 numVertex
)
    {
    jmdlMTGFacets_addCoordinateChainExt (pFacetHeader, pVertex, pNormal, numVertex);
    }


/**
* Add a chain (polyline) to the graph.
* The entire chain is left as a nonexterior.
* @param pFacetHeader    <=> facet set to receive new face
* @param pNodeIdArray <= array of one node id at each vertex.
* @param pVertex => point array.
* @param pNormal => normal array
* @param numVertex => number of vertices
* @return Node id at base of the chain.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGFacets_addCoordinateChainExt
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal,
int                 numVertex
)
    {
    int i;
    MTGNodeId  baseNodeId, nodeId0, nodeId1;
    int vertexId, normalId;

    MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    int vertexLabel = pFacetHeader->vertexLabelOffset;
    int normalLabel = pFacetHeader->normalLabelOffset;
    int lastInternalIndex = numVertex - 2;
    int firstInternalIndex = 1;
    MTGNodeId returnNodeId = MTG_NULL_NODEID;

    if (numVertex < 1)
        return returnNodeId;

    if (pFacetHeader->normalMode == MTG_Facets_NormalPerVertex)
        {
        jmdlMTGGraph_createEdge (pGraph, &nodeId0, &nodeId1);
        returnNodeId = nodeId0;

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[0], &pNormal[0]);
        jmdlMTGGraph_setLabel (pGraph, nodeId0, vertexLabel, vertexId);

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[numVertex - 1], &pNormal[0]);
        jmdlMTGGraph_setLabel (pGraph, nodeId1, vertexLabel, vertexId);

        baseNodeId = nodeId0;

        for (i = firstInternalIndex; i <= lastInternalIndex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i], &pNormal[i]);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);
            baseNodeId = nodeId0;
            }
        }
    else if (pFacetHeader->normalMode == MTG_Facets_VertexOnly)
        {
        jmdlMTGGraph_createEdge (pGraph, &nodeId0, &nodeId1);
        returnNodeId = nodeId0;

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[0], NULL);
        jmdlMTGGraph_setLabel (pGraph, nodeId0, vertexLabel, vertexId);

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[numVertex - 1], NULL);
        jmdlMTGGraph_setLabel (pGraph, nodeId1, vertexLabel, vertexId);

        baseNodeId = nodeId0;

        for (i = firstInternalIndex; i <= lastInternalIndex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i], NULL);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);
            baseNodeId = nodeId0;
            }
        }
    else if (pFacetHeader->normalMode == MTG_Facets_SeparateNormals)
        {
        jmdlMTGGraph_createEdge (pGraph, &nodeId0, &nodeId1);
        returnNodeId = nodeId0;

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[0], NULL);
        jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);

        normalId = jmdlMTGFacets_addVertex (pFacetHeader, NULL, &pNormal[0]);
        jmdlMTGGraph_setLabel   (pGraph, nodeId0, normalLabel, normalId);

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[numVertex - 1], NULL);
        jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);

        normalId = jmdlMTGFacets_addVertex (pFacetHeader, NULL, &pNormal[numVertex - 1]);
        jmdlMTGGraph_setLabel   (pGraph, nodeId1, normalLabel, normalId);

        baseNodeId = nodeId0;

        for (i = firstInternalIndex; i <= lastInternalIndex; i++)
            {
            jmdlMTGGraph_splitEdge (pGraph, &nodeId0, &nodeId1, baseNodeId);
            vertexId = jmdlMTGFacets_addVertex (pFacetHeader, &pVertex[i], NULL);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, vertexLabel, vertexId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, vertexLabel, vertexId);

            normalId = jmdlMTGFacets_addNormal (pFacetHeader, &pNormal[i]);
            jmdlMTGGraph_setLabel   (pGraph, nodeId0, normalLabel, normalId);
            jmdlMTGGraph_setLabel   (pGraph, nodeId1, normalLabel, normalId);
            baseNodeId = nodeId0;
            }
        }
    return returnNodeId;
    }


/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlMTGFacets_splitEdge
(
        MTGFacets               *pFacetHeader,
        MTGNodeId               node0Id,
        DPoint3d                *pXYZ
)
    {
    MTGGraph *pGraph = &pFacetHeader->graphHdr;
    int vertexOffset = pFacetHeader->vertexLabelOffset;
    MTGNodeId   leftNodeId, rightNodeId;
    int vertexId;
    bool    stat = false;

    if (pFacetHeader->normalMode == MTG_Facets_VertexOnly)
        {
        stat = true;
        jmdlMTGGraph_splitEdge (pGraph, &leftNodeId, &rightNodeId, node0Id);

        vertexId = jmdlMTGFacets_addVertex (pFacetHeader, pXYZ, NULL);
        jmdlMTGGraph_setLabel   (pGraph, leftNodeId, vertexOffset, vertexId);
        jmdlMTGGraph_setLabel   (pGraph, rightNodeId, vertexOffset, vertexId);
        }
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* Copy coordinate data around a face into a flat array.  Indices from indicated label
*   of each node are used to access data in source array.
* @param pGraph   => topology structure
* @param pDestArray <=> array to receive face coordinates
* @param pSoureArray => coordinate source array
* @param startId => any node on the face.
* @param offset  => index for label access.
* @return true if all coordinates were available in the source array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getFaceCoordinates
(
const MTGGraph *        pGraph,
EmbeddedDPoint3dArray           *pDestArray,
const EmbeddedDPoint3dArray   *pSourceArray,
MTGNodeId               startId,
        int             offset
)
    {
    int vertexIndex;
    DPoint3d currPoint;
    jmdlEmbeddedDPoint3dArray_empty (pDestArray);

    MTGARRAY_FACE_LOOP (currId, pGraph, startId)
        {
        if (    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currId, offset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pSourceArray, &currPoint, vertexIndex))
            {
            pDestArray->push_back (currPoint);
            }
        else
            {
            return false;
            }
        }
    MTGARRAY_END_FACE_LOOP (currId, pGraph, startId);

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* Copy vertex coordinates around a facet.
* @param pFacets => facet set
* @param pDestArray <=> array to receive face coordinates
* @param startId => any node on the face.
* @param duplicateStartVertex => to request duplicate start/end coordinates
* @return true if all coordinates were available in the source array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getFacetCoordinates
(
const MTGFacets *       pFacets,
EmbeddedDPoint3dArray           *pDestArray,
MTGNodeId               startId,
bool                    duplicateStartVertex
)
    {
    if (jmdlMTGFacets_getFaceCoordinates (
                &pFacets->graphHdr,
                pDestArray,
                &pFacets->vertexArrayHdr,
                startId,
                pFacets->vertexLabelOffset
                ))
        {
        DPoint3d point0;
        if (   duplicateStartVertex
            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pDestArray, &point0, 0)
           )
           {
           pDestArray->push_back (point0);
           }
        return true;
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* Get the (unnormalized) normal from a triangle.  If the face does not have
* exactly 3 nodes, return false and a zero normal.
* @param pFacets => facet set
* @param pNormal <= normal components.
* @param nodeId => id of any node on the triangle.
* @return true if the face has exactly 3 nodes.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool             jmdlMTGFacets_getTriangleNormal
(
const MTGFacets     *pFacets,
DPoint3d            *pNormal,
MTGNodeId           nodeId
)
    {
    int numVertex;
    DPoint3d xyz[4];

    if (jmdlMTGFacets_getFaceCoordinatesToBuffer (
                &pFacets->graphHdr,
                xyz,
                &numVertex,
                4,
                &pFacets->vertexArrayHdr,
                nodeId,
                pFacets->vertexLabelOffset
                )
        && numVertex == 3)
        {
        pNormal->CrossProductToPoints (xyz[0], xyz[1], xyz[2]);
        return true;
        }
    pNormal->Zero ();
    return false;
    }

/**
* Computes the (unnormalized) normal of the triangle.  To reduce roundoff error,
* the normal is computed at the largest interior angle of the triangle.  If the
* largest interior angle is too large, the normal is computed at one of the
* subangles formed by the median.
*
* @param pFacets    => facets set
* @param pNormal    <= normal (or NULL)
* @param pMag2      <= squared magnitude of normal (or NULL)
* @param pMaxNodeId <= ID of node at largest angle (or NULL)
* @param nodeId     => ID of node in triangle
* @param eps2       => minimum value of sin^2(angle) allowable in squared normal magnitude
* @return false if face not a triangle
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getSafeTriangleNormal
(
const MTGFacets     *pFacets,
DPoint3d            *pNormal,
double              *pMag2,
MTGNodeId           *pMaxNodeId,
MTGNodeId           nodeId,
double              eps2
)
    {
    const MTGGraph  *pGraph = &pFacets->graphHdr;
    DPoint3d        vert[3];
    MTGNodeId       node[3];
    DPoint3d        normal = {0.0, 0.0, 0.0};
    double          mag2 = 0.0;
    MTGNodeId       maxNodeId = MTG_NULL_NODEID;
    bool            bStatus = false;

    node[0] = nodeId;
    node[1] = jmdlMTGGraph_getFSucc (pGraph, node[0]);
    node[2] = jmdlMTGGraph_getFSucc (pGraph, node[1]);

    // face should be a triangle
    if ((nodeId == jmdlMTGGraph_getFSucc (pGraph, node[2]))
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &vert[0], node[0])
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &vert[1], node[1])
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &vert[2], node[2]))
        {
        int maxIndex;

        mag2 = bsiGeom_triangleNormal (&normal, &maxIndex, vert, eps2);
        maxNodeId = node[maxIndex];

        bStatus = true;
        }

    if (pNormal)
        *pNormal = normal;

    if (pMag2)
        *pMag2 = mag2;

    if (pMaxNodeId)
        *pMaxNodeId = maxNodeId;

    return bStatus;
    }


/*---------------------------------------------------------------------------------**//**
* @param pHeader =>  facet set to access.
* @param pPoint <= coordinates of given node.
* @param nodeId => node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeCoordinates
(
MTGFacetsCP                    pHeader,
DPoint3d                *pPoint,
MTGNodeId               nodeId
)
    {
    int vertexIndex;

    if (    jmdlMTGGraph_getLabel ((&pHeader->graphHdr),
                            &vertexIndex,
                            nodeId,
                            pHeader->vertexLabelOffset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d ((&pHeader->vertexArrayHdr), pPoint, vertexIndex)
        )
        return true;

    return false;
    }

bool MTGFacets::SetPointIndex (MTGNodeId nodeId, size_t index)
    {
    return graphHdr.TrySetLabel (nodeId, vertexLabelOffset, (int)index);
    }


bool MTGFacets::NodeToVertexCoordinatesAroundFace (MTGNodeId nodeId, bvector<DPoint3d> &xyz, int numWrap) const
  {
  xyz.clear ();
  bool ok = true;
  MTGGraphCP graph = GetGraphCP ();
  DPoint3d xyzA;
  MTGARRAY_FACE_LOOP (sector, graph, nodeId)
      {
      if (NodeToVertexCoordinates (sector, xyzA))
          xyz.push_back (xyzA);
      else
          ok = false;
      }
  MTGARRAY_END_FACE_LOOP (sector, graph, nodeId)

  if (ok && numWrap > 0)
      {
      for (int i = 0; i < numWrap; i++)
          {
          xyzA = xyz.back (); // copy out to avoid reallocation trap
          xyz.push_back (xyzA);
          }
      }
  return ok;
  }

bool MTGFacets::NodeToVertexCoordinates (MTGNodeId nodeId, DPoint3dR xyz) const
  {
  return jmdlMTGFacets_getNodeCoordinates (this, &xyz, nodeId);
  }
bool MTGFacets::NodeToVertexIndex (MTGNodeId nodeId, ptrdiff_t &index) const
  {
  int integerIndex;
  bool ok = jmdlMTGFacets_getNodeVertexIndex (this, &integerIndex, nodeId);
  index = (ptrdiff_t)integerIndex;
  return ok;
  }



/*---------------------------------------------------------------------------------**//**
* @param pHeader =>   facet set to access.
* @param pPoint => coordinates to set
* @param nodeId => node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_setNodeCoordinates
(
MTGFacetsP     pHeader,
const DPoint3d                *pPoint,
MTGNodeId               nodeId
)
    {
    int vertexIndex;

    if (    jmdlMTGGraph_getLabel (&pHeader->graphHdr,
                            &vertexIndex,
                            nodeId,
                            pHeader->vertexLabelOffset)
        && vertexIndex >= 0 && (size_t) vertexIndex < pHeader->vertexArrayHdr.size ())
            {
            pHeader->vertexArrayHdr[(size_t)vertexIndex] = *pPoint;
            return true;
            }
      return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param pHeader => facet set to access.
* @param pNormal    <= normal at given node.
* @param nodeId     => node whose normal is returned.
* @return true if node id is valid and has normal data.
* @bsihdr                                       DavidAssaf      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeNormal
(
const MTGFacets *pHeader,
DPoint3d        *pNormal,
MTGNodeId       nodeId
)
    {
    int normalIndex;

    if (   jmdlMTGGraph_getLabel (&pHeader->graphHdr, &normalIndex, nodeId,
                pHeader->normalLabelOffset)
        && normalIndex >= 0     /* node may not have normal info */
        && jmdlEmbeddedDPoint3dArray_getDPoint3d (&pHeader->normalArrayHdr,
                        pNormal, normalIndex)
        )
        return true;

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param pHeader =>   facet set to access.
* @param pPoint <= coordinates of given node.
* @param nodeId => node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeVertexIndex
(
MTGFacetsCP                    pHeader,
int                     *pVertexIndex,
MTGNodeId               nodeId
)
    {
    int vertexIndex = 0;

    if (    jmdlMTGGraph_getLabel ((&pHeader->graphHdr),
                            &vertexIndex,
                            nodeId,
                            pHeader->vertexLabelOffset)
        )
        {
        *pVertexIndex = vertexIndex;
        return true;
        }
    *pVertexIndex = -1;
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param pHeader =>   facet set to access.
* @param pNormalIndex   <= index to normal
* @param nodeId         => node whose normal index is returned.
* @return true if node id is valid and has normal data.
* @bsihdr                                       DavidAssaf      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeNormalIndex
(
const MTGFacets *pHeader,
int             *pNormalIndex,
MTGNodeId       nodeId
)
    {
    int normalIndex = 0;

    if (jmdlMTGGraph_getLabel (&pHeader->graphHdr, &normalIndex, nodeId,
            pHeader->normalLabelOffset))
        {
        *pNormalIndex = normalIndex;
        return true;
        }
    *pNormalIndex = -1;
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @param pHeader =>   facet set to access.
* @param pPoint <= parameter coordinates of given node.
* @param nodeId => node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeParametricCoordinates
(
const MTGFacets *       pHeader,
DPoint3d                *pPoint,
MTGNodeId               nodeId
)
    {
    int vertexIndex;
    /* NEEDS WORK: separate normals needs different label access?? */
    if (    jmdlMTGGraph_getLabel ((&pHeader->graphHdr),
                            &vertexIndex,
                            nodeId,
                            pHeader->vertexLabelOffset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d ((&pHeader->param1ArrayHdr), pPoint, vertexIndex)
        )
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Copy coordinate data around a face into a flat array.  At each node, indicated label value
*       is index into source array.
* @param pGraph =>   source graph.
* @param pArray <= array of coordinates
* @param pNumVertex <= number of vertex coordinates retrieved
* @param    maxVertex   => size of receiving buffer.
* @param pSourceArray   => vertex coordinate array.
* @param startId        => node id for first node on face
* @param offset         => offset to label.
* @return true if face index is valid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool             jmdlMTGFacets_getFaceCoordinatesToBuffer
(
const MTGGraph *        pGraph,
DPoint3d                *pArray,
      int               *pNumVertex,
      int               maxVertex,
const EmbeddedDPoint3dArray   *pSourceArray,
MTGNodeId               startId,
        int             offset
)
    {
    int vertexIndex;
    int count = 0;

    MTGARRAY_FACE_LOOP (currId, pGraph, startId)
        {
        if (count >= maxVertex)
            {
            *pNumVertex =count;
            return false;
            }
        if (    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currId, offset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pSourceArray, &pArray[count], vertexIndex))
            {
            count++;
            }
        else
            {
            *pNumVertex = count;
            return false;
            }
        }
    MTGARRAY_END_FACE_LOOP (currId, pGraph, startId);
    *pNumVertex = count;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @param pDestFacets <= destination facet set
* @param pSourceFacets  => source facet set
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_copy
(
MTGFacets *     pDestFacets,
const MTGFacets *    pSourceFacets
)
    {
    jmdlMTGFacets_empty (pDestFacets);
    jmdlMTGGraph_copy ((&pDestFacets->graphHdr), (&pSourceFacets->graphHdr));

    pDestFacets->vertexArrayHdr = pSourceFacets->vertexArrayHdr;
    pDestFacets->normalArrayHdr = pSourceFacets->normalArrayHdr;
    pDestFacets->param1ArrayHdr = pSourceFacets->param1ArrayHdr;
    pDestFacets->param2ArrayHdr = pSourceFacets->param2ArrayHdr;

    pDestFacets->normalMode = pSourceFacets->normalMode;
    pDestFacets->vertexLabelOffset = pSourceFacets->vertexLabelOffset;
    pDestFacets->normalLabelOffset = pSourceFacets->normalLabelOffset;
    pDestFacets->status = pSourceFacets->status;
    }



/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader   => mesh to test
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_isEmptyFacetSet
(
const   MTGFacets * pFacetHeader
)
    {
    return jmdlMTGGraph_getNodeCount ((&pFacetHeader->graphHdr)) <= 0;
    }


/*---------------------------------------------------------------------------------**//**

* @param pFacetHeader    => mesh to test
* @see
* @return
*                   0 if no normals or can't find a face with 3 or 4 nodes
*                   1 if normals appear to be outward from CCW faces
*                   -1 if normals appear to be inward to CCW faces
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_checkNormalOrientation
(
const MTGFacets *               pFacetHeader
)
    {
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pNormalArray = (&pFacetHeader->normalArrayHdr);
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);

    int vertexOffset = pFacetHeader->vertexLabelOffset;
    int normalOffset = pFacetHeader->normalLabelOffset;

    int normalIndex;

    DPoint3d diag0, diag1, cross;
    DPoint3d storedNormal;

    DPoint3d coords[4];
    if (pFacetHeader->normalMode != MTG_Facets_NormalPerVertex
       && pFacetHeader->normalMode != MTG_Facets_SeparateNormals)
        return 0;

    MTGARRAY_SET_LOOP (startNodeId, pGraph)
        {
        if (    !jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK))
            {
            int numNode = (int)pGraph->CountNodesAroundFace (startNodeId);
            if (numNode >= 3 && numNode <= 4
                && jmdlMTGFacets_getFaceCoordinatesToBuffer (pGraph, coords, &numNode, 4,
                                            pVertexArray, startNodeId, vertexOffset)
                && jmdlMTGGraph_getLabel (pGraph, &normalIndex, startNodeId, normalOffset)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d (pNormalArray, &storedNormal, normalIndex)
               )
                {
                if (numNode == 4)
                    {
                    // cross the diagonals to get a normal.  Could be wrong on weird face...
                    diag0.DifferenceOf (coords[2], coords[0]);
                    diag1.DifferenceOf (coords[3], coords[1]);
                    cross.CrossProduct (diag0, diag1);
                    }
                else
                    {
                    cross.CrossProductToPoints (coords[0], coords[1], coords[2]);
                    }
                double dot = cross.DotProduct (storedNormal);
                if (dot < 0.0)
                    {
                    return -1;      // The normals look reversed.
                    }
                else if (dot > 0.0)
                    {
                    return 1;       // The normals look right.
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currId, pGraph)
    // Fell through with no testable faces.  Give up.
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* Reverse the orientation of the topology.  If normals are present,
* reverse them.
* @param pFacetHeader    <=> facets whose normals and face loop order are to be reversed.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_reverseOrientation
(
MTGFacets *             pFacetHeader
)
    {

    if (pFacetHeader)
        {
        jmdlMTGGraph_reverse ((&pFacetHeader->graphHdr));

        for (auto &xyz : pFacetHeader->normalArrayHdr)
            xyz.Scale (xyz, -1.0);
        }
    }

/// 2nd moment integrand u * v = (u1 s + u2 t)(v1 s + v2 t)
/// integrated over triangle [0<=u<=1][0<=v<=1-u]
/// with origin at u=au, v=av
static double triangleAreaMoment
(
double area,
double u1,
double v1,
double u2,
double v2,
double au,
double av
)
    {
    double f = area * (u1 * v1 + 0.5 * (u1 * v2 + u2 * v1) + u2 * v2 ) / 6.0;
    double Mu = area * (u1 + u2) / 3.0;
    double Mv = area * (v1 + v2) / 3.0;
    return f + au * Mv + av * Mu + au * av * area;
    }
/*-------------------------------------------------------------------*//**
<summary> Accumulate volume properties of a tetrahedron into sums. </summary>
+----------------------------------------------------------------------*/
static void accumulateTriangleAreaProperties
(
DVec3d  &u1,
DVec3d  &u2,
DVec3d  &u3,
DVec3d  &referenceNormal,
double  &areaSum,
DVec3d  &moment1Sum,
double  &ixxSum,
double  &iyySum,
double  &izzSum,
double  &ixySum,
double  &ixzSum,
double  &iyzSum
)
    {
    DVec3d v12, v13;

    v12.DifferenceOf (u2, u1);
    v13.DifferenceOf (u3, u1);
    double area = 0.5 * v12.TripleProduct (v13, referenceNormal);

    areaSum += area;

    double momentScale;
    momentScale = area /3.0;
    moment1Sum.x += momentScale * (u1.x + u2.x + u3.x);
    moment1Sum.y += momentScale * (u1.y + u2.y + u3.y);
    moment1Sum.z += momentScale * (u1.z + u2.z + u3.z);

    ixxSum += triangleAreaMoment (area, v12.x, v12.x, v13.x, v13.x, u1.x, u1.x);
    iyySum += triangleAreaMoment (area, v12.y, v12.y, v13.y, v13.y, u1.y, u1.y);
    izzSum += triangleAreaMoment (area, v12.z, v12.z, v13.z, v13.z, u1.z, u1.z);

    ixySum += triangleAreaMoment (area, v12.x, v12.y, v13.x, v13.y, u1.x, u1.y);
    ixzSum += triangleAreaMoment (area, v12.x, v12.z, v13.x, v13.z, u1.x, u1.z);
    iyzSum += triangleAreaMoment (area, v12.y, v12.z, v13.y, v13.z, u1.y, u1.z);
    }

/*-------------------------------------------------------------------*//**
<summary> Accumulate volume properties of a tetrahedron into sums. </summary>
+----------------------------------------------------------------------*/
static void accumulateTetrahedralVolumeProperties
(
DVec3d  &u1,
DVec3d  &u2,
DVec3d  &u3,
double  &volumeSum,
DVec3d  &moment1Sum,
double  &ixxSum,
double  &iyySum,
double  &izzSum,
double  &xySum,
double  &xzSum,
double  &yzSum
)
    {
    double volume;
    double momentScale;
    double uuScale, uvScale;

    volume = u1.TripleProduct (u2, u3) / 6.0;
    volumeSum += volume;
    momentScale = volume * 0.25;

    moment1Sum.x += momentScale * (u1.x + u2.x + u3.x);
    moment1Sum.y += momentScale * (u1.y + u2.y + u3.y);
    moment1Sum.z += momentScale * (u1.z + u2.z + u3.z);

    uuScale = volume / 10.0;

    ixxSum += uuScale *
                    (u1.x * u1.x + u2.x * u2.x + u3.x * u3.x +
                    u1.x * u2.x + u1.x * u3.x + u2.x * u3.x);

    iyySum += uuScale *
                   (u1.y * u1.y + u2.y * u2.y + u3.y * u3.y +
                    u1.y * u2.y + u1.y * u3.y + u2.y * u3.y);

    izzSum += uuScale *
                   (u1.z * u1.z + u2.z * u2.z + u3.z * u3.z +
                    u1.z * u2.z + u1.z * u3.z + u2.z * u3.z);

    uvScale = volume / 20.0;

    xySum += uvScale * (2.0 * (u1.x * u1.y + u2.x * u2.y + u3.x * u3.y) +
                                        u1.x * u2.y + u2.x * u1.y +
                                        u1.x * u3.y + u3.x * u1.y +
                                        u2.x * u3.y + u3.x * u2.y);

    xzSum += uvScale * (2.0 * (u1.x * u1.z + u2.x * u2.z + u3.x * u3.z) +
                                        u1.x * u2.z + u2.x * u1.z +
                                        u1.x * u3.z + u3.x * u1.z +
                                        u2.x * u3.z + u3.x * u2.z);

    yzSum += uvScale * (2.0 * (u1.z * u1.y + u2.z * u2.y + u3.z * u3.y) +
                                        u1.z * u2.y + u2.z * u1.y +
                                        u1.z * u3.y + u3.z * u1.y +
                                        u2.z * u3.y + u3.z * u2.y);
    }

/*---------------------------------------------------------------------------------**//**
* For each face, assemble nodeId and coordinates into arrays and pass to a callback function.

* @param pFacets    => facets
* @param faceFunc => function to call per face.
    faceFunc (  MTGFacets *pFacets,
                EmbeddedIntArray *pNodeIdArray,
                EmbeddedDPoint3dArray *pXYZArray,
                void *vpContext);
     false return from faceFunc terminates the scan.
* @param vpContext IN context pointer for callback.
* @param returns true if callback returned true for all faces.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_scanFaces
(
MTGFacets *pFacets,
MTGFacets_FaceScanFunction  faceFunc,
void            *vpContext
)
    {
    bool    boolstat = false;
    MTGGraph * pGraph = (&pFacets->graphHdr);
    EmbeddedIntArray *pNodeIdArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    DPoint3d xyz;

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGARRAY_SET_LOOP (node0Id, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, node0Id, visitMask))
            {
            jmdlEmbeddedDPoint3dArray_empty (pXYZArray);
            jmdlEmbeddedIntArray_empty (pNodeIdArray);
            jmdlMTGGraph_setMaskAroundFace (pGraph, node0Id, visitMask);

            MTGARRAY_FACE_LOOP (currNodeId, pGraph, node0Id)
                {
                if (!jmdlMTGFacets_getNodeCoordinates (pFacets,
                                                &xyz, currNodeId))
                        goto wrapup;
                jmdlEmbeddedIntArray_addInt (pNodeIdArray, currNodeId);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &xyz);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, node0Id)
            if (!faceFunc (pFacets, pNodeIdArray, pXYZArray, vpContext))
                goto wrapup;
            }
        }
    MTGARRAY_END_SET_LOOP (node0Id, pGraph)

    boolstat = true;
wrapup:
    jmdlEmbeddedIntArray_drop (pNodeIdArray);
    jmdlEmbeddedDPoint3dArray_drop (pXYZArray);

    return boolstat;
    }

// Area and volume sums are accumulated into an instance of this structure.
typedef struct
    {
    int numFace;
    bool    bOriginDefined;
    DPoint3d origin;

    int errors;
    double volumeSum;
    double perTriangleAreaSum;
    double perFaceAreaSum;

    double perimeterSum;

    double planarityError;
    DVec3d   balanceMoments;
    double ixx;
    double iyy;
    double izz;
    double ixy;
    double ixz;
    double iyz;
    // Not available until translateMoments ...
    DPoint3d centroid;
    DVec3d   spinMoments;

    void clear ()
        {
        errors = 0;
        numFace = 0;
        volumeSum = perFaceAreaSum = perTriangleAreaSum = perimeterSum = 0.0;
        planarityError = 0.0;
        bOriginDefined = false;
        origin.Zero ();
        centroid.Zero ();
        balanceMoments.Zero ();
        ixx = iyy = izz = 0.0;
        ixy = ixz = iyz = 0.0;
        }

    bool    translateMoments (int dimensionSelect)
        {
        double q = dimensionSelect == 3 ? volumeSum : perFaceAreaSum;
        if (errors == 0 && q != 0.0)
            {
            DVec3d localCentroid;
            localCentroid.Scale (balanceMoments, 1.0 / q);
            centroid = DPoint3d::FromSumOf (origin, localCentroid);

            ixx -= q * localCentroid.x * localCentroid.x;
            iyy -= q * localCentroid.y * localCentroid.y;
            izz -= q * localCentroid.z * localCentroid.z;

            ixy -= q * localCentroid.x * localCentroid.y;
            ixz -= q * localCentroid.x * localCentroid.z;
            iyz -= q * localCentroid.y * localCentroid.z;

            spinMoments.x = iyy + izz;
            spinMoments.y = ixx + izz;
            spinMoments.z = ixx + iyy;
            return true;
            }
        return false;
        }

    } MomentContext;

/// <summary>Return the range of dot product values in the expression
///         (XYZ[i] - Origin) dot Vector)
/// </summary>
static void bsiDPoint3dArray_dotDifferenceRange
(
DPoint3d    *pOrigin,
DVec3d      *pVector,
DPoint3d    *pXYZ,
int         numXYZ,
double      &aMin,
double      &aMax
)
    {
    aMin = aMax = pOrigin->DotDifference(*pXYZ, *pVector);
    for (int j = 1; j < numXYZ; j++)
        {
        double a = pXYZ[ j].DotDifference(*pXYZ, *pVector);
        if (a > aMax)
            aMax = a;
        else if (a < aMin)
            aMin = a;
        }
    }

// Walk the coordinates of one face, accumulating tetrahedral contributions
//    to volume properties.
static bool    accumulateVolumeMomentsForFace
(
MTGFacets *pFacets,
EmbeddedIntArray *pNodeIdArray,
EmbeddedDPoint3dArray *pXYZArray,
void *vpSums
)
    {
    MTGGraph *pGraph = jmdlMTGFacets_getGraph (pFacets);
    MomentContext *pSums = (MomentContext*)vpSums;
    DPoint3d *pXYZ = jmdlEmbeddedDPoint3dArray_getPtr (pXYZArray, 0);
    int numXYZ = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);

    if (numXYZ < 3)
        return true;    // Nothing here, but don't call it an error.

    MTGNodeId nodeId0;
    jmdlEmbeddedIntArray_getInt (pNodeIdArray, &nodeId0, 0);
    if (jmdlMTGGraph_getMask (pGraph, nodeId0, MTG_EXTERIOR_MASK))
        {
        pSums->errors++;
        return false;
        }

    if (!pSums->bOriginDefined)
        {
        pSums->origin = pXYZ[0];
        pSums->bOriginDefined = true;
        }

    DVec3d vectorA, vectorB, vectorC;
    // vectorA is from origin to fixed point around face.
    // vectorB, vectorC are from origin to a pair of adjacent vertices moving along edges
    //  other than the first, last.
    vectorA.DifferenceOf (pXYZ[0], pSums->origin);
    vectorB.DifferenceOf (pXYZ[1], pSums->origin);
    DVec3d summedNormal;
    DVec3d currNormal;
    summedNormal.Zero ();

    for (int i = 2; i < numXYZ; i++, vectorB = vectorC)
        {
        vectorC.DifferenceOf (pXYZ[i], pSums->origin);
        accumulateTetrahedralVolumeProperties
                    (
                    vectorA, vectorB, vectorC,
                    pSums->volumeSum, pSums->balanceMoments,
                    pSums->ixx, pSums->iyy, pSums->izz,
                    pSums->ixy, pSums->ixz, pSums->iyz
                    );
        currNormal.CrossProductToPoints (pXYZ[0], pXYZ[i - 1], pXYZ[i]);
        summedNormal.Add (currNormal);
        }

    double currArea = 0.5 * summedNormal.Normalize ();

    if (currArea > 0.0)
        {
        pSums->perimeterSum += currArea;
        if (currArea > 0.0 && numXYZ > 3)
            {
            double aMin = 0.0, aMax = 0.0;
            for (int j = 1; j < numXYZ; j++)
                {
                double a = pXYZ[ j].DotDifference(*pXYZ, currNormal);
                if (a > aMax)
                    aMax = a;
                else if (a < aMin)
                    aMin = a;
                }
            double dist = aMax - aMin;
            if (dist > pSums->planarityError)
                pSums->planarityError = dist;
            }
        }

    pSums->numFace++;
    return true;
    }

MTGGraphP MTGFacets::GetGraphP ()
    {
    return &graphHdr;
    }

MTGGraphCP MTGFacets::GetGraphCP () const
    {
    return &graphHdr;
    }

/*---------------------------------------------------------------------------------**//**
* 
+---------------+---------------+---------------+---------------+---------------+------*/
bool MTGFacets::AddFacesToPolyface
(
IPolyfaceConstructionR builder,
bvector<MTGNodeId> const &nodePerFace,
bool reverseFaceOrder
)
    {
    // Get a builder index for each point used by the nodePerFace array..
    bvector<size_t> mtgVertexIndexToPolyfaceVertexIndex;
    MTGGraphCP graph = GetGraphCP ();
    size_t numMTGVertex = vertexArrayHdr.size ();
    const size_t unusedVertexIndex = SIZE_MAX;
    mtgVertexIndexToPolyfaceVertexIndex.resize (numMTGVertex, unusedVertexIndex);

    DPoint3d xyz;
    for (MTGNodeId faceSeed : nodePerFace)
        {
        ptrdiff_t mtgVertexIndex;
        MTGNodeId currentNode = faceSeed;
        size_t numNodesAroundFace = graph->CountNodesAroundFace (faceSeed);

        for (size_t i = 0; i < numNodesAroundFace;
              i++,
              currentNode = reverseFaceOrder
                          ? graph->FPred (currentNode)
                          : graph->FSucc (currentNode)
              )
            {
            if (NodeToVertexIndex (currentNode, mtgVertexIndex))
                {
                size_t polyfacePointIndex = mtgVertexIndexToPolyfaceVertexIndex[mtgVertexIndex];
                // If first touch of this vertex, add it to the index ...
                if (polyfacePointIndex == unusedVertexIndex)
                    {
                    NodeToVertexCoordinates (currentNode, xyz);
                    polyfacePointIndex
                      = mtgVertexIndexToPolyfaceVertexIndex  [mtgVertexIndex]
                      = builder.FindOrAddPoint (xyz);
                    }
                if (polyfacePointIndex != unusedVertexIndex)
                    builder.AddPointIndex (polyfacePointIndex, true);
                }
            }
        builder.AddPointIndexTerminator ();
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the face normal. (No test for out-of-plane variance)
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPlane3d MTGFacets::NodeToFacePlaneNormal
(
MTGNodeId nodeId,
bvector<DPoint3d> &xyz
) const
    {
    if (NodeToVertexCoordinatesAroundFace (nodeId, xyz, 0))
        {
        DPoint3d centroid;
        DVec3d   normal;
        double area;
        if (PolygonOps::CentroidNormalAndArea (xyz, centroid, normal, area))
            {
            return ValidatedDPlane3d
                (
                DPlane3d::FromOriginAndNormal (centroid, normal),
                true
                );
            }
        }
    return ValidatedDPlane3d ();    // and this is invalid
    }

/*---------------------------------------------------------------------------------**//**
* Access the (stored) normal at a node.
* an arbitrary point and each face.
+---------------+---------------+---------------+---------------+---------------+------*/
bool MTGFacets::NodeToNormalCoordinates
(
MTGNodeId nodeId,
DVec3dR   normal
) const
    {

    int normalIndex;
    if (normalLabelOffset > 0
        && graphHdr.TryGetLabel (nodeId, normalLabelOffset, normalIndex)
        && normalIndex >= 0
        && normalIndex < (int)normalArrayHdr.size ()
        )
        {
        normal = DVec3d::From (normalArrayHdr[(size_t)normalIndex]);
        return true;
        }
    return false;
    }

bool MTGFacets::BuildFacePlaneNormals ()
    {
    normalArrayHdr.clear ();
    if (normalLabelOffset < 0)
        normalLabelOffset = graphHdr.DefineLabel (-2, MTG_LabelMask_SectorProperty, -1);
    graphHdr.SetLabel (normalLabelOffset, -1);

    MTGGraph::ScopedMask visitMask (graphHdr);
    size_t numErrors = 0;
    bvector<DPoint3d> xyz;
    MTGARRAY_SET_LOOP (seedNodeId, &graphHdr)
        {
        if (!visitMask.GetAt (seedNodeId)
            && !graphHdr.GetMaskAt (seedNodeId, MTG_EXTERIOR_MASK)
            )
            {
            visitMask.SetAroundFace (seedNodeId);
            auto plane = NodeToFacePlaneNormal (seedNodeId, xyz);
            if (!plane.IsValid ())
                numErrors++;        // leave the indices undefined.
            else
                {
                int normalIndex = (int)normalArrayHdr.size ();
                normalArrayHdr.push_back (plane.Value ().normal);
                graphHdr.SetLabelAroundFace (seedNodeId, normalLabelOffset, normalIndex);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, &graphHdr)
    return numErrors == 0;
    }
/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
+---------------+---------------+---------------+---------------+---------------+------*/
double MTGFacets::SumTetrahedralVolumeFromNodesPerFace
(
DPoint3dCR origin,
bvector<MTGNodeId> const &nodePerFace
) const
    {
    MTGGraphCP graph = GetGraphCP ();
    double sum = 0.0;
    DPoint3d xyzEdges[3];

    for (MTGNodeId seed : nodePerFace)
        {
        int numSector = 0;
        MTGARRAY_FACE_LOOP (sector, graph, seed)
            {
            DPoint3d xyz;
            NodeToVertexCoordinates (sector, xyz);
            if (numSector < 2)
                xyzEdges[numSector] = xyz;
            else
                {
                xyzEdges[2] = xyz;
                double volume = origin.TripleProductToPoints (xyzEdges[0], xyzEdges[1], xyzEdges[2]);
                sum += volume / 6.0;
                xyzEdges[1] = xyzEdges[2];
                }

            numSector++;
            }
        MTGARRAY_END_FACE_LOOP (sector, graph, seed)
        }
    return sum;
    }

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d MTGFacets::RangeFromNodesPerFace
(
bvector<MTGNodeId> const &nodePerFace
) const
    {
    MTGGraphCP graph = GetGraphCP ();
    DRange3d range;
    range.Init ();
    for (MTGNodeId seed : nodePerFace)
        {
        MTGARRAY_FACE_LOOP (sector, graph, seed)
            {
            DPoint3d xyz;
            NodeToVertexCoordinates (sector, xyz);
            range.Extend (xyz);
            }
        MTGARRAY_END_FACE_LOOP (sector, graph, seed)
        }
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d MTGFacets::SumTetrahedralVolumeChecksumFromNodesPerFace
(
DPoint3dCR origin,
bvector<MTGNodeId> const &nodePerFace
) const
    {
    MTGGraphCP graph = GetGraphCP ();
    DVec3d sum;
    sum.Zero ();
    DVec3d edgeTargets[3];
    DVec3d crossXY, crossYZ, crossZX;
    for (MTGNodeId seed : nodePerFace)
        {
        int numSector = 0;
        MTGARRAY_FACE_LOOP (sector, graph, seed)
            {
            DPoint3d xyz;
            NodeToVertexCoordinates (sector, xyz);
            if (numSector < 2)
                edgeTargets[numSector] = DVec3d::FromStartEnd (origin, xyz);
            else
                {
                edgeTargets[2] = DVec3d::FromStartEnd (origin, xyz);
                crossYZ.CrossProduct (edgeTargets[1], edgeTargets[2]);
                crossZX.CrossProduct (edgeTargets[2], edgeTargets[0]);
                crossXY.CrossProduct (edgeTargets[0], edgeTargets[1]);
                sum.Add (crossYZ);
                sum.Add (crossZX);
                sum.Add (crossXY);
                edgeTargets[1] = edgeTargets[2];
                }

            numSector++;
            }
        MTGARRAY_END_FACE_LOOP (sector, graph, seed)
        }
    return sum;
    }



/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacets => facets
* @param pVolume <= volume
* @param pArea <= area
* @param pMaxPlanarityError <= max deviation of a face from planarity
* @param pCentroid <= centroid coordinates.
* @param pAxisMoment2 <= squared moments are x, y, z axes at centroid.
* @param pXYMoment <= xy moment at centroid.
* @param pXZMoment <= xz moment at centroid.
* @param pYZMoment <= yz moment at centroid.
* @return SUCCESS if all coordinates acccessible and no boundary edges.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_volumeProperties
(
MTGFacets *pFacets,
double          *pVolume,
double          *pArea,
double          *pMaxPlanarityError,
DPoint3d        *pCentroid,
DVec3d          *pAxisMoment2,   // (yy+zz, xx+zz, xx+yy)
double          *pXYMoment,
double          *pXZMoment,
double          *pYZMoment
)
    {
    MomentContext sums;
    memset (&sums, 0, sizeof (sums));
    sums.clear ();
    bool    boolstat = jmdlMTGFacets_scanFaces (pFacets, accumulateVolumeMomentsForFace, &sums);

    if (!boolstat)
        sums.clear ();
    else
        boolstat = sums.translateMoments (3);

    if (pVolume)
        *pVolume = sums.volumeSum;
    if (pCentroid)
        *pCentroid = sums.centroid;

    if (pAxisMoment2)
        *pAxisMoment2 = sums.spinMoments;

    if (pXYMoment)
        *pXYMoment = sums.ixy;
    if (pXZMoment)
        *pXZMoment = sums.ixz;
    if (pYZMoment)
        *pYZMoment = sums.iyz;

    if (pArea)
        *pArea = sums.perimeterSum;

    return boolstat;
    }



// Walk the coordinates of one face, accumulating tetrahedral contributions
//    to volume properties.
static bool    accumulateAreaSumsForFace
(
MTGFacets *pFacets,
EmbeddedIntArray *pNodeIdArray,
EmbeddedDPoint3dArray *pXYZArray,
void *vpSums
)
    {
    MTGGraph *pGraph = jmdlMTGFacets_getGraph (pFacets);
    MomentContext *pSums = (MomentContext*)vpSums;
    DPoint3d *pXYZ = jmdlEmbeddedDPoint3dArray_getPtr (pXYZArray, 0);
    MTGNodeId *pNodeId = jmdlEmbeddedIntArray_getPtr (pNodeIdArray, 0);
    int numXYZ = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);

    if (numXYZ != jmdlEmbeddedIntArray_getCount (pNodeIdArray))
        return false;   // Flagrant foul.

    if (numXYZ < 3)
        return true;    // Nothing here, but don't call it an error.

    MTGNodeId nodeId0;
    jmdlEmbeddedIntArray_getInt (pNodeIdArray, &nodeId0, 0);
    if (jmdlMTGGraph_getMask (pGraph, nodeId0, MTG_EXTERIOR_MASK))
        {
        // Do nothing on boundary face
        return true;
        }

    if (!pSums->bOriginDefined)
        {
        pSums->origin = pXYZ[0];
        pSums->bOriginDefined = true;
        }

    DVec3d vectorA, vectorB, vectorC;
    // vectorA is from origin to fixed point around face.
    // vectorB, vectorC are from origin to a pair of adjacent vertices moving along edges
    //  other than the first, last.
    DVec3d summedNormal;
    DVec3d currNormal;
    summedNormal.Zero ();

    // First triangle scan establishes normal vector..
    vectorA.DifferenceOf (pXYZ[0], pSums->origin);
    vectorB.DifferenceOf (pXYZ[1], pSums->origin);
    int i;
    for (i = 2; i < numXYZ; i++, vectorB = vectorC)
        {
        vectorC.DifferenceOf (pXYZ[i], pSums->origin);
        currNormal.CrossProductToPoints (pXYZ[0], pXYZ[i - 1], pXYZ[i]);
        summedNormal.Add (currNormal);
        }

    double currArea = 0.5 * summedNormal.Normalize ();

    // Second triangle scan gathers signed contribution from each triangle.
    vectorA.DifferenceOf (pXYZ[0], pSums->origin);
    vectorB.DifferenceOf (pXYZ[1], pSums->origin);
    for (i = 2; i < numXYZ; i++, vectorB = vectorC)
        {
        vectorC.DifferenceOf (pXYZ[i], pSums->origin);
        accumulateTriangleAreaProperties (vectorA, vectorB, vectorC,
                    summedNormal,
                    pSums->perFaceAreaSum,
                    pSums->balanceMoments,
                    pSums->ixx,
                    pSums->iyy,
                    pSums->izz,
                    pSums->ixy,
                    pSums->ixz,
                    pSums->iyz
                    );
        }

    for (i = 0; i < numXYZ; i++)
        {
        MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, pNodeId[i]);
        if (jmdlMTGGraph_getMask (pGraph, mateNodeId, MTG_EXTERIOR_MASK))
            {
            pSums->perimeterSum += pXYZ[i].Distance (pXYZ[i+1]);
            }
        }


    if (currArea > 0.0 && numXYZ > 3)
        {
        double aMin, aMax;
        bsiDPoint3dArray_dotDifferenceRange (pXYZ, &summedNormal, pXYZ, numXYZ, aMin, aMax);
        double dist = aMax - aMin;
        if (dist > pSums->planarityError)
            pSums->planarityError = dist;
        }

    pSums->numFace++;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacets => facets
* @param pVolume <= volume
* @param pArea <= area
* @param pMaxPlanarityError <= max deviation of a face from planarity
* @param pCentroid <= centroid coordinates.
* @param pAxisMoment2 <= squared moments are x, y, z axes at centroid.
* @param pXYMoment <= xy moment at centroid.
* @param pXZMoment <= xz moment at centroid.
* @param pYZMoment <= yz moment at centroid.
* @return SUCCESS if all coordinates acccessible and no boundary edges.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_areaProperties
(
MTGFacets *pFacets,
double          *pArea,
double          *pPerimeter,
double          *pMaxPlanarityError,
DPoint3d        *pCentroid,
DVec3d          *pAxisMoment2,   // (yy+zz, xx+zz, xx+yy)
double          *pXYMoment,
double          *pXZMoment,
double          *pYZMoment
)
    {
    MomentContext sums;
    sums.clear ();
    bool    boolstat = jmdlMTGFacets_scanFaces (pFacets, accumulateAreaSumsForFace, &sums);

    if (!boolstat)
        sums.clear ();
    else
        boolstat = sums.translateMoments (2);

    if (pArea)
        *pArea = sums.perFaceAreaSum;
    if (pPerimeter)
        *pPerimeter = sums.perimeterSum;
    if (pCentroid)
        *pCentroid = sums.centroid;

    if (pAxisMoment2)
        *pAxisMoment2 = sums.spinMoments;

    if (pXYMoment)
        *pXYMoment = sums.ixy;
    if (pXZMoment)
        *pXZMoment = sums.ixz;
    if (pYZMoment)
        *pYZMoment = sums.iyz;

    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacetHeader    => facets
* @param pVolume <= volume
* @see
* @return SUCCESS if all coordiantes acccessible.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_volume
(
const MTGFacets    *pFacetHeader,
double              *pVolume
)
    {
    bool    boolstat = false;
    double volume;
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    int i;
    MTGNodeId node0Id, node1Id, node2Id;
    DPoint3d point0, point1, point2, origin;
    DPoint3d vector0, vector1, vector2;

    volume = 0.0;

    for (i = 0; jmdlEmbeddedIntArray_getInt (pFaceStart, &node0Id, i);i++)
        {
        if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader,
                                        &point0, node0Id))
            goto wrapup;
        if (i == 0)
            origin = point0;

        node2Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
        if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader,
                                        &point2, node2Id))
                goto wrapup;

        vector0.DifferenceOf (point0, origin);
        vector2.DifferenceOf (point2, origin);

        for (;;)
            {
            node1Id = node2Id;
            node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
            if (node2Id == node0Id)
                break;

            point1  = point2;
            vector1 = vector2;
            if (!jmdlMTGFacets_getNodeCoordinates (pFacetHeader,
                                        &point2, node2Id))
                goto wrapup;
            vector2.DifferenceOf (point2, origin);
            volume += vector0.TripleProduct (vector1, vector2);
            }
        }
    boolstat = true;
wrapup:
    jmdlEmbeddedIntArray_drop (pFaceStart);
    *pVolume = volume / 6.0;
    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader    => mesh to query
* @param pRange <= computed range.
* @return true if the vertex array has a well defined range.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getRange
(
const MTGFacets *pFacetHeader,
      DRange3d   *pRange
)
    {
    *pRange = DRange3d::From (pFacetHeader->vertexArrayHdr);
    return pFacetHeader->vertexArrayHdr.size () > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @param  pFacetHeader   => mesh to query
* @return number of vertices in the vertex array.  Note that this count may be larger
*       than the number of vertices observed visusuall because (a) some coordinate
*       may have been entered into the coordinate array but not referenced from the
*       facet connectivity, and/or (b) multiple edges at a common vertex may have
*       be created refering to multiple copies of the coordinates.=
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_getVertexCount
(
const MTGFacets *pFacetHeader
)
    {
    return jmdlEmbeddedDPoint3dArray_getCount ((&pFacetHeader->vertexArrayHdr));
    }

/*---------------------------------------------------------------------------------**//**
* Transform both the points and normals of a facet set.
* There are two options for handling normals.  If
* inverseTransposeEffects is true, normals are mutliplied by the
* inverse trasnspose of the matrix part of the transform.  (Why? Let
* U and V be an two vectors in the plane originally normal to the
* normal. (e.g. Two insurface tangents in model space.)  Prior to
* transformation, U dot N is zero.  Let UU and VV be the same vectors
* after the surface is transformed.  Then the zero dot product is
* preserved.)
* If inverseTransposeEffects is false, the normals are just multiplied
* directly by the matrix part.
*
* @param pFacetHeader    <=> facets to transform
* @param pTransform => transform to apply
* @param inverseTransposeEffects => true to have normals
*                                         multiplied by inverse transpose.  This
*                                         is the usual practice for viewing ops.
*                                         false to have normals directly
*                                         multiplied by the matrix part.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_transform
(
MTGFacets *             pFacetHeader,
const Transform *       pTransform,
bool                    inverseTransposeEffects
)
    {
    RotMatrix matrixPart, transpose, inverseTranspose;
    pTransform->Multiply (pFacetHeader->vertexArrayHdr, pFacetHeader->vertexArrayHdr);

    if (pFacetHeader->normalArrayHdr.size () > 0)
        {
        if (inverseTransposeEffects)
            {
            matrixPart.InitFrom(*pTransform);
            transpose = matrixPart;
            transpose.Transpose ();

            if (inverseTranspose.InverseOf (transpose))
                {
                for (auto &normal : pFacetHeader->normalArrayHdr)
                    inverseTranspose.Multiply (normal);
                }
            else
                {
                for (auto &normal : pFacetHeader->normalArrayHdr)
                    matrixPart.Multiply (normal);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Expand an (integer) mask into an ascii buffer, appropriate
*   for use in a printf statement.
* @param pBuffer <= buffer of up to 64 ascii characeters (32 bits, various spaces)
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGFacets_maskToAscii01
(
    char    *pBuffer,
MTGMask     mask
)
    {
    static char mnemonic[] =
            "11111111111111111111SHEDVBvuVUBX";
        /*  "84218421842184218421842184218421";     */
    int i, j;
    int k;
    for (i = j = 0; j < 32; j++)
        {
        k = 31 - j;
        pBuffer[i++] = (mask >> k )  & 0x01 ? mnemonic[j] : '_';
        if ((j & 0x07) == 0x07)
            pBuffer[i++] = ' ';
        }
    pBuffer[i] = 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Print all connectivity data for the graph.
* @param pFacetHeader => facet set
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printConnectivity
(
MTGFacets  *pFacetHeader
)
    {
    char maskBuffer[128];
    MTGGraph *pGraph = &pFacetHeader->graphHdr;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    int vertexIndex;
        printf("   (node   FSucc  VSucc  EMate   Vertex)\n");
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset);
        jmdlMTGFacets_maskToAscii01 (maskBuffer, jmdlMTGGraph_getMask (pGraph, currNodeId, MTG_ALL_MASK_BITS));
        printf("  (%5d %5df %5dv %5de  V%5d %s)\n",
                    currNodeId,
                    jmdlMTGGraph_getFSucc (pGraph, currNodeId),
                    jmdlMTGGraph_getVSucc (pGraph, currNodeId),
                    jmdlMTGGraph_getEdgeMate (pGraph, currNodeId),
                    vertexIndex,
                    maskBuffer
                    );
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }

static void getShortMaskString
(
const MTGGraph *pGraph,
char *pStringBuffer,
MTGNodeId nodeId,
MTGMask maskA,
char codeA
)
    {
    int numChar = 0;
    MTGMask mask = jmdlMTGGraph_getMask (pGraph, nodeId, MTG_ALL_MASK_BITS);
    if (mask & MTG_EXTERIOR_MASK)
        pStringBuffer[numChar++] = 'X';
    if (mask & MTG_PRIMARY_EDGE_MASK)
        pStringBuffer[numChar++] = 'E';
    if (mask & MTG_PRIMARY_VERTEX_MASK)
        pStringBuffer[numChar++] = 'V';
    if (mask & MTG_BOUNDARY_MASK)
        pStringBuffer[numChar++] = 'B';
    if (mask & maskA)
        pStringBuffer[numChar++] = codeA;
    pStringBuffer[numChar++] = 0x00;
    }

/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId vertexIndex) around faces
* @param pFacetHeader => facet set
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printFaceLoops (MTGFacets  *pFacetHeader)
    {
    jmdlMTGFacets_printFaceLoops (pFacetHeader, 0, ' ');
    }

/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId vertexIndex) around faces
* @param pFacetHeader => facet set
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printFaceLoops
(
MTGFacets  *pFacetHeader,
MTGMask maskA,
char    codeA
)
    {
    MTGGraph*   pGraph = &pFacetHeader->graphHdr;
    MTGMask     visitMask;
    char        stringBuffer[128];
    int         counter = 0, loopCount, vertexIndex, vertexIdOffset = pFacetHeader->vertexLabelOffset;
    int         internalIndex, internalIndexOffset = jmdlMTGFacets_getLabelOffset (pFacetHeader, MTGReservedLabelTag_InternalIndex);

    static int s_nodesPerLine = 6;

    printf("   (FaceLoops\n");

    visitMask = jmdlMTGGraph_grabMask (&pFacetHeader->graphHdr);
    jmdlMTGGraph_clearMaskInSet (&pFacetHeader->graphHdr, visitMask);

    loopCount = 0;
    MTGARRAY_SET_LOOP (faceNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, faceNodeId, visitMask))
            {
            printf("      (");
            counter = 0;
            loopCount++;
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
                {
                jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset);
                getShortMaskString (pGraph, stringBuffer, currNodeId, maskA, codeA);

                if (internalIndexOffset >= 0)
                    {
                    jmdlMTGGraph_getLabel (pGraph, &internalIndex, currNodeId, internalIndexOffset);
                    printf ("  (%d%s v%d i%d)", currNodeId, stringBuffer, vertexIndex, internalIndex);
                    }
                else
                    printf ("  (%d%s vs%d V%d)", currNodeId, stringBuffer, pGraph->VSucc (currNodeId), vertexIndex);

                if (++counter >= s_nodesPerLine)
                    {
                    counter = 0;
                    printf ("\n       +     ");
                    }
                jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)

            printf("\n");
            }
        }
    MTGARRAY_END_SET_LOOP (faceNodeId, pGraph)

    if (counter > 0)
        printf ("\n     ");
    printf("   ) // %d FaceLoops\n", loopCount);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }


/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId mateId farVertexIndex) around vertices
* @param pFacetHeader => facet set
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printVertexLoops
(
MTGFacets  *pFacetHeader
)
    {
    MTGGraph *pGraph = &pFacetHeader->graphHdr;
    MTGMask   visitMask;
    MTGNodeId  mateNodeId;
    DPoint3d xyz;
    int counter = 0;
    static int s_nodesPerLine = 4;
    int loopCount;


    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    int vertexIndex;
        printf("   (VertexLoops (vertex (nodeId mateId farVertexIndex)\n");

    visitMask = jmdlMTGGraph_grabMask (&pFacetHeader->graphHdr);
    jmdlMTGGraph_clearMaskInSet (&pFacetHeader->graphHdr, visitMask);

    loopCount = 0;
    MTGARRAY_SET_LOOP (vertNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, vertNodeId, visitMask))
            {
            jmdlMTGGraph_getLabel (pGraph, &vertexIndex, vertNodeId, vertexIdOffset);
            printf("  (%d v%d)",
                        vertNodeId,
                        vertexIndex
                        );
            counter = 0;
            loopCount++;
            xyz = pFacetHeader->vertexArrayHdr[(size_t)vertexIndex];
            printf ("(%lf,%lf,%lf):(node edgemate vertex)\n", xyz.x, xyz.y, xyz.z);

            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, vertNodeId)
                {
                mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                jmdlMTGGraph_getLabel (pGraph, &vertexIndex, mateNodeId, vertexIdOffset);

                printf(" (%d e%d v%d)",
                            currNodeId,
                            mateNodeId,
                            vertexIndex
                            );
                if (++counter >= s_nodesPerLine)
                    {
                    counter = 0;
                    printf ("\n           +");
                    }
                jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, vertNodeId)

            printf("\n");
            }
        }
    MTGARRAY_END_SET_LOOP (vertNodeId, pGraph)

    if (counter > 0)
        printf ("\n     ");
    printf("   ) // %d VertexLoops\n", loopCount);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }


/*----------------------------------------------------------------------------------*//**
* Return angle between incident edges at the node in [0,Pi) if no normal is
* given, otherwise use normal to return angle in [0,2Pi).
*
* @param        pFacets         => MTG facets
* @param        nodeId          => node at angle to measure
* @param        pNormal         => reference normal (or NULL)
* @return angle between incident edges
* @bsimethod                                                    DavidAssaf      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   jmdlMTGFacets_getAngleAtNode
(
const MTGFacets *pFacets,
MTGNodeId       nodeId,
const DPoint3d  *pNormal
)
    {
    DPoint3d    leftEdge, rightEdge, vert;
    double      angle = 0.0;

    if (   pFacets
        && nodeId != MTG_NULL_NODEID
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &vert, nodeId)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &rightEdge,
                jmdlMTGGraph_getFSucc (&pFacets->graphHdr, nodeId))
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &leftEdge,
                jmdlMTGGraph_getFPred (&pFacets->graphHdr, nodeId))
        )
        {
        rightEdge.Subtract (vert);
        leftEdge.Subtract (vert);

        if (pNormal)
            {
            angle = rightEdge.SignedAngleTo (leftEdge, *pNormal);
            if (angle < 0.0)
                {
                angle = msGeomConst_pi - angle;
                }
            }
        else
            {
            angle = rightEdge.AngleTo (leftEdge);
            }
        }
    return angle;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
