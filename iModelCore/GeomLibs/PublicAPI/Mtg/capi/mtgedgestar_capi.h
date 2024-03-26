/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Get the successor of a node around its edge star, i.e. counterclockwise around the edge
* when viewed with the edge pointing at the eye.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getEdgeStarSucc
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
);

/*---------------------------------------------------------------------------------**//**
* Get the predecessor of a node around its edge star, i.e. clockwise around the edge
* when viewed with the edge pointing at the eye.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getEdgeStarPred
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
);

/*---------------------------------------------------------------------------------**//**
* Get a node's partner "through" its face.  This is the node at the same vertex
* and on the other side of the face.
bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_getFacePartner
(
const MTGGraph    *pGraph,
MTGNodeId       nodeId,
int             partnerLabelOffset
);

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundEdgeStar
(
const MTGGraph    *pGraph,
MTGNodeId       seedNodeId,
int             partnerLabelOffset
);

/*---------------------------------------------------------------------------------**//**
* @return the number edges set in the star loop.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_setOrClearMaskAroundEdgeStar
(
MTGGraph        *pGraph,
MTGNodeId       seedNodeId,
int             partnerLabelOffset,
MTGMask         mask,
bool            newValue,
bool            markEdgeMates
);

/*---------------------------------------------------------------------------------**//**
* Search a double-sided face structure to (a) mark special edges at the fringe
* and multiple-face internal junctions, and (b) assign front and back faces.
* Some typical mask combinations are:
*<LIST>
*<LI>(m0,null,null) -- Mark just the fringe. Leave internal T junctions unmarked.</LI>
*<LI>(m0,null,m0) -- Mark the fringe and T junctions with the same mask.</LI>
*<LI>(m0,null,m1) -- Mark the fringe and T junctions with distinct masks.
*</LIST>
*
* The backsideMask is typically an exterior mask.
*
* The front/back mark is skipped if (a) backsideMask is null or (b) mask1 and mask3 are both
*   null.
*
* @param partnerLabelOffset IN      at each node, this label lead through the face to the
*           node's partner on the opposite side of the face.
* @param mask1 IN      mask to apply on edges with only a single incident face. (These
*       are exposed fringe edges.)
* @param mask2 IN      mask to apply to edges with exactly two incident faces (these are
*       conventional interior edges).
* @param mask3 IN      mask to apply to edges with 3 or more incident faces (these
*       are T junctions.)
* @param backsideMask IN      mask to apply to an arbitrarily assigned backside of
*       faces.  This mask is applied during a search finding connected components not
*       broken by a barrier of mask1 | mask3 edges.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeStarClassificationMasks
(
MTGGraph    *pGraph,
int        partnerLabelOffset,
MTGMask     mask1,
MTGMask     mask2,
MTGMask     mask3,
MTGMask     backsideMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

