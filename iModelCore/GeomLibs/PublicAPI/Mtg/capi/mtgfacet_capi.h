/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Allocate a facet header to the caller.  If possible, the header
* is taken from a cache of headers that were previously used and hence
* may have preallocated memory for connectivity and coordinate data.
* Use jmdlMTGFacets_drop to return the header to the cache.
* @param void
* @see
* @return pointer to a borrowed facet header
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacets * jmdlMTGFacets_grab
(
void
);

/**
* Allocate a facet header from the system heap.
* @param void
* @see
* @return pointer to newly allocated and initialized facet header structure.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacetsP jmdlMTGFacets_new
(
void
);

/**
* Initialize a facet header structure.   Prior contents ignored and destroyed.
* @param pFacetHeader OUT     initialized structure.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_init
(
MTGFacets *pFacetHeader
);

/**
* Return a facet header to the system heap.
* @param     IN      Header for facets to free
* @see
* @return MTGFacets *
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGFacetsP jmdlMTGFacets_free
(
MTGFacetsP      pFacetHeader
);

/**
* Return the associated memory (but not the facet header itself) to
* the sytem heap.
* @param pFacetHeader    IN      header for facets to free.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_releaseMem
(
MTGFacets *     pFacetHeader
);

/**
* Set all counts (nodes, vertices, normals, parameter values) in a
* facet set to 0, but retain associated memory for possible reuse
* if the facet set is repopulated.
* @param    pFacetHeader
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_empty
(
MTGFacets *     pFacetHeader
);

/**
* Return a facet header to the facet cache.
* @param  pFacetHeader   IN      header to return
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_drop
(
MTGFacets *     pFacetHeader        /* IN      header to return */
);

/**
* Call this immediately after grab, new, empty, or releaseMem to
* indicate the type of geometry data that will be stored with the
* facets.
* @param pFacetHeader    IN OUT  Header whose normal mode is to be set.
* @param normalMode IN      Indicates how normal data is to be constructed.
* @param numVertex IN      approximate number of vertices.  May be zero.
* @param numNormal IN      approximate number of normals.  May be zero.
*                       Only applied if normals are separate from vertices.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_setNormalMode
(
MTGFacets *         pFacetHeader,
MTGFacets_NormalMode normalMode,
int                 numVertex,
int                 numNormal
);

/**
* Returns the underlying topology structure for the facets.
* @param pFacetHeader
* @see
* @return pointer to the connectivity graph for the facets.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGFacets_getGraph
(
MTGFacetsP pFacetHeader
);

/**

* @param pFacetHeader
* @param    userTag IN      * @param    labelType IN      * @param    defaultValue IN      * @see
* @return label offset.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_defineLabel
(
MTGFacets *         pFacetHeader,
int                 userTag,
MTGLabelMask        labelType,
int                 defaultValue
);

/**

* @param pFacetHeader
* @param userTag IN      * @see
* @return label offset for the given tag.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_getLabelOffset
(
const MTGFacets*    pFacetHeader,
int                 userTag
);

/**
* Add a vertex to a facet set.  If the facet set has normalMode
* MTG_Facets_NormalPerVertex, also add the corresponding normal.
* (Otherwise ignore the normal data.)
* @param pFacetHeader    IN OUT  Header whose normal mode is to be set.
*                           if normals are separate from vertices.
* @param pVertex    IN      vertex coordinates.
* @param pNormal    IN      normal coordinates.   Only used when facet set
*                             normal mode is MTG_Facets_NormalPerVertex.
* @see MTGFacets.IVertexFormat
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_addVertex
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal
);

/**
*
* Add to the normal array for a facet set.
* It is only valid to do this normalonly addition if the facet set
* is designated MTG_Facets_SeparateNormals.
*
* @param  pFacetHeader  IN OUT  Header whose normal mode is to be set.
*                       if normals are separate from vertices.
* @param pNormal    IN      normal coordinates.
* @see MTGFacets.IVertexFormat
* @return id of the newly added normal.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_addNormal
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pNormal
);

/* METHOD(default,none,addIndexedEdge)
/**
*
* @param pFacetHeader   IN OUT  facet set to receive new edge.
* @param nodeId0 IN      start node
* @param nodeId1 IN      end node
* @param mask0 IN      mask for start node
* @param mask1 IN      mask for end node
* @param vertexIndex0 IN      vertex index for start node
* @param vertexIndex1 IN      vertex index for end node
* @param normalIndex0 IN      normal index for start node. Ignored unless SeparateNormals facet type.
* @param normalIndex1 IN      normal index for end node. Ignored unless SeparateNormals facet type.

* @bsimethod
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
);

/*---------------------------------------------------------------------*//**
* @param pFacetHeader   IN OUT  facet set to receive new face.
* @param pVertex    IN      vertex array.
* @param pNormal    IN      normal array (NULL if not being used ..)
* @param numVertex  IN      number of vertices in loops.   First, last not doubled.
* @param labelTag   IN      tag for labels being attached.
* @param pLabel     IN      optional array of labels, indexed same as vertice.
* @param headLabel  IN      true if the label is to be at the head vertex, false for tail.
* @see
* @return id the node corresponding to vertex 0 of the array.
* @bsimethod
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
);

/**
* Add a face using addCoordinateFace, with no normals or other labels.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
* <P>
* Vertex coordinates are tested with the given tolerances to detect adjacent
* duplicates.
*
* @param pFacets    IN OUT  facet set to receive new face
* @param partnerLabelOffset IN      label offset where the through id is installed
* @param pXYZ               IN      vertex array
* @param numXYZ             IN      number of vertices
* @param absTol             IN      absolute tolerance for identical point test.
* @param relTol             IN      relative tolerance for identical point test.
* @return id of the node corresponding to vertex 0 of the array or MTG_NULL_NODEID
*   if face is degenerate.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addDoubleFaceTol
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const DPoint3d  *pXYZArray,
int             numXYZ,
double          absTol,
double          relTol
);

/**
* Add a face using addCoordinateFace, with no normals or other labels.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
* <P>
* Vertex coordinates are tested with default tolerances to detect adjacent
* duplicates.
*
* @param pFacets            IN OUT  facet set to receive new face
* @param partnerLabelOffset IN      label offset where the through id is installed
* @param pXYZ               IN      vertex array
* @param numXYZ             IN      number of vertices
* @return id of the node corresponding to vertex 0 of the array or MTG_NULL_NODEID
*   if face is degenerate.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addDoubleFace
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const DPoint3d  *pXYZArray,
int             numXYZ
);

/*---------------------------------------------------------------------*//**
* @param pFacetHeader   IN OUT  facet set to receive new face.
* @param pVertex    IN      vertex array.
* @param pNormal    IN      normal array (NULL if not being used ..)
* @param numVertex  IN      number of vertices in loops.   First, last not doubled.
* @param labelTag   IN      tag for labels being attached.
* @param pLabel     IN      optional array of labels, indexed same as vertice.
* @param headLabel  IN      true if the label is to be at the head vertex, false for tail.
* @see
* @return id the node corresponding to vertex 0 of the array.
* @bsimethod
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
);

/**
* Add a face, with no normals or other lables.
* Immediately traverse the face loop and install "through" ids leading from each
* node to its vertex successor (i.e. its partner on the other side of the
* double-sided face.)
*
* @param pFacets    IN OUT  facet set to receive new face
* @param partnerLabelOffset IN      label offset where the through id is installed
* @param pIndexArray        IN      vertex index array.
* @param numXYZ             IN      number of vertices
* @return id of the node corresponding to vertex 0 of the array
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_addIndexedDoubleFace
(
MTGFacets       *pFacets,
int             partnerLabelOffset,
const int       *pXYZIndexArray,
const int       *pNormalIndexArray,
int             numXYZ
);

/**
* Add a chain (polyline) to the graph.
* The entire chain is left as a nonexterior.
* @param pFacetHeader    IN OUT  facet set to receive new face
* @param pVertex IN      point array.
* @param pNormal IN      normal array
* @param numVertex IN      number of vertices
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_addCoordinateChain
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal,
int                 numVertex
);

/**
* Add a chain (polyline) to the graph.
* The entire chain is left as a nonexterior.
* @param pFacetHeader    IN OUT  facet set to receive new face
* @param pNodeIdArray OUT     array of one node id at each vertex.
* @param pVertex IN      point array.
* @param pNormal IN      normal array
* @param numVertex IN      number of vertices
* @return Node id at base of the chain.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGFacets_addCoordinateChainExt
(
MTGFacets *         pFacetHeader,
const DPoint3d      *pVertex,
const DPoint3d      *pNormal,
int                 numVertex
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlMTGFacets_splitEdge
(
        MTGFacets               *pFacetHeader,
        MTGNodeId               node0Id,
        DPoint3d                *pXYZ
);

/*---------------------------------------------------------------------------------**//**
* Copy coordinate data around a face into a flat array.  Indices from indicated label
*   of each node are used to access data in source array.
* @param pGraph   IN      topology structure
* @param pDestArray IN OUT  array to receive face coordinates
* @param pSoureArray IN      coordinate source array
* @param startId IN      any node on the face.
* @param offset  IN      index for label access.
* @return true if all coordinates were available in the source array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getFaceCoordinates
(
const MTGGraph *        pGraph,
EmbeddedDPoint3dArray           *pDestArray,
const EmbeddedDPoint3dArray   *pSourceArray,
MTGNodeId               startId,
        int             offset
);

/*---------------------------------------------------------------------------------**//**
* Copy vertex coordinates around a facet.
* @param pFacets IN      facet set
* @param pDestArray IN OUT  array to receive face coordinates
* @param startId IN      any node on the face.
* @param duplicateStartVertex IN      to request duplicate start/end coordinates
* @return true if all coordinates were available in the source array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getFacetCoordinates
(
const MTGFacets *       pFacets,
EmbeddedDPoint3dArray           *pDestArray,
MTGNodeId               startId,
bool                    duplicateStartVertex
);

/*---------------------------------------------------------------------------------**//**
* Get the (unnormalized) normal from a triangle.  If the face does not have
* exactly 3 nodes, return false and a zero normal.
* @param pFacets IN      facet set
* @param pNormal OUT     normal components.
* @param nodeId IN      id of any node on the triangle.
* @return true if the face has exactly 3 nodes.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool             jmdlMTGFacets_getTriangleNormal
(
const MTGFacets     *pFacets,
DPoint3d            *pNormal,
MTGNodeId           nodeId
);

/**
* Computes the (unnormalized) normal of the triangle.  To reduce roundoff error,
* the normal is computed at the largest interior angle of the triangle.  If the
* largest interior angle is too large, the normal is computed at one of the
* subangles formed by the median.
*
* @param pFacets    IN      facets set
* @param pNormal    OUT     normal (or NULL)
* @param pMag2      OUT     squared magnitude of normal (or NULL)
* @param pMaxNodeId OUT     ID of node at largest angle (or NULL)
* @param nodeId     IN      ID of node in triangle
* @param eps2       IN      minimum value of sin^2(angle) allowable in squared normal magnitude
* @return false if face not a triangle
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getSafeTriangleNormal
(
const MTGFacets     *pFacets,
DPoint3d            *pNormal,
double              *pMag2,
MTGNodeId           *pMaxNodeId,
MTGNodeId           nodeId,
double              eps2
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pPoint OUT     coordinates of given node.
* @param nodeId IN      node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeCoordinates
(
MTGFacetsCP                    pHeader,
DPoint3d                *pPoint,
MTGNodeId               nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pPoint IN      coordinates to set
* @param nodeId IN      node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_setNodeCoordinates
(
MTGFacetsP     pHeader,
const DPoint3d                *pPoint,
MTGNodeId               nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pNormal    OUT     normal at given node.
* @param nodeId     IN      node whose normal is returned.
* @return true if node id is valid and has normal data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeNormal
(
const MTGFacets *pHeader,
DPoint3d        *pNormal,
MTGNodeId       nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pPoint OUT     coordinates of given node.
* @param nodeId IN      node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeVertexIndex
(
MTGFacetsCP                    pHeader,
int                     *pVertexIndex,
MTGNodeId               nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pNormalIndex   OUT     index to normal
* @param nodeId         IN      node whose normal index is returned.
* @return true if node id is valid and has normal data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeNormalIndex
(
const MTGFacets *pHeader,
int             *pNormalIndex,
MTGNodeId       nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pHeader IN      facet set to access.
* @param pPoint OUT     parameter coordinates of given node.
* @param nodeId IN      node whose coordiantes are returned.
* @return true if node id is valid and has coordinate data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getNodeParametricCoordinates
(
const MTGFacets *       pHeader,
DPoint3d                *pPoint,
MTGNodeId               nodeId
);

/*---------------------------------------------------------------------------------**//**
* Return vertex indices and coordinates around a given face.
* @param pFacetHeader IN      facet set
* @param pVertexIndexArray OUT     buffer receiving vertex indices.  May be null.
* @param pVertexXYZArray OUT     buffer receiving vertex xyz coordinates.  May be null.
* @param pVertexNormalArray OUT     buffer receiving vertex normal. May be null.
* @param pVertexParam1Array OUT     buffer receiving vertex uvw coordinates. May be null.
* @param pVertexParam2Array OUT     buffer receiving vertex auxiliary data. May be null.
* @param pNumNode OUT     number of indices returned.
* @param maxVertex IN      max number of indices that can be placed in the buffer.
* @return true if the complete array was returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getVertexDataAroundFace
(
const   MTGFacets   *pFacetHeader,
        int         *pVertexIndexArray,
        DPoint3d    *pXYZArray,
        DPoint3d    *pNormalArray,
        DPoint3d    *pParam1Array,
        DPoint3d    *pParam2Array,
        int         *pNumVertex,
        int         maxVertex,
        MTGNodeId   startNodeId
);

/*---------------------------------------------------------------------------------**//**
* Copy coordinate data around a face into a flat array.  At each node, indicated label value
*       is index into source array.
* @param pGraph IN      source graph.
* @param pArray OUT     array of coordinates
* @param pNumVertex OUT     number of vertex coordinates retrieved
* @param    maxVertex   IN      size of receiving buffer.
* @param pSourceArray   IN      vertex coordinate array.
* @param startId        IN      node id for first node on face
* @param offset         IN      offset to label.
* @return true if face index is valid.
* @bsimethod
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
);

/*---------------------------------------------------------------------------------**//**
* @param pDestFacets OUT     destination facet set
* @param pSourceFacets  IN      source facet set
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_copy
(
MTGFacets *     pDestFacets,
const MTGFacets *    pSourceFacets
);

/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader   IN      mesh to test
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_isEmptyFacetSet
(
const   MTGFacets * pFacetHeader
);

/*---------------------------------------------------------------------------------**//**

* @param pFacetHeader    IN      mesh to test
* @see
* @return
*                   0 if no normals or can't find a face with 3 or 4 nodes
*                   1 if normals appear to be outward from CCW faces
*                   -1 if normals appear to be inward to CCW faces
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_checkNormalOrientation
(
const MTGFacets *               pFacetHeader
);

/*---------------------------------------------------------------------------------**//**
* Reverse the orientation of the topology.  If normals are present,
* reverse them.
* @param pFacetHeader    IN OUT  facets whose normals and face loop order are to be reversed.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_reverseOrientation
(
MTGFacets *             pFacetHeader
);

/*---------------------------------------------------------------------------------**//**
* For each face, assemble nodeId and coordinates into arrays and pass to a callback function.

* @param pFacets    IN      facets
* @param faceFunc IN      function to call per face.
    faceFunc (  MTGFacets *pFacets,
                EmbeddedIntArray *pNodeIdArray,
                EmbeddedDPoint3dArray *pXYZArray,
                void *vpContext);
     false return from faceFunc terminates the scan.
* @param vpContext IN context pointer for callback.
* @param returns true if callback returned true for all faces.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_scanFaces
(
MTGFacets *pFacets,
MTGFacets_FaceScanFunction  faceFunc,
void            *vpContext
);

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacets IN      facets
* @param pVolume OUT     volume
* @param pArea OUT     area
* @param pMaxPlanarityError OUT     max deviation of a face from planarity
* @param pCentroid OUT     centroid coordinates.
* @param pAxisMoment2 OUT     squared moments are x, y, z axes at centroid.
* @param pXYMoment OUT     xy moment at centroid.
* @param pXZMoment OUT     xz moment at centroid.
* @param pYZMoment OUT     yz moment at centroid.
* @return SUCCESS if all coordinates acccessible and no boundary edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_volumeProperties
(
MTGFacets *pFacets,
double          *pVolume,
double          *pArea,
double          *pMaxPlanarityError,
DPoint3d        *pCentroid,
DVec3d          *pAxisMoment2,   /* (yy+zz, xx+zz, xx+yy) */
double          *pXYMoment,
double          *pXZMoment,
double          *pYZMoment
);

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacets IN      facets
* @param pVolume OUT     volume
* @param pArea OUT     area
* @param pMaxPlanarityError OUT     max deviation of a face from planarity
* @param pCentroid OUT     centroid coordinates.
* @param pAxisMoment2 OUT     squared moments are x, y, z axes at centroid.
* @param pXYMoment OUT     xy moment at centroid.
* @param pXZMoment OUT     xz moment at centroid.
* @param pYZMoment OUT     yz moment at centroid.
* @return SUCCESS if all coordinates acccessible and no boundary edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlMTGFacets_areaProperties
(
MTGFacets *pFacets,
double          *pArea,
double          *pPerimeter,
double          *pMaxPlanarityError,
DPoint3d        *pCentroid,
DVec3d          *pAxisMoment2,   /* (yy+zz, xx+zz, xx+yy) */
double          *pXYMoment,
double          *pXZMoment,
double          *pYZMoment
);

/*---------------------------------------------------------------------------------**//**
* Compute volume of (closed) facets by summing tetrahedra between
* an arbitrary point and each face.
* @param pFacetHeader    IN      facets
* @param pVolume OUT     volume
* @see
* @return SUCCESS if all coordiantes acccessible.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_volume
(
const MTGFacets    *pFacetHeader,
double              *pVolume
);

/*---------------------------------------------------------------------------------**//**
*
* Find planar and non planar faces.  Set and clear designated masks
* on each.   Either mask may be zero.
*
* @param pFacetHeader    IN OUT  facets
* @param planarMask IN      mask to apply on planar faces
* @param nonPlanarMask IN      mask to apply on non-planar faces.
* @param tol IN      tolerance for nonplanar vertices.
*                 If zero, something near machine precision is
*                       used as a relative error.
* @see
* @return number of non-planar faces.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_markPlanarFaces
(
MTGFacets *             pFacetHeader,
MTGMask         planarMask,
MTGMask         nonPlanarMask,
double                  tol
);

/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader    IN      mesh to query
* @param pRange OUT     computed range.
* @return true if the vertex array has a well defined range.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFacets_getRange
(
const MTGFacets *pFacetHeader,
      DRange3d   *pRange
);

/*---------------------------------------------------------------------------------**//**
* @param  pFacetHeader   IN      mesh to query
* @return number of vertices in the vertex array.  Note that this count may be larger
*       than the number of vertices observed visusuall because (a) some coordinate
*       may have been entered into the coordinate array but not referenced from the
*       facet connectivity, and/or (b) multiple edges at a common vertex may have
*       be created refering to multiple copies of the coordinates.=
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGFacets_getVertexCount
(
const MTGFacets *pFacetHeader
);

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
* @param pFacetHeader    IN OUT  facets to transform
* @param pTransform IN      transform to apply
* @param inverseTransposeEffects IN      true to have normals
*                                         multiplied by inverse transpose.  This
*                                         is the usual practice for viewing ops.
*                                         false to have normals directly
*                                         multiplied by the matrix part.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_transform
(
MTGFacets *             pFacetHeader,
TransformCP             pTransform,
bool                    inverseTransposeEffects
);

/*---------------------------------------------------------------------------------**//**
* @description Print all connectivity data for the graph.
* @param pFacetHeader IN      facet set
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printConnectivity
(
MTGFacets  *pFacetHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId vertexIndex) around faces
* @param pFacetHeader IN      facet set
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printFaceLoops
(
MTGFacets  *pFacetHeader
);
/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId vertexIndex) around faces
* @param pFacetHeader IN      facet set
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printFaceLoops
(
MTGFacets  *pFacetHeader,
MTGMask mask,
char maskChar
);
/*---------------------------------------------------------------------------------**//**
* @description Print (nodeId mateId farVertexIndex) around vertices
* @param pFacetHeader IN      facet set
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_printVertexLoops
(
MTGFacets  *pFacetHeader
);

/*----------------------------------------------------------------------------------*//**
* Return angle between incident edges at the node in [0,Pi) if no normal is
* given, otherwise use normal to return angle in [0,2Pi).
*
* @param        pFacets         IN      MTG facets
* @param        nodeId          IN      node at angle to measure
* @param        pNormal         IN      reference normal (or NULL)
* @return angle between incident edges
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   jmdlMTGFacets_getAngleAtNode
(
const MTGFacets *pFacets,
MTGNodeId       nodeId,
const DPoint3d  *pNormal
);

END_BENTLEY_GEOMETRY_NAMESPACE

