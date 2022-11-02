/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Triangulate a face that is monotone in the v-coordinate.
* @remarks For best results, startP should be a lexical minimum (i.e., it should have the
*       least u-coordinate among those nodes with the least v-coordinate); otherwise, a triangulation
*       is attempted starting at the centroid of the face.
* @param graphP IN OUT  graph header
* @param startP IN      v-minimum node in face
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_triangulateByVerticalSweep
(
VuSetP          graphP,
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
* @nodoc
* @description Slice a graph with the given vertical and horizontal lines and optionally triangulate the resulting partitions.
* @remarks This routine works best if each face of the graph is monotone in either the u- or v-coordinate.
* @param graphP             IN OUT  graph header
* @param uValP              IN      ascending array of u-coordinates of vertical split lines
* @param uMaskP             IN      array of masks to install on new vertical edges
* @param uValCount          IN      number of vertical split lines
* @param vValP              IN      ascending array of v-coordinates of horizontal split lines
* @param vMaskP             IN      array of masks to install on new horizontal edges
* @param vValCount          IN      number of horizontal split lines
* @param faceArrayP         IN OUT  scratch array for original faces
* @param uSliceMinArrayP    IN OUT  scratch array for faces after u-slices
* @param vSliceMinArrayP    IN OUT  scratch array for faces after v-slices
* @param triangulate        IN      whether to triangulate each slice with a v-sweep
* @see vu_splitMonotoneFaceOnSortedHorizontalLines
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sliceAndTriangulate
(
VuSetP          graphP,
double          *uValP,
VuMask          *uMaskP,
int             uValCount,
double          *vValP,
VuMask          *vMaskP,
int             vValCount,
VuArrayP        faceArrayP,
VuArrayP        uSliceMinArrayP,
VuArrayP        vSliceMinArrayP,
bool            triangulate
);

/*---------------------------------------------------------------------------------**//**
* @description Remove edges between interior faces if the union of the faces is convex.
* @param graphP IN OUT  graph header
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_removeEdgesToExpandConvexInteriorFaces
(
VuSetP  graphP
);

/*---------------------------------------------------------------------------------**//**
* @description Remove non-masked if the union of the faces is convex.
* @param graphP IN OUT  graph header
* @group "VU Edges"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_removeEdgesToExpandConvexFaces
(
VuSetP  graphP,
VuMask  barrierMask
);
/*---------------------------------------------------------------------------------**//**
* @description Triangulate monotone faces of the graph.
* @remarks It is assumed that the interior faces of the graph are v-monotone.  If they are u-monotone, pass true for flipCoordinates;
*       the graph will be rotated internally by 90 degrees, triangulated by vertical sweep, then rotated back.
* @param graphP             IN OUT  graph header
* @param flipCoordinates    IN      whether to triangulate by horizontal sweep instead of vertical sweep.
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_triangulateMonotoneInteriorFaces
(
VuSetP      graphP,
bool        flipCoordinates
);

/*---------------------------------------------------------------------------------**//**
* @description Split all v-monotone faces of the graph that have more than maxEdge edges.
* @param graphP     IN OUT  graph header
* @param maxEdge    IN      maximum number of edges for a new face
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_splitMonotoneFacesToEdgeLimit
(
VuSetP          graphP,
int             maxEdge
);

/*---------------------------------------------------------------------------------**//**
* @description Split an edge at the given 2D point and insert its coordinates on both sides.
* @param graphP IN OUT  graph header
* @param leftP  OUT     new node at point on same side as edgeP
* @param rightP OUT     new node at point on opposite side
* @param edgeP  IN      base node of edge to split
* @param point  IN      coordinates of new nodes
* @group "VU Edges"
* @see vu_splitEdge
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdgeAtPoint
(
VuSetP graphP,
VuP *leftP,
VuP *rightP,
VuP edgeP,
DPoint2d *point
);

/*---------------------------------------------------------------------------------**//**
* @description Split an edge at the given 3D point and insert its coordinates on both sides.
* @param graphP IN OUT  graph header
* @param leftP  OUT     new node at pointP on same side as edgeP
* @param rightP OUT     new node at pointP on opposite side
* @param edgeP  IN      base node of edge to split
* @param pointP IN      coordinates of new nodes
* @group "VU Edges"
* @see vu_splitEdge
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdgeAtDPoint3d
(
VuSetP graphP,
VuP *leftP,
VuP *rightP,
VuP edgeP,
DPoint3d *pointP
);

END_BENTLEY_GEOMETRY_NAMESPACE

