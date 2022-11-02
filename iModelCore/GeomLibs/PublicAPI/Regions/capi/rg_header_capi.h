/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Allocate a new region header.  This should be returned to jmdlRG_free
* @return pointer to region header.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP RG_Header    *jmdlRG_new
(
void
);

/*---------------------------------------------------------------------------------**//**
* Free a region header and all associated memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_free
(
RG_Header       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Set the context pointer passed back to all curve callbacks.
* @param pContext IN      application-specific pointer arg.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setFunctionContext
(
RG_Header   *pRG,
RIMSBS_Context   *pContext
);

/*---------------------------------------------------------------------------------**//**
* Get the context pointer passed back to all curve callbacks.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP RIMSBS_Context   * jmdlRG_getFunctionContext
(
RG_Header   *pRG
);

/*---------------------------------------------------------------------------------**//**
* Set a static flag to generate output during merge.
* @param pContext IN      application-specific pointer arg.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setNoisy
(
int noisy
);

/*---------------------------------------------------------------------------------**//**
* Return the static flag for debug output.
* @param pContext IN      application-specific pointer arg.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlRG_getNoisy
(
);

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setOutputLinestringFunction
(
RG_Header   *pRG,
RG_OutputLinestring pFunc,
void *pDebugContext
);

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_debugLinestring
(
RG_Header   *pRG,
const DPoint3d *pPoints,
int numPoints,
int color,
int weight,
int     drawmode
);

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_debugSegment
(
RG_Header   *pRG,
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
int color,
int weight,
int     drawmode
);

/*---------------------------------------------------------------------------------**//**
* Set the function pointer for debug linestrings.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_debugCircle
(
RG_Header   *pRG,
const DPoint3d *pCenter,
double radius,
int    color,
int    weight,
int     drawmode
);

/*---------------------------------------------------------------------------------**//**
* Set the minimum tolerance for the region graph.   Actual tolerance may be larger
* to accomodate large data ranges.
* @param tolerance IN      small distance.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setDistanceTolerance
(
RG_Header   *pRG,
double      tolerance
);

/*---------------------------------------------------------------------------------**//**
* Set the abort function to be called during long computations.
* @param abortFunction IN      (native) test function.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setAbortFunction
(
RG_Header   *pRG,
RGC_AbortFunction abortFunction
);

/*---------------------------------------------------------------------------------**//**
* Check if an abort has been requested, either (a) as recorded in the header or
* (b) as returned by a call to the saved abort function.
* @param [in] pRG regions context.
* @param [in] period if nonzero, abort check only happens when
   the internal counter is a multiple of the period.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlRG_checkAbort
(
RG_Header   *pRG,
int period = 0
);

/*---------------------------------------------------------------------------------**//**
* Test and set the abort flag.
* @param newValue IN      new abort flag.
* @return prior abort flag.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlRG_setAbortFlag
(
RG_Header   *pRG,
bool        newValue
);

/*---------------------------------------------------------------------------------**//**
* Return the range of the graph, as recorded by observing all incoming geometry.  Because
* this range is not recomputed if geometry is deleted, it may be larger than the
* actual geometry range after deletions.
*
* @param pRange IN      Range recorded with the graph.
* This may be larger than the actual range if extremal geometry was removed.
* Return true if the range is well defined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_getRange
(
RG_Header   *pRG,
DRange3d    *pRange
);

/*---------------------------------------------------------------------------------**//**
* If possible, return a transform whose xy plane contains the geometry.
* @param pRG IN      region context.
* @param pTransform OUT     returned transform.
* @param pRange OUT     range of data in transformed system.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_getPlaneTransform
(
RG_Header   *pRG,
Transform   *pTransform,
DRange3d    *pRange
);

/*---------------------------------------------------------------------------------**//**
* Multiply geometry (i.e. vertices and curves) by a transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_multiplyByTransform
(
RG_Header   *pRG,
Transform   *pTransform
);

/*---------------------------------------------------------------------------------**//**
* Resolve "any" node around a face to a consistent reference node of the face.
* @param seedNodeId IN      starting node for search.
* @return reference nodeId.  As long as the face is not modified, the same
*           reference node will be returned from any seed node.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlRG_resolveFaceNodeId
(
RG_Header   *pRG,
MTGNodeId   seedNodeId
);

/*---------------------------------------------------------------------------------**//**
* Save functions to be called for geometric computations on edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setCurveFunctions
(
RG_Header                           *pRG,
RGC_GetCurveRangeFunction           getCurveRange,
RGC_EvaluateCurveFunction           evaluateCurve,
RGC_IntersectCurveCurveFunction     intersectCurveCurve,
RGC_IntersectSegmentCurveFunction   intersectSegmentCurve,
RGC_CreateSubcurveFunction          createSubcurve,
RGC_SweptCurvePropertiesFunction    sweptCurveProperties,
RGC_EvaluateDerivativesFunction     evaluateDerivatives,
RGC_GetClosestXYPointOnCurveFunction    getClosestXYPointOnCurve,
RGC_IntersectCurveCircleXYFunction  intesectCurveCircleXY,
RGC_GetGroupIdFunction              getGroupId,
RGC_ConsolidateCoincidentGeometryFunction     consolidateCoincidentGeometry
);

/*---------------------------------------------------------------------------------**//**
* Save functions to be called for geometric computations on edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_setCurveFunctions01
(
RG_Header                           *pRG,
RGC_AppendAllCurveSamplePointsFunction appendAllCurveSamplePoints,
RGC_TransformCurveFunction          transformCurve,
RGC_TransformAllCurvesFunction      transformAllCurves
);

/*---------------------------------------------------------------------------------**//**
* Call at conclusion of defining an edge.  Updates the graph's range box to include the
* edge, and enters the edge into the incremental range tree (if the range tree is active).
* @param nodeId IN      node on the edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_completeEdgeEntry
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Call along with other header initialization steps to indicate that edge
* range tree is to be maintained incrementally.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlRG_enableIncrementalEdgeRangeTree
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN OUT  region context
* @param pChainEnds OUT     node ids at start, end of new chain
* @param pPointArray IN      array of polyline points.
* @param numPoint IN      number of points.
* @param parentId IN      label to store with all edges.
* @param leftMask IN      left-side mask
* @param rightMask IN      right-side mask
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_addMaskedLinear
(
RG_Header       *pRG,
MTGNodeIdPair   *pChainEnds,
DPoint3dCP      pPointArray,
int             numPoint,
int             parentId,
MTGMask         leftMask,
MTGMask         rightMask
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN OUT  region context
* @param pChainEnds OUT     node ids at start, end of new chain
* @param curveId IN      curve id (preexisting) to reference from the edge.
* @param parentId IN      label to store with all edges.
* @param leftMask IN      left-side mask
* @param rightMask IN      right-side mask
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_addMaskedCurve
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pChainEnds,
RG_CurveId                      curveId,
RG_CurveId                      parentId,
MTGMask                 leftMask,
MTGMask                 rightMask
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN      region header
* @param pPointArray IN      linestring coordinates
* @param numPoint IN      number of points.
* @param closed IN      if true, first and last node id's are joined topologically.
* @param parentId IN      label to store with all edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_addLinear
(
RG_Header                       *pRG,
DPoint3dCP                      pPointArray,
int                             numPoint,
bool                            closed,
int                             parentId
);

/*---------------------------------------------------------------------------------**//**
* Add a curve (defined by its curve id)
* @param pRG IN      region header
* @param curveId IN      id of (preexisting) curve.
* @param closed IN      if true, first and last node id's are joined topologically.
* @param parentId IN      label to store with all edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId       jmdlRG_addCurve
(
RG_Header                       *pRG,
RG_CurveId                      curveId,
RG_CurveId                      parentId
);

/*---------------------------------------------------------------------------------**//**
* Prepare for input sequence using "current pair" context.
* Pairing logic aids construction of closed loops from curve sequences which
* are in order but have no prior indication of when there is a jump from one loop to the
* next.  During pairing, a "current pair" indicates the start and end nodes of the
* current evolving open path.   As new edges are added, comparison to the current pair
* coordinates bulid extended paths.
* @param => region context.
* @param IN      pair to initialize.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_initPairing
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pPair
);

/*---------------------------------------------------------------------------------**//**
* Test if head of pair0 has same coordinates as tail of pair1; if so, join
* the pairs.
* @param pRG IN      region context
* @param pMergedPair OUT     combined pair.
* @param pPair0 IN      old pair
* @param pPair1 IN      new edge or path to be joined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_joinPairs
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pMergedPair,
const   MTGNodeIdPair           *pPair0,
const   MTGNodeIdPair           *pPair1
);

/*---------------------------------------------------------------------------------**//**
* "Close" a pairing sequence by joining its ends.
* @param pRG IN      region header.
* @param pPair IN      start / end description of completed path (to be closed if
*           start and end coordiantes match).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_closePairing
(
RG_Header                       *pRG,
MTGNodeIdPair                   *pPair
);

/*---------------------------------------------------------------------------------**//**
* Query the graph structure part of the region context.
* @instace pRG IN      region context.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraph        *jmdlRG_getGraph
(
RG_Header       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Evaluate an edge, given as edge data, at a parameter.
* @param pRG IN      region context
* @param pPoint OUT     evaluated point
* @param pEdgeData IN      edge summary
* @param param IN      evluation parameter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_evaluateEdgeData
(
RG_Header           *pRG,
DPoint3d            *pPoint,
RG_EdgeData         *pEdgeData,
double              param
);

/*---------------------------------------------------------------------------------**//**
* Evaluate length of edge as approximated by chords.  Intended for use in detecting
*    zero-length edges, hence speed more important than accuracy.
* @param pRG IN      region context
* @param pEdgeData IN      edge summary
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     jmdlRG_quickChordLength
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 numChord
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN OUT  region context
* @param pPoint OUT     point coordinates.
* @param vertexIndex OUT     vertex index; may be a preexisting end vertex or a newly
*           created interior index.
* @param pEdgeData IN      edge summary.
* @param s IN      parameer where vertex is to be placed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_findOrCreateVertexFromEdgeData
(
RG_Header           *pRG,
        DPoint3d    *pPoint,
int                 *pVertexIndex,
const   RG_EdgeData *pEdgeData,
        double      s
);

/*---------------------------------------------------------------------------------**//**
* Set the vertex label on a given node.
* @param pRG IN OUT  region context
* @param nodeId IN      existing node id.
* @param vertexIndex IN      index to store. Not tested for validity in vertex table.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_setVertexIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 vertexIndex
);

/*---------------------------------------------------------------------------------**//**
* Set the curve label on a given node.
* @param pRG IN OUT  region context
* @param nodeId IN      existing node id
* @param curveIndex IN      curve index to store.  Not tested for validity in curve manager.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_setCurveIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 curveIndex
);

/*---------------------------------------------------------------------------------**//**
* @description Get the curve label from a node.
* @param pRG IN      region context.
* @param pCurveIndex OUT     returned curve index.
* @param nodeId IN      base node id of edge.
* @return true if valid node id.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_getCurveIndex
(
RG_Header           *pRG,
int                 *pCurveIndex,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* Set the parent curve label on a given node.
* @param pRG IN OUT  region context
* @param nodeId IN      existing node id
* @param curveIndex IN      parent curve index to store.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_setParentCurveIndex
(
RG_Header           *pRG,
MTGNodeId           nodeId,
int                 curveIndex
);

/*---------------------------------------------------------------------------------**//**
* Get the parent curve index from a node.
* @param pRG IN      region context.
* @param pParentIndex OUT     returned parent index.
* @param nodeId IN      base node id of edge.
* @return true if valid node id.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_getParentCurveIndex
(
RG_Header           *pRG,
int                 *pParentIndex,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* Set the curve label on a given node.
* @param pRG IN OUT  region header
* @param nodeId IN      node id to set
* @param value IN      true to indicate this node is the start (rather than end)
*                   of the (directed) edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_setEdgeStartBit
(
RG_Header           *pRG,
MTGNodeId           nodeId,
bool                value
);

/*---------------------------------------------------------------------------------**//**
* Set masks, vertex label, and edge label at a ndoe.
* @param pRG         IN      region context
* @param        nodeId      IN      node to label
* @param        vertexId    IN      vertex id to attach
* @param        curveId     IN      curve id to attach
* @param        isBase      IN      true if this node is the base of the edge.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRG_labelNode
(
RG_Header       *pRG,
MTGNodeId      nodeId,
int             vertexId,
int             curveId,
int             parentId,
MTGMask mask
);

/*---------------------------------------------------------------------------------**//**
* Return the combined mask for a primary edge with optional direction indicator and user mask
* @param isBase IN      true if the node being labeld is the start (rather than end)
*               of the edge.
* @param userMask IN      additional mask.
* @return composite mask.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask  jmdlRG_constructPrimaryEdgeMask
(
bool        isBase,
MTGMask    userMask
);

/*---------------------------------------------------------------------------------**//**
* Like crossing an edge to its mate, but also jump over null faces
*   on the mate side.
* Strictly: Cross to edge mate.  Search that vertex loop (starting at the edge mate)
*       for an edge not marked a null face.  Since null faces always have exactly two edges, each
*       step around this vertex steps over another null face. If no non-null is found,
*       give up and return original edge mate.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlRG_skipNullFacesToEdgeMate
(
RG_Header           *pRG,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* Evaluate the coordinates and vertex id for a node.  Optionally use a specified parametric
*   offset "away from" the node as evaluation point.
* @param pRG IN      region header.
* @param pX OUT     evaluated coordinates and derivatives
* @param numDerivative IN      number of deriviates to evaluate. (0 fo rjust the point)
* @param pVertexId IN      vertex id at the node.
* @param offset IN      paramter space offset towards middle of edge.
* @return true if all data accessible
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getVertexData
(
RG_Header           *pRG,
DPoint3d            *pX,
int                 numDerivative,
int                 *pVertexId,
MTGNodeId           nodeId,
double              offset
);

/*---------------------------------------------------------------------------------**//**
* Get vertex coordinates.
* @param pRG IN      region header.
* @param pX OUT     evaluated coordinates and derivatives
* @param nodeId IN      node id to query
* @return true if all data accessible
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getVertexDPoint3d
(
RG_Header           *pRG,
DPoint3d            *pX,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* @return true if the given nodes are at vertices that are within the region context's
*   proximity tolerance.
* @param pRG IN      region context.
* @param nodeId0 IN      first node id to test.
* @param nodeId1 IN      second node id to test.
* @return true if all data accessible and vetices close together.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_areVerticesCloseXY
(
RG_Header           *pRG,
MTGNodeId           nodeId0,
MTGNodeId           nodeId1
);

/*---------------------------------------------------------------------------------**//**
* Move vertex coordinates.  Warning: No update of auxiliary structures
* (e.g. range tree, curve data) which may rely on the vertex coordinates.
* vertex index is absolute -- negative 1 is NOT a reference to final vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_moveVertex
(
RG_Header           *pRG,
int                 vertexId,
DPoint3d            *pXYZ
);

/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @param pRG IN      region context.
* @param pEdgeData OUT     edge coordinate and curve id data.
* @param node0Id IN      node id whose edge is being inspected.
* @return true if edge data accessible.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getEdgeData
(
RG_Header           *pRG,
RG_EdgeData         *pEdgeData,
MTGNodeId           node0Id
);

/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @param pRG IN      region context.
* @param pCurveIndex OUT     geometric curve index.
* @param pIsReversed OUT     true if this node (aka half edge, coedge, directed edge) is oriented
*           in the opposite direction of the geometric curve's parameterization
* @param pStartPoint OUT     start point of edge.
* @param pEndPoint OUT     end point of edge
* @param node0Id IN      node id on (half, co, directed) edge.
* @return true if edge data accessible.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getCurveData
(
RG_Header           *pRG,
int                 *pCurveIndex,
bool                *pIsReversed,
DPoint3d            *pStartPoint,
DPoint3d            *pEndPoint,
MTGNodeId           node0Id
);

/*---------------------------------------------------------------------------------**//**
* Look up the parent curve id for a given node, and then the group id from the curve.
* @return true if groupId is defined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getGroupId
(
RG_Header           *pRG,
int                 *pGroupId,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* Look up edge id and both vertex coordinates and ids for a given node.
* @param pRG IN      region context.
* @param pCurveIndex    OUT     curve index in geometry context.
* @param pAuxIndex      OUT     auxiliary curve index
* @param pIsReversed    OUT     indicates if moving in opposite direction from original curve
* @param pStartPoint    OUT     start coordinates
* @param pEndPoint      OUT     end coordinates
* @param node0Id        IN      node to look up.
* @return true if edge data is accessible.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getAuxCurveData
(
RG_Header           *pRG,
int                 *pCurveIndex,
int                 *pAuxIndex,
bool                *pIsReversed,
DPoint3d            *pStartPoint,
DPoint3d            *pEndPoint,
MTGNodeId           node0Id
);

/*---------------------------------------------------------------------------------**//**
* Compute the xy intersections of an edge with an xy circle.
*
* Returned parameter always starts at 0 at the given base node --- i.e. the fractional
*   paramerization is on this side of the edge.
* @param pRG IN      region context.
* @param pParameterArray OUT     array of fractional parameters.
* @param pPointArray OUT     array of intersection points, taken from the curve to obtain a z.
* @param nodeId IN      base node id of (half) edge.
* @param pCenter IN      center of circle.
* @param radius IN      circle radius
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_edgeCircleXYIntersection
(
RG_Header           *pRG,
bvector<double> *pParameterArray,
EmbeddedDPoint3dArray *pPointArray,
int                 nodeId,
DPoint3d            *pCenter,
double              radius
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN      region context.
* @param nodeId IN      either of the two nodes on the edge to drop.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_dropEdge
(
RG_Header           *pRG,
MTGNodeId           nodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN      region context.
* @return proximity tolerance for the graph.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double          jmdlRG_getTolerance
(
RG_Header           *pRG
);

/*---------------------------------------------------------------------------------**//**
* Update the session's maxRelTol, this is used by jmdlRG_updateTolerance to compute the 
* maxTol (using maxTol = s_maxRelTol * maxCoordinate).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double          jmdlRG_setMaxRelTol
(
double newMaxRelTol
);

/*---------------------------------------------------------------------------------**//**
* Update the graph tolerance, based on the (previously recorded) minimum tolerance and the
* relative tolerance times the data range (as recorded by the header --- not recomputed)
* @return true if a global tolerance could be determined.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_updateTolerance
(
RG_Header           *pRG
);

/*---------------------------------------------------------------------------------**//**
* Collect the coordinates of all vertices around the given face.
* @param pRG IN      region context.
* @param pPointArrayHdr IN OUT  rubber array header to receive points.
* @param startId IN      any node id on the face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlRG_collectVerticesAroundFace
(
RG_Header                   *pRG,
EmbeddedDPoint3dArray               *pPointArrayHdr,
MTGNodeId                   startId
);

/*---------------------------------------------------------------------------------**//**
* Pass point array for vertex coordinates around each face to a callback.
* This passes ONLY the vertices --- no subdividsion of curves.
* @param pRG IN      region context
* @param F IN      function to call, args F(pArg0, pArg1, pUserData, pPointBuffer, numVertex startNodeId)
*               (Note: this is enough args to allow F=dlmSystem_callMdlFunction,
*               pArg0=mdl descriptor, pArg1=mdl function address, pUserData=user data in mdl app.
* @param pArg0 IN      caller's context.
* @param pArg1 IN      caller's context.
* @param pUserData IN      caller's context.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_emitFaceVertexLoops
(
RG_Header                       *pRG,
RG_FaceLoopFunction             F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
);

/*---------------------------------------------------------------------------------**//**
* call F(pArg0, pArg1, pUserData, &range, nodeId) with the range of each edge.
* @pRG IN      region context
* @param F IN      function to invoke per edge.
* @param pArg0 IN      caller's context.
* @param pArg1 IN      caller's context.
* @param pUserData IN      caller's context.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_emitEdgeRanges
(
RG_Header                       *pRG,
RG_RangeFunction                F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
);

/*---------------------------------------------------------------------------------**//**
* Issue callbacks for each containing face; each callback passes an array of coordinates around
*   the face.
*
* Function call is
*<pre>
*       F(pArg0, pArg1, pUserData, pPointArray, numPoint, nodeId);
*</pre>
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_emitFaceLoopsByFaceRangeSearch
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
RG_FaceLoopFunction             F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData
);

/*---------------------------------------------------------------------------------**//**
* Issue callback for each face surrounding the pick point.   Each callback indicates one node
* of the face.
* Callback args are
*<pre>
*   F
*       (
*       void    *pArg0,             IN      user context (e.g. mdl descriptor)
*       void    *pArg1,             IN      user context (e.g. mdl function address)
*       void    *pUserData0,        IN      user context.
*       RG_Header   *pRG,           IN      region graph.
*       MTGGraph   *pGraph,         IN      connectivity graph for traversal.
*       MTGNodeId  nodeId,          IN      seed node for the face loop.
*       MTGNodeId  nearestNodeId,   IN      node at base of nearest edge.
*       void        *pUserData1,    IN      user data.
*       int         userInt,        IN      user data.
*       DPoint3d    *pPoint0,       IN      The test point.
*       DPoint3d    *pPoint1        IN      Nearest point.
*       );
*</pre>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_emitFaceNodesByFaceRangeSearch
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
RG_NodeFunction                 F,
void                            *pArg0,
void                            *pArg1,
void *                          pUserData0,
void *                          pUserData1,
int                             userInt
);

/*---------------------------------------------------------------------------------**//**
* Find the smallest face surrounding a point.
* @param pRG IN      region graph to search.
* @param    pNodeId OUT     node at the base of the edge that comes nearest to the test point.
*           MTG_NULL_NODEID if there is no surrounding face.
* @param pPoint IN      search point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_smallestContainingFace
(
RG_Header                       *pRG,
MTGNodeId                       *pNearNodeId,
const DPoint3d                  *pPoint
);

/*---------------------------------------------------------------------------------**//**
* Find the smallest face surrounding a point.
* @param pRG IN      region graph to search.
* @param    pNodeId OUT     node at the base of the edge that comes nearest to the test point.
*           MTG_NULL_NODEID if there is no surrounding face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_smallestContainingFaceExt
(
RG_Header                       *pRG,
MTGNodeId                       *pNearNodeId,
MTGNodeId                       *pRefNodeId,
const DPoint3d                  *pPoint
);

/*---------------------------------------------------------------------------------**//**
* Get the range of all vertices on the graph.
* (Overall graph may be larger if curves extend beyond vertices)
* @param pRange IN      range of vertices.
* @return true if vertices are present.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getVertexRange
(
RG_Header                       *pRG,
DRange3d                        *pRange
);

/*---------------------------------------------------------------------------------**//**
* Return true if the nodeId is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getEdgeRange
(
RG_Header                       *pRG,
DRange3d                        *pRange,
int                             nodeId
);

/*---------------------------------------------------------------------------------**//**
* Eliminate edges shorter than tolerance.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_dropShortEdges
(
RG_Header                       *pRG,
double                          minEdgeLength
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.   Split edges and reorder vertex loops.
* Reconstruct edge range tree at end if in incremental merge mode
* @return true if merge completed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_merge
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* @return the vertex array header pointer.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray* jmdlRG_getVertexArray
(
RG_Header *pRG
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.   Split edges and reorder vertex loops.
* Reconstruct edge range tree at end if in incremental merge mode
* Optionally close gaps.
* @param vertexVertexTolerance IN      tolerance for closing large gaps between preexisting
*       vertices.   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @param vertexEdgeTolerance IN      tolerance for closing large gaps between a vertex and
*       and edge,   This is an application data tolerance, which may be
*       much larger than computational tolerances.
* @return true if merge completed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_mergeWithGapTolerance
(
RG_Header                       *pRG,
double                          vertexVertexTolerance,
double                          vertexEdgeTolerance
);

/*---------------------------------------------------------------------------------**//**
* Return arrays showing matched vertices.
* @param pLinkedLists IN      array in which at each vertex index the stored int is the
*       index of a "next" vertex of a cluster.
* @param pVertexBatch IN      array of contiguous blocks of vertex indices, each
*               terminated by a -1 index.
* @param toleranceFactor IN      multiplies the overall graph tolerance.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_identifyMatchedVertices
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pLinkedLists,
EmbeddedIntArray                        *pVertexBatch,
double                          toleranceFactor
);

/*---------------------------------------------------------------------------------**//**
* Search a (merged) region structure for all faces that intersect a given polygon.
*
* @param    pFaceNodeIdArray    IN      Array of faces contacted by the polygon.
* @param    pEdgeNodeIdArray    IN      Array of edges contacted by the polygon.
* @return true if search completed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_collectUnderPolyline
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pFaceNodeIdArray,
EmbeddedIntArray                        *pEdgeNodeIdArray,
DPoint3d                        *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* Search for edges that conflict with a given polyline.  Only valid if graph is
* "incremental" edge range mode.
*
* @param    pEdgeNodeIdArray    IN      Array of edges contacted by the polygon.
* @return true if search completed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_collectEdgesUnderPolyline
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pEdgeNodeIdArray,
DPoint3d                        *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_freeEdgeRangeTree
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_freeFaceRangeTree
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree and build a new one.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_buildEdgeRangeTree
(
RG_Header                       *pRG,
double                          lowOffset,
double                          highOffset
);

/*---------------------------------------------------------------------------------**//**
* Return an array of all nodeId's which are base of an edge in the region graph.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getEdgeList
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pEdgeStart
);

/*---------------------------------------------------------------------------------**//**
* Toss the old range tree and build a new one.
*
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_buildFaceRangeTree
(
RG_Header                       *pRG,
double                          lowOffset,
double                          highOffset
);

/*---------------------------------------------------------------------------------**//**
* Toss the old faceHole array and construct a new one.  This assumes that the face range
* tree has been constructed.
* The face hole array is a set of node id pairs.  Each pair gives (outerNodeId, innerNodeId)
* for a hole.
* In addition, each node in the graph is marked so it can be quickly tested to see
* which of the four face types it is:
*<ul>
*<li>True exterior -- negative area, not contained in any other face.</li>
*<li>Plain face -- positive area, no holes.</li>
*<li>Has holes -- positive area, has holes.</li>
*<li>Hole -- negative area, contained in another face.</li>
*</ul>
*
* Face range tree must be constructed prior to calling this function.
* (so negative area bits are set)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_buildFaceHoleArray
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Build bridge edges as indicted by the face-hole array.
* @bsimethod                                                    BentleySystems  11/08/00   (The day without a president)
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_buildBridgeEdges
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Drop (delete) all bridge edges.
* @param pRG IN OUT  region header.
* @bsimethod                                                    BentleySystems  11/08/00   (The day without a president)
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_dropBrideEdges
(
RG_Header                       *pRG
);

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face is true exterior, i.e.
*   negative area and not contained in another face.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_faceIsTrueExterior
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* @description Test if nodeId has been marked as a null face, e.g., by jmdlRG_mergeWithGapTolerance.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlRG_faceIsNull
(
RG_Header   *pRG,
MTGNodeId   nodeId
);

/*---------------------------------------------------------------------------------**//**
* @description Test if nodeId has been marked as the start of a directed edge.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlRG_edgeIsDirected
(
RG_Header   *pRG,
MTGNodeId   nodeId
);

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face has negative area, i.e. might be
*   a true exterior or hole exterior.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_faceIsNegativeArea
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Test if nodeId is a bridge edge.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_isBridgeEdge
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face has holes, i.e.
*   positive area and has recorded holes.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_faceHasHoles
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Test pre-recorded hole array markup to see if face is a negative area (hole)
*   loop in a larger postive face.
* @param pRG IN      region header.
* @param nodeId IN      node to test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_faceIsHoleLoop
(
RG_Header                       *pRG,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Return the holeNodeId part of each nodeId pair of the form (outerFaceNodeId,holeNodeId)
* in the face-hole array as constructed by jmdlRG_buildFaceHoleNodeIdArray.
* @param pRG IN      region header.
* @param pHoleNodeId OUT     array containing pairs of nodes.
* @param nodeId IN      any node on the outer (positive area) loop of the face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_searchFaceHoleArray
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pHoleNodeIdArray,
MTGNodeId                       outerFaceNodeId
);

/*---------------------------------------------------------------------------------**//**
* Search for outerloop and array of inner loop nodes for the face containing
* given node.   The seed node may be on any inner or outer face.
*
* @param pHoleNodeIdArray OUT     If the face containing seedNodeId has is not part of a
* face with holes, return in pHoleNodeIdArray a single node, which is some node on the
* face containing the seed.  (Possibly the seed, possibly elsewhere.)
*  If the face containing the seed is part of a multi-loop face, the array
*       contains the outer loop representative followed by all inner loop representatives.
* @return 0 if the seed node is not part of a multiloop face, 1 if it is an outer
*               loop, -1 if it is an inner loop.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRG_resolveHoleNodeId
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pHoleNodeIdArray,
MTGNodeId                       seedNodeId
);

/*---------------------------------------------------------------------------------**//**
* Add a single face to a caller-supplied range tree.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_addFaceToXYRangeTree
(
RG_Header                       *pRG,
XYRangeTreeNode                 *pTree,
MTGNodeId                       faceStartId,
double                          lowOffset,
double                          highOffset
);

/*---------------------------------------------------------------------------------**//**
* Find conflicts between a polyline/polygon and faces indexed by a range tree.
*
* Each hit is recorded as two integers in the conflict array:
*       First int: One of RG_PFC_EDGE_CONFLICT, RG_PFC_POLYLINE_IN_FACE, RG_PFC_FACE_IN_POLYGON
*       Second int:  seed nodeId for the face
* @param closed IN      true if point array is to be closed (and hence can produce RG_PFC_FACE_IN_POLYGON)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_searchFaceRangeTreeForPolylineConflict
(
RG_Header                       *pRG,
XYRangeTreeNode                 *pTree,
EmbeddedIntArray                        *pConflictArray,
DPoint3d                        *pPointArray,
int                             numPoint,
bool                            closed
);

/*---------------------------------------------------------------------------------**//**
* Search the edge range tree for edges that are fully contained in a given polygon.
* Each hit is recorded as two integers in the conflict array:
*       First int:   RG_PFC_EDGE_IN_POLYTON
*       Second int:  seed nodeId for the edge
* @param pRG IN      region context.
* @param pConflictArray OUT     array of typecode and node id pairs.
* @param pPointArray IN      polygon coordinates
* @param numPoint IN      number of points in polygon.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_searchEdgeRangeTreeForPolygonContainment
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pConflictArray,
DPoint3d                        *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**

* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_collectXYEdgeRangeHits
(
RG_Header           *pRG,
EmbeddedIntArray    *pNodeArray,
DRange3d            *pRange
);

/*---------------------------------------------------------------------------------**//**
* Compute the area and angle swept by a ray from a from a given origin as it sweeps
* edge.
* @param pRG IN      region context
* @param pSweptArea OUT     (signed) area swept by line from pPoint to edge.
* @param pSweptAngle OUT     (signed) angle swept by line from pPoint to edge.
* @param pPoint IN      fixed point of sweep.
* @param edgeNodeId IN      node on edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getEdgeSweepProperties
(
RG_Header                       *pRG,
double                          *pSweptArea,
double                          *pSweptAngle,
DPoint3d                        *pPoint,
MTGNodeId                       edgeNodeId
);

/*---------------------------------------------------------------------------------**//**
* Project from test point to (bounded) edge.
* @param pRG IN      region context.
* @param pMinParam OUT     parameter of closest point.
* @param pMinDistSquared OUT     squared distance.
* @param pMinPont OUT     closest point.
* @param pMinTangent OUT     tangent at closest point.
* @param pPoint IN      test point.
* @param edgeNodeId IN      node on edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getClosestXYPointOnEdge
(
RG_Header                       *pRG,
double                          *pMinParam,
double                          *pMinDistSquared,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent,
const DPoint3d                  *pPoint,
MTGNodeId                       edgeNodeId
);

/*---------------------------------------------------------------------------------**//**
* Determine the direction that the face "passed by" pPoint.   Assumes the given face point
* is the result of a "nearest point on face" search, so 0 and 1 parameter cases have specific
* interpretations.
* Various input parameters are the results of prior search for minimum distance point on face.
* @param pRG IN      region context.
* @param pPoint IN      test point.
* @param mindNodeId IN      node on closest edge.
* @param minParam IN      parameter on closest edge.
* @param pMinPoint IN      closest point.
* @param pMinTangent IN      edge tangent at closest point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_classifyPassingDirection
(
RG_Header                       *pRG,
DPoint3d                        *pPoint,
MTGNodeId                       minNodeId,
double                          minParam,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent
);

/*---------------------------------------------------------------------------------**//**
* Search all the edges around a face for the edge point closest to the test point.
* @param pRG IN      region context.
* @param pMinNodeId OUT     base node for closest edge.
* @param pMinParam OUT     parameter on closest edge.
* @param pMinDist  OUT     distance to closest edge.
* @param pMinPoint OUT     closest point.
* @param pminCCW OUT     true if face is passing by in CCW direction, i.e. point is
*               inside face.
* @param pPoint IN      test point.
* @param faceNodeId IN      any start node on the face loop.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getClosestPointAroundFace
(
RG_Header                       *pRG,
MTGNodeId                       *pMinNodeId,
double                          *pMinParam,
double                          *pMinDist,
DPoint3d                        *pMinPoint,
bool                            *pMinCCW,
DPoint3d                        *pPoint,
MTGNodeId                       faceNodeId
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN      region context.
* @param pArea OUT     computed area
* @param pSweptAngle OUT     computed angle swept by the face as viewed from the point.
* @param pPoint IN      reference point for sweep calculations.
* @param faceNodeId IN      any start node on the face.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRG_getFaceSweepProperties
(
RG_Header                       *pRG,
double                          *pArea,
double                          *pSweptAngle,
DPoint3d                        *pPoint,
MTGNodeId                       faceNodeId
);

/*---------------------------------------------------------------------------------**//**
* Print a summary of the region graph.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRG_print
(
RG_Header                       *pRG,
const char                      *pTitle
);

/*---------------------------------------------------------------------------------**//**
*
* Link to application-supplied function to intersect a circle and an edge
* @param pRG IN      region context
* @param pParameterArray OUT     array of intersection parameters.
* @param pPointArray OUT     array of intersection points.
* @param pEdgeData IN      edge data.
* @param pCenter IN      circle center.
* @param radius IN      circle radius.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_linkToCurveCircleXYIntersection
(
RG_Header           *pRG,
bvector<double> *pParameterArray,
EmbeddedDPoint3dArray *pPointArray,
RG_EdgeData         *pEdgeData,
DPoint3d            *pCenter,
double              radius
);

/*---------------------------------------------------------------------------------**//**
*
* Link to application-supplied function to intersect a line segment and
* a curve.
* @param pRG IN      region context
* @param    pIL IN      intersection list to receive announcements of intersections
*                   and near approaches.
* @param    pEdge0Data IN      First edge, known to be linear
* @param    pEdge1Data IN      Second edge, known to be curve.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_linkToSegmentCurveIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0Data,
RG_EdgeData         *pEdge1Data
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN      region context.
* @param pArea OUT     area of rule surface from the fixed point to the edge.
* @param pSweptAngle OUT     computed angle swept by the edge as viewed from the point.
* @param pPoint IN      reference point for sweep calculations.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_linkToEdgeSweepProperties
(
RG_Header           *pRG,
RG_EdgeData         *pEdgeData,
double              *pArea,
double              *pSweptAngle,
const DPoint3d      *pPoint
);

/*---------------------------------------------------------------------------------**//**
* Link to application-supplied function to intersect two curves.
* @param pRG IN      region context
* @param    pIL IN      intersection list to receive announcements of intersections
*                   and near approaches.
* @param    pEdge0Data IN      First edge, known to be curve
* @param    pEdge1Data IN      Second edge, known to be curve.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRG_linkToCurveCurveIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0Data,
RG_EdgeData         *pEdge1Data
);

/*---------------------------------------------------------------------------------**//**
* @return true if the linkage was completed and the nearest ponit computed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_linkToGetClosestXYPointOnEdge
(
RG_Header                       *pRG,
const RG_EdgeData               *pEdgeData,
double                          *pMinParam,
double                          *pMinDistSquared,
DPoint3d                        *pMinPoint,
DPoint3d                        *pMinTangent,
const DPoint3d                  *pPoint
);

/*---------------------------------------------------------------------------------**//**
* @param pPoint OUT     returned point.
* @param endIndex IN      0 for start, 1 for end.
* @return true if the linkage was completed and the point and tangent computed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_linkToEvaluateCurve
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
DPoint3d    *pPoint,
DPoint3d    *pTangent,
double      *pParameter,
int         numParam
);

/*---------------------------------------------------------------------------------**//**
* @param pX OUT     returned curve point plus derivatives
* @return true if the linkage was completed and the point and tangent computed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_linkToEvaluateDerivatives
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
DPoint3d    *pX,
int         numDerivative,
double      param
);

/*---------------------------------------------------------------------------------**//**
* @param pRG IN OUT  region context.
* @param pEdgeData IN      parent edge description.
* @param pNewCurveIndex OUT     newly created subcurve index.
* @param s0 IN      start of parametric interval.
* @param s1 IN      end of parametric interval.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_linkToCreateSubcurve
(
RG_Header   *pRG,
const RG_EdgeData *pEdgeData,
RG_CurveId  *pNewCurveIndex,
double      s0,
double      s1
);

/*---------------------------------------------------------------------------------**//**
* Evaluate the range of a specified curve.
* @param pEdgeData IN      edge whose range is computed.
* @return true if the range computation succeeds.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRG_linkToGetRange
(
RG_Header   *pRG,
RG_EdgeData *pEdgeData,
DRange3d    *pRange
);

/*---------------------------------------------------------------------------------**//**
* @return Application curve id from the edge data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     jmdlRGEdge_getCurveId
(
RG_EdgeData *pEdgeData
);

/*---------------------------------------------------------------------------------**//**
* @return sense flag from edge data
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRGEdge_isReversed
(
RG_EdgeData *pEdgeData
);

/*---------------------------------------------------------------------------------**//**
* Extract a start or endpoint node id for a region edge.
* @param endIndex IN      0 for start, 1 for end.
* @return selected node id from edge summary data.   MTG_NULL_NODEID if the end index
*           is invalid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId     jmdlRGEdge_getNodeId
(
RG_EdgeData *pEdgeData,
int         endIndex
);

/*---------------------------------------------------------------------------------**//**
* @param endIndex IN      0 for start, 1 for end.
* @return selected vertex id from edge summary data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlRGEdge_getVertexId
(
RG_EdgeData *pEdgeData,
int         endIndex
);

/*---------------------------------------------------------------------------------**//**
* Extract start or endpoint xyz from edge data
* @param pPoint OUT     returned point.
* @param endIndex IN      0 for start, 1 for end.
* @return true if the end index is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRGEdge_getXYZ
(
RG_EdgeData *pEdgeData,
DPoint3d    *pPoint,
int         endIndex
);

END_BENTLEY_GEOMETRY_NAMESPACE

