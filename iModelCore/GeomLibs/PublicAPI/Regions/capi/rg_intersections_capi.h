/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @return true if an intersection record was returned.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlRGI_get
(
RG_IntersectionList *pIL,
RG_Intersection     *pIntersection,
int                 index
);

/*---------------------------------------------------------------------------------**//**
* @return the number of intersection descriptors in the list.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlRGIL_getCount
(
RG_IntersectionList             *pRGIL
);

/*---------------------------------------------------------------------------------**//**
* Merge the clusters for the intersection descriptors with given indices.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_mergeClustersOf
(
RG_IntersectionList *pIL,
int                 index0,
int                 index1
);

/*---------------------------------------------------------------------------------**//**
* Set the seed nodes on intersection descriptors with indices index0 OUT     i < index1
* @param index0 IN      first index to set
* @param index1 IN      one past last index to set
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_setSeeds
(
RG_IntersectionList *pIL,
int                 index0,
int                 index1,
MTGNodeId          seedNodeId
);

/*---------------------------------------------------------------------------------**//**
* @return the resolved cluster index of the given descriptor. The resolved cluster
*   index is saved in the descriptor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     jmdlRGIL_resolveClusterOf
(
RG_IntersectionList *pIL,
int                 intersectionIndex
);

/*---------------------------------------------------------------------------------**//**
*
* Initialize an intersectionlist
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_init
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
*
* Empty an already-initialized intersection list.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_empty
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
*
* Free the secondary memory allocated to an intersection list.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_releaseMem
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_declareIsolatedBreak
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    sNodeId,
      double        s               /* IN      parameter on curve */
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_declareSimpleIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    sNodeId,
      double        s,              /* IN      parameter on first curve */
      MTGNodeId    tNodeId,
      double        t               /* IN      parameter on second curve */
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_declareVertexOnSegment
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      int           where0,
      MTGNodeId    edgeBaseNodeId1,
      double        t               /* IN      parameter on second curve */
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_declareVertexOnSegmentByProximityTest
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      int           where0,
      MTGNodeId    edgeBaseNodeId1,
      double        t               /* IN      parameter on second curve */
);

/*---------------------------------------------------------------------------------**//**
*
* Add intersection records for coincident vertices.  HOWEVER ... test for common
* vertex id, and do NOT add if already identical.
* @param where0 IN      0,1 end id on edge0
* @param where1 IN      0,1 end id on edge1
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_declareVertexOnVertex
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           edgeBaseNodeId0,
int                 where0,
MTGNodeId           edgeBaseNodeId1,
int                 where1
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_addCloseApproachPoint
(
RG_IntersectionList *pIL,
      MTGNodeId    edgeBaseNodeId0,
      double        s,              /* IN      parameter on first curve */
const DPoint3d      *pSPoint,       /* IN      point on first curve */
      MTGNodeId    edgeBaseNodeId1,
      double        t,              /* IN      parameter on second curve */
const DPoint3d      *pTPoint        /* IN      point on second curve */
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRGI_declareShortEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGNodeId                       nodeId
);

/*---------------------------------------------------------------------------------**//**
* Lexical sort by (nodeId, parameter)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_sortByNodeIdAndParameter
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
* Lexical sort by (clusterid)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_sortByClusterId
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
* Lexical sort by (seedNodeId)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlRGIL_sortBySeedNodeId
(
RG_IntersectionList *pIL
);

/*---------------------------------------------------------------------------------**//**
* Fill pArray with the unique node ids from the intersection array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRGIL_getUniqueSeedNodeIdArray
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pArray,
RG_IntersectionList             *pRGIL
);

/*---------------------------------------------------------------------------------**//**
* Fill pArray with the unique node ids from the intersection array.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlRGIL_getUniqueNodeIdArray
(
RG_Header                       *pRG,
EmbeddedIntArray                        *pArray,
RG_IntersectionList             *pRGIL
);

END_BENTLEY_GEOMETRY_NAMESPACE

