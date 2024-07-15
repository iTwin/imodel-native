/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Check that all nodes in a graph satisfy the successor/predecessor requirements.
* @param pGraph   IN      graph whose successor/predecessor relations are to be verified.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_verifyGraph
(
const MTGGraph *  pGraph
);

/**
* Build a rectangular face grid.
* @param pGraph    IN OUT  Containing graph
* @param nu IN      number of edges along constant u direction
* @param nv IN      number of edges along constant v direction
* @param nuVertexPerRow IN      number of vertices in stored rows.  Typically nu or nu+1
* @param uPeriodic      IN      true if seam at left/right
* @param vPeriodic      IN      true if seam at top/bottom
* @param pMasks         IN      masks
* @param normalMode     IN      indicates which labels are to be applied.
* @param vertexIdOffset IN      offset for vertex id labels.
* @param vertex00Id     IN      label for lower left vertex.  -1 if no labels needed.
*                          Vertices are labeled in row-major order from here.
* @param sectorIdOffset IN      offset for sector id labels.  -1 if no labels needed.
* @param sector00Id     IN      label for lower left vertex sector on main grid.
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildRectangularGrid
(
MTGGraph *      pGraph,                 /* IN OUT  Containing graph */
int             nu,                     /* IN      number of edges along constant u direction */
int             nv,                     /* IN      number of edges along constant v direction */
int             nuVertexPerRow,         /* IN      number of vertices in stored rows.  Typically */
                                        /*  nu or nu+1 */
bool            uPeriodic,              /* IN      true if seam at left/right */
bool            vPeriodic,              /* IN      true if seam at top/bottom */
MTG_RectangularGridMasks *pMasks,       /* IN      masks */
MTGFacets_NormalMode normalMode,       /* IN      indicates which labels are to be applied. */
int             vertexIdOffset,         /* IN      offset for vertex id labels. */
int             vertex00Id,             /* IN      label for lower left vertex.  -1 if no labels needed. */
                                        /*    Vertices are labeled in row-major order from here. */
int             sectorIdOffset,         /* IN      offset for sector id labels.  -1 if no labels needed. */
int             sector00Id              /* IN      label for lower left vertex sector on main grid. */
);

/**
* Build a rectangular face grid.
* @param pGraph    IN OUT  Containing graph
* @param nu IN      number of edges along constant u direction
* @param nv IN      number of edges along constant v direction
* @param pMasks         IN      masks
* @param normalMode     IN      indicates which labels are to be applied.
* @param vertexIdOffset IN      offset for vertex id labels.
* @param vertex00Id     IN      label for lower left vertex.  -1 if no labels needed.
*                          Vertices are numbered along u direction first.
* @return MTGNodeId of outside node at 00 point.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildTriangularGrid
(
MTGGraph *      pGraph,                 /* IN OUT  Containing graph */
int             numEdge,                /* IN      number of edges along each direction */
MTG_RectangularGridMasks *pMasks,       /* IN      masks */
int             vertexLabelOffset,      /* IN      offset for vertex labels. */
int             vertex00Id              /* IN      label for lower left vertex.  -1 if no labels needed. */
                                        /*    Vertices are labeled in row-major order from here. */
);

/**
* Build a rectangular face grid.
* @param    pGraph          IN OUT  Containing graph
* @param numRay             IN      number of rays in fan.
* @param periodic           IN      true if seam at left/right
* @param pMasks             IN      masks
* @param normalMode         IN      indicates which labels are to be applied.
* @param vertexIdOffset     IN      offset for vertex id labels.
* @param vertex00Id         IN      label for lower left vertex.  -1 if no labels needed.
*                               Vertices are labeled in row-major order from here.
* @param sectorIdOffset     IN      offset for sector id labels.  -1 if no labels needed.
* @param sector00Id         IN      label for lower left vertex sector on main grid.
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildPlanarFan
(
MTGGraph *      pGraph,
int             numRay,
bool            periodic,
MTG_RectangularGridMasks *pMasks,
MTGFacets_NormalMode normalMode,
int             vertexIdOffset,
int             vertex00Id,
int             sectorIdOffset,
int             sector00Id
);

/**
* Build a cube topology.  Uses standard numbering (see doc for pictures)
* @param    pGraph IN OUT  Containing graph
* @param nodeMask IN      mask to apply to all nodes
* @param normalMode IN      Indicates how normal data is to be constructed.
*                       For vertexOnly, vertex indices 0..7 are assigned
*                       For normalPerVertex, vertex indices 0..23 are assigned
*                           exactly matching the node ids.
* @param vertex0Id          IN      index to use for the zero'th vertex of 6
* @param vertexIdOffset     IN      offset for vertex id labels.
* @param vertex00Id         IN      label for first vertex
* @param sectorIdOffset     IN      offset for sector id labels.  -1 if no labels needed.
* @param sector00Id         IN      label for first sector
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildLabeledCubeTopology
(
MTGGraph *      pGraph,
MTGMask nodeMask,
MTGFacets_NormalMode normalMode,
int             vertex0Id,
int             vertexIdOffset,
int             vertex00Id,
int             sectorIdOffset,
int             sector00Id
);

END_BENTLEY_GEOMETRY_NAMESPACE

