/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
*
* @param    pi0 OUT     index of nearer line end.
* @param    pi1 OUT     index of further line end.
* @param    s   IN      parameter along line.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGMerge_selectOrigin
(
int     *pi0,
int     *pi1,
double  s
);

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between the xy projections of two line segments.
* @param    nodeAId IN      * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRGMerge_segmentSegmentIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId     nodeId0,
const DPoint3d      *pPoint00,
const DPoint3d      *pPoint01,
      MTGNodeId     nodeId1,
const DPoint3d      *pPoint10,
const DPoint3d      *pPoint11
);

/*---------------------------------------------------------------------------------**//**
*
* Test for vertex on edge conditions.
* @param pEdge0 IN      edge whose end is being tested.
* @param endSelect IN      0 for start, 1 for end.
* @param pEdge1 IN      edge whose body is being tested.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRGMerge_testVertexOnEdgeStrictInterior
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0,
int                 endSelect,
RG_EdgeData         *pEdge1
);

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId IN      * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRGMerge_cciEdgeEdge
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0,
RG_EdgeData         *pEdge1
);

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId IN      * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRGMerge_cciNodeNode
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           nodeAId,
MTGNodeId           nodeBId
);

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId IN      * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlRGMerge_cciNodeEdge
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           node0Id,
RG_EdgeData         *pEdge1
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGMerge_findAllIntersections
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGMerge_findAllIntersectionsFromRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRGIL_initTempEdge
(
RG_EdgeData                     *pEdgeData,
DRange3d                        *pRange,
const DPoint3d                  *pPoint0,
const DPoint3d                  *pPoint1,
int                             auxIndex
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_findPolylineIntersectionsFromFaceRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
const DPoint3d                  *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_findPolylineIntersectionsFromEdgeRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
const DPoint3d                  *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in a face
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_findPolylineIntersectionsFromFace
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
MTGNodeId                       faceStartNodeId,
const DPoint3d                  *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* Find all intersections of an edge with a polyline.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_findPolylineIntersectionsFromEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
MTGNodeId                       edgeNodeId,
const DPoint3d                  *pPointArray,
int                             numPoint
);

/*---------------------------------------------------------------------------------**//**
* See if an intersection list (form a temporary edge ray cast??) indicates a clear parity change.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_intersectionListHasClearParity
(
RG_IntersectionList             *pRGIL,
bool                            *pParityChanged
);

/*---------------------------------------------------------------------------------**//**
* @return index i1 such that all records i0<=i<i1 are have the same nodeId as i0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRGIL_findUpperIndexByNodeId
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0
);

/*---------------------------------------------------------------------------------**//**
* @return index i1 such that all records i0<=i<i1 are have the same clusterIndex as i0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRGIL_findUpperIndexByClusterIndex
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0
);

/*---------------------------------------------------------------------------------**//**
* @return number of subedges defined by the record range
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRGIL_countSubEdges
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0,
int                             i1
);

/*---------------------------------------------------------------------------------**//**
* Scan the descriptor range i0 OUT     i < i1 and set the label paramter to the
* index of the descriptor which is its 'master' in the range.  Each master is the
* first of the contiguous subrange with common parameter.
* @return number of edges which will be present after subdivision.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRGIL_markDistinctParameters
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0,
int                             i1,
double                          minParamStep,
double                          minGeomStep
);

/*---------------------------------------------------------------------------------**//**
* Insert edge with given ends in place of the parent edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGIL_replaceEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGNodeId                       parentNode0Id,
MTGNodeId                       childNode0Id,
MTGNodeId                       childNode1Id
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRGIL_print
(
RG_IntersectionList             *pIL,
const char                      *pTitle
);

/*---------------------------------------------------------------------------------**//**
* Merge precomputed array of curve-curve intersections in the graph.
*
* @param An array for efficient marking of active nodes.
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGMerge_mergeIntersections
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGMask                         nullFaceMask
);

/*---------------------------------------------------------------------------------**//**
* Add gap edges to merged geometry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGMerge_closeGaps
(
RG_Header       *pRG,
double          vertexVertexTolerance,
double          vertexEdgeTolerance
);

/*---------------------------------------------------------------------------------**//**
* Search for connected components contained in others.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlRGMerge_connectHolesToParents
(
RG_Header                       *pRG
);

END_BENTLEY_GEOMETRY_NAMESPACE

