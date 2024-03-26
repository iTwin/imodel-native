/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*
@description Find parameters at which two linestrings intersect and overlap.
Output is in the form of graphics points.  Each graphics points contains xyz coordinates of
intersection or overlap point, and also indicates a segment number and fraction where the point
occured in the original array.

The graphics points are collected into arrays related to polylines A and B.  Corresponding entries in the arrays
represent the same point in the respective polyline.

Intersectons and overlap intervals are recorded separately.  In the overlap arrays, graphics point are in strict pairs
for start and end of overlap intervals.  Because the polylines may pass through the overlap in opposing
directions, the (segment,fraction) pairs may have algebraically reversed direction within a pair.

Overlap description returned from here are "segment by segment".  That is, if 4 consecutive segments have overlap,
it is reported as 8 points in each interval array.  If a compressed form indicating only the overall start and end
of overlap is required, call jmdlDPoint3dPolyline_compressIntervals to sort and compress to another array.

The order in which overlaps and intersections appear in the arrays is not predictable externally.  Various
optimizations to the search may result in out-of-order appearance.  (But the correspondence between outputs
for A, B is maintained.)

Any of the Graphics Point Arrays may be passed as NULL, i.e. that piece of output is not required.

@param pIntersectionArrayA OUT array of simple intersection points along polyline A
@param pIntersectionArrayB OUT array of simple intersection points along polyline B.  Indexed in parallel with
        pIntersectionArrayA.
@param pIntervalArrayA OUT array of intervals (graphics point pairs) along polyline A. Graphics points 2k and 2k+1
    in this array give the start and end of a portion of polyline A which is overlapped by a portion of polyline B.
    The corresponding points 2k and 2k+1 in pIntervalArrayB place the overlap in polyline B.
@param pIntervalArrayB OUT array of overlap intervals along polyline B.  Indexed in parallel to pIntervalArrayA.
*/
Public void jmdlDPoint3dPolyline_intersectLinestrings
(
GraphicsPointArray *pIntersectionArrayA,
GraphicsPointArray *pIntersectionArrayB,
GraphicsPointArray *pIntervalArrayA,
GraphicsPointArray *pIntervalArrayB,
const DPoint3d *pXYZA,
int nA,
const DPoint3d *pXYZB,
int nB,
const double tolerance
);

/*
@description Copy from a an unsorted, undirected interval descriptions to
sorted, compressed, directed interval descriptions.
@param pCompressedIntervals OUT array in which (userData,a) fields describe segment index and fraction
    of start and end of intervals, with intervals spanning multiple segments.
@param pIntervals IN array in which intervals may be unsorted, undirected, and overlapping.
@param fractionalTolerance IN tolerance for considering fractions equal.
        If 0 is entered, a near-machine-precision tolerance is used.
*/
Public void jmdlDPoint3dPolyline_compressIntervals
(
GraphicsPointArray *pCompressedIntervals,
GraphicsPointArray *pIntervals,
double fractionalTolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

