/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio of 3D coordinates.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* @param pGraph IN OUT  graph being modified
* @param vertexLabelOffset IN      offset to vertex label.
* @param pCoordinates IN      coordinate array.
* @param fixedEdgeMask IN      mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticXYZAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
);

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on dihedral angle at each edge and the 4
*   edges of the adjacent triangles.
* @param pGraph <=> graph being modified
* @param vertexLabelOffset => offset to vertex label.
* @param pCoordinates => coordinate array.
* @param fixedEdgeMask => mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveDihedralAngle
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
);


/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio of 3D coordinates.   Maintains ruled surface conditions by requiring
* that the quadrilateral formed around a flipped edge have opposing rails identified
* by a mask.
* @param pGraph IN OUT  graph being modified
* @param vertexLabelOffset IN      offset to vertex label.
* @param pCoordinates IN      coordinate array.
* @param fixedEdgeMask IN      mask identifying edges that cannot be flipped.
* @param railMask IN      mask identifying boundary edges on left.   Must be non-null.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticRuledXYZAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask,
MTGMask         railMask
);

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio.
* aspect ratio.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* @param pGraph IN OUT  graph being modified
* @param vertexLabelOffset IN      offset to vertex label.
* @param pCoordinates IN      coordinate array.
* @param fixedEdgeMask IN      mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveQuadraticAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
);

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, based on quadratic aspect ratio
* after scaling.  (Quadratic aspect ratio is the triangle area divided by
* the sum of squared edge lengths.  This is equivalent to the Delauney circle
* center condition.)
* aspect ratio.
* @param pGraph IN OUT  graph being modified
* @param vertexLabelOffset IN      offset to vertex label.
* @param pCoordinates IN      coordinate array.
* @param fixedEdgeMask IN      mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveScaledQuadraticAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask,
double xScale,
double yScale
);

/*---------------------------------------------------------------------------------**//**
* Swap triangles to improve triangle quality, using only the x coordinate
* to define aspect ratio.
* @param pGraph IN OUT  graph being modified
* @param vertexLabelOffset IN      offset to vertex label.
* @param pCoordinates IN      coordinate array.
* @param fixedEdgeMask IN      mask identifying edges that cannot be flipped.
* @return number of flips performed.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGSwap_flipTrianglesToImproveUAspectRatio
(
MTGGraph        *pGraph,
int             vertexLabelOffset,
EmbeddedDPoint3dArray *pCoordinates,
MTGMask         fixedEdgeMask
);

END_BENTLEY_GEOMETRY_NAMESPACE

