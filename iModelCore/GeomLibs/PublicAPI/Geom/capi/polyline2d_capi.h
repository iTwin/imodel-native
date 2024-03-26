/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Compute the nearest point on the polyline.
//! @param pPoint OUT     interpolated point
//! @param pParam OUT     global parameter at nearest point
//! @param pNearSegment OUT     index of nearest segment
//! @param pNearSegmentParam OUT     local parameter at nearest point
//! @param pPointArray IN      points in polyline
//! @param n IN      number of points in polyline
//! @param pTestPoint IN      space point being projected
//! @param isClosed IN      polyline is closed
//! @return true if polyline has 1 or more points.
//!
Public GEOMDLLIMPEXP bool            bsiVector2d_nearestPointOnPolyline
(
DPoint2dP pPoint,
double          *pParam,
int             *pNearSegment,
double          *pNearSegmentParam,
DPoint2dCP pPointArray,
int              n,
DPoint2dCP pTestPoint,
bool             isClosed
);

//!
//! @description Compute the length of the polyline.
//! @param pPointArray    IN      points in polyline
//! @param numPoints      IN      number of points in polyline
//! @return length of polyline
//!
Public GEOMDLLIMPEXP double   bsiDPoint2d_polylineLength
(
DPoint2dCP  pPointArray,
int             numPoints
);

END_BENTLEY_GEOMETRY_NAMESPACE

