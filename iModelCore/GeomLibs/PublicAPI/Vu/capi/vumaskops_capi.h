/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description Find each edge with mask.
*   Spread the mask to all other edges around those faces.
* @group "VU Node Masks"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_spreadMaskAroundFaces
(
VuSetP pGraph,
VuMask mask
);

/*---------------------------------------------------------------------------------**//**
@description Find all appearances of triggerMask.
At every vertex around that face, walk the vertex loop and set markMask on all incident faces.
The usual use is that
<ul>
<li>triggerMask and markMask are both VU_EXTERIOR_MASK
<li>triggerMask is set on one "obvious" exterior face
<li>calling this function makes all incident faces also exterior.
</ul>
 @param [in] graph graph to update
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GEOMDLLIMPEXP      vu_spreadExteriorMasksToAdjacentFaces
(
VuSetP pGraph,
bool negativeSeedsOnly,
VuMask triggerMask, // Identifies candidate faces.
VuMask markMask    //  mask to apply
);
/**
@description  Clear mask throughout set.
   Find all negative area faces. Recursively apply an exterior mask, with exterior defined by parity count
   that flip whenever crossing an edge with specified boundary mask on either side.
@param pGraph IN graph to search
@param boundaryMask IN mask of uncrossable edges.
@param exteriorMask IN mask to apply
@return number of distinct components found.
*/
Public GEOMDLLIMPEXP int vu_parityFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask boundaryMask,
VuMask exteriorMask
);


/**
@description  Clear mask throughout set.
   Find all negative area faces. Recursively apply an exterior mask, with exterior defined by winding number
    count that goes up when crossing an edge with singleExteriorMask on the entry side and down when
    crossing an edge that has singleExteriorMask on exit side.
@param pGraph IN graph to search
@param singleExteriorMask IN mask of uncrossable edges.
@param compositeExteriorMask IN mask to apply
@param numberForInterior IN winding number for "inside".  1 is union, 2 is "two or more"
@return number of distinct components found.
*/
Public GEOMDLLIMPEXP int vu_windingFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask singleExteriorMask,
VuMask compositeExteriorMask,
int numberForInterior = 1
);

/**
@description
<pre>
<ul>
<li> Find all negative area faces.
<li> Search recursively, increasing and decreasing winding number when singleExterioMask is on the outside
      of a crossing, decreasing it when singleExteriorMask is inside.
</pre>    
@param pGraph IN graph to search
@param singleExteriorMask IN mask for winding number change.
@param nodeDepth array giving one node per face and its depth.
@return number of distinct components found.
*/
Public GEOMDLLIMPEXP int vu_windingFloodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask singleExteriorMask,
bvector<struct VuPInt> &nodeDepth
);

/*---------------------------------------------------------------------------------**//**
@description  Clear mask throughout set.
   Find all negative area faces.   Recursively apply mask to all faces reached without crossing
   barrier mask.
@param pGraph IN graph to search
@param barrierMask IN mask of uncrossable edges.
@param floodMask IN mask to apply
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_floodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask barrierMask,
VuMask floodMask
);

/*---------------------------------------------------------------------------------**//**
@description
 For each face
    recursively search to neightbors without crossing any edge with barrier mask.
    (This visits a connected component bounded by the specified mask.)
    Delete the edges crossed in the search.
 (I.E.: Form a spanning tree over edge-connected faces.   Delete the edges crossed by the tree.
    Leave non-tree edges in place.)
@param pGraph IN graph to search
@param barrierMask IN mask of uncrossable edges.
@remarks
    In a graph marked with VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE,
    passing VU_BOUNDARY_EDGE will expand both interior and exterior faces, leaving the minimal number of
    edges (both interior and exterior) needed to provide connectivity among the boundary edges.
    Passing (VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE) will only expand interior faces.
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_expandFacesToBarrier
(
VuSetP pGraph,
VuMask barrierMask
);

/*---------------------------------------------------------------------------------**//**
@description
 Apply parity markup to entire graph.
@param pGraph IN graph to search
@param mBoundary IN mask of uncrossable edges.
@param mExterior IN mask to apply as "exterior"
@remarks
   In an edge-connected component, there is (only!!) one face with negative area.
   The negative area face itself is marked mExterior.
   Parity changes at each boundary crossing.
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markExteriorByParity
(
VuSetP pGraph,
VuMask mBoundary,
VuMask mExterior
);

//! @description Search for the face with most negative area.
Public GEOMDLLIMPEXP VuP vu_findMostNegativeAreaFace (VuSetP pGraph);

/*---------------------------------------------------------------------------------**//**
@description
 For each face 
    recursively search to neightbors without crossing any edge with barrier mask.
    (This visits a connected component bounded by the specified mask.)
    Delete the edges crossed in the search.
 (I.E.: Form a spanning tree over edge-connected faces, but without using any vertex twice within a face.
    Delete the edges crossed by the tree.
    Leave non-tree edges in place.)
@param pGraph IN graph to search
@param barrierMask IN mask of uncrossable edges.
@remarks
    In a graph marked with VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE,
    passing VU_BOUNDARY_EDGE will expand both interior and exterior faces, leaving the minimal number of
    edges (both interior and exterior) needed to provide connectivity among the boundary edges.
    Passing VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE will only expand interior faces.
@group "VU Node Masks"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_expandFacesToBarrierWithDistinctVertices
(
VuSetP pGraph,
VuMask barrierMask
);

/*---------------------------------------------------------------------------------**//**
@description Return vectors of nodes for edges going around maximal interior faces.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_searchMaximalUnmaskedFaces
(
VuSetP graph,                   //!< [in] graph to search
VuMask exteriorMask,            //!< [in] mask applied consistently to exterior faces
bvector<bvector<VuP>> &loops    //!< [out] node loops
);

/*---------------------------------------------------------------------------------**//**
@description Return vectors of nodes for faces with specified mask value.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectMaskedFaces
(
VuSetP graph,                   //!< [in] graph to search
VuMask exteriorMask,            //!< [in] mask applied consistently to exterior faces
bool maskCondition,             //!< [in] true for mask on, false for mask off.
bvector<bvector<VuP>> &loops    //!< [out] node loops
);

/*---------------------------------------------------------------------------------**//**
@description Find null faces (2 edges).  If the edge mates of each have indicated mask,
   reorder both vertex loops so the marked sides are "inside" the null face.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_exchangeNullFacesToBringMaskInside (VuSetP pGraph, VuMask mask);

/*---------------------------------------------------------------------------------**//**
@description Within each bundle of null faces (2 nodes), reorder edges so that those with stated mask value
   precede all others.  All other orderings are preserved.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void vu_sortMaskToFrontOfBundle (VuSetP pGraph, VuMask mask, bool targetMask = true);

/*---------------------------------------------------------------------------------**//**
@description test if specified mask has identical value at each node and its face successor.
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool  vu_isMaskConsistentByFSucc (VuSetP pGraph, VuMask mask);

END_BENTLEY_GEOMETRY_NAMESPACE

