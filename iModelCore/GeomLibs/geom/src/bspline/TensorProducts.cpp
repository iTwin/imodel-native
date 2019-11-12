/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
namespace TensorProducts {
/*---------------------------------------------------------------------------------**//**
   result <== sum (0<=i<n) (coffs[i] * data[i])
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void Sum (DPoint3dR result, DPoint3dCP data, double const *coffs, size_t n)
    {
    double a;
    result.x = result.y = result.z = 0.0;
    for (size_t i = 0; i < n; i++, data++)
        {
        a = coffs[i];
        result.x += data->x * a;
        result.y += data->y * a;
        result.z += data->z * a;
        }
    }

/*---------------------------------------------------------------------------------**//**
   result <== sum (0<=i<n) (coffs[i] * data[i])
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void Sum (double &result, double const *data, double const *coffs, size_t n)
    {
    result = 0.0;
    for (size_t i = 0; i < n; i++, data++)
        {
        result += coffs[i] * (*data);
        }
    }


/*---------------------------------------------------------------------------------**//**
   result <== sum (0<=i<n) (coffs[i] * data[i])
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void Sum (DPoint4dR result, DPoint4dCP data, double const *coffs, size_t n)
    {
    double a;
    result.x = result.y = result.z = result.w = 0.0;
    for (size_t i = 0; i < n; i++, data++)
        {
        a = coffs[i];
        result.x += data->x * a;
        result.y += data->y * a;
        result.z += data->z * a;
        result.w += data->w * a;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bool Template_EvaluateGrid2D
(
T *result,
double const *uBasis,
size_t uPoints,
double const *vBasis,
size_t vPoints,
T const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;

    T sectionPoints[MAX_BEZIER_CURVE_ORDER];
    T q;
    double const *vCoff0 = &vBasis[0];
    for (size_t u = 0; u < uPoints; u++)
        {
        const double *uCoffs = &uBasis[u * uOrder];
        // Apply u basis values to each v section
        for (size_t vSection = 0; vSection < vOrder; vSection++)
            Sum (sectionPoints[vSection], controlPoints + vSection * uOrder, uCoffs, uOrder);
        for (size_t v = 0; v < vPoints; v++)
            {
            Sum (q, sectionPoints, vCoff0 + v * vOrder, vOrder);
            result[v * uPoints + u] = q;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bool Template_Evaluate
(
T &result,
double const *uBasis,
double const *vBasis,
T const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;

    T sectionPoints[MAX_BEZIER_CURVE_ORDER];
    for (size_t vSection = 0; vSection < vOrder; vSection++)
        Sum (sectionPoints[vSection], controlPoints + vSection * uOrder, uBasis, uOrder);
    Sum (result, sectionPoints, vBasis, vOrder);
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateGrid2D
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<DPoint3d>&controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    size_t numUPoint = uBasis.size () / uOrder;
    size_t numVPoint = vBasis.size () / vOrder;
    size_t numResult = numUPoint * numVPoint;
    result.resize (numResult);
    return Template_EvaluateGrid2D <DPoint3d> (
        &result[0], &uBasis[0], numUPoint, &vBasis[0], numVPoint, &controlPoints[0], uOrder, vOrder);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
bvector<DPoint3d> &f,
bvector<DPoint2d> const &uv,
DPoint3d *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double uBasis[MAX_BEZIER_ORDER];
    double vBasis[MAX_BEZIER_ORDER];
    size_t n = uv.size();
    f.resize (n);
    for (size_t i = 0; i < n; i++)
        {
        bsiBezier_evaluateBasisFunctions (uBasis, (int)uOrder, uv[i].x);
        bsiBezier_evaluateBasisFunctions (vBasis, (int)vOrder, uv[i].y);
        Template_Evaluate <DPoint3d> (f[i], uBasis, vBasis, controlPoints, uOrder, vOrder);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
bvector<double> &f,
bvector<DPoint2d> const &uv,
double *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double uBasis[MAX_BEZIER_ORDER];
    double vBasis[MAX_BEZIER_ORDER];
    size_t n = uv.size();
    f.resize (n);
    for (size_t i = 0; i < n; i++)
        {
        bsiBezier_evaluateBasisFunctions (uBasis, (int)uOrder, uv[i].x);
        bsiBezier_evaluateBasisFunctions (vBasis, (int)vOrder, uv[i].y);
        Template_Evaluate <double> (f[i], uBasis, vBasis, controlPoints, uOrder, vOrder);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
bvector<double> &f,
bvector<double> &dfdu,
bvector<double> &dfdv,
bvector<DPoint2d> const &uv,
double *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double basisU[MAX_BEZIER_ORDER];
    double dBasisU[MAX_BEZIER_ORDER];
    double basisV[MAX_BEZIER_ORDER];
    double dBasisV[MAX_BEZIER_ORDER];
    size_t n = uv.size();
    f.resize (n);
    dfdu.resize (n);
    dfdv.resize (n);
    for (size_t i = 0; i < n; i++)
        {
        bsiBezier_evaluateBasisFunctions (basisU, (int)uOrder, uv[i].x);
        bsiBezier_evaluateDerivativeBasisFunctions (dBasisU, (int)uOrder, uv[i].x);

        bsiBezier_evaluateBasisFunctions (basisV, (int)vOrder, uv[i].y);
        bsiBezier_evaluateDerivativeBasisFunctions (dBasisV, (int)vOrder, uv[i].y);

        Template_Evaluate <double> (f[i], basisU, basisV, controlPoints, uOrder, vOrder);
        Template_Evaluate <double> (dfdu[i], dBasisU, basisV, controlPoints, uOrder, vOrder);
        Template_Evaluate <double> (dfdv[i], basisU, dBasisV, controlPoints, uOrder, vOrder);
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
bvector<DPoint3d> &f,
bvector<DPoint3d> &dfdu,
bvector<DPoint3d> &dfdv,
bvector<DPoint2d> const &uv,
DPoint3d *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double basisU[MAX_BEZIER_ORDER];
    double dBasisU[MAX_BEZIER_ORDER];
    double basisV[MAX_BEZIER_ORDER];
    double dBasisV[MAX_BEZIER_ORDER];
    
    size_t n = uv.size();
    f.resize (n);
    dfdu.resize (n);
    dfdv.resize (n);
    for (size_t i = 0; i < n; i++)
        {
        bsiBezier_evaluateBasisFunctions (basisU, (int)uOrder, uv[i].x);
        bsiBezier_evaluateDerivativeBasisFunctions (dBasisU, (int)uOrder, uv[i].x);

        bsiBezier_evaluateBasisFunctions (basisV, (int)vOrder, uv[i].y);
        bsiBezier_evaluateDerivativeBasisFunctions (dBasisV, (int)vOrder, uv[i].y);

        Template_Evaluate <DPoint3d> (f[i], basisU, basisV, controlPoints, uOrder, vOrder);
        Template_Evaluate <DPoint3d> (dfdu[i], dBasisU, basisV, controlPoints, uOrder, vOrder);
        Template_Evaluate <DPoint3d> (dfdv[i], basisU, dBasisV, controlPoints, uOrder, vOrder);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
DPoint4dR f,
DPoint4dR dfdu,
DPoint4dR dfdv,
DPoint2dCR uv,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double basisU[MAX_BEZIER_ORDER];
    double dBasisU[MAX_BEZIER_ORDER];
    double basisV[MAX_BEZIER_ORDER];
    double dBasisV[MAX_BEZIER_ORDER];
    
    bsiBezier_evaluateBasisFunctions (basisU, (int)uOrder, uv.x);
    bsiBezier_evaluateDerivativeBasisFunctions (dBasisU, (int)uOrder, uv.x);

    bsiBezier_evaluateBasisFunctions (basisV, (int)vOrder, uv.y);
    bsiBezier_evaluateDerivativeBasisFunctions (dBasisV, (int)vOrder, uv.y);

    Template_Evaluate <DPoint4d> (f,    basisU, basisV, controlPoints, uOrder, vOrder);
    Template_Evaluate <DPoint4d> (dfdu, dBasisU, basisV, controlPoints, uOrder, vOrder);
    Template_Evaluate <DPoint4d> (dfdv, basisU, dBasisV, controlPoints, uOrder, vOrder);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP bool Evaluate
(
DPoint4dR f,
DPoint2dCR uv,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    if (uOrder > MAX_BEZIER_CURVE_ORDER
        || vOrder > MAX_BEZIER_CURVE_ORDER)
        return false;
    double basisU[MAX_BEZIER_ORDER];
    double basisV[MAX_BEZIER_ORDER];
    
    bsiBezier_evaluateBasisFunctions (basisU, (int)uOrder, uv.x);

    bsiBezier_evaluateBasisFunctions (basisV, (int)vOrder, uv.y);

    Template_Evaluate <DPoint4d> (f,    basisU, basisV, controlPoints, uOrder, vOrder);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateGrid2D
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
DPoint3d const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    size_t numUPoint = uBasis.size () / uOrder;
    size_t numVPoint = vBasis.size () / vOrder;
    size_t numResult = numUPoint * numVPoint;
    result.resize (numResult);
    return Template_EvaluateGrid2D <DPoint3d> (
        &result[0], &uBasis[0], numUPoint, &vBasis[0], numVPoint, controlPoints, uOrder, vOrder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 09/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateGrid2D
(
bvector<DPoint4d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
DPoint4d const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    size_t numUPoint = uBasis.size () / uOrder;
    size_t numVPoint = vBasis.size () / vOrder;
    size_t numResult = numUPoint * numVPoint;
    result.resize (numResult);
    return Template_EvaluateGrid2D <DPoint4d> (
        &result[0], &uBasis[0], numUPoint, &vBasis[0], numVPoint, controlPoints, uOrder, vOrder);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateGrid2D
(
bvector<double> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
double const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    size_t numUPoint = uBasis.size () / uOrder;
    size_t numVPoint = vBasis.size () / vOrder;
    size_t numResult = numUPoint * numVPoint;
    result.resize (numResult);
    return Template_EvaluateGrid2D <double> (
        &result[0], &uBasis[0], numUPoint, &vBasis[0], numVPoint, controlPoints, uOrder, vOrder);
    }



bool EvaluateGrid2D
(
bvector<double> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<double>&controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    size_t numUPoint = uBasis.size () / uOrder;
    size_t numVPoint = vBasis.size () / vOrder;
    size_t numResult = numUPoint * numVPoint;
    result.resize (numResult);
    return Template_EvaluateGrid2D <double> (
        &result[0], &uBasis[0], numUPoint, &vBasis[0], numVPoint, &controlPoints[0], uOrder, vOrder);
    }

 bool EvaluateGrid2D
(
double *result,
double const *uBasis,
size_t uPoints,
double const *vBasis,
size_t vPoints,
double const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    return Template_EvaluateGrid2D <double> (result, uBasis, uPoints, vBasis, vPoints,
                controlPoints, uOrder, vOrder);
    }

 bool EvaluateGrid2D
(
DPoint3d *result,
double const *uBasis,
size_t uPoints,
double const *vBasis,
size_t vPoints,
DPoint3d const *controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    return Template_EvaluateGrid2D <DPoint3d> (result, uBasis, uPoints, vBasis, vPoints,
                controlPoints, uOrder, vOrder);
    }





 bool EvaluateGrid2D_Direct
(
bvector<DPoint3d> &result,
bvector<double> &uBasis,
bvector<double> &vBasis,
bvector<DPoint3d>&controlPoints,
size_t uOrder,
size_t vOrder
)
    {
    result.clear ();
    size_t uBlocks = uBasis.size () / uOrder;
    size_t vBlocks = vBasis.size () / vOrder;
    if (uOrder * vOrder != controlPoints.size ())
        return false;
    for (size_t v = 0; v < vBlocks; v++)
        {
        for (size_t u = 0; u < uBlocks; u++)
            {
            DPoint3d q;
            q.Zero ();
            for (size_t i = 0; i < uOrder; i++)
                {
                for (size_t j = 0; j < vOrder; j++)
                    {
                    q.SumOf (q, controlPoints[j * uOrder + i], uBasis[u*uOrder + i] * vBasis[v*vOrder + j]);
                    }
                }
            result.push_back (q);
            }
        }
    return true;
    }

 void EvaluateBezierBasisFunctionGrid1D (bvector<double> &basisValues, size_t order, size_t numPoint)
    {
    basisValues.clear();
    for (size_t i = 0; i < numPoint; i++)
        {
        double u = i / double (numPoint - 1);
        size_t uIndex0 = basisValues.size ();
        basisValues.resize (basisValues.size () + order);
        bsiBezier_evaluateBasisFunctions (&basisValues[uIndex0], (int)order, u);
        }
    }

 void EvaluateBezierDerivativeBasisFunctions (bvector<double> &basisDerivativeValues, bvector<double> const &uValues, size_t order)
    {
    basisDerivativeValues.clear();
    size_t n = uValues.size ();
    basisDerivativeValues.resize (n * order);
    size_t step = (size_t) order;
    for (size_t i = 0; i < n; i++)
        bsiBezier_evaluateDerivativeBasisFunctions (&basisDerivativeValues[i * step], (int)order, uValues[i]);
    }



 void EvaluateBezierBasisFunctionGrid1D (bvector<double> &uValues, bvector<double> &basisValues, size_t order, double u0, double u1, size_t numU)
    {
    basisValues.clear();
    uValues.clear ();
    if (numU < 1 || order < 1)
        return;
    uValues.push_back (u0);
    if (numU < 2)
        return;
    uValues.reserve (numU);
    size_t numStep = numU - 1;
    // Do direct assigment on end values, symmetric weigthing on interiors:
    double divM = 1.0 / (double)numStep;
    for (size_t i = 1; i < numStep; i++)
        uValues.push_back ( ((numStep - i) * u0 + i * u1) * divM);
    uValues.push_back (u1);
    basisValues.resize (order * numU);
    for (size_t i = 0; i < numU; i++)
        bsiBezier_evaluateBasisFunctions (&basisValues[i * order], (int)order, uValues[i]);
    }


 void AssembleGrid2D (DPoint2d *uvValues, double const *uValues, size_t numU, double const *vValues, size_t numV)
    {
    size_t k = 0;
    for (size_t j = 0; j < numV; j++)
        {
        double v = vValues[j];
        for (size_t i = 0; i < numU; i++)
            {
            uvValues[k].x = uValues[i];
            uvValues[k].y = v;
            k++;
            }
        }
    }

 void AssembleGrid2D (bvector<DPoint2d> &uvValues, bvector<double> &uValues, bvector<double> &vValues)
    {
    uvValues.clear ();
    size_t numU = uValues.size ();
    size_t numV = vValues.size ();
    if (numU == 0 || numV == 0)
        return;
    uvValues.resize (numU * numV);
    AssembleGrid2D (&uvValues[0], &uValues[0], numU, &vValues[0], numV);
    }

}   // Tensor products namespace
END_BENTLEY_GEOMETRY_NAMESPACE