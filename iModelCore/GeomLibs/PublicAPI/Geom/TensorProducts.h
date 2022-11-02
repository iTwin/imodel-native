/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

namespace TensorProducts
{
//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder v direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<DPoint3d>&controlPoints,
size_t uOrder,
size_t vOrder
);

//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder v direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
bvector<double> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<double>&controlPoints,
size_t uOrder,
size_t vOrder
);

//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] uPoints number of points in u direction (i.e. uBasis array has {uPoints * uOrder} doubles)
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] vPoints number of points in v direction (i.e. uBasis array has {vPoints * vOrder} doubles)
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder v direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
double *result,
double const *uBasis,
size_t uPoints,
double const *vBasis,
size_t vPoints,
double const *controlPoints,
size_t uOrder,
size_t vOrder
);

//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder v direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
bvector<double> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
double const *controlPoints,
size_t uOrder,
size_t vOrder
);


//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder v direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
DPoint3d const *controlPoints,
size_t uOrder,
size_t vOrder
);

//! Evaluate a grid of tensor product values by fastest summation for dense grids
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] uPoints number of points in u direction (i.e. uBasis array has {uPoints * uOrder} doubles)
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] vPoints number of points in v direction (i.e. uBasis array has {vPoints * vOrder} doubles)
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder u direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D
(
DPoint3d *result,
double const *uBasis,
size_t uPoints,
double const *vBasis,
size_t vPoints,
DPoint3d const *controlPoints,
size_t uOrder,
size_t vOrder
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool EvaluateGrid2D
(
bvector<DPoint4d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
);


GEOMDLLIMPEXP bool Evaluate
(
bvector<DPoint3d> &xyz,
bvector<DPoint2d> const &uv,
DPoint3d *controlPoints,
size_t uOrder,
size_t vOrder
);

GEOMDLLIMPEXP bool Evaluate
(
bvector<double> &f,
bvector<DPoint2d> const &uv,
double *controlPoints,
size_t uOrder,
size_t vOrder
);


GEOMDLLIMPEXP bool Evaluate
(
bvector<double> &f,
bvector<double> &dfdu,
bvector<double> &dfdv,
bvector<DPoint2d> const &uv,
double *controlPoints,
size_t uOrder,
size_t vOrder
);

GEOMDLLIMPEXP bool Evaluate
(
bvector<DPoint3d> &f,
bvector<DPoint3d> &dfdu,
bvector<DPoint3d> &dfdv,
bvector<DPoint2d> const &uv,
DPoint3d *controlPoints,
size_t uOrder,
size_t vOrder
);

GEOMDLLIMPEXP bool Evaluate
(
DPoint4dR f,
DPoint4dR dfdu,
DPoint4dR dfdv,
DPoint2dCR uv,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
);

GEOMDLLIMPEXP bool Evaluate
(
DPoint4dR f,
DPoint2dCR uv,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
);

//! Evaluate a grid of tensor product values by direct sum at each point.
//! @param [in,out] result Array of values symboloically result[p,q] = SUM(i,j) controlPoints[i,j] * uBasis[p,i] * vBasis[q,j]
//! @param [in] uBasis blocks basis values at u coordinates
//! @param [in] vBasis blocsk of basis values at v coordinates
//! @param [in] controlPoints array of {uOrder * vOrder] control points
//! @param [in] uOrder u direction support count
//! @param [in] uOrder u direction support count
GEOMDLLIMPEXP bool EvaluateGrid2D_Direct
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<DPoint3d>&controlPoints,
size_t uOrder,
size_t vOrder
);
//! Compute numU u values and their bezier basis function blocks.
//! @param [in,out] uValues vector of numU u values.
//! @param [in,out] basisValues vector of numU blocks each containing order bezier basis function values at corresponding uValues entry.
//! @param [in] order bezier polynomial order
//! @param [in] u0 start value for u values
//! @param [in] u1 end value for u values
//! @param [in] numU number of u values.
GEOMDLLIMPEXP void EvaluateBezierBasisFunctionGrid1D (bvector<double> &uValues, bvector<double> &basisValues, size_t order, double u0, double u1, size_t numU);

//! Compute blocked basis function derivatives
//! @param [in,out] basisDerivativeValues blocks of basis functions for derivatives.
//! @param [in] uValues vector of numU u values.
//! @param [in] order bezier polynomial order
GEOMDLLIMPEXP void EvaluateBezierDerivativeBasisFunctions (bvector<double> &basisDerivativeValues, bvector<double> const &uValues, size_t order);


//! Assemble the 1D arrays of u and v values into "u varies fast" grid of uv points
//! @param [in,out] uvValues preallocated buffer of numU * numV points.  Filled with (u,v) pairs, with u varying fastest
//! @param [in] uValues source array for u
//! @param [in] numU number of u values
//! @param [in] vValues source array for v
//! @param [in] numU number of v values
GEOMDLLIMPEXP void AssembleGrid2D (DPoint2d *uvValues, double const *uValues, size_t numU, double const *vValues, size_t numV);

//! Assemble the 1D arrays of u and v values into "u varies fast" grid of uv points
//! @param [in,out] uvValues filled with uValues.size () * vValues.size () (u,v) pairs.
//! @param [in] uValues source array for u
//! @param [in] vValues source array for v
GEOMDLLIMPEXP void AssembleGrid2D (bvector<DPoint2d> &uvValues, bvector<double> &uValues, bvector<double> &vValues);

};
END_BENTLEY_GEOMETRY_NAMESPACE

