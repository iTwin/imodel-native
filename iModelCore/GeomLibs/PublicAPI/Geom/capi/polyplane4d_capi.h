/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Test point a point is inside a convex region bounded by planes.
//! @param pPoint IN      point to test.
//! @param pPlaneArray IN      array of plane coefficients.  A zero or negative
//!       value of the dot product (x,y,z,1) * (a,b,c,d)
//!   is considered "in".
//! @param numPlane IN      number of planes
//! @return true if the point is in the region.
//!
Public GEOMDLLIMPEXP bool    bsiHVec_isPointInConvexRegion
(
DPoint3dCP pPoint,          /* IN      point to test */
DPoint4dCP pPlaneArray,   /* IN      plane equations */
int         numPlane        /* number of planes */
);

//!
//! Test points in an array to see if they are inside a convex
//! region bounded by planes.
//! @param pPointArray IN      array of points to test.
//! @param numPoint IN      number of points
//! @param pPlaneArray IN      array of plane coefficients.  A zero or negative
//!       value of the dot product (x,y,z,1) * (a,b,c,d)
//!   is considered "in".
//! @param numPlane IN      number of planes
//! @return the number of points which are "in" all the planar halfspaces.
//!
Public GEOMDLLIMPEXP int bsiHVec_numLeadingPointsInConvexRegion
(
DPoint3dCP pPointArray,   /* IN      point to test */
int         numPoint,       /* IN      number of points in array */
DPoint4dCP pPlaneArray,   /* IN      plane equations */
int         numPlane        /* number of planes */
);

//!
//! Search an array for the closest point, using only
//! x and y components.  Useful for screenproximity
//! tests between points at different depths.
//!
//! @param pPoint IN      fixed point for tests
//! @param pArray IN      array of test points
//! @param nPoint IN      number of points
//! @return index of closest point
//!
Public GEOMDLLIMPEXP int bsiGeom_closestXYDPoint4d
(
DPoint3dCP pPoint,
DPoint4dCP pArray,
int             nPoint
);

END_BENTLEY_GEOMETRY_NAMESPACE

