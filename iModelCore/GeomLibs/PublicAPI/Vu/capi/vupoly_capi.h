/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//--------------------------------------------------------------------------------------
// Construct a single loop from an array of points, omitting duplicates by an xy tolerance.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP VuP      vu_makeIndexedLoopFromArray  /* OUT     pointer to an arbitrary vu on the loop */
(
VuSetP          graphP,                 /* IN OUT  VU set in which a loop is to be constructed */
DPoint3d        *pointP,                /* IN      array of uv coordinates of the points, */
int             nPoints,                /* IN      number of points */
VuMask          leftMask,               /* IN      mask to apply to nominal left side of loop */
VuMask          rightMask,              /* IN      mask to apply to nominal right side of loop */
double          xyTolerance             /* IN      tolerance for declaring continguous points
                                                identical. */
);

/*-----------------------------------------------------------------*//**
* Triangulate a single xy polygon.  Triangulation preserves original
*   indices.
* @param pSignedOneBasedIndices OUT     array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pExteriorLoopIndices OUT     array of indices of actual outer loops. (optional)
*           (These are clockwise loops as viewed.)
* @param pXYZOut OUT     output points.  The first numPoint points are exactly
*           the input points.   If necessary and permitted, additional                                           cw
*           xyz are added at crossings.  In the usual case in which crossings
*           are not expected, this array may be NULL.
* @param pointP IN      array of polygon points.
* @param numPoint IN      number of polygon points.
* @param xyTolerance IN      tolerance for short edges on input polygon.
* @param signedOneBasedIndices IN      if true, output indices.
* @param addVerticesAtCrossings IN      true if new coorinates can be added to pXYZOut
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateXYPolygonExt2
(
EmbeddedIntArray    *pIndices,
EmbeddedIntArray    *pExteriorLoopIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool                signedOneBasedIndices,
bool                addVerticesAtCrossings
);

/*-----------------------------------------------------------------*//**
* Triangulate a single xy polygon.  Triangulation preserves original
*   indices.
* @param pSignedOneBasedIndices OUT     array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pXYZOut OUT     output points.  The first numPoint points are exactly
*           the input points.   If necessary and permitted, additional
*           xyz are added at crossings.  In the usual case in which crossings
*           are not expected, this array may be NULL.
* @param pointP IN      array of polygon points.
* @param numPoint IN      number of polygon points.
* @param xyTolerance IN      tolerance for short edges on input polygon.
* @param signedOneBasedIndices IN      if true, output indices.
* @param addVerticesAtCrossings IN      true if new coorinates can be added to pXYZOut
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateXYPolygon
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool                signedOneBasedIndices,
bool                addVerticesAtCrossings
);

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon. Best effort to handle non-planar polygons.
* @param pSignedOneBasedIndices <= array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pointP => array of polygon points.
* @param numPoint => number of polygon points.
* @param xyTolerance => tolerance for short edges on input polygon.
* @param signedOneBasedIndices => if false, output indices are 0-based,
*           with -1 as separator.  If true, indcies are one-based, and interior
*           edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateSpacePolygon
(
EmbeddedIntArray    *pSignedOneBasedIndices,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool             signedOneBasedIndices
);

/*-----------------------------------------------------------------*//**
Compute a local to world transformation for a polygon (disconnects allowed)
Favor first polygon CCW for upwards normal.
Favor the first edge as x direction.
Favor first point as origin.
If unable to do that, use GPA code which will have random
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_coordinateFrameOnPolygon
(
DPoint3d const *pXYZ,
int            numXYZ,
Transform       *pLocalToWorld,
Transform       *pWorldToLocal
);



/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               <=  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pXYZ                   <=  array of output points, or NULL to disallow adding points at crossings.
* @param pointP                 =>  array of input polygon points.
* @param numPoint               =>  number of polygon points.
* @param xyTolerance            =>  tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices =>  if FALSE, output indices are 0-based, with -1 as separator.  If TRUE, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateSpacePolygonExt
(
EmbeddedIntArray*       pIndices,
EmbeddedDPoint3dArray*  pXYZ,
DPoint3d*               pPoints,
int                     numPoint,
double                  xyTolerance,
bool                 bSignedOneBasedIndices
);


/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon as projected in caller-supplied coordinate frame.
  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               <=  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pExteriorLoopIndices   <= array giving vertex sequence for each exterior loop.
* @param pXYZ                   <=  array of output points, or NULL to disallow adding points at crossings.
* @param pLocalToWorld          => local to world transformation
* @param pWorldToLoal           => world to local transformation
* @param pointP                 =>  array of input polygon points.
* @param numPoint               =>  number of polygon points.
* @param xyTolerance            =>  tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices =>  if FALSE, output indices are 0-based, with -1 as separator.  If TRUE, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @param maxPerFace => 3 for triangulation
* @return ERROR if nonsimple polygon.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateProjectedPolygon
(
EmbeddedIntArray*       pIndices,
EmbeddedIntArray*       pExteriorLoopIndices,
EmbeddedDPoint3dArray*  pXYZ,
Transform const         *pLocalToWorld,
Transform const         *pWorldToLocal,
DPoint3d* const         pPoints,
int                     numPoint,
double                  xyTolerance,
bool                    bSignedOneBasedIndices,
int                     maxPerFace = 3
);

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               OUT     array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pExteriorLoopIndices   OUT     array giving vertex sequence for each exterior loop.
* @param pXYZ                   OUT     array of output points, or NULL to disallow adding points at crossings.
* @param pLocalToWorld          OUT     local to world transformation
* @param pWorldToLocal          IN      world to local transformation
* @param pointP                 IN      array of input polygon points.
* @param numPoint               IN      number of polygon points.
* @param xyTolerance            IN      tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices IN      if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateSpacePolygonExt2
(
EmbeddedIntArray*       pIndices,
EmbeddedIntArray*       pExteriorLoopIndices,
EmbeddedDPoint3dArray*  pXYZ,
Transform               *pLocalToWorld,
Transform               *pWorldToLocal,
DPoint3d*               pPoints,
int                     numPoint,
double                  xyTolerance,
bool                    bSignedOneBasedIndices,
int                     maxPerFace = 3
);

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon. Best effort to handle non-planar polygons.
* @param pSignedOneBasedIndices OUT     array of indices.  Each face is added
*           as one-based indices, followed by a 0 (terminator).
*           Interior edges are optionally negated.
* @param pointP IN      array of polygon points.
* @param numPoint IN      number of polygon points.
* @param xyTolerance IN      tolerance for short edges on input polygon.
* @param signedOneBasedIndices IN      if false, output indices are 0-based,
*           with -1 as separator.  If true, indcies are one-based, and interior
*           edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  vu_triangulateSpacePolygon
(
EmbeddedIntArray    *pIndices,
DPoint3d            *pPoints,
int                 numPoint,
double              xyTolerance,
int                 maxPerFace,
bool                signedOneBasedIndices
);

/*-----------------------------------------------------------------*//**
* Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
* @param pIndices               OUT     array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
*                                   and whether or not interior edges are negated.
* @param pXYZ                   OUT     array of output points, or NULL to disallow adding points at crossings.
* @param pointP                 IN      array of input polygon points.
* @param numPoint               IN      number of polygon points.
* @param xyTolerance            IN      tolerance for short edges on input polygon.
* @param bSignedOneBasedIndices IN      if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
*                                   interior edges are negated, with 0 as separator.
* @return ERROR if nonsimple polygon.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_triangulateSpacePolygonExt
(
EmbeddedIntArray*       pIndices,
EmbeddedDPoint3dArray*  pXYZ,
DPoint3d*               pPoints,
int                     numPoint,
double                  xyTolerance,
bool                    bSignedOneBasedIndices
);

/*-----------------------------------------------------------------*//**
* @description Adjust the triangulation indices returned by ~mvu_triangulateSpacePolygon to match the orientation of the
*       original polygon.
* @remarks Although the triangles returned by ~mvu_triangulateSpacePolygon will have consistent orientation, they are not
*       guaranteed to have the same orientation as the input polygon.
* @remarks The original polygon is ASSUMED to be non-self-intersecting.  This function may incorrectly decide to reorient
*       the triangulation if the original polygon is self-intersecting.
* @param pIndices               OUT     array of triangulation indices, formatted as per bSignedOneBasedIndices
* @param pbReversed             OUT     true if orientation was reversed; false if orientation unchanged (can be NULL).
* @param bSignedOneBasedIndices IN      If true, indices are one-based, interior edges are negated, and 0 is the separator;
*                                  if false, indices are 0-based and -1 is the separator.
* @return ERROR if invalid index array
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  vu_reorientTriangulationIndices
(
EmbeddedIntArray    *pIndices,
bool                *pbReversed,
bool                bSignedOneBasedIndices
);

/*-----------------------------------------------------------------*//**
* Perform a boolean union operation and return all true outer (negative area)
*   faces.
* Input and output are structured as packed arrays of coordinates with parallel count
*   arrays.
* Replication of final point in inputs optional in inputs.
* @param opCode = 0 for union of all loops, 1 for intersection of all loops
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void             vu_polygonBoolean
(
EmbeddedDPoint3dArray   *pOutputXYZ,
EmbeddedIntArray        *pOutputCounts,
EmbeddedDPoint3dArray   *pInputXYZ,
EmbeddedIntArray        *pInputCounts,
bool                    duplicateFirstPointInOutputLoops,
int                     opCode
);


/*-----------------------------------------------------------------*//**
* Triangulate an xy polygon (optionally with DISCONNECT-deliminted hole loops),
* with interior points added on a grid as needed.
* @param [out] pIndices signed, one-based indices for interior polygons
* @param [out] pXYZOut  output points.
* @param [in] pointP  array of polygon points.
* @param [in] numPoint  number of polygon points.
* @param [in] maxEdgeLength  length for edge subdivision. (Zero to ignore.)
* @param [in] meshXLength  grid size in x direction.
* @param [in] meshYLength  grid size in y direction.
* @param [in] smoothTriangulation true to adjust mesh vertices to get smooth flow lines in the triangulation.
* @param [in] radiansTolForRemovingQuadDiagonals tolerance for removing edges to form quads.
* @param [in] meshOrigin optional mesh origin.
* @returns ERROR if internal limits on mesh counts were enforced.
* @remarks The smoothTriangulation and quad diagonal removal are performed in sequence.  Both steps are permitted,
*       but in practice callers that want "highly rectangular" quads will NOT enable triangle smoothing, and
*       callers that want "smooth triangles" will disable the quad diagonal step.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_subtriangulateXYPolygon
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
int                 numPoint,
double              maxEdgeLength,
double              meshXLength,
double              meshYLength,
bool                smoothTriangulation,
double              radiansTolForRemovingQuadDiagonals,
DPoint3d            *meshOrigin
);
/*-----------------------------------------------------------------*//**
* Triangulate an xy polygon (optionally with DISCONNECT-deliminted hole loops),
* with interior points added on a grid as needed.
* @param [out] pIndices signed, one-based indices for interior polygons
* @param [out] pXYZOut  output points.
* @param [in] pPoints  array of polygon points.
* @param [in] pEdgeVisible optional array of per-edge visibity indicators.
* @param [in] numPoint  number of polygon points.
* @param [in] maxEdgeLength  length for edge subdivision. (Zero to ignore.)
* @param [in] meshXLength  grid size in x direction.
* @param [in] meshYLength  grid size in y direction.
* @param [in] smoothTriangulation TRUE to adjust mesh vertices to get smooth flow lines in the triangulation.
* @param [in] radiansTolForRemovingQuadDiagonals tolerance for removing edges to form quads.
* @param [in] meshOrigin optional mesh origin.
* @returns ERROR if internal limits on mesh counts were enforced.
* @remarks The smoothTriangulation and quad diagonal removal are performed in sequence.  Both steps are permitted,
*       but in practice callers that want "highly rectangular" quads will NOT enable triangle smoothing, and
*       callers that want "smooth triangles" will disable the quad diagonal step.
+----------------------------------------------------------------------*/
Public StatusInt vu_subtriangulateXYPolygonExt
(
EmbeddedIntArray    *pIndices,
EmbeddedDPoint3dArray *pXYZOut,
DPoint3d            *pPoints,
bool                *pEdgeVisible,
int                 numPoint,
double              maxEdgeLength,
double              meshXLength,
double              meshYLength,
bool             smoothTriangulation,
double              radiansTolForRemovingQuadDiagonals,
DPoint3d            *meshOrigin
);

/*-----------------------------------------------------------------*//**
Split edges into pieces no longer than maxEdgeLength, but with no more than maxSubEdge pieces.
Return number of splits applied.
+----------------------------------------------------------------------*/
Public int vu_splitLongEdges (VuSetP graphP, double maxEdgeLength, int maxSubEdge);

/*-----------------------------------------------------------------*//**
Split edges into pieces no longer than maxEdgeLength true or maxXLengh in X, or maxYLength in Y, but with no more than maxSubEdge pieces.
+----------------------------------------------------------------------*/
Public int vu_splitLongEdges (VuSetP graphP, double maxEdgeLength, double maxXLength, double maxYLength, int maxSubEdge);

Public int vu_splitLongEdges (VuSetP graphP, VuMask candidateMask, double maxEdgeLength, double maxXLength, double maxYLength, int maxSubEdge);

END_BENTLEY_GEOMETRY_NAMESPACE

