/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@description Set the x-coordinate of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setX
(
VuP             nodeP,          /* IN      known vertex use */
double          xCoord          /* IN      coordinate to install */
);

/*---------------------------------------------------------------------------------**//**
@description Set the y-coordinate of a node.
@param nodeP IN OUT node to update
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setY
(
VuP             nodeP,
double          yCoord
);

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXY
(
VuP             nodeP,
double          xCoord,
double          yCoord
);

/*---------------------------------------------------------------------------------**//**
@description Set coordinates of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@param zCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYZ
(
VuP             nodeP,
double          xCoord,
double          yCoord,
double          zCoord
);

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of a node.
@param nodeP IN OUT node to update
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setDPoint2d
(
VuP             nodeP,
DPoint2d *      pointP
);

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYAroundVertex
(
VuP             nodeP,
double          xCoord,
double          yCoord
);

/*---------------------------------------------------------------------------------**//**
@description Set coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@param zCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYZAroundVertex
(
VuP             nodeP,
double          xCoord,
double          yCoord,
double          zCoord
);

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setDPoint2dAroundVertex
(
VuP             nodeP,
DPoint2d *      pointP
);

/*---------------------------------------------------------------------------------**//**
@description Copy coordinates from one node to another.
@param destP IN OUT destination node
@param sourceP IN destination node
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_copyCoordinates
(
VuP destP,
VuP sourceP
);

/*---------------------------------------------------------------------------------**//**
@description Query the x-coordinate of a node.
@param nodeP IN node to query
@return x-coordinate of node.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getX
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query the y-coordinate of a node.
@param nodeP IN node to query
@return y-coordinate of node.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getY
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-coordinates of a node.
@param xCoordP OUT x-coordinate
@param yCoordP OUT y-coordinate
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_getXY
(
double*         xCoordP,
double*         yCoordP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-coordinates of a node.
@param pointP OUT xy-coordinates
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_getDPoint2d
(
DPoint2d *      pointP,         /* OUT     point to receive coordinates */
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_getDX
@description Query the x-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return x-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   getDX
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query the x-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return x-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getDX
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_getDY
@description Query the y-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return y-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   getDY
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query the y-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return y-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getDY
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query a single coordinate from a single node, using integer index.
@param nodeP IN node to query
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@return requested coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getIndexedXYZ
(
VuP             nodeP,
int             index
);

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for a single node, using integer index.
@param nodeP IN OUT node to update
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZ
(
VuP             nodeP,
int             index,
double          a
);

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for all nodes around a vertex, using integer index.
@param nodeP IN OUT any node at the vertex
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZAroundVertex
(
VuP             nodeP,
int             index,
double          a
);

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for all nodes around a face, using integer index.
@param nodeP IN OUT any node in the face.
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZAroundFace
(
VuP             nodeP,
int             index,
double          a
);

/*---------------------------------------------------------------------------------**//**
@description Search a face loop for a node at which the indexed coordinate attains its minimum value.
@remarks There is no lexical treatment of multiple such minima in the face loop, nor of nodes on the other sides of the edges of the face.
@param pSeed IN start node for face loop search
@param index IN coordinate index for ~mvu_getIndexedXYZ calls
@return pointer to any node with minimum coordinate in specified index around the face.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP vu_findIndexedMinAroundFace
(
VuP pSeed,
int index
);

/*---------------------------------------------------------------------------------**//**
@description Search a face loop for a node at which the indexed coordinate attains its maximum value.
@remarks There is no lexical treatment of multiple such minima in the face loop, nor of nodes on the other sides of the edges of the face.
@param pSeed IN start node for face loop search.
@param index IN coordinate index for ~mvu_getIndexedXYZ calls.
@return pointer to any node with maximum coordinate in specified index around the face.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP vu_findIndexedMaxAroundFace
(
VuP pSeed,
int index
);

/*---------------------------------------------------------------------------------**//**
@description Walk around a face, assigning coordinates so each node is within half a period of its face predecessor.
@remarks It is assumed that no edge traverses more than half a period.
@param pGraph IN OUT graph header
@param pStartNode IN OUT first node of face.
@param a0 IN reference coordinate for first node.  First node is assigned within half a period of this value.
@param indexA IN index of coordinate being changed.
@param periodA IN period for changed coordinate.
@param assignAroundCompleteVertexLoop IN If true, each coordinate update is applied
          around complete vertex loop.  If false, it is only applied for the node in the face,
          and others (outside the face) in the same vertex loop are unchanged.
@return the winding value upon return to start node.  For a normal loop, this should
      be zero.  For polar loop this should be plus or minus the period.
      In any case the value will have tolerance fuzz because it is obtained by from subtraction and addition.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double vu_setPeriodicCoordinatesAroundFaceLoop
(
VuSetP pGraph,
VuP pStartNode,
double a0,
int indexA,
double periodA,
bool    assignAroundCompleteVertexLoop
);

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-components of the vector from the given node to its face sucessor.
@param dXP OUT x-component of vector
@param dYP OUT y-component of vector
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDXY
(
double          *dXP,
double          *dYP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-components of the vector from the given node to its face sucessor.
@param vectorP OUT vector components
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint2dDXY
(
DPoint2d        *vectorP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Test if a node is "below" another, using secondary left right test
    to resolve cases where the nodes are on a horizontal line.
@param node0P IN first node for comparison
@param node1P IN second node for comparison
@return true iff node0P is below node1P in the lexical ordering with y-cooordinate taken as primary sort key
    and x-component as secondary sort key.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_below
(
VuP             node0P,
VuP             node1P
);

/*---------------------------------------------------------------------------------**//**
@description Test if a node is to the left of another.
@param node0P IN first node
@param node1P IN second node
@return true iff node0P is left of node1P in the lexical ordering with x-cooordinate taken as primary sort key
    and negative y-component as secondary sort key.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_leftOf
(
VuP             node0P,
VuP             node1P
);

/*---------------------------------------------------------------------------------**//**
@description Test for identical coordinates of a node.
@param node0P IN first node for comparison
@param node1P IN second node for comparison
@return true if coordinates of node0P and node1P are identical.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_sameUV
(
VuP             node0P,
VuP             node1P
);

/*---------------------------------------------------------------------------------**//**
@description Compare the coordinates of two vu nodes using vertical sweep lexical ordering.
@remarks Nodes are passed with double indirection, as typcially required when a system sort routine is
    applied to an array of VU pointers.
@remarks Same as ~mvu_compareLexicalUV, only without the optional argument, so that standard qsort can use it.
@param node0PP IN first node (pointer to pointer, qsort style)
@param node1PP IN second node (pointer to pointer, qsort style)
@return -1,0,1 according to sort relationship.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     vu_compareLexicalUV0
(
const VuP * node0PP,
const VuP * node1PP
);

/*---------------------------------------------------------------------------------**//**
@description Compare the coordinates of two vu nodes using vertical sweep lexical ordering.
@remarks Nodes are passed with double indirection, as typcially required when a system sort routine is
    applied to an array of VU pointers.
@param node0PP IN first node (pointer to pointer, qsort style)
@param node1PP IN second node (pointer to pointer, qsort style)
@param optArg IN unused
@return -1,0,1 according to sort relationship.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int             vu_compareLexicalUV
(
const VuP * node0PP,
const VuP * node1PP,
const void *optArg
);

/*---------------------------------------------------------------------------------**//**
@description Set the coordinates of a node.
@param nodeP IN OUT node to change
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_setDPoint3d
(
VuP             nodeP,
DPoint3d *      pointP
);

/*---------------------------------------------------------------------------------**//**
@description Set the coordinates of a node.
@param nodeP IN OUT node to change
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_setDPoint3d
(
VuP             nodeP,
DPoint3dCR      point
);


/*---------------------------------------------------------------------------------**//**
@description Set coordinates of all nodes around a vertex.
@param nodeP IN OUT any node in the vertex loop
@param pointP IN point coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_setDPoint3dAroundVertex
(
VuP             nodeP,
DPoint3d *      pointP
);

/*---------------------------------------------------------------------------------**//**
@description Query the z-coordinate of a node.
@param nodeP IN node to query
@return z-coordinate of node
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_getZ
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Query the coordinates of a node.
@param xCoordP OUT x-coordinate
@param yCoordP OUT y-coordinate
@param zCoordP OUT z-coordinate
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getXYZ
(
double*         xCoordP,
double*         yCoordP,
double*         zCoordP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute coordinates at a fractional position along an edge.
@param nodeA IN base node of edge
@param fraction IN fractional coordinate
@return computed point.
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP DPoint3d vu_pointAtFraction (VuP nodeA, double fraction);

/*---------------------------------------------------------------------------------**//**
@description Query the coordinates of a node.
@param pointP OUT coordinates
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint3d
(
DPoint3d *      pointP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge.
@param dXP OUT x-component of vector
@param dYP OUT y-component of vector
@param dZP OUT z-component of vector
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDXYZ
(
double          *dXP,
double          *dYP,
double          *dZP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge.
@param vectorP OUT computed vector
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint3dDXY
(
DPoint3d        *vectorP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge, with start and end node coordinates
    adjusted to be within half a period.
@param vectorP OUT computed vector
@param graphP IN graph header (provides periods)
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getPeriodicDPoint3dDXYZ
(
DPoint3d        *vectorP,
VuSetP          graphP,
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the squared xy-distance between two nodes.
@remarks This funcion is identical to ~mvu_distanceSquared.
@param node0P IN origin of measurement
@param node1P IN target of measurement
@return the squared xy-distance between node0P and node1P.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_getXYDistanceSquared
(
VuP             node0P,
VuP             node1P
);

/*---------------------------------------------------------------------------------**//**
@description Compute the squared xy-distance between two nodes.
@remarks This funcion is identical to ~mvu_getXYDistanceSquared.
@param startP IN origin of measurement
@param endP IN target of measurement
@return the squared xy-distance between startP and endP.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_distanceSquared
(
VuP             startP,
VuP             endP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the xy length of an edge.
@param nodeP IN origin of measurement
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_edgeLengthXY
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D vector from startP to endP.
@param vectorP OUT vector from startP to endP
@param startP IN base node of vector
@param endP IN target node of vector
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_vector
(
DPoint2d *vectorP,
VuP startP,
VuP endP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D vector from startP to endP, adjusting coordinates so the
    vector spans no more than half a period.
@param vectorP OUT vector from startP to endP
@param graphP IN graph header (provides periods)
@param startP IN base node of vector
@param endP IN target node of vector
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_periodicVector
(
DPoint2d *vectorP,
VuSetP          graphP,
VuP startP,
VuP endP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D cross product of vectors from node0P to node1P and node1P to node2P.
@remarks Typically, the nodes passed in satisfy, in order: vu_fpred(node), node, vu_fsucc(node).
@param node0P IN first node
@param node1P IN second node
@param node2P IN third node
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_cross
(
VuP node0P,
VuP node1P,
VuP node2P
);

/*---------------------------------------------------------------------------------**//**
@description Test if the angle from incoming to outgoing edge of a node is a strict left turn.
@param nodeP IN node to examine
@return true if there is a left hand turn.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_isStrictLeftTurn
(
VuP nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D cross product of vectors from node0P to node1P and node1P to pointP.
@param node0P IN first node
@param node1P IN second node
@param pointP IN third point
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_crossPoint
(
VuP             node0P,
VuP             node1P,
DPoint2d *      pointP
);

/*---------------------------------------------------------------------------------**//**
@description Test if a node is strictly within the sector determined by another node.
@param nodeP IN node to test
@param sectorP IN node whose incoming and outgoing edges bound the sector
@return true iff nodeP is within (but not on) the sector formed by the two edges at sectorP
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_nodeInSector
(
VuP             nodeP,
VuP             sectorP
);

/*---------------------------------------------------------------------------------**//**
@description Test if a point is strictly within the sector determined by another node.
@param pointP IN point to test
@param sectorP IN node whose incoming and outgoing edges bound the sector
@return true iff pointP is within (but not on) the sector formed by the two edges at sectorP
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_pointInSector
(
DPoint2d *      pointP,
VuP             sectorP
);

/*---------------------------------------------------------------------------------**//**
@description Examine the horizontal neighborhood of a node.
@remarks The return code has individual bits VU_LEFT_NEIGHBORHOOD and/or
 VU_RIGHT_NEIGHBORHOOD set according to whether points just to the left and right
 are visible in (but not on) the sector at the node.
@param nodeP IN node to examine.
@return logical OR of left and right visibility conditions on horizontal line
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuNeighborhoodMask   vu_horizontalNeighborhood
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the xy-intersection of two edges.
@param fractionAP OUT fractional parameter of intersection along edge starting at nodeAP
@param fractionBP OUT fractional parameter of intersection along edge starting at nodeBP
@param xyzAP OUT xyz coordinates of intersection along edge starting at nodeAP
@param xyzBP OUT xyz coordinates of intersection along edge starting at nodeBP
@param nodeAP IN base node of first edge
@param nodeBP IN base node of second edge
@return true if an intersection is computed
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_intersectXYEdges
(
double          *fractionAP,
double          *fractionBP,
DPoint3d        *xyzAP,
DPoint3d        *xyzBP,
VuP             nodeAP,
VuP             nodeBP
);

/*---------------------------------------------------------------------------------**//**
@description Project a point onto the unbounded line containing an edge.
@remarks if the edge is degenerate (single point), the projection is to that point and
    the parameter is zero.
@remarks Only xy parts of coordinates are used.
@param fractionAP OUT fractional parameter of projection along edge starting at nodeAP
@param xyzOutP OUT xyz coordinates of projection along edge starting at nodeAP
@param nodeAP IN base node of edge
@param xyzP IN point to project onto edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_projectToUnboundedXYEdge
(
double          *fractionAP,
DPoint3d        *xyzOutP,
VuP             nodeAP,
const DPoint3d  *xyzP
);

/*---------------------------------------------------------------------------------**//**
@description Project a point onto the (bounded) edge.
@remarks Only xy parts of coordinates are used.
@param fractionAP OUT fractional parameter of projection along edge starting at nodeAP
@param xyzOutP OUT xyz coordinates of projection along edge starting at nodeAP
@param nodeAP IN base node of edge
@param xyzP IN point to project onto edge
@return true if projection was within the closed [0,1] fractional range.
      In case of false return, the fraction and distance are still valid, but are capped to
      the nearest endpoint.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_projectToBoundedXYEdge
(
double          *fractionAP,
DPoint3d        *xyzOutP,
VuP             nodeAP,
const DPoint3d  *xyzP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the point at the given fractional parameter along an edge.
@param xyzAP OUT xyz coordinates of point along the edge starting at nodeAP
@param nodeAP IN base node of edge.
@param fractionA IN fractional parameter
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_fractionParameterToDPoint3d
(
DPoint3d        *xyzAP,
VuP             nodeAP,
double          fractionA
);
/*---------------------------------------------------------------------------------**//**
@description Compute the point at the given fractional parameter between two nodes.
@param node0 IN node at fraction 0
@param fractionA IN fractional parameter
@param node1 IN node at fraction 1
@return interpolated coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP DPoint3d vu_interpolate (VuP node0, double f, VuP node1);

/*---------------------------------------------------------------------------------**//**
@description Attempt to interpolate the z-coordinate of a node from those of its neighbors.
@remarks The interpolation is according to the relative xy distances along the incoming and outgoing edges.
@remarks This is most commonly used just after splitting an edge at an internal point, hence the distance logic amounts to simple
    interpolation.
@remarks If the two nodes have identical coordinates, the z-coordinate from the predecessor is installed and the return value is ERROR.
@remarks The formula computes xy-distance along each adjacent edge (i.e., takes sqrt), so it produces a z-coordinate even if the node
    is in a corner.  If doing pure parametric subdivision, there may be a performance benefit to using the parameter directly, as the
    xy-coordinates are computed.
@param nodeP IN OUT node to update
@return SUCCESS if z-coordinate was computed, ERROR if coincident nodes.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    vu_interpolateZFromFaceNeighbors
(
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Search the graph for the maximal absolute value of any xy-coordinate.
@param graphP IN graph header.
@return maximum x or y found in graph.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double     vu_maxAbsXYInGraph
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Return a tolerance that is appropriate to the size of coordinates in the graph.
@param graphP IN graph header
@param abstol IN absolute tolerance
@param reltol IN relative tolerance
@return the larger of the given abstol, and the given reltol times the maximum xy-coordinate in the graph
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double     vu_toleranceFromGraphXY
(
VuSetP          graphP,
double          abstol,
double          reltol
);

/*---------------------------------------------------------------------------------**//**
@description Collect the array of nodes which are local extrema in either x or y.
@remarks A local extrema is determined by lexical comparison to immediate neighbors.
@remarks Nodes masked by VU_EXTERIOR_EDGE are not considered.
@param nodeArrayP OUT array of extremal nodes
@param graphP IN graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_collectLocalExtrema
(
VuArrayP        nodeArrayP,
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Collect arrays of nodes which are extremal in their v- (y-) coordinates.
@remarks A local extrema is determined by lexical comparison to immediate neighbors.
@param minArrayP OUT array of local min-v nodes.
@param maxArrayP OUT array of local max-v nodes.
@param graphP IN graph header.
@param mExclude IN mask for edges to be EXCLUDED from acting as starts of face loops.  Typically, this is VU_EXTERIOR_EDGE.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_collectVExtrema
(
VuArrayP        minArrayP,
VuArrayP        maxArrayP,
VuSetP          graphP,
VuMask          mExclude
);

/*---------------------------------------------------------------------------------**//**
@description Search a graph for nodes with extremal coordinates.
@param graphP IN graph header.
@param minUP OUT node with minimum according to ~mvu_below.
@param maxUP OUT node with maximum according to ~mvu_below.
@param minVP OUT node with minimum according to ~mvu_leftOf.
@param maxVP OUT node with maximum according to ~mvu_leftOf.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_findExtremaInGraph
(
VuSetP         graphP,
VuP            *minUP,
VuP            *maxUP,
VuP            *minVP,
VuP            *maxVP
);

/*---------------------------------------------------------------------------------**//**
@description Search a single face for nodes with extremal coordinates.
@param startP IN any node on the face.
@param minUP OUT node with minimum according to ~mvu_below.
@param maxUP OUT node with maximum according to ~mvu_below.
@param minVP OUT node with minimum according to ~mvu_leftOf.
@param maxVP OUT node with maximum according to ~mvu_leftOf.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_findExtrema
(
VuP             startP,         /* OUT     any start vu on the face */
VuP            *minUP,
VuP            *maxUP,
VuP            *minVP,
VuP            *maxVP
);

/*---------------------------------------------------------------------------------**//**
@description Rotate all x- and y-coordinates in the graph by 180 degrees around the origin, (i.e., negate each x and y coordinate).
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate180
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Apply the given DTransform3d to the xyz-coordinates in each node.
@param graphP IN OUT graph header
@param transformP IN transform to apply
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_transform3d
(
      VuSetP          graphP,
const DTransform3d  *transformP
);

/*---------------------------------------------------------------------------------**//**
@description Apply the given DTransform3d to the xyz-coordinates in each node.
@param graphP IN OUT graph header
@param transformP IN transform to apply
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_transform
(
VuSetP          graphP,
TransformCP transformP
);

/*---------------------------------------------------------------------------------**//**
@description Apply the 2d parts of the given DTransform3d to the xy-coordinates in each node.
@param graphP IN OUT graph header
@param transformP IN transform to apply
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_transform2d
(
      VuSetP          graphP,
const Transform  *transformP
);

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by 90 degrees counter-clockwise around the origin.
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate90CCW
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by 90 degrees clockwise around the origin.
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate90CW
(
VuSetP          graphP
);

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by the specified angle around the origin.
@param graphP IN OUT graph header
@param theta IN rotation angle in radians, measured counter-clockwise from global x-axis
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_rotateGraph
(
VuSetP graphP,
double theta
);

/*---------------------------------------------------------------------------------**//**
@description Set the z coordinate in all nodes in the graph.
@param [in] graphP graph header
@group [in] z "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setZInGraph
(
VuSetP graphP,
double z
);

/*---------------------------------------------------------------------------------**//**
@description Sum steps around a face loop, applying periodic adjustments to bring each xy-coordinate to
    the period centered on its predecessor.
@param pGraph IN OUT graph header (provides periods)
@param pSeed IN OUT start node of face
@param index IN coordinate selector: 0 or 1
@return sum of periodic steps
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double vu_sumIndexedPeriodicStepsAroundLoop
(
VuSetP pGraph,
VuP pSeed,
int index
);

/*---------------------------------------------------------------------------------**//**
@description Within each face of the graph, apply periodic logic to bring nodes close to the same x- and/or y-periods as their
    face predecessor; except for faces which wrap around a pole this makes each face internally self-consistent with
    respect to the period.
@remarks The search process chooses unvisited faces in arbitrary order; once a start is chosen, a depth-first search
    order is used, so faces are assigned to a period based on a neighboring face.
@param pGraph IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setPeriodicCoordinatesWithinFaces
(
VuSetP pGraph
);

/*---------------------------------------------------------------------------------**//**
@description Set all xy-coordinates to within one half period of the coordinates of a reference point.
@param pGraph IN graph header (provides periods)
@param pXYZ0 IN reference point coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setPeriodicCoordinatesAroundReferencePoint
(
VuSetP pGraph,
DPoint3d *pXYZ0
);

/*---------------------------------------------------------------------------------**//**
@description Count and record faces which traverse a pole.
@param pGraph IN graph header
@param pXArray OUT array of seeds to faces whose x-coordinates circle a pole.
@param pNumInteriorX OUT number of x-polar faces whose seed is not marked exterior.
@param pNumExteriorX OUT number of x-polar faces whose seed is marked exterior.
@param pYArray OUT array of seeds to faces whose y-coordinates circle a pole.
@param pNumInteriorY OUT number of y-polar faces whose seed is not marked exterior.
@param pNumExteriorY OUT number of y-polar faces whose seed is marked exterior.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_countPolarLoops
(
VuSetP pGraph,
VuArrayP pXArray,
int *pNumInteriorX,
int *pNumExteriorX,
VuArrayP pYArray,
int *pNumInteriorY,
int *pNumExteriorY
);

/*---------------------------------------------------------------------------------**//**
@description Compute the area of a face.
@param startP IN any node around the face
@return face area
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_area
(
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
@description Compute face area, simultaneously setting a mask bit around the face.
@param startP IN any node around the face
@param mask IN mask to set
@return face area
@group "VU Coordinates"
@see vu_setMaskByArea, vu_markExteriorFacesByArea
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_markFaceAndComputeArea
(
VuP             startP,
VuMask          mask
);

/*---------------------------------------------------------------------------------**//**
@description Compute the centroid and area of a face.
@param uvP OUT centroid of the face.  If zero area, coordinates of start node are used.
@param areaP OUT area of the face
@param nPosP OUT number of positive area triangles as seen from the start node
@param nNegP OUT number of negative area triangles as seen from the start node
@param startP IN any node around the face
@return SUCCESS if the face has nonzero area, ERROR otherwise.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    vu_centroid
(
DPoint2d*       uvP,
double          *areaP,
int             *nPosP,
int             *nNegP,
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the centroid and area of a face, using a specified origin for
    generating the triangles whose signed areas are summed.
@param uvP OUT centroid of face.  If zero area, coordinates of start node are used.
@param areaP OUT area of face
@param nPosP OUT number of positive area triangles as seen from specified origin
@param nNegP OUT number of negative area triangles as seen from specified origin
@param originP IN origin for triangles
@param startP IN any node around the face
@return SUCCESS if the face has nonzero area, ERROR otherwise.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  vu_centroidFromOrigin
(
DPoint2d*       uvP,
double          *areaP,
int             *nPosP,
int             *nNegP,
const DPoint2d *originP,
VuP             startP
);

/*---------------------------------------------------------------------------------**//**
@description Test if the specified coordinates of an edge are in the specified closed interval.
@param startP IN base node of edge
@param axisSelect IN component to inspect (0 for u-coordinate, 1 for v-coordinate)
@param cMin IN min coordinate of band
@param cMax IN max coordinate of band
@return true iff the edge is completely within the band
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_isEdgeInBand
(
VuP startP,
int axisSelect,
double cMin,
double cMax
);

/*---------------------------------------------------------------------------------**//**
* @description Search a vu graph and mark those edges whose specified coordinates are in the
*       specified closed interval.
* @param graphP       IN OUT  graph header
* @param axisSelect   IN      component to inspect (0 for u-coordinate, 1 for v-coordinate)
* @param cMin         IN      min coordinate of band
* @param cMax         IN      max coordinate of band
* @param mask         IN      mask to apply to nodes of edges within the band
* @group "VU Coordinates"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markEdgesInBand
(
VuSetP graphP,
int axisSelect,
double cMin,
double cMax,
VuMask mask
);

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_markEdgesInBand
@description Search a vu graph and mark those edges whose specified coordinates are in the specified closed interval.
@remarks This function's name suggests testing for near horizontal or vertical edges, in which case the
    implementation is probably a bug.
@param graphP IN OUT graph header
@param axisSelect IN component to inspect (0 for u-coordinate, 1 for v-coordinate)
@param cMin IN min coordinate of band
@param cMax IN max coordinate of band
@param mask IN mask to apply to nodes of edges within the band
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setIsoCoordinateMask
(
VuSetP graphP,
int axisSelect,
double cMin,
double cMax,
VuMask mask
);

/*---------------------------------------------------------------------------------**//**
@description For each value in the first array, apply (logical OR) a mask if the same value appears in the second array.
@remarks Don't try to interpret the names x and y geometrically: this is just a search for identical values in presorted arrays of doubles.
@param xP IN array of ascending double values
@param flagP IN OUT array of masks corresponding to values in xP; if mask is applied, a value was found in yP identical to this value in xP.
@param nx IN number of x values
@param yP IN array of ascending double values
@param ny IN number of y values
@param mask IN mask to apply
@param tol IN tolerance for identical values
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_augmentMasks
(
double *xP,
VuMask *flagP,
int nx,
double *yP,
int ny,
VuMask mask,
double tol
);

/*------------------------------------------------------------------*//**
@description Compute the 2D cross product of vectors from node0 to node1
  and node1 to node2, reducing each to half-period equivalent as needed.
@param pGraph IN graph header (provides periods)
@param pNode0 IN first node
@param pNode1 IN second node
@param pNode2 IN third node
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double vu_periodicCross
(
VuSetP pGraph,
VuP pNode0,
VuP pNode1,
VuP pNode2
);

/*------------------------------------------------------------------*//**
* @description Compute 2D coordinate range for the entire graph.
* @param graphP IN  graph header
* @param pMin   OUT minimum coordinates
* @param pMax   OUT maximum coordinates
* @return semiperimeter of 2D range box
* @group "VU Coordinates"
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_graphCoordinateRange
(
VuSetP          graphP,
DPoint2d        *pMin,
DPoint2d        *pMax
);

/*---------------------------------------------------------------------------------**//**
@description  Collect xyz coordinates around a face.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getFaceLoopCoordinates
(
VuP faceP,
bvector<DPoint3d> &xyz
);


END_BENTLEY_GEOMETRY_NAMESPACE

