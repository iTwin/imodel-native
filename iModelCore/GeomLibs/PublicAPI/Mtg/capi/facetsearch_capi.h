/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Search for the face face, edge, or vertex containing given parametric coordinates.
* @param pSearch IN OUT search context, as initialized by initConvexSearchUV and modified
*           by subsequent calls to searchUV.
* @param pNodeId OUT a node on the surrounding face.
* @param pResultType OUT integer indicating search result.  Possible values are
*       ISearchResult.INTERIOR_FACE -- the point is contained in the interior face.
*       ISearchResult.EXTERIOR_FACE -- the point is outside the facets.
*       ISearchResult.ON_EDGE       -- the point is on an edge.
*       ISearchResult.ON_VERETX     -- the point is on a vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlMTGFacets_searchUV
(
const   MTGFacets                   *pFacetHeader,
        MTGFacetsSearchContext      *pSearch,
        MTGNodeId                   *pNodeId,
        int                         *pResultType,
const   DPoint3d                    *pPoint
);

/*---------------------------------------------------------------------------------**//**
* Initialize a search structure for subseqent use calling jmdlMTGFacets_searchUV
* @param pSearch OUT initialized search context.
* @param absTol IN absolute tolerace for on-edge tests.
* @param relTol IN relative tolerance for on-edge tests, as a fraction of the full
*                   coordinate range of the facets at the time of this call.
* @return true if an interior facet was found for subsequent use.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlMTGFacets_initConvexSearchUV
(
const   MTGFacets                   *pFacetHeader,
        MTGFacetsSearchContext      *pSearch,
        double                      absTol,
        double                      relTol
);

END_BENTLEY_GEOMETRY_NAMESPACE

