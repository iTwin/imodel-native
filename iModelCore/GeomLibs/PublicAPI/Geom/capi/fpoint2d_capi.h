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
Public GEOMDLLIMPEXP void     bsiFPoint2d_setXYFloat
(
FPoint2dP pPoint,
float fx,
float fy
);

//!
//! Initialize with given double values.
//! 
//! 
//!
Public GEOMDLLIMPEXP void     bsiFPoint2d_setXY
(
FPoint2dP pPoint,
double x,
double y
);

//!
//! Initialize from a double point.
//! 
//!
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromDPoint3d
(
FPoint2dP pPoint,
DPoint3dCP pDPoint
);

//!
//! Initialize from a double point.
//! 
//!
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromFPoint3d
(
FPoint2dP pPoint,
FPoint3dCP pDPoint
);

//!
//! Initialize from a double point.
//! 
//!
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromDPoint2d
(
FPoint2dP pPoint,
DPoint2dCP pDPoint
);

//!
//! Computes the (cartesian) distance between this instance and the parameter vector.
//!
//! @param pInstance IN      first point
//! @param pPoint IN      second point
//! @return distance to second point.
//! 
//!
Public GEOMDLLIMPEXP double bsiFPoint2d_distance
(
FPoint2dCP pInstance,
FPoint2dCP pPoint
);

//!
//! Computes the squared magnitude of this instance vector.
//!
//! @param pInstance IN      vector whose squared length is computed
//! @return squared magnitude of the vector.
//! 
//!
Public GEOMDLLIMPEXP double bsiFPoint2d_magnitudeSquared (FPoint2dCP pInstance);

//!
//! Computes the squared distance from this instance to the point.
//!
//! @param pInstance IN      base point
//! @param pPoint IN      target point
//! @return squared distance between the points.
//! 
//!
Public GEOMDLLIMPEXP double bsiFPoint2d_distanceSquared
(
FPoint2dCP pInstance,
FPoint2dCP pPoint
);

//!
//! Computes the magnitude of this instance vector.
//!
//! @param pInstance IN      vector
//! @return length of the vector
//! 
//!
Public GEOMDLLIMPEXP double bsiFPoint2d_magnitude (FPoint2dCP pInstance);

//!
//! Finds the largest absolute value among the components of this instance.
//!
//! @param pInstance IN      vector
//! @return largest absoluted value among point coordinates.
//! 
//!
Public GEOMDLLIMPEXP double bsiFPoint2d_maxAbs (FPoint2dCP pInstance);

//!
//! Tests for exact equality between the instance and the parameter vector.
//!
//! @param pInstance IN      first point
//! @param pVec2 IN      second vector
//! @return true if the points are identical.
//! 
//!
Public GEOMDLLIMPEXP bool    bsiFPoint2d_pointEqual
(
FPoint2dCP pInstance,
FPoint2dCP pVec2
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
Public GEOMDLLIMPEXP bool    bsiFPoint2d_pointEqualTolerance
(
FPoint2dCP pInstance,
FPoint2dCP pVec2,
double                  tolerance
);

END_BENTLEY_GEOMETRY_NAMESPACE

