/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/GeomApi.h>


#include <Mtg/MtgStructs.h>

#include "capi/facetout_capi.h"

#include "capi/facetmemory_capi.h"
#include "capi/mtgarray_capi.h"
#include "capi/mtgbase_capi.h"
#include "capi/mtgclip_capi.h"
#include "capi/mtgfacet_capi.h"
#include "capi/mtgfacet3_capi.h"
#include "capi/mtgloop_capi.h"
#include "capi/mtgedgestar_capi.h"
#include "capi/mtgmarkset_capi.h"
#include "capi/mtgmask_capi.h"
#include "capi/mtgmemory_capi.h"
#include "capi/mtgmesh_capi.h"
#include "capi/mtgparity_capi.h"
#include "capi/mtgreg_capi.h"
#include "capi/mtgstitch_capi.h"
#include "capi/mtgswap_capi.h"
#include "capi/mtgreduce_capi.h"

#ifdef __cplusplus
#include "capi/mtgchain_capi.h"
#include "capi/unfold_capi.h"
#include "capi/polylineintersect_capi.h"
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| Structure for compact triangle lists for rendering.                   |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    DPoint3d *pPointArray;
    DPoint3d *pNormalArray;
    DPoint2d *pUVArray;
    int     *pTriangleToVertexArray;
    int     *pTriangleToTriangleArray;
    int     maxPoint;
    int     maxTri;
    int     numPoint;
    int     numTri;
    } TriangleIndices;
END_BENTLEY_GEOMETRY_NAMESPACE
#include "capi/TriangleIndices_capi.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifndef SupressUnionFindDecls
// Deprecated VArray support needed in regions.dll..
/*--------------------------------------------------------------------*//*
* @param pXYZArray IN      array of n points, containing possibly matched points.
* @param pCycleArray IN      array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray IN      array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVertices
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
);

/*--------------------------------------------------------------------*//*
* @param pXYZArray IN      array of n points, containing possibly matched points.
* @param pCycleArray IN      array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray IN      array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVerticesXY
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
);

/*---------------------------------------------------------------------------------**//**
* Create a new cluster index for a union-find algorithm.
*
* @param    pInstance IN      int array being used for union find.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_newClusterIndex
(
EmbeddedIntArray  *pInstance
);

/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance IN OUT  int array being used for union find.
* @param cluster0 IN      first cluster id
* @return the merged cluster index.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndexExt
(
EmbeddedIntArray  *pInstance,
int         cluster,
int         depth
);

/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance IN OUT  int array being used for union find.
* @param cluster0 IN      first cluster id
* @return the merged cluster index.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndex
(
EmbeddedIntArray  *pInstance,
int         cluster
);

/*---------------------------------------------------------------------------------**//**
* @param    pInstance IN OUT  int array being used for union find.
* @param cluster0 IN      first cluster id
* @param cluster1 IN      second cluster id
* @return the merged cluster index (may be different from both!!)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_mergeClusters
(
EmbeddedIntArray  *pInstance,
int         cluster0,
int         cluster1
);
#endif

#ifdef __cplusplus
/*-----------------------------------------------------------------*//**
* @description Create an MTGFacets set from polyface mesh arrays using the double face
*       stitcher.  Partner faces are found via the MTG_NODE_PARTNER_TAG label of each
*       MTG node.  Relevant edges are masked with MTG_BOUNDARY_MASK and/or
*       MTG_EXTERIOR_MASK.  Optionally return maps for MTGFacets to/from mesh.
* @remark this uses the pre-2015 stitch function (jmdl_mtgStitch)
* @param pFacets                        <= facet set
* @param pNodeIdFromMeshVertex          <= an MTG nodeId in the vertex loop at mesh vertex i (or NULL)
* @param pPolyfaceArrayIndexFromNodeId  <= index in pIndexArray to face loop entry corresponding to MTG nodeId i (or NULL)
* @param polyface => source mesh
* @param absTol                         => absolute tolerance for identical point test
* @param relTol                         => relative tolerance for identical point test
* @return true if MTG was successfully stitched
+---------------+---------------+---------------+---------------+------*/
bool GEOMDLLIMPEXP PolyfaceToMTG
(
MTGFacets*                      pFacets,
bvector<MTGNodeId>      *pNodeIdFromMeshVertex,
bvector<size_t>         *pPolyfaceArrayIndexFromNodeId,
PolyfaceQueryCR             polyface,
bool                        dropToSingleFace,
double                          absTol,
double                          relTol
);

/*-----------------------------------------------------------------*//**
* @description Create an MTGFacets set from polyface mesh arrays using the double face
*       stitcher.  Partner faces are found via the MTG_NODE_PARTNER_TAG label of each
*       MTG node.  Relevant edges are masked with MTG_BOUNDARY_MASK and/or
*       MTG_EXTERIOR_MASK.  Optionally return maps for MTGFacets to/from mesh.
* @param pFacets                        <= facet set
* @param pNodeIdFromMeshVertex          <= an MTG nodeId in the vertex loop at mesh vertex i (or NULL)
* @param pPolyfaceArrayIndexFromNodeId  <= index in pIndexArray to face loop entry corresponding to MTG nodeId i (or NULL)
* @param polyface => source mesh
* @param absTol                         => absolute tolerance for identical point test
* @param relTol                         => relative tolerance for identical point test
* @param stitchSelect => 0 for the pre-2015 merge (stitch) function.
* @return true if MTG was successfully stitched
+---------------+---------------+---------------+---------------+------*/
bool GEOMDLLIMPEXP PolyfaceToMTG
(
MTGFacets*                      pFacets,
bvector<MTGNodeId>      *pNodeIdFromMeshVertex,
bvector<size_t>         *pPolyfaceArrayIndexFromNodeId,
PolyfaceQueryCR             polyface,
bool                        dropToSingleFace,
double                          absTol,
double                          relTol,
int                            stitchSelect
);



//!-----------------------------------------------------------------
//! Create MTGFacets from polyface.  Only the polyface connectivity is used -- no merging of vertex coordinates
//!   other than what is already indicated in the connectivity.
//! The graph is marked as follows:
//! 1) Any nonmanifold edge (other than two adjacent faces) is masked MTG_BOUNDARY_MASK.
//! 2) The outside of the nonmanifold edges is masked MTG_EXTERIOR_MASK.
//! 3) An outside edge with identical start and end vertex indices is also masked MTG_POLAR_LOOP_MASK
//! 4) Any edge marked visible in the polyface is masked MTG_PRIMARY_EDGE_MASK
//! 5) A label with tag MTG_LABEL_TAG_POLYFACE_READINDEX is added (if necessary) with a back index to the read index
//!      that the node came from.   This label is -1 for exterior nodes.
//! 6) facets->vertexLabelOffset is the 0 based (!!) vertex index.
//!*----------------------------------------------------------------

bool GEOMDLLIMPEXP PolyfaceToMTG_FromPolyfaceConnectivity
(
MTGFacets*       pFacets,
PolyfaceQueryCR  polyface
);

bool GEOMDLLIMPEXP PolyfaceToMTG_FromPolyfaceConnectivity
(
MTGFacets*       pFacets,   //! [out]constructed facets
PolyfaceQueryCR  polyface,  //!< [in] source polyface
bool ignoreDegeneracies   //!< [in] true to ignore degenerate edges (e.g. polar edges on sphere)
);

//!-----------------------------------------------------------------
//! For each start point:
//! <ul>
//! <li>Find the containing face.
//! <li>Move in the direction of steepest descent, stepping from facet to facet until a low point is reached.
//! <li>Return the path as an array in paths.
//! </ul>
//! @param facets facets to search.
//! @param startPoints array of start points.
//! @param paths array of arrays, each leaf array being a path from one start point.
//!
//!*----------------------------------------------------------------
void  GEOMDLLIMPEXP MTGFacets_CollectFlowPaths
(
    MTGFacets &facets,
    bvector<DPoint3d> const &startPoints,
    bvector<bvector<DPoint3d>> &paths
);

/*-----------------------------------------------------------------*//**
* @description Add MTGFacets to an indexed polyface.
* @param [in] pFacets source facets
* @param [in,out] polyface destination polyfaace
* @param [in] exclusionMask mask to identify faces to omit.
* @param [in] visibleMask mask to identify visible edges.  If this is MTG_NULL_MASK all edges are visible.
* @return number of facets added to polyface
+---------------+---------------+---------------+---------------+------*/
size_t GEOMDLLIMPEXP AddMTGFacetsToIndexedPolyface
(
MTGFacets*                      pFacets,
PolyfaceHeaderR             polyface,
MTGMask                     exclusionMask = MTG_EXTERIOR_MASK,
MTGMask                     visibleMask   = MTG_DIRECTED_EDGE_MASK
);


/*-----------------------------------------------------------------*//**
* @description Create a polyface containing a subset of faces in an MTGFacets.
* The subset is defined by the array of MTGNodeId and exclusionMask.
+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr GEOMDLLIMPEXP MTGFacetsSubsetToIndexedPolyface
(
MTGFacets*                      pFacets,
MTGMask                     exclusionMask,
MTGMask                     visibleEdgeMask,
bvector<MTGNodeId> const &  nodes
);
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
