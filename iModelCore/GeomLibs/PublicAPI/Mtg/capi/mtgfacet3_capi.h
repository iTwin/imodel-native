/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader IN      facet set to search.
* @param pProj OUT     nearest point
* @param pStartPoint OUT     edge start
* @param pEndPoint OUT     edge end
* @param pLambda OUT     fractional coordinate along edge
* @param pMinDist OUT     in-plane distance to point
* @param piNode OUT     base mtg node of picked edge
* @param pPickPoint IN      pick point
* @param aperture IN      maximum distance to consider
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateEdgePick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pProj,
        DPoint3d    *pStartPoint,
        DPoint3d    *pEndPoint,
        double      *pLambda,
        double      *pMinDist,
        int         *piNode,
const   DPoint3d    *pPickPoint,
        double      aperture
);

/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader    IN      mesh to pick
* @param pPierce        OUT     pierce point with max z
* @param pFaceNodeId    OUT     any node on the face
* @param pPickPoint     IN      pick point
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateConvexFacetPick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pPierce,
        MTGNodeId  *pFaceNodeId,
const   DPoint3d    *pPickPoint
);

/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader   IN      mesh to test
* @param pNearPoint OUT     nearest point
* @param pMinDist OUT     in-plane distance to point
* @param piVertex OUT     vertex index
* @param pPickPoint IN      pick point
* @param aperture IN      maximum distance to consider
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateVertexPick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pNearPoint,
        double      *pMinDist,
        int         *piVertex,
const   DPoint3d    *pPickPoint,
        double      aperture
);

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* coordianates.
* @param pFacetHeader    IN      mesh to output
* @param pVertexFunc IN      output function for faces without normals
* @param pVertexAndNormalFunc IN      output function for faces with normal data
* @param useNormalsIfAvailable IN      true to output normal data when available
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitFloatLoops
(
const MTGFacets *    pFacetHeader,
MTG_floatFaceFunction   pVertexFunc,
MTG_floatFaceFunction   pVertexAndNormalFunc,
bool                    useNormalsIfAvailable
);

/*---------------------------------------------------------------------------------**//**
* Visit all interior faces.  Subdivide each to triangles.  Pack coordiantes into
* output arrays.
* Warning: Triangulation may assume convex faces.
* @param pFacetHeader    IN      mesh to output
* @param pDestPointArray OUT     array of triangle vertex coordinates
* @param pDestNormalArray OUT     array of triangle normal coordinates
* @param pDestParamArray OUT     array of triangle vertex parameters
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectPackedCoordinateTriangles
(
const MTGFacets         *pFacetHeader,
      EmbeddedDPoint3dArray   *pDestPointArray,
      EmbeddedDPoint3dArray   *pDestNormalArray,
      EmbeddedDPoint3dArray   *pDestParam1Array,
      EmbeddedDPoint3dArray   *pDestParam2Array
);

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* coordianates with RGB's.
* @param pFacetHeader    IN      mesh to output
* @param pFunc IN      output function f(xyzArray, rgbArray, numVertex, pUserData)
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitFloatRGBLoops
(
const MTGFacets *    pFacetHeader,
MTG_floatXYZRGBFunction  pFunc,
void    *pUserData
);

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* integer indices taken from the "vertex label" from the nodes.
* @param pFacetHeader    IN      mesh to emit
* @param reverseLoops IN      true to emit loops in reverse order
* @param pIndices IN      array of alternate indices.
* @param numIndex IN      number of alternate indices
* @param pFunc IN      output function
* @param userData IN      extra arg for output function
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitReindexedLoops
(
const MTGFacets *           pFacetHeader,
      bool                  reverseLoops,
      int                   *pIndices,
      int                   numIndex,
MTG_indexedFaceFunction     pFunc,
      int                   userData
);

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* integer points, under callersupplied transformation.
* @param pFacetHeader    IN      mesh to emit
* @param pStartPolyFunc IN      function to start polygon
* @param pAddLoopFunc IN      function to add loop to polygon
* @param pFinishPolyFunc IN      function to end polygon
* @param pUserData1 IN      arg for 'finish' function
* @param pUserData2 IN      arg for 'finish' function
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitVArrayDPoint3dLoops
(
const MTGFacets *               pFacetHeader,
MTG_startPolygonFunction        pStartPolyFunc,
MTG_addLoopToPolygonFunction    pAddLoopFunc,
MTG_finishPolygonFunction       pFinishPolyFunc,
        void                    *pUserData1,
        void                    *pUserData2
);

/*---------------------------------------------------------------------------------**//**
* Assemble a face from signed, one-based indices into a vertexToNode array.
*
* @param        pVertexToNodeId_hdr IN OUT  indices from vertices to any nodeId in the vertex loops.
* @param        pVertexAroundFace IN      array of vertex indices around the face.
* @param        numVertexAroundFace IN      number of vertices around the face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_addSignedIndexedFace
(
MTGFacets  *pFacetHeader,
EmbeddedIntArray  *pVertexToNodeId_hdr,
const int         *pVertexAroundFace,
int         numVertexAroundFace
);

/*---------------------------------------------------------------------------------**//**
* Search the param1 array for all vertices that fall within a specified parametric circle
* @param    pFacets IN OUT  Containing graph
* @param    pIntArray OUT     array of integer vertex id's.
* @param        u IN      center u of search circle.
* @param        v IN      center v of search circle.
* @param        r IN      radius of search circle.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_searchVertexIdByParameterCircle
(
MTGFacets               *pFacets,
EmbeddedIntArray                *pIntArray,
double                  u,
double                  v,
double                  r
);

/*---------------------------------------------------------------------------------**//**
* Search the param1 array for all vertices that fall within a specified
* spatial sphere.
* @param    pFacets IN OUT  Containing graph
* @param    pIntArray OUT     array of integer vertex id's.
* @param    pCenter IN      center of sphere.
* @param        r IN      radius of search circle.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_searchVertexIdByXYZSphere
(
MTGFacets               *pFacets,
EmbeddedIntArray                *pIntArray,
const DPoint3d          *pCenter,
double                  r
);

/*---------------------------------------------------------------------------------**//**
* Return the index of any triangular interior face which contains the specified
* uv parametric coordinates.
* @param    pFacets IN OUT  Containing graph
* @param    pBarycentric IN      barycentric coordinates within triangle.  May be NULL.
* @param    pNode0Id IN      first node id on triangle.  May be NULL.
* @param    pNode1Id IN      second node id on triangle.  May be NULL.
* @param    pNodeId2 IN      third node id on triangle.  May be NULL.
* @param    pParam0 IN      parametric coordinates of first vertex.  May be NULL.
* @param    pParam1 IN      parametric coordinates of second vertex.  May be NULL.
* @param    pParam2 IN      parametric coordinates of third vertex.  May be NULL.
* @param        u IN      center u of search point.
* @param        v IN      center v of search point.
* @return true if a triangle was located.  If false, output valuees are left uninitialized.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_searchTriangleByParameter
(
MTGFacets       *pFacets,
DPoint3d        *pBarycentric,
MTGNodeId       *pNode0Id,
MTGNodeId       *pNode1Id,
MTGNodeId       *pNode2Id,
DPoint3d        *pParam0,
DPoint3d        *pParam1,
DPoint3d        *pParam2,
int             *pVertex0Index,
int             *pVertex1Index,
int             *pVertex2Index,
double          u,
double          v
);

/*---------------------------------------------------------------------------------**//**
* Regularize the graph using xy coordinates in the vertex array.
* @param   pFacets  IN OUT  Containing facets
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_regularizeXY
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Regularize the graph using uv coordinates in the parameter array.
* @param   pFacets  IN OUT  Containing facets
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_regularizeUV
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Triangulate the graph using xy coordinates in the vertex array.
* @param  pFacets   IN OUT  Containing facets
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_triangulateXY
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Triangulate the graph using uv coordinates in the parameter array.
* @param  pFacets   IN OUT  Containing facets
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_triangulateUV
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xy coordinates
* in the vertex array.
* @param  pFacets   IN OUT  Containing facets
* @returns number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipXY
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.
* @param  pFacets   IN OUT  Containing facets
* @returns number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipXYZ
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to reduce angles across edges.
* @param  pFacets   <=> Containing facets
* @returns number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipDihedral (MTGFacets *pFacets);

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.   Only "ruled" edges are flipped -- must run from one
* rail to another both before and after flip.
* @param  pFacets   IN OUT  Containing facets
* @param railMask IN      mask identifying rail edges.
* @returns number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipRuledXYZ
(
MTGFacets               *pFacets,
MTGMask                 railMask
);

/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using uv coordinates
* in the vertex array.
* @param  pFacets   IN OUT  Containing facets
* @returns number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipUV
(
MTGFacets               *pFacets
);

/*---------------------------------------------------------------------------------**//**
* Use UV area to identify true exterior faces; start recursive traversal
* from each negative area face to set exterior masks according to parity
* rules as boundary edges are crossed.
* @param  pFacets   IN OUT  Containing facets
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_setUVExteriorMasks
(
MTGFacets               *pFacets
);

/*----------------------------------------------------------------------------------*//**
* Search nodes at the vertex of the given node for an edge ending at the vertex
* at which oppNode lies.  If thruOffset is 0, the search only considers the
* immediate vertex loop; otherwise, adjacent vertex loops are also searched,
* up to the maximum recursion nesting level given in maxDepth.
*
* @param        pFacets         IN      MTG Facets
* @param        nodeId          IN      node at base vertex
* @param        oppNodeId       IN      node at opposite vertex
* @param        tol2            IN      square of absolute tolerance for comparing opposite vertex coords
* @param        thruOffset      IN      label offset to "other side" of face (or 0)
* @param        maxDepth        IN      maximum nested calls in recursion (if thruOffset > 0)
* @return base nodeId of edge if found, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_findEdgeAtVertexCloud
(
const MTGFacets *pFacets,
MTGNodeId       nodeId,
MTGNodeId       oppNodeId,
double          tol2,
int             thruOffset,
int             maxDepth
);

/*----------------------------------------------------------------------------------*//**
* Compares given vertices to those of both faces attached at this edge.  The
* nodeId output on a successful return corresponds to the first given vertex,
* even when those vertices match the MTG face in the opposite orientation.
*
* @param        pFacets     IN      MTG Facets
* @param        pbFlipped   OUT     true if match found to reversed vertex order (or NULL)
* @param        pNodeId     IN OUT  edge node (on return, may be vertex predecessor)
* @param        pXYZ        IN      vertex coords to compare
* @param        numXYZ      IN      # vertices to compare
* @param        tol2        IN      square of absolute tolerance for comparing vertex coords
* @param        presentMask IN      one of these masks must be present on matched face (or zero)
* @param        absentMask  IN      none of these masks must be present on matched face (or zero)
* @return true if a match was found
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_matchMaskedFaceAtEdge
(
const   MTGFacets*  pFacets,
        bool*       pbFlipped,
        MTGNodeId*  pNodeId,
const   DPoint3d*   pXYZ,
        int         numXYZ,
        double      tol2,
        MTGMask     presentMask,
        MTGMask     absentMask
);

/*----------------------------------------------------------------------------------*//**
* Compares given vertices to those of both faces attached at this edge.  The
* nodeId output on a successful return corresponds to the first given vertex,
* even when those vertices match the MTG face in the opposite orientation.
*
* @param        pFacets     IN      MTG Facets
* @param        pbFlipped   OUT     true if match found to reversed vertex order
* @param        pNodeId     IN OUT  edge node (on return, may be vertex predecessor)
* @param        pXYZ        IN      vertex coords to compare
* @param        numXYZ      IN      # vertices to compare
* @param        tol2        IN      square of absolute tolerance for comparing vertex coords
* @return true if a match was found
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_matchFaceAtEdge
(
const MTGFacets *pFacets,
bool            *pbFlipped,
MTGNodeId       *pNodeId,
const DPoint3d  *pXYZ,
int             numXYZ,
double          tol2
);

/*----------------------------------------------------------------------------------*//**
"Cut the corner" at every vertex.
Introduces a new face at each vertex.  Each edge receives two new points, at specified
  fractional distance from the ends.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_cutCorners
(
MTGFacets *pFacets,
double fraction
);

/*----------------------------------------------------------------------------------*//**
Quadratic style subdivsion
Expand each vertex to a face (same degree as vertex), and each edge to a quad face.
Place new vertices midway towards centroid of face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_subdivideA
(
MTGFacets *pFacets
);

/*----------------------------------------------------------------------------------*//**
Catmull Clark subdivision
Expand each vertex to a face (same degree as vertex), and each edge to a quad face.
Place new vertices midway towards centroid of face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_subdivideCatmullClark
(
MTGFacets *pFacets
);

//! For each point in points[]:
//! 1) find a containing facet in top view.
//! 2) follow gravity flow line.
//! 3) Add flowline points to flowLines.   Terminate each such linestring
//!     with a disconnect.
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectFlowPaths
(
MTGFacetsP facets,
DPoint3dCP points,
int   numPoints,
EmbeddedDPoint3dArrayP flowLines
);

END_BENTLEY_GEOMETRY_NAMESPACE

