/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_addDPoint2dLoop
(
VuSetP      graphP,
DPoint2d    *pointP,
int         numPoint
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_addBsurfBoundaries
(
VuSetP          graphP,
BsurfBoundary   *boundsP,
int             numBounds
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// Add a rectangular loop to the graph.
Public GEOMDLLIMPEXP void vu_addRange
(
VuSetP          graphP,
double          xmin,
double          ymin,
double          xmax,
double          ymax
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_collectConnectedComponent
(
VuSetP          graphP,             /* IN OUT  graph to search and mark */
VuArrayP        componentArrayP,    /* OUT     array of component members */
VuArrayP        stackP,             /* IN OUT  work array */
VuP             startP,             /* IN      seed of component */
VuMask          mask                /* IN      mask to apply to visited nodes */

);

//--------------------------------------------------------------------------------------
// Fixup loops with xor (parity) merge.   Eliminate "external" edges.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_markConnectedParity
(
VuSetP                  graphP          /* IN OUT  analyzed graph.  Changed by analysis */
);

//--------------------------------------------------------------------------------------
// Split vertex loops as needed so that no more than one outgoing edge has a given mask.
// Usual use: call with VU_EXTERIOR_MASK so that multiple polygons that share a vertx become disconnected.
// Returns the number of splits performed.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int vu_breakCompoundVertices
(
VuSetP          graphP,             /* IN OUT  containing graph */
VuMask          mask
);

//--------------------------------------------------------------------------------------
// Return the value of the coordinate going transverse to the specified seam direction.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP double   vu_transverseSeamCoordinate
(
VuP             nodeP,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
);

//--------------------------------------------------------------------------------------
// Return the value of the coordinate going along the specified seam direction
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP double   vu_seamCoordinate
(
VuP             nodeP,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
);

//--------------------------------------------------------------------------------------
// Test if nodeP is an exterior node on the specified seam.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP bool     vu_isExteriorSeamNode
(
VuP             nodeP,
double          param,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
);

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP int      vu_pasteSeams
(
VuSetP          graphP,             /* IN OUT  containing graph */
VuMask          seamJumpMask,       /* IN      Mask to apply to (both sides of)
                                                    edges which jump the seam. */

double          lowParam,
double          highParam,
bool            isVerticalSeam      /* True for vertical seam (left and right),
                                        False for horizontal seam (top and bottom)*/
);

//--------------------------------------------------------------------------------------
// Collect parent-child polygons from a graph with full parity and connectivity.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_collectPolygonsExt
(
VuSetP          graphP,
BsurfBoundary   **boundsPP,     /* OUT     array of loops, parent first then children */
int             **parentPP,     /* OUT     for the boundsPP[i], index of parent loop */
int             *numBoundsP,
bool            duplicateStart, /* true to have the start point
                                    duplicated at the end of each loop. */
int             direction,       /* IN      0 for all CCW
                                      1 for all CW
                                      2 for all outer CCW, inner CW
                                      3 for outer CW, inner CCW
                                */
VuMask          seamJumpMask    /* mask to mark which jump seam. */
);

//--------------------------------------------------------------------------------------
// Collect parent-child polygons from a graph with full parity and connectivity.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void vu_collectPolygons
(
VuSetP          graphP,
BsurfBoundary   **boundsPP,     /* OUT     array of loops, parent first then children */
int             **parentPP,     /* OUT     for the boundsPP[i], index of parent loop */
int             *numBoundsP,
bool            duplicateStart, /* true to have the start point
                                    duplicated at the end of each loop. */
int             direction       /* IN      0 for all CCW
                                      1 for all CW
                                      2 for all outer CCW, inner CW
                                      3 for outer CW, inner CCW
                                */
);

/*---------------------------------------------------------------------------------**//**
* @description Do parity fixup on 2d loop data.
* @param [out] outLoops returned loops
* @param [in] rawLoops input loops
* @param [in] minEdge minimum number of edges in output loops (usually 3, to eliminate null faces)
* @param [in] numWrap number of wraparound coordinates.  (e.g. 1 to get first point duplicated)
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_fixupLoopParity
(
bvector <bvector <DPoint2d> > &outLoops,
bvector <bvector <DPoint2d> > &rawLoops,
size_t minCount = 3,
size_t numWrap = 1
);

/*---------------------------------------------------------------------------------**//**
* @description Collect xy data of interior loops
* @param [in] graph source graph
* @param [out] loops collected loops
* @param [in] minEdge minimum number of edges in output loops (usually 3, to eliminate null faces)
* @param [in] numWrap number of wraparound coordinates.  (e.g. 1 to get first point duplicated)
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_collectInteriorLoopCoordinates
(
VuSetP graph,
bvector<bvector<DPoint2d> > &loops,
size_t minCount = 3,
size_t numWrap = 1
);

END_BENTLEY_GEOMETRY_NAMESPACE

