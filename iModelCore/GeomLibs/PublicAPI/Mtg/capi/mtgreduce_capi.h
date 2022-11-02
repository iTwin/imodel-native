/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* For every planar superface in a graph, mark edges (with bridge mask) that
* connect "hole" superfaces with their "containing" superface.
*
* On input, planar superfaces should be marked with the vertex/edge masks.
* All boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  Superfaces with the
* temp1 mask (to signify nonplanar faces) or with exterior mask are not
* processed.
*
* On output, planar superfaces with hole(s) are masked with the punctured face
* mask (this includes bridge edges and hole loops).
*
* @param    pGraph  IN OUT  connectivity
* @param pMasks     IN OUT  node masks used (in called functions):
*                           preset      : boundary, edge, exterior, temp1, vertex
*                           grabbed     : temp2, visited
*                           set         : (bridge), punctured, temp2, (visited)
*                           freed       : temp2, visited
* @return false if error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_createBridgesInSet
(
MTGGraph        *pGraph,
MTG_ReduceMasks *pMasks
);

/**
* Search the punctured superface loop at the given node for other superface
* loops attached by bridge edges and return the number of interior superfaces
* (holes) found.
*
* Optionally return a list of superedge nodes for the holes and (separately) a
* node on the exterior superface with the largest range box diagonal.
*
* On input, the input superface loop must be marked with the punctured face
* mask.  In addition, planar superfaces should be marked with the edge mask.
* All boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  It is assumed that
* punctured edges that are not edge-masked (bridges) form trees that:
* <UL>
* <LI> join the exterior superface to its interior superfaces (holes)
* <LI> are disjoint except possibly at their root
* <LI> may only touch the exterior superface loop at their root
* </UL>
*
* The input node is not guaranteed to be included among the output node(s).
*
* @param    pFacets     IN OUT  connectivity + geometry
* @param pHoleIds       OUT     one superedge nodeId per hole found (or NULL)
* @param pExtNodeId     OUT     superedge nodeId on exterior superface loop (or NULL)
* @param pMasks          IN      node masks used:
*                               preset  : boundary, edge, punctured
* @param nodeId          IN      node on start superface loop
* @see #jmdlMTGGraph_createBridgesInSet
* @return number of holes found or negative if error
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGFacets_collectAndNumberHolesInSuperFace
(
MTGFacets               *pFacets,   /* temporary visit mask grabbed, set, dropped */
EmbeddedIntArray        *pHoleIds,
MTGNodeId               *pExtNodeId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
);

/**
* Collect the coordinates of the vertices on the superface loop at the given
* node.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pFacets     IN OUT  connectivity + geometry
* @param pVerts         IN OUT  array to fill with coordinates
* @param nodeId          IN      node on the desired superface loop
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @return # vertices stored (= # superedges), or negative if error
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGFacets_getSuperFaceCoordinates
(
const MTGFacets         *pFacets,
EmbeddedDPoint3dArray   *pVerts,
MTGNodeId               nodeId,
MTGMask                 edgeMask,
MTGMask                 boundaryMask
);

/**
* Search the given node and its vertex loop for a node for which the given masks
* are present and absent.  Use zero if either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN      connectivity
* @param nodeId         IN      node in the vertex loop
* @param presentMask    IN      mask to filter returned node
* @param absentMask     IN      mask to reject returned node
* @param boundaryMask   IN      edge mask preset along MTG boundary (optional)
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundManifoldVertex
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         boundaryMask
);

/**
* Set the given mask around the vertex loop of the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the vertex loop
* @param mask            IN      mask to set
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setMaskAroundManifoldVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         boundaryMask
);

/**
* Clear the given mask around the vertex loop of the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the vertex loop
* @param mask            IN      mask to clear
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearMaskAroundManifoldVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         boundaryMask
);

/**
* Search the the superface loop containing the given node for a superedge node
* for which the given masks are present and absent.  Use zero if either mask is
* unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN      connectivity
* @param nodeId         IN      node in the superface loop
* @param presentMask    IN      mask to filter returned node
* @param absentMask     IN      mask to reject returned node
* @param edgeMask       IN      superedges have one or more of these masks
* @param boundaryMask   IN      edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findEdgeMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Sets the given mask around superedges in the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      edge mask to set
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Clears the given mask around superedges in the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      edge mask to clear
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Search the the superface loop containing the given node for a node for which
* the given masks are present and absent.  Use zero if either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN      connectivity
* @param nodeId         IN      node in the superface loop
* @param presentMask    IN      mask to filter returned node
* @param absentMask     IN      mask to reject returned node
* @param edgeMask       IN      superedges have one or more of these masks
* @param boundaryMask   IN      edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findFaceMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Set the given mask on all nodes of the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      face mask to set
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setFaceMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Clear the given mask on all nodes of the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      face mask to clear
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearFaceMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Search the the superface loop containing the given node for a supervertex
* containing a node for which the given masks are present and absent.  Use zero if
* either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN      connectivity
* @param nodeId         IN      node in the superface loop
* @param presentMask    IN      mask to filter returned node
* @param absentMask     IN      mask to reject returned node
* @param edgeMask       IN      superedges have one or more of these masks
* @param boundaryMask   IN      edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findVertexMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Sets the given mask around all supervertices in the superface loop containing
* the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      vertex mask to set
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setVertexMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Clears the given mask around all supervertices of the superface loop
* containing the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      vertex mask to clear
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearVertexMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Set the given mask around all edges at nodes (for which the given masks are
* present and absent) around supervertices of the superface loop containing the
* given node.  Use zero if either mask is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      edge mask to set
* @param presentMask     IN      mask to filter target nodes
* @param absentMask      IN      mask to reject target nodes
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeMaskAroundMaskedSuperFaceVertices
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Sets the given mask on all nodes for which the given masks are present/absent
* around the superface loop containing the given node.  Use zero if either mask
* is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      face mask to set
* @param presentMask     IN      mask to filter target nodes
* @param absentMask      IN      mask to reject target nodes
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setFaceMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Clear the given mask on all nodes for which the given masks are present/absent
* around the superface loop containing the given node.  Use zero if either mask
* is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      face mask to clear
* @param presentMask     IN      mask to filter target nodes
* @param absentMask      IN      mask to reject target nodes
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Sets the given mask on all nodes for which the given masks are present and absent
* around a supervertex in the superface loop containing the given node.  Use
* zero if either mask is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      IN OUT  connectivity
* @param nodeId          IN      node in the superface loop
* @param mask            IN      mask to set
* @param presentMask     IN      mask to filter target nodes
* @param absentMask      IN      mask to reject target nodes
* @param edgeMask        IN      superedges have one or more of these masks
* @param boundaryMask    IN      edge mask preset along MTG boundary (optional)
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setVertexMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
);

/**
* Drop edges that form trees in the graph.
* <P>
* Algorithm: start at leaf and delete edges that connect it to the next branch.
*
* @param    pGraph      IN OUT  topology
* @return # edges dropped
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlMTGGraph_dropTrees
(
MTGGraph    *pGraph
);

/**
* Create a superface set out of the given facet set by masking edges and
* vertices which separate unions of adjacent coplanar triangles.  The resultant
* superfaces are planar within the given angular tolerance.
*
* Bridge edges to holes in these superfaces will be masked with the given bridge
* mask (but not masked with the given edge/vertex masks).  Planar superfaces
* with hole(s) are masked with the punctured face mask (this includes bridge
* edges and hole loops).
*
* The coalesced superfaces are just faces if edges that don't receive either
* edge/bridge mask are dropped from the graph (as will be done if bCleanUp is
* true).
*
* On input, boundary-masked edges should demarcate the extent of manifold
* portions of the MTG.  If this is the case, superfaces will be guaranteed to
* consist of triangles from only one side of the MTG---the side without the
* exterior mask.  Use zero boundary mask if facet geometry is guaranteed to be
* 2-manifold.
*
* @param    pFacets     IN OUT  connectivity + geometry
* @param    pMasks      IN OUT  node masks used (in called functions):
*                               preset  : boundary, exterior
*                               grabbed : temp1
*                               set     : (bridge), edge, (punctured), temp1, vertex
*                               freed   : temp1
* @param    bCleanUp     IN      true to remove trees, non-superedges/bridges in MTG
* @param    eps2         IN      max sin^2(angle) allowed between coplanar normals
* @return false on error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_coalesceCoplanarTriangles
(
MTGFacets           *pFacets,
MTG_ReduceMasks     *pMasks,
bool                bCleanUp,
double              eps2
);

/**
* In the facet set of superedges (denoted by the given edge mask), detect and
* mask superedge cycles (not necessarily superfaces) which approximate circles.
*
* On input, superfaces should be marked with the edge mask.  All boundary-masked
* edges must also be superface edges; boundary masked edges demarcate the extent
* of manifold portions of the MTG.  Superfaces with exterior mask are not
* processed.
*
* On output, each circular superedge cycle is masked with the given circle
* (half-edge) mask.
*
* @param    pFacets     IN OUT  connectivity + geometry
* @param    pMasks      IN OUT  node masks used (in called functions):
*                               preset  : boundary, edge, exterior, punctured
*                               set     : (circle)
* @param    eps2         IN      min squared radian difference in angle between edges
* @see #jmdlMTGFacets_getMaskedCircleInfo
* @return false if error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_maskCircles
(
MTGFacets               *pFacets,
const MTG_ReduceMasks   *pMasks,
double                  eps2
);

/**
* Gets information on the circle approximated by the circular superedge cycle at
* nodeId.  The normal will be set closest to the optional vector.  The returned
* information can be used directly by mdlEllipse_create, for example (provided
* the matrix is correctly converted to MDL).
*
* On input, the circular superedge cycle should be masked with the circle
* (half-edge) mask.
*
* @param    pFacets      IN      connectivity + geometry
* @param    pCenter     OUT     center of circle (or NULL)
* @param    pRadius     OUT     radius of circle (or NULL)
* @param    pMatrix     OUT     normalized orientation (col[2] = normal) (or NULL)
* @param    pMasks       IN      node masks used:
*                               preset  : boundary, circle
* @param    nodeId       IN      node on circular superedge cycle
* @param    pDirection   IN      determines normal direction (or NULL)
* @see #jmdlMTGFacets_maskCircles
* @return false on error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getMaskedCircleInfo
(
const MTGFacets         *pFacets,
DPoint3d                *pCenter,
double                  *pRadius,
RotMatrix               *pMatrix,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId,
const DPoint3d          *pDirection
);

/**
* In the facet set in which circular superedge loops have been masked by
* jmdlMTGFacets_maskCircles, detect faces that form part of the curved surface
* of a generalized cylinder with (not necessarily parallel, congruent or both
* nondegenerate) circular caps.
*
* On input, superfaces should be marked with the edge mask and circular
* superedge cycles should be masked with the circle (half-edge) mask.  All
* boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  Superfaces with
* exterior mask are not processed.
*
* On output, the superedges between circular caps are masked with the cylsurf
* mask if they are determined to form a cylindrical surface between the caps.
*
* @param    pFacets     IN OUT  connectivity + geometry
* @param    pMasks      IN OUT  node masks used (in called functions):
*                               preset  : boundary, bridge, circle, (edge), exterior
*                               grabbed : temp1, temp2, visited
*                               set     : cylsurf, temp1, temp2, visited
*                               freed   : temp1, temp2, visited
* @see #jmdlMTGFacets_maskCircles
* @see #jmdlMTGFacets_getMaskedCylinderInfo
* @return false on error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_maskCylinders
(
MTGFacets           *pFacets,
MTG_ReduceMasks     *pMasks
);

/**
* Gets information on the (circular) cylindrical surface containing the
* cylsurf-masked superedge at nodeId.  The end vertices of this superedge are
* scanned for circle-masked nodes, and the first such node found defines the
* corresponding circular cap: it is important therefore that any circle-masked
* nodes at a vertex refer to the same circle.  If exactly one end vertex lacks a
* circle-masked node, information is returned for the corresponding cone.
*
* The translation from base to top cap is returned in the point parameters, so
* there is no translation component in the returned transform.  The returned
* information can be used directly by mdlSurface_project, for example (if the
* transform is correctly converted to MDL).
*
* Rule lines for the cylinder will be parallel to the shortest cylinder edge at
* the given node's vertex.
*
* @param    pFacets      IN      connectivity + geometry
* @param    pOrigin     OUT     center of base cap (or NULL)
* @param    pTarget     OUT     center of top cap (or NULL)
* @param    pTransform  OUT     transformation from base cap to top cap (or NULL)
* @param    pBaseId     OUT     circle-masked node at base cap (or NULL)
* @param    pTopId      OUT     circle-masked node at top cap or MTG_NULL_NODEID if cone (or NULL)
* @param    pMasks       IN      node masks used:
*                               preset  : boundary, circle, cylsurf
* @param    nodeId       IN      cylsurf-masked node on a cylinder face at the base cap
* @see #jmdlMTGFacets_maskCylinders
* @see #jmdlMTGFacets_maskCircles
* @return false on error, true otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getMaskedCylinderInfo
(
const MTGFacets         *pFacets,
DPoint3d                *pOrigin,
DPoint3d                *pTarget,
Transform               *pTransform,
MTGNodeId               *pBaseId,
MTGNodeId               *pTopId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
);

/**
* DEPRECATED
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_vertexHasAmbiguousCircle
(
const MTGGraph          *pGraph,
MTGNodeId               *pCapId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
);

/**
* DEPRECATED
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setMaskAroundMaskedVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         queryMask,
int             bState
);

/**
* DEPRECATED
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setHalfEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         manifoldMask
);

/**
* DEPRECATED
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearHalfEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         manifoldMask
);

/**
* DEPRECATED
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGGraph_collectAndNumberHolesInSuperFace
(
MTGFacets               *pFacets,
EmbeddedIntArray        *pHoleIds,
MTGNodeId               *pExtNodeId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
);

END_BENTLEY_GEOMETRY_NAMESPACE

