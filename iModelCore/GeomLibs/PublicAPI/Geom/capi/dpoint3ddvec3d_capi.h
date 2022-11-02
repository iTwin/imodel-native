/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Initialize a point by copying x,y,z from a vector.
//! @param pPoint OUT The point
//! @param pVector IN The vecotr
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromDVec3d
(
DPoint3dP pPoint,
DVec3dCP pVector
);

//!
//! @description Compute the sum of a point and a vector
//!
//! @param pSum OUT The computed sum.
//! @param pBase IN The the first point or vector
//! @param pVector IN The second point or vector
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dDVec3d
(
DPoint3dP pSum,
DPoint3dCP pBase,
DVec3dCP pVector
);

//!
//! @description Subtract a vector from a point.
//!
//! @param pSum OUT The computed sum.
//! @param pBase IN The the first point or vector
//! @param pVector IN The second point or vector
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dDVec3d
(
DPoint3dP pSum,
DPoint3dCP pBase,
DVec3dCP pVector
);

//!
//! @description Adds a vector to a pointer or vector, returns the result in place.
//!
//! @param pSum <IN The point or vector to be modified.
//! @param pVector IN The vector to add.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addDVec3d
(
DPoint3dP pSum,
DVec3dCP pVector
);

//!
//! @description Subtract a vector from a point, returning the result in place of the point.
//!
//! @param pVector2 <IN The point or vector to be modified.
//! @param pVector2 IN The vector to subtract.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDVec3d
(
DPoint3dP pVector1,
DVec3dCP pVector2
);

//!
//! @description Adds an origin and a scaled vector.
//!
//! @param pSum OUT The sum
//! @param pOrigin IN Origin for the sum.
//! @param pVector IN The vector to be added.
//! @param scale IN The scale factor.
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDVec3d
(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector,
double           scale
);

//!
//! @description Adds an origin and two scaled vectors.
//!
//! @param pSum OUT sum
//! @param pOrigin IN The origin.  May be null.
//! @param pVector1 IN The first direction vector
//! @param scale1 IN The first scale factor
//! @param pVector2 IN The second direction vector
//! @param scale2 IN The second scale factor
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_add2ScaledDVec3d
(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector1,
double           scale1,
DVec3dCP pVector2,
double           scale2
);

//!
//! @description Adds an origin and three scaled vectors.
//!
//! @param pSum OUT The sum.
//! @param pOrigin IN The origin. May be null
//! @param pVector1 IN The first direction vector
//! @param scale1 IN The first scale factor
//! @param pVector2 IN The second direction vector
//! @param scale2 IN The second scale factor
//! @param pVector3 IN The third direction vector
//! @param scale3 IN The third scale factor
//!
Public GEOMDLLIMPEXP void bsiDPoint3d_add3ScaledDVec3d
(
DPoint3dP pSum,
DPoint3dCP pOrigin,
DVec3dCP pVector1,
double          scale1,
DVec3dCP pVector2,
double          scale2,
DVec3dCP pVector3,
double          scale3
);

END_BENTLEY_GEOMETRY_NAMESPACE

