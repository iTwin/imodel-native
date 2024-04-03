/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct VuCoordinateMappingFunction
{
// return mapped coordinate for given node and parameters.  Return false if unable to map.
// (The uv parameter will be as just accessed from the node)
GEOMAPI_VIRTUAL bool TryMapCoordinates (VuSetP graph, VuP node, DPoint2dCR uv, DPoint3dR xyz) = 0;
};


//! Static methods for flipping triangles .
struct VuFlipFunctions
{
static GEOMDLLIMPEXP int FlipTrianglesToImproveMappedCoordinateAspectRatio
(
VuSetP  graphP,
VuCoordinateMappingFunction *mapper,
int maxFlipPerEdge = 40
);
};

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveQuadraticAspectRatio
(
VuSetP  graphP
);




/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio, using only marked edges as starters.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveQuadraticAspectRatio
(
VuSetP  graphP,
VuMarkedEdgeSetP edgeSet
);



/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio, using caller-supplied coordinate mapper for distance and area calculations.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveMappedCoordinateAspectRatio
(
VuSetP  graphP,
VuCoordinateMappingFunction *mapper,
int maxFlipPerEdge = 40
);

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths, with area and edge length computed
        <em>after</em> applying scale factors to the coordinates.
@param graphP IN OUT graph header
@param xScale IN scale factor for x-coordinates, or zero for no scale
@param yScale IN scale factor for y-coordinates, or zero for no scale
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveScaledQuadraticAspectRatio
(
VuSetP  graphP,
double xScale,
double yScale
);

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths, with area and edge length computed
        <em>after</em> applying scale factors to the coordinates, and with coordinates interpreted periodically.
@param graphP IN OUT graph header
@param xScale IN scale factor for x-coordinates, or zero for no scale
@param yScale IN scale factor for y-coordinates, or zero for no scale
@param xPeriod IN period for x-coordinates
@param yPeriod IN period for y-coordinates
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveScaledPeriodicQuadraticAspectRatio
(
VuSetP  graphP,
double xScale,
double yScale,
double xPeriod,
double yPeriod
);

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to correct incircle condition
@remarks This is a substantial full-graph modification function.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesForIncircle
(
VuSetP  graphP,
VuMarkedEdgeSetP edgeSet = nullptr
);

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges to improve aspect ratio in the u-parameter direction.
@remarks This is a substantial full-graph modification function.
@remarks This variant is for triangles on a surface ruled in the y-coordinate direction and curved in the x-coordinate direction, thus long
    triangles in the y-coordinate direction are preferred.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveUAspectRatio
(
VuSetP  graphP
);

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges to improve aspect ratio in the v-parameter direction.
@remarks This is a substantial full-graph modification function.
@remarks This variant is for triangles on a surface ruled in the x-coordinate direction and curved in the y-coordinate direction, thus long
    triangles in the x-coordinate direction are preferred.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveVAspectRatio
(
VuSetP  graphP
);

/*---------------------------------------------------------------------------------**//**
@description Triangulate a single face by adding edges outward from a fan point.
@param graphP IN OUT graph header
@param P IN fixed point of fan
@group "VU Meshing"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_triangulateFromFixedPoint
(
VuSetP          graphP,
VuP             P
);

/*---------------------------------------------------------------------------------**//**
@description Unconditionally flip a single edge.
@remarks When the edge separates two triangular faces, this has the effect of flipping the diagonal of the quad.
@param graphP IN OUT graph header
@param AP IN base node of edge to flip
@group "VU Meshing"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_flipOneEdge
(
VuSetP  graphP,
VuP AP
);

/*---------------------------------------------------------------------------------**//**
@description Subdivide the edges of a triangulation using the given test function, and retriangulate the affected faces.
@param graphP       IN OUT  graph header
@param F            IN      edge test function
@param userDataP    IN      argument for test function
@group "VU Meshing"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_subdivideAndRetriangulate
(
VuSetP graphP,
VuSubdivideEdgeFuncP F,
void *userDataP
);

/*---------------------------------------------------------------------------------**//**
@description Add edges so that holes are connected to outer boundaries, and each face
    has only one chain of continguous "up" edges and one chain of contiguous "down" edges.
@remarks This is a critical step in triangulation.   Once the faces are regularized, triangulation
    edges are easy to add in a single bottom-to-top sweep.
@remarks Currently, this function is a synonym for ~mvureg_regularizeGraph.
@param graphP IN OUT graph header
@group "VU Meshing"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_regularizeGraph
(
VuSetP graphP
);

END_BENTLEY_GEOMETRY_NAMESPACE

