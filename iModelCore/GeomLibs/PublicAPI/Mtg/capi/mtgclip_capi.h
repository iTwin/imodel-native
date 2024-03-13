/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**

* @param pFacetHeader    IN OUT  facets to clip
* @param pClipped OUT     set true if clipped
* @param pRange IN      range cube defining planes
* @param planeSelector IN      bit map of active planes
* @param clipType IN      one of:
*                   MTG_ClipOp_KeepIn -- usual inside clip.
*                   MTG_ClipOp_KeepOut -- clip away the convex region.
*                   MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @param cutMask IN      mask to apply to edges created by the clip
* @see MTGFacets.IClipType
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_clipToRange
(
        MTGFacets      *pFacetHeader,
        bool            *pClipped,      /* OUT     set true if clipped */
const   DRange3d        *pRange,
        RangePlaneMask planeSelector,
        MTGClipOp       clipType,
        MTGMask cutMask
);

/**

* @param pFacetHeader    IN OUT  facets to clip
* @param pClipped OUT     set true if clipped
* @param pHPlane IN      plane equations
* @param numPlane IN      number of planes
* @param clipType IN      one of:
*                   MTG_ClipOp_KeepIn -- usual inside clip.
*                   MTG_ClipOp_KeepOut -- clip away the convex region.
*                   MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @param cutMask IN      mask to apply to edges created by the clip
* @see MTGFacets.IClipType
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_clipToPlanes
(
MTGFacets *     pFacetHeader,
        bool            *pClipped,      /* OUT     set true if clipped */
const   DPoint4d        *pHPlane,
        int             numPlane,
        MTGClipOp       clipType,
        MTGMask cutMask
);

/**
* @param pFacetHeader           IN OUT  facets to clip
* @param pClipped       OUT     true if graph was changed
* @param pPointArray    IN      points in polygon
* @param numPoint       IN      number of planes
* @param pDirection     IN      sweep direction
* @param clipType       IN      one of:
*                       MTG_ClipOp_KeepIn -- usual inside clip.
*                       MTG_ClipOp_KeepOut -- clip away the convex region.
*                       MTG_ClipOp_KeepOn -- union of section cuts on all planes.
* @see MTGFacets.IClipType
* @return SUCCESS if hole punched successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlMTGFacets_punchPolygon
(
        MTGFacets      *pFacetHeader,
        bool           *pClipped,
const   DPoint3d       *pPointArray,
        int             numPoint,
const   DPoint3d       *pDirection,
        MTGClipOp       clipType
);

/**
* @param pFacetHeader       IN OUT  facets to clip
* @param pHPlane    IN      plane equation
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_sectionByPlane
(
        MTGFacets      *pFacetHeader,
const   DPoint4d        *pHPlane
);

/**
* Collect edges for which the adjacent facet normals have different
* directions relative to a view vector, and at least one of the two
* normals is not perpendicular to the view normal.   Facet normals
* are determined by taking cross products of 3 consecutive points on
* each side of the edge.
* @param pNodeArray OUT     array with one node per silhouette edge
* @param pFacetHeader    IN      facets to examine
* @param pViewVector IN      view direction vector.
* @param ignoreMask IN      identifies egdges NOT to test
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_findSilhouetteEdges
(
        EmbeddedIntArray        *pNodeArray,
const   MTGFacets *     pFacetHeader,
const   DPoint3d        *pViewVector,
        MTGMask ignoreMask
);

/**
* Any of the node arrays may be NULL.   The same pointer may be
* passed for several different arrays, e.g. the zero and ambiguous
* faces.
* @param pPositive          OUT     array of node ids whose edges
*                               are on the silhouette and adjacent face
*                               point towards the eye.  May be NULL
* @param pNegative          IN      array of node ids whose edges
*                               are on the silhouette and adjacent face points
*                               away. May be NULL
* @param pFringe            IN      array of node ids whose edges are on a fringe
*                               this face postive or negative, adjacent face
*                               ambiguous or excluded. May be NULL
* @param pZero              IN      array with ONE node id for each face that is
*                               perpendicular to the view direction.
* @param pAmbiguous         IN      array with ONE node id for each face that has
*                               ambiguous visibility
* @param pFacetHeader       IN      facets to search
* @param excludeFaceMask    IN      mask for faces to include.  Assumed to be set
*                               entirely around each face
* @param excludeEdgeMask    IN      mask for edges to exclude.  Tested edge by edge
* @param pEyePoint          IN      eyepoint for visibility tests.
* @param inTol              IN      tolerance for flush-face test.
*                               If zero, machine tolerance is used.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_findSilhouette
(
MTGFacets *             pFacetHeader,
EmbeddedIntArray                *pPositive,
EmbeddedIntArray                *pNegative,
EmbeddedIntArray                *pFringe,
EmbeddedIntArray                *pZero,
EmbeddedIntArray                *pAmbiguous,
MTGMask         excludeFaceMask,
MTGMask         excludeEdgeMask,
DPoint4d                *pEyePoint,
double                  inTol
);

END_BENTLEY_GEOMETRY_NAMESPACE

