/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Split edges in the given faces wherever they cross the given horizontal lines.
* @remarks The mask is set on a node for which the intersecting altitude is strictly interior to the face on both sides of the node;
*       otherwise, the mask is cleared.  More specifically:
*   <ul>
*   <li>When a face loop touches an altitude line at a single point "from below" such that neighborhoods of both sides of the line are
*       strictly interior to the face, then the touch node is entered as a rightArrayP node, and marked by mPointUp.</li>
*   <li>When a face loop touches an altitude line at a single point "from above" such that neighborhoods of both sides of the line are
*       strictly interior to the face, then the touch node is entered as a rightArrayP node, and marked by mPointDn.</li>
*   <li>When a face loop touches an altitude line at a single point from either above or below, but neighborhoods of both sides of the line
*       are never strictly interior to the face, <em>no</em> entry is made.</li>
*   </ul>
* @remarks The following logic generates the proper number of start-stop points for interval management:
*   <ul>
*   <li>Every leftArrayP entry generates one interval parameter, and it will always be a left value.</li>
*   <li>Every rightArrayP entry generates one interval parameter that will be a right value.</li>
*   <li>A rightArrayP entry with either mPointUp or mPointDn mask set also generates a left parameter.</li>
*   </ul>
* @param graphP         IN OUT  graph header
* @param loopArrayP     IN      array of nodes, one per face loop to split
* @param leftArrayP     OUT     array to collect added/existing nodes that lie on the given altitudes and whose edge is to their left, e.g.,
*                               nodes that can be the left end of segments of the horizontal line split by the face.
* @param rightArrayP    OUT     array to collect added/existing nodes that lie on the given altitudes and whose edge is to their right, e.g.,
*                               nodes that can be the right end of segments of the horizontal line split by the face.
* @param v              IN      ascending array of altitudes (y-coordinates of horizontal split lines)
* @param nv             IN      number of altitudes
* @param mPointUp       IN      mask set for a VU at a face corner that points up and touches an altitude line
* @param mPointDn       IN      mask set for a VU at a face corner that points down and touches an altitude line
* @group "VU Meshing"
* @see vu_joinSortedPairs, vu_splitOnSortedHorizontalLines
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_horizontalSplit
(
VuSetP          graphP,
VuArrayP        loopArrayP,

VuArrayP        leftArrayP,
VuArrayP        rightArrayP,
double         *v,
int             nv,
VuMask          mPointUp,
VuMask          mPointDn
);

/*---------------------------------------------------------------------------------**//**
* @description Add edges between pairs of nodes in the split lists returned by ~mvu_splitFacesOnLine.
* @remarks Assumptions:
*   <ul>
*   <li>Double points are present only in both lists with identical pointers.</li>
*   <li>The slicing began with a list of double-sided face loops.  That is, the intersection line coming in from the left first strikes a
*       "right" node, then starts a left-right node pair.  Similarly, at the end of that line there is a dangling edge from a "left" node.</li>
*   </ul>
* @param graphP             IN OUT  graph header
* @param newEdgeArrayP      OUT     array to collect the (start) nodes of new edges
* @param leftArrayP         IN OUT  array of nodes that intersect the line and whose edge is to their left, e.g.,
*                                   nodes that can be the left end of segments of the line.  Sorted lexically on return.
* @param rightArrayP        IN OUT  array of nodes that intersect the line and whose edge is to their right, e.g.,
*                                   nodes that can be the right end of segments of the line.  Sorted lexically on return.
* @param startPointP        IN      start point of section line (unused)
* @param directionVectorP   IN      direction vector of section line
* @param mPointUp           IN      mask set for a VU at a face corner that points up and touches the line
* @param mPointDn           IN      mask set for a VU at a face corner that points down and touches the line
* @param mLeft              IN      mask to set on left end of new edges
* @param mRight             IN      mask to set on right end of new edges
* @group "VU Meshing"
* @see vu_splitInteriorFacesOnLine, vu_splitFacesOnLine
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_joinLineSplitPoints
(
VuSetP          graphP,
VuArrayP        newEdgeArrayP,
VuArrayP        leftArrayP,
VuArrayP        rightArrayP,
DPoint2d        *startPointP,
DPoint2d        *directionVectorP,
VuMask          mPointUp,
VuMask          mPointDn,
VuMask          mLeft,
VuMask          mRight
);

/*---------------------------------------------------------------------------------**//**
* @description Split edges in the given faces wherever they cross the given line.
* @remarks The mask is set on a node for which the line is strictly interior to the face on both sides of the node;
*       otherwise, the mask is cleared.  More specifically:
*   <ul>
*   <li>When a face loop touches the line at a single point "from below" such that neighborhoods of both sides of the line are
*       strictly interior to the face, then the touch node is entered as a rightArrayP node, and marked by mPointUp.</li>
*   <li>When a face loop touches the line at a single point "from above" such that neighborhoods of both sides of the line are
*       strictly interior to the face, then the touch node is entered as a rightArrayP node, and marked by mPointDn.</li>
*   <li>When a face loop touches the line at a single point from either above or below, but neighborhoods of both sides of the line
*       are never strictly interior to the face, <em>no</em> entry is made.</li>
*   </ul>
* @remarks The following logic generates the proper number of start-stop points for interval management:
*   <ul>
*   <li>Every leftArrayP entry generates one interval parameter, and it will always be a left value.</li>
*   <li>Every rightArrayP entry generates one interval parameter that will be a right value.</li>
*   <li>A rightArrayP entry with either mPointUp or mPointDn mask set also generates a left parameter.</li>
*   </ul>
* @param graphP             IN OUT  graph header
* @param leftArrayP         OUT     array to collect added/existing nodes that intersect the line and whose edge is to their left, e.g.,
*                                   nodes that can be the left end of segments of the line.
* @param rightArrayP        OUT     array to collect added/existing nodes that intersect the line and whose edge is to their right, e.g.,
*                                   nodes that can be the right end of segments of the line.
* @param loopArrayP         IN      array of nodes, one per face loop to split
* @param startPointP        IN      start point of section line
* @param directionVectorP   IN      direction vector of section line
* @param mPointUp           IN      mask set for a VU at a face corner that points up and touches the line
* @param mPointDn           IN      mask set for a VU at a face corner that points down and touches the line
* @group "VU Meshing"
* @see vu_splitInteriorFacesOnLine, vu_joinLineSplitPoints
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitFacesOnLine
(
VuSetP          graphP,
VuArrayP        leftArrayP,
VuArrayP        rightArrayP,
VuArrayP        loopArrayP,
DPoint2d        *startPointP,
DPoint2d        *directionVectorP,
VuMask          mPointUp,
VuMask          mPointDn
);

/*---------------------------------------------------------------------------------**//**
* @description Split the interior faces of the graph at their intersection(s) with the given line.
* @param graphP             IN OUT  graph header
* @param newEdgeArrayP      OUT     array to collect the (start) nodes of new edges
* @param pStartPoint        IN      start point of section line
* @param pDirectionVector   IN      direction vector of section line
* @param mLeft              IN      mask to set on left end of new edges
* @param mRight             IN      mask to set on right end of new edges
* @group "VU Meshing"
* @see vu_splitFacesOnLine, vu_joinLineSplitPoints
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_splitInteriorFacesOnLine
(
VuSetP          graphP,
VuArrayP        newEdgeArrayP,
DPoint2d        *pStartPoint,
DPoint2d        *pDirectionVector,
VuMask          mLeft,
VuMask          mRight
);

/*---------------------------------------------------------------------------------**//**
* @description Add horizontal edges between pairs of nodes in the split lists returned by ~mvu_horizontalSplit.
* @remarks Assumptions:
*   <ul>
*   <li>Both node lists are lexically sorted into horizontal sweeps, e.g., with ~mvu_compareLexicalUV0.</li>
*   <li>Double points are present only in both lists with identical pointers.</li>
*   <li>The slicing began with a list of double-sided face loops.  That is, the intersection line coming in from the left first strikes a
*       "right" node, then starts a left-right node pair.  Similarly, at the end of that line there is a dangling edge from a "left" node.</li>
*   </ul>
* @param graphP             IN OUT  graph header
* @param leftArrayP         IN      array of nodes that intersect the line and whose edge is to their left, e.g.,
*                                   nodes that can be the left end of segments of the line.
* @param rightArrayP        IN      array of nodes that intersect the line and whose edge is to their right, e.g.,
*                                   nodes that can be the right end of segments of the line.
* @param doubleMaxMask      IN      mask set for a VU at a face corner that points up and touches the line
* @param doubleMinMask      IN      mask set for a VU at a face corner that points down and touches the line
* @group "VU Meshing"
* @see vu_horizontalSplit, vu_splitOnSortedHorizontalLines
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_joinSortedPairs
(
VuSetP          graphP,
VuArrayP        leftArrayP,
VuArrayP        rightArrayP,
VuMask          doubleMaxMask,
VuMask          doubleMinMask
);

/*---------------------------------------------------------------------------------**//**
* @description Split the interior faces of the graph at their intersection(s) with the given horizontal lines.
* @param graphP         IN OUT  graph header
* @param faces          IN      array of nodes, one per face loop to split
* @param v              IN      ascending array of altitudes (y-coordinates of horizontal split lines)
* @param nv             IN      number of altitudes
* @param leftNodes      IN      working array
* @param rightNodes     IN      working array
* @group "VU Meshing"
* @see vu_horizontalSplit, vu_joinSortedPairs
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitOnSortedHorizontalLines
(
VuSetP          graphP,
VuArrayP        faces,
double         *v,
int             nv,
VuArrayP        leftNodes,
VuArrayP        rightNodes
);

/*---------------------------------------------------------------------------------**//**
* @description Insert horizontal edges across a face that is known to be monotone in the v-coordinate.
* @param graphP         IN OUT  graph header
* @param sliceArrayP    OUT     array to receive the sliced faces, one node per new face
* @param startP         IN      any starter node in the face
* @param v              IN      ascending array of altitudes (v-coordinates of horizontal split lines)
* @param maskP          IN      array of masks to install on new edges
* @param nv             IN      number of altitudes/masks
* @param fixedMask      IN      additional mask to apply to new edges
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitMonotoneFaceOnSortedHorizontalLines
(
VuSetP          graphP,
VuArrayP        sliceArrayP,
VuP             startP,
double          *v,
VuMask          *maskP,
int             nv,
VuMask          fixedMask
);

/*---------------------------------------------------------------------------------**//**
* @description If the computed area of the face is positive, apply positiveSideMask to each edge and negativeSideMask to mates;
*       if negative area, apply masks in reverse.
* @param pSeed IN OUT seed node for face loop
* @param positiveSideMask IN mask to apply to positive side of face
* @param negativeSideMask IN mask to apply to negative side of face
* @group "VU Node Masks"
* @see vu_markFaceAndComputeArea, vu_markExteriorFacesByArea
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setMaskByArea
(
VuP pSeed,
VuMask positiveSideMask,
VuMask negativeSideMask
);

/*---------------------------------------------------------------------------------**//**
* @description Set all the v-coordinates around a face to a constant value.
* @param startP IN OUT  start node on the face
* @param v      IN      v-coordinate to set
* @group "VU Coordinates"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setVAroundFace
(
VuP startP,
double v
);

/*---------------------------------------------------------------------------------**//**
* @description Make an isolated chain of edges in the graph, with coordinates taken from the given array of 2D points.
* @remarks Also set inside and outside masks.
* @param graphP         IN OUT  graph header in which a loop is to be constructed
* @param firstNodePP    OUT     first node in chain
* @param lastNodePP     OUT     last node in chain
* @param uvP            IN      xy-coordinates to install
* @param id0            IN      index for first node
* @param id1            IN      index for last node
* @param closed         IN      whether the loop is closed
* @param leftMask       IN      mask to apply to the left of the chain
* @param rightMask      IN      mask to apply to the right of the chain
* @group "VU Edges"
* @see vu_joinIndexedChains
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_makeIndexedChain
(
VuSetP graphP,
VuP *firstNodePP,
VuP *lastNodePP,
DPoint2d *uvP,
int id0,
int id1,
bool    closed,
VuMask leftMask,
VuMask rightMask
);

/*---------------------------------------------------------------------------------**//**
* @description Join the endpoints of two chains by a seam edge if they are both closed.
* @remarks First and last nodes of chains are as returned from prior calls to ~mvu_makeIndexedChain
*       and ~mvu_joinIndexedChains.
* @param graphP             IN OUT  graph header
* @param firstBottomNodeP   IN OUT  first node on bottom chain, advanced to top on output
* @param lastBottomNodeP    IN OUT  last node on bottom chain, advanced to top on output
* @param bottomClosed       IN      whether bottom chain is closed
* @param firstTopNodeP      IN      first node on top chain
* @param lastTopNodeP       IN      last node on top chain
* @param topClosed          IN      whether top chain is closed
* @group "VU Edges"
* @see vu_makeIndexedChain
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_joinIndexedChains
(
VuSetP graphP,
VuP *firstBottomNodeP,
VuP *lastBottomNodeP,
bool    bottomClosed,
VuP *firstTopNodeP,
VuP *lastTopNodeP,
bool    topClosed
);

/*---------------------------------------------------------------------------------**//**
* @description Apply an affine mapping to the 2D coordinates of every node.
* @param graphP IN OUT  graph header
* @param uMin0  IN      old minimum u-coordinate
* @param uMax0  IN      old maximum u-coordinate
* @param uMin1  IN      new minimum u-coordinate
* @param uMax1  IN      new maximum u-coordinate
* @param vMin0  IN      old minimum v-coordinate
* @param vMax0  IN      old maximum v-coordinate
* @param vMin1  IN      new minimum v-coordinate
* @param vMax1  IN      new maximum v-coordinate
* @group "VU Coordinates"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_mapCoordinates
(
VuSetP graphP,
double uMin0,
double uMax0,
double uMin1,
double uMax1,
double vMin0,
double vMax0,
double vMin1,
double vMax1
);

/*---------------------------------------------------------------------------------**//**
* @description Remove all edges in a graph, except those with the given mask on exactly one side.
* @param graphP IN OUT  graph header
* @param mask   IN      mask to query
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_removeAllButSingleMarkedEdges
(
VuSetP  graphP,
VuMask  mask
);

END_BENTLEY_GEOMETRY_NAMESPACE

