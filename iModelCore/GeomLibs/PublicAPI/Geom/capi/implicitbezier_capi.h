/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Compute bezier coefficients for the determinant of
//!   (1-u) * ascale * A + u * bscale * B
//! where A and B are 3x3 matrices.
//! @param pCoffs OUT     coefficients of polynomial determinant.
//! @param pOrder OUT     order of polynomial.
//! @param maxOrder OUT     max order permitted.
//! @param pA IN      first matrix.
//! @param pB IN      second matrix.
//!
Public GEOMDLLIMPEXP bool    bsiBezier_expandDMatrix3dPencilDeterminant
(
double  *pCoffs,
int     *pOrder,
int     maxOrder,
      double      ascale,
const DMatrix3d   *pA,
      double      bscale,
const DMatrix3d   *pB
);

//!
//! Compute bezier coefficients for the determinant of
//!   (1-u) * ascale * A + u * bscale * B
//! where A and B are 4x4 matrices.
//! @param pCoffs OUT     coefficients of polynomial determinant.
//! @param pOrder OUT     order of polynomial.
//! @param maxOrder OUT     max order permitted.
//! @param pA IN      first matrix.
//! @param pB IN      second matrix.
//!
Public GEOMDLLIMPEXP bool    bsiBezier_expandDMatrix4dPencilDeterminant
(
double  *pCoffs,
int     *pOrder,
int     maxOrder,
      double      ascale,
const DMatrix4d   *pA,
      double      bscale,
const DMatrix4d   *pB
);

//!
//!
//! @param
//! @return true if sufficient storage to solve.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectXYExt
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      double    *pConditionRatio,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB,
      double    conditionLimit
);

//!
//!
//! @param
//! @return true if sufficient storage to solve.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectXY
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB
);

//!
//! Compute apparent intersection of a conic and bezier curve.
//! @return true if sufficient storage to solve.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectDConic4dXYExt
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointConic,
      double    *pAngleConic,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DConic4d  *pConic,
      bool      extendConic
);

//!
//! Compute apparent intersection of a conic and bezier curve.
//! @param
//! @return true if sufficient storage to solve.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_intersectDConic4dXY
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointConic,
      double    *pAngleConic,
      int       *pNumIntersection,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DConic4d  *pConic
);

END_BENTLEY_GEOMETRY_NAMESPACE

