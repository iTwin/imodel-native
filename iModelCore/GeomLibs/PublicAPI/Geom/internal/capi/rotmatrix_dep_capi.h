/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_NAMESPACE

//!
//! Computes the product {M*P} where M is this instance matrix and P the input pPoint.
//! The result overwrites the previous coordinates in pPoint.
//!
//! @param pMatrix IN      The matrix to apply
//! @param pPoint IN OUT  The point to be updated
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint3d
(
RotMatrixCP pMatrix,
DPoint3dP pPoint
);

//!
//! @description Returns the product of a matrix times a point.
//! @param pMatrix IN      The matrix.
//! @param pResult OUT     result of the multiplication.
//! @param pPoint IN      The known point.
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixDPoint3d
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint
);

//!
//! @description Returns the product of a matrix transpose times a point.
//! @param pMatrix IN      The the matrix.
//! @param pResult OUT     result of the multiplication.
//! @param pPoint IN      The known point.
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyRotMatrixTransposeDPoint3d
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint
);

//!
//! @description Returns the product of a matrix times a point,
//!           with the point given as separate components.
//!
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     result of multiplication
//! @param x IN      The x component of input point
//! @param y IN      The y component of input point
//! @param z IN      The z component of input point
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyComponents
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
double         x,
double         y,
double         z
);

//!
//! @description Computes the product {P*M} where M is this instance matrix and P is the input point
//! The result overwrites the previous coordinates in the point.
//! @param pMatrix IN      The matrix to apply
//! @param pPoint IN OUT  The point to be multiplied
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeDPoint3d
(
RotMatrixCP pMatrix,
DPoint3dP pPoint
);

//!
//! @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
//!           the product point.
//! @param pMatrix IN      The matrix to apply
//! @param pPoint OUT     product point
//! @param x IN      The x component
//! @param y IN      The y component
//! @param z IN      The z component
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeComponents
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
double      x,
double      y,
double      z
);

//!
//! Computes {M*P[i]} where M is this instance matrix and each P[i] is a point in the input array pPoint.
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyDPoint3dArray
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint,
int            numPoint
);

//!
//! Computes {P[i]*M} where M is this instance matrix and each P[i] is a point
//!   in the input array pPoint, transposed into row form.  (Equivalently, multiply M^*p[i], where M^ indicates transpose.)
//! Each result is placed in the corresponding entry in the output array pResult.   The same array
//! may be named for the input and output arrays.
//! @param pMatrix IN      The matrix to apply
//! @param pResult OUT     output points
//! @param pPoint IN      The input points
//! @param numPoint IN      The number of points
//!
//!
Public GEOMDLLIMPEXP void bsiRotMatrix_multiplyTransposeDPoint3dArray
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint,
int            numPoint
);

//!
//! @description Return the product of a matrix inverse and a point.
//! @vbexception Throws an exception if the matrix is singular (can't be inverted).
//! @param pMatrix        IN      The matrix
//! @param    pResult   OUT     the unknown point
//! @param    pPoint     IN      The The known point
//! @return false if this instance is singular.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3d
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint
);

//!
//! @description Return the product of a matrix inverse transpose and a point.
//! @vbexception Throws an exception if the matrix is singular (can't be inverted).
//! @param pMatrix IN      The matrix whose transpose is inverted
//! @param    pResult OUT     result of the multiplication
//! @param    pPoint IN      The known point multipled by the matrix inverse.
//! @return false if this instance is singular.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3dTranspose
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint
);

//!
//! @description Solves the matrix equation AX=B, where A is this instance, B is the matrix
//! of numPoints input points and X is the matrix of numPoints output points.
//! pPoint and pResult may have identical addresses.
//!
//! @param pMatrix                IN      The matrix
//! @param    pResult   OUT     column points of solution matrix to system
//! @param    pPoint    IN      The column points of constant matrix of system
//! @param    numPoints   IN      The number of input/output points
//! @return false if this instance is singular.
//!
Public GEOMDLLIMPEXP bool    bsiRotMatrix_solveDPoint3dArray
(
RotMatrixCP pMatrix,
DPoint3dP pResult,
DPoint3dCP pPoint,
int         numPoints
);

END_BENTLEY_NAMESPACE

