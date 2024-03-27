/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Initialize with given float values.
//!
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_setXYZFloat
(
FPoint3dP pPoint,
float fx,
float fy,
float fz
);

//!
//! Initialize with given double values.
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDoubleArrayXYZ
(
FPoint3dP pPoint,
double      *pXYZ
);

//!
//! Initialize with given float values.
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromFloatArrayXYZ
(
FPoint3dP pPoint,
float       *pXYZ
);

//!
//! Initialize with given double values.
//!
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_setXYZ
(
FPoint3dP pPoint,
double x,
double y,
double z
);

//!
//! Initialize from a double point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDPoint3d
(
FPoint3dP pPoint,
DPoint3dCP pSource
);

//!
//! Initialize from an xy point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromFPoint2d
(
FPoint3dP pPoint,
FPoint2dCP pSource
);

//!
//! Initialize from a double point.
//!
//!
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDPoint2d
(
FPoint3dP pPoint,
DPoint2dCP pSource
);

//!
//! Computes the (cartesian) distance between this instance and the parameter vector.
//!
//! @param pInstance IN      first point
//! @param pPoint IN      second point
//! @return distance to second point.
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_distance
(
FPoint3dCP pInstance,
FPoint3dCP pPoint
);

//!
//! Computes the squared magnitude of this instance vector.
//!
//! @param pInstance IN      vector whose squared length is computed
//! @return squared magnitude of the vector.
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_magnitudeSquared (FPoint3dCP pInstance);

//!
//! Computes the squared distance from this instance to the point.
//!
//! @param pInstance IN      base point
//! @param pPoint IN      target point
//! @return squared distance between the points.
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_distanceSquared
(
FPoint3dCP pInstance,
FPoint3dCP pPoint
);

//!
//! Computes the squred distance between this instance and the point, using only
//! the x and y components.
//!
//! @param pInstance IN      base point
//! @param pPoint IN      target point.
//! @return squared distance between the XY projections ofhte two points.
//!               (i.e. any z difference is ignored)
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_distanceSquaredXY
(
FPoint3dCP pInstance,
FPoint3dCP pPoint
);

//!
//! Computes the magnitude of this instance vector.
//!
//! @param pInstance IN      vector
//! @return length of the vector
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_magnitude (FPoint3dCP pInstance);

//!
//! Finds the largest absolute value among the components of this instance.
//!
//! @param pInstance IN      vector
//! @return largest absoluted value among point coordinates.
//!
//!
Public GEOMDLLIMPEXP double bsiFPoint3d_maxAbs (FPoint3dCP pInstance);

//!
//! Tests for exact equality between the instance and the parameter vector.
//!
//! @param pInstance IN      first point
//! @param pVec2 IN      second vector
//! @return true if the points are identical.
//!
//!
Public GEOMDLLIMPEXP bool    bsiFPoint3d_pointEqual
(
FPoint3dCP pInstance,
FPoint3dCP pVec2
);

//!
//! Test if the x, y, and z components of this instance and parameter vectors
//! are within a tolerance of each other.
//! Tests are done independently using the absolute value of each component differences
//! (i.e. not the magnitude or sum of squared differences)
//!
//! @param pInstance IN      vector
//! @param pVec2 IN      vector
//! @param tolerance IN      tolerance
//! @return true if all components are within given tolerance of each other.
//!
//!
Public GEOMDLLIMPEXP bool    bsiFPoint3d_pointEqualTolerance
(
FPoint3dCP pInstance,
FPoint3dCP pVec2,
double                  tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

