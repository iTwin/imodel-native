/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-------------------------------------------------------------------*//**
* Search the source facets from a starting edge.  As each face is reached,
* add a transformed copy to the destination facets.
* @param destOrigin IN      origin for data in the destination facets.
* @param destDir IN      edge direction in the destination facets.
* @param destNormal IN      out-of-plane normal in destination facets.
* @param visitMask IN      mask to apply in source facets indicating nodes are visited.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFold_appendUnfolded
(
      MTGFacets     *pSourceFacets,
      MTGFacets     *pDestFacets,
      MTGNodeId     seedNodeId,
const DPoint3d      *pDestOrigin,
const DPoint3d      *pDestDir,
const DPoint3d      *pDestNormal,
      MTGMask       visitMask
);

/*-------------------------------------------------------------------*//**
* Unfold facets, using as many seed nodes as possible.
*   (Using jmdlMTGFold_appendUnfolded, which only unfolds from a single seed node.)
* @param destOrigin IN      origin for data in the destination facets.
* @param destDir IN      edge direction in the destination facets.
* @param destNormal IN      out-of-plane normal in destination facets.
* @param layoutSpace IN      global coordinates space between right of first placement
*           range box and left of second.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGFold_appendAllUnfolded
(
      MTGFacets     *pSourceFacets,
      MTGFacets     *pDestFacets,
const DPoint3d      *pDestOrigin,
const DPoint3d      *pDestDir,
const DPoint3d      *pDestNormal,
      double        layoutSpace
);

END_BENTLEY_GEOMETRY_NAMESPACE

