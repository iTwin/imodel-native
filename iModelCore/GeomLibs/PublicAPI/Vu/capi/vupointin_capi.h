/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Over all edges of the face, search for the edge with <em>smallest</em> max distance to any vertex.
* @remarks For a triangle, this distance is the smallest of the altitudes.  For a convex face, this is the smallest vertical height as the
*       shape is rolled from one edge to another along a horizontal line.
* @param seedP  IN  node in face
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double vu_minEdgeVertexDistanceAroundFace
(
VuP seedP
);

/*-----------------------------------------------------------------*//**
* @description Find any point strictly interior to a polygon.  Only xy coordinates are examined.
* @param pXYOut OUT     coordinates of point in polygon.
* @param pLoopPoints IN      array of points in polygon.   Multiple loops
*               may be entered with the value DISCONNECT as x and y parts
*               of a separator point.
* @param numLoopPoints IN      number of points in array.
* @return SUCCESS if point found.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt vu_anyInteriorPointInPolygon
(
DPoint3d                *pXYOut,
DPoint3dCP              pLoopPoints,
int                     numLoopPoints
);

/*-----------------------------------------------------------------*//**
* @description Return a graph whose interior points are at least (ax,bx) distance from 
*     loop interior points to the left and right, and (ay, by) below and above.
* @param loops > array of loops.  In these loops, inside is defined by parity.
* @param ax => rightward offset from an edge on the left side.
* @param bx => leftward offset from edge on the right side.
* @param ax => rightward offset from an edge on the top side.
* @param bx => leftward offset from edge on the bottom side.
* @param interiorLoops <= loops containing valid region.
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_createXYOffsetLoops
(
bvector<bvector<DPoint3d>> &loops,
double ax,
double bx, 
double ay,
double by,
bvector<bvector<DPoint3d>> &interiorLoops
);
END_BENTLEY_GEOMETRY_NAMESPACE

