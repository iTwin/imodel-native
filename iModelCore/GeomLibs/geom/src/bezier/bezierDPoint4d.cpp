/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_unitWeightRelTol = 1.0e-8;

/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Given two control points (xyzw) of an xyw curve, compute and scale the cross product
* of the xyw parts as used in Sederberg's implicitization.
* @param pProduct => plane coefficients, with z==0, scaled by iCn jCn
* @param pPointi => i'th pole
* @param pPointj => j'th pole
* @param i       => pole index
* @param j       => pole index
* @param order   => curve order.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_lineProductXYW
(
DPoint4d    *pProduct,
const DPoint4d    *pPointi,
const DPoint4d    *pPointj,
int         i,
int         j,
int         order
)
    {
    double s = bsiBezier_pascalProductInRow (i, j, order - 1);

    pProduct->x = s * (pPointi->y * pPointj->w - pPointi->w * pPointj->y);
    pProduct->y = s * (pPointi->w * pPointj->x - pPointi->x * pPointj->w);
    pProduct->z = 0.0;
    pProduct->w = s * (pPointi->x * pPointj->y - pPointi->y * pPointj->x);
    }

/*---------------------------------------------------------------------------------**//**
* Given two section curves on a surface, compute and scale the cross product
* of stringers of designated poles as used in Sederberg's implicitization.
* @param pProduct => poles for plane coefficients, with z==0, scaled by iCn jCn
* @param pPointi => i'th pole stringer
* @param pPointj => j'th pole stringer
* @param order => order of both i, j.
* @param stride => stride between adjacent DPoint4d's within each stringer.
* @param i       => pole index
* @param j       => pole index
* @param order   => curve order.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_stringerLineProductXYW
(
DPoint4d    *pProductPoles,
int         *pProductOrder,
int         maxProductOrder,
const DPoint4d    *pPointPolesi,
const DPoint4d    *pPointPolesj,
int   stringerOrder,
int   stride,
int         stringerI,
int         stringerJ,
int         lineProductOrder
)
    {
    double s = bsiBezier_pascalProductInRow (stringerI, stringerJ, lineProductOrder - 1);
    int productOrder = 2 * stringerOrder - 1;
    int i;
    int inputStrideDoubles  = 4 * stride;
    bool    boolstat = false;
    int outputStrideDoubles = 4;
    int out, in0, in1;
    static int outputComponent[3] = {0, 1, 3};
    static int inputComponent0[3] = {1, 3, 0};
    static int inputComponent1[3] = {3, 0, 1};

    *pProductOrder = 0;
    if (productOrder <= maxProductOrder && productOrder <= MAX_BEZIER_ORDER)
        {
        *pProductOrder = productOrder;
        memset (pProductPoles, 0, productOrder * sizeof (DPoint4d));
        /* Remark: We really expect that passing our MAX_BEZIER_ORDER test is sufficient
            to assure that all products can be completed.  However, we're going to
            test every step just in case the other side changes.... */
        boolstat = true;
        for (i = 0; i < 3 && boolstat; i++)
            {
            out = outputComponent[i];
            in0 = inputComponent0[i];
            in1 = inputComponent1[i];
            if (boolstat)
                boolstat= bsiBezier_accumulateUnivariateProduct (
                        (double*)pProductPoles, out, outputStrideDoubles,
                        s,
                        const_cast<double*>((double const*)pPointPolesi), stringerOrder, in0, inputStrideDoubles,
                        const_cast<double*>((double const*)pPointPolesj), stringerOrder, in1, inputStrideDoubles
                        );
            if (boolstat)
                boolstat= bsiBezier_accumulateUnivariateProduct (
                        (double*)pProductPoles, out, outputStrideDoubles,
                        -s,
                        const_cast<double*>((double const*)pPointPolesi), stringerOrder, in1, inputStrideDoubles,
                        const_cast<double*>((double const*)pPointPolesj), stringerOrder, in0, inputStrideDoubles
                        );
            }
        }
    return boolstat;
    }




/*---------------------------------------------------------------------------------**//**
* Evaluate poles the psuedo tangent function for 1, 2, or 3 dimensions of a possibly homogeneous curve.
* The pseudo tangent of a nonrational curve is "just" the simple derivative of the curve
*   components. (e.g. the equation above with w(t) identically 1, w'(t) identically 0)
* For a rational curve, the true (properly scaled) tangent vector x component is
*               (x'(t) w(t) - x(t) w'(t)) / w(t)^2
* If only the direction, but not the magnitude, is of interest, many problems (e.g. range,
*   nearest point) can be formulated ignoring the division by w(t)^2.  The numerator
*   term is the pseudo tangent.
* @param pPoleOut       <= tangent poles, pure cartesian vector components.
* @param pOrderOut      <= order of the tangent.
* @param maxOrderOut    => maximum allowed order of the tangent.  Be prepared for 2*order - 1 !!!
* @param pPoleIn        => curve poles, full DPoint4d with possibly constant weights
* @param order          => curve order
* @param numDim         => 1, 2, or 3 -- number of derivative components desired. Unused
*                           components of the DPoint3d outputs are zero.
* @return true if the pseudo tangent was computed in the allowed order.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_pseudoTangent
(
DPoint3d        *pPoleOut,
int             *pOrderOut,
int             maxOrderOut,
const DPoint4d  *pPoleIn,
int             order,
int             numDim
)
    {
    double poleXWPrime[MAX_BEZIER_ORDER];
    double poleXPrimeW[MAX_BEZIER_ORDER];
    double poleDiff[MAX_BEZIER_ORDER];
    int i;
    DPoint4d derivativePoles[MAX_BEZIER_ORDER];
    int derivativeOrder = order - 1;
    int productOrder = order + derivativeOrder - 1;

    *pOrderOut = 0;
    bsiBezier_derivativePoles ((double *)derivativePoles, const_cast<double*>((double const*)pPoleIn), order, 4);

    if (bsiBezierDPoint4d_isUnitWeight (pPoleIn, order, s_unitWeightRelTol))
        {
        if (derivativeOrder > maxOrderOut)
            return false;
        *pOrderOut = derivativeOrder;

        if (numDim < 3)
            memset (pPoleOut, 0, derivativeOrder * sizeof (DPoint3d));

        for (i = 0; i < numDim; i++)
            {
            bsiBezier_copyComponent
                        (
                        (double *)pPoleOut, i, 3,
                        (double *)derivativePoles, i, 4,
                        derivativeOrder
                        );
            }
        }
    else
        {
        if (productOrder > maxOrderOut)
            return false;

        *pOrderOut = productOrder;
        if (numDim < 3)
            memset (pPoleOut, 0, productOrder * sizeof (DPoint3d));

        for (i = 0; i < numDim; i++)
            {
            bsiBezier_univariateProduct (poleXPrimeW, 0, 1,
                            (double *)derivativePoles, derivativeOrder, i, 4,
                            const_cast<double*>((double const*)pPoleIn), order, 3, 4
                            );
            bsiBezier_univariateProduct (poleXWPrime, 0, 1,
                            const_cast<double*>((double const*)pPoleIn), order, i, 4,
                            (double *)derivativePoles, derivativeOrder, 3, 4);
            bsiBezier_subtractPoles (poleDiff, poleXPrimeW, poleXWPrime, productOrder, 1);
            bsiBezier_copyComponent ((double *)pPoleOut, i, 3, poleDiff, 0, 1, productOrder);
            }
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate poles of a function which evaluates to a vector from a fixed point towards
* points on the curve.
* For a rational curve, the true (properly scaled) difference vector x component is
*               (x(t) a.w - a.x w(t)) / (w(t) a.w)
* If the curve weight w(t) is identically 1, this simplifies to
*               (x(t) a.w - a.x) / a.w
* If only the direction, but not the magnitude, is of interest, many problems (e.g.
*   nearest point) can be formulated ignoring the division .  The numerator
*   term (x(t) a.w - a.x w(t)) is the pseudo tangent; the function uses the simplified
*   form if w(t) is one.
*
* If the a.w is zero, the "vector" part of A is returned as a constant (order 1!!!) bezier.
*
* @param pPoleOut       <= tangent poles, pure cartesian vector components.
* @param pOrderOut      <= order of the tangent.
* @param maxOrderOut    => maximum allowed order of the tangent.  Be prepared for 2*order - 1 !!!
* @param pPoleIn        => curve poles, full DPoint4d with possibly constant weights
* @param order          => curve order
* @param pPointA        => fixed point.
* @param numDim         => 1, 2, or 3 -- number of tangent components desired. Unused
*                           components of the DPoint3d outputs are zero.
* @return true if the pseudo tangent was computed in the allowed order.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_pseudoVectorFromDPoint4d
(
DPoint3d        *pPoleOut,
int             *pOrderOut,
int             maxOrderOut,
const DPoint4d  *pPoleIn,
int             order,
const DPoint4d  *pPointA,
int             numDim
)
    {
    int i;
    double aw = pPointA->w;
    if (order > maxOrderOut)
        return false;
    *pOrderOut = 0;

    if (numDim < 3)
        memset (pPoleOut, 0, order * sizeof (DPoint3d));

    if (aw == 0.0)
        {
        if (1 > maxOrderOut)
            return false;
        *pOrderOut = 1;
        pPointA->GetXYZ (*pPoleOut);
        }
    else if (bsiBezierDPoint4d_isUnitWeight (pPoleIn, order, s_unitWeightRelTol))
        {
        if (order > maxOrderOut)
            return false;
        *pOrderOut = order;
        if (numDim > 0)
            for (i = 0; i < order; i++)
                pPoleOut[i].x = pPoleIn[i].x * aw - pPointA->x;

        if (numDim > 1)
            for (i = 0; i < order; i++)
                pPoleOut[i].y = pPoleIn[i].y * aw - pPointA->y;

        if (numDim > 2)
            for (i = 0; i < order; i++)
                pPoleOut[i].z = pPoleIn[i].z * aw - pPointA->z;
        }
    else
        {
        if (order > maxOrderOut)
            return false;
        *pOrderOut = order;
        if (numDim > 0)
            for (i = 0; i < order; i++)
                pPoleOut[i].x = pPoleIn[i].x * aw - pPointA->x * pPoleIn[i].w;

        if (numDim > 1)
            for (i = 0; i < order; i++)
                pPoleOut[i].y = pPoleIn[i].y * aw - pPointA->y * pPoleIn[i].w;

        if (numDim > 2)
            for (i = 0; i < order; i++)
                pPoleOut[i].z = pPoleIn[i].z * aw - pPointA->z * pPoleIn[i].w;
        }
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* Compute the (exact) 3d range of a homogeneous bezier
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_getDRange3d
(
        DRange3d    *pRange,
const   DPoint4d    *pPoles,
        int         order
)
    {
    DPoint3d tangentPoles[MAX_BEZIER_ORDER];
    double componentPoles[MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    int tangentOrder;
    int i, j;
    DPoint4d extremePoint;
    int numRoot;
    bsiBezierDPoint4d_pseudoTangent (tangentPoles, &tangentOrder, MAX_BEZIER_ORDER,
                            pPoles, order, 3);
    pRange->Init ();
    pRange->Extend (pPoles[0]);
    pRange->Extend (pPoles[ order - 1]);

    if (order <= 2)
        return;

    for (i = 0; i < 3; i++)
        {
        bsiBezier_copyComponent (componentPoles, 0, 1, (double *)tangentPoles, i, 3, tangentOrder);
        bsiBezier_univariateRoots (root, &numRoot, componentPoles, tangentOrder);
        for (j = 0; j < numRoot; j++)
            {
            bsiBezier_functionAndDerivative ((double*)&extremePoint, NULL, const_cast<double*>((double const*)pPoles), order, 4, root[j]);
            pRange->Extend (extremePoint);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Compute the (exact) 3d range of a homogeneous bezier
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_getDRange3d
(
 DRange3dP  pRange,
DPoint3dCP  pPoles,
int         order
)
    {
    double tangentPoles[3][MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    int tangentOrder = order - 1;
    DPoint3d extremePoint;
    int numRoot;

    pRange->Init ();

    if (order < 1)
        return;

    // Compute component differences in to flat arrays.
    for (int i = 0; i < tangentOrder; i++)
        {
        tangentPoles[0][i] = pPoles[i+1].x - pPoles[i].x;
        tangentPoles[1][i] = pPoles[i+1].y - pPoles[i].y;
        tangentPoles[2][i] = pPoles[i+1].z - pPoles[i].z;
        }

    pRange->Extend (pPoles[0]);
    pRange->Extend (pPoles[ order - 1]);

    for (int componentIndex = 0; componentIndex < 3; componentIndex++)
        {
        bsiBezier_univariateRoots (root, &numRoot, tangentPoles[componentIndex], tangentOrder);
        for (int rootIndex = 0; rootIndex < numRoot; rootIndex++)
            {
            bsiBezier_functionAndDerivative ((double*)&extremePoint, NULL, const_cast<double*>((double const*)pPoles), order, 3, root[rootIndex]);
            pRange->Extend (extremePoint);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Return a single number which represents the numeric data range in the bezier,
* using only the normalized xy parts.  This is useful as a maximum coordiante for tolerance
* determination.
*
* @param pPoles => poles whose range is computed.
* @param order  => number of poles.
* @return a single double representing the normalized xy range of the poles.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_getLargestCoordinateXY
(
const   DPoint4d    *pPoles,
        int         order
)
    {
    DRange3d range;

    range.Init ();
    range.Extend (pPoles, order);
    return range.IsNull () ? 1.0 : range.LargestCoordinate ();
    }

typedef struct
    {
    const DPoint4d *pPoleArray;
    int   order;
    int   count;
    RotMatrix worldToLocal;
    } ArcLengthParams;

/*---------------------------------------------------------------------------------**//**
* Compute the arc length integrand at given parameter.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    cbArcLengthIntegrand
(
double  *pTangentMagnitude,
double  s,
ArcLengthParams *pParams
)
    {
    DPoint4d X, dX;
    double ex, ey, ez;
    bsiBezier_functionAndDerivative ((double*)&X, (double*)&dX,
                        const_cast<double*>((double const*)pParams->pPoleArray), pParams->order, 4, s);

    pParams->count++;
    if (X.w == 1.0 && dX.w == 0.0)
        {
        ex = dX.x;
        ey = dX.y;
        ez = dX.z;
        *pTangentMagnitude = sqrt (ex * ex + ey * ey + ez * ez);
        }
    else
        {
        double w = X.w;
        double dw = dX.w;
        if (w == 0.0)
            *pTangentMagnitude = 0.0;

        ex = w * dX.x - dw * X.x;
        ey = w * dX.y - dw * X.y;
        ez = w * dX.z - dw * X.z;

        *pTangentMagnitude = sqrt (ex * ex + ey * ey + ez * ez) / (w * w);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the arc length integrand at given parameter.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    cbArcLengthIntegrandWithWorldToLocal
(
double  *pTangentMagnitude,
double  s,
ArcLengthParams *pParams
)
    {
    DPoint4d X, dX;
    double ex, ey, ez;
    bsiBezier_functionAndDerivative ((double*)&X, (double*)&dX,
                        const_cast<double*>((double const*)pParams->pPoleArray), pParams->order, 4, s);
    DVec3d vector;
    pParams->count++;
    if (X.w == 1.0 && dX.w == 0.0)
        {
        ex = dX.x;
        ey = dX.y;
        ez = dX.z;
        pParams->worldToLocal.MultiplyComponents (vector, ex, ey, ez);
        *pTangentMagnitude = vector.Magnitude ();
        }
    else
        {
        double w = X.w;
        double dw = dX.w;
        if (w == 0.0)
            *pTangentMagnitude = 0.0;

        ex = w * dX.x - dw * X.x;
        ey = w * dX.y - dw * X.y;
        ez = w * dX.z - dw * X.z;
        pParams->worldToLocal.MultiplyComponents (vector, ex, ey, ez);

        *pTangentMagnitude = vector.Magnitude () / (w * w);
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the arc length of a homogeneous bezier curve.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_arcLength
(
const   DPoint4d    *pPoles,
        int         order,
        double      s0,
        double      s1
)
    {
    ArcLengthParams params;
    double arcLength, error;
    int count;
    static double s_relTol = 1.0e-12;

    params.pPoleArray   = pPoles;
    params.order        = order;
    params.count        = 0;
    /*
    bsiMath_recursiveSimpson (&arcLength, &error, &count, s0, s1, 0.0, s_relTol, cbArcLengthIntegrand, &params);
    */
    bsiMath_recursiveNewtonCotes5 (&arcLength, &error, &count, s0, s1, 0.0, s_relTol,
                (PFScalarIntegrand)cbArcLengthIntegrand, &params);
    return arcLength;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the arc length of a homogeneous bezier curve.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_arcLength
(
RotMatrixCP worldToLocal,
const   DPoint4d    *pPoles,
        int         order,
        double      s0,
        double      s1
)
    {
    ArcLengthParams params;
    double arcLength, error;
    int count;
    static double s_relTol = 1.0e-12;

    params.pPoleArray   = pPoles;
    params.order        = order;
    params.count        = 0;
    params.worldToLocal = *worldToLocal;
    /*
    bsiMath_recursiveSimpson (&arcLength, &error, &count, s0, s1, 0.0, s_relTol, cbArcLengthIntegrand, &params);
    */
    bsiMath_recursiveNewtonCotes5 (&arcLength, &error, &count, s0, s1, 0.0, s_relTol,
                (PFScalarIntegrand)cbArcLengthIntegrandWithWorldToLocal, &params);
    return arcLength;
    }


/*---------------------------------------------------------------------------------**//**
* Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
* @param pX         <= array of points on curve.
* @param pdX        <= array of tangent vectors.
* @param pddX       <= array of second derivative vectors
* @param pPoleArray => poles of spline curve
* @param order      => spline order
* @param pParam     => array of parameter values
* @param numParam   => number of points to evaluate.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint3dArrayExt
(
        DPoint3d    *pX,
        DPoint3d    *pdX,
        DPoint3d    *pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
)
    {
    DPoint4d X, dX, ddX;
    DPoint3d tempX;
    double divW;
    double divW2;
    double divW4;
    double a;
    double w, dw, ddw;
    int i;
    for (i = 0; i < numParam; i++)
        {
        bsiBezier_functionAndDerivativeExt ((double*)&X, (double*)&dX, (double*)&ddX,
                        const_cast<double*>((double const*)pPoleArray), order, 4, pParam[i]);

        w = X.w;
        dw = dX.w;
        ddw = ddX.w;
        DoubleOps::SafeDivide (divW, 1.0, w, 0.0);
        divW2 = divW * divW;
        if (pX)
            pX[ i].Scale (*((DPoint3d *)&X), divW);
        if (pdX)
            pdX[ i].SumOf(*((DPoint3d *)&dX), divW, *((DPoint3d*)&X), - dw * divW2);
        if (pddX)
            {
            divW4 = divW2 * divW2;
            a = -2.0 * w * dw * divW4;
            tempX.SumOf(*((DPoint3d *) &ddX), w * divW2, *((DPoint3d *)&X), - ddw * divW2);
            pddX[ i].SumOf(tempX, *((DPoint3d *) &dX), w * a, *((DPoint3d *)&X), -dw * a);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute point and tangent at equally spaced parameters parameters.  Zero weight points are set to zero.
* @param pX         <= array of points on curve.
* @param pdX        <= array of tangent vectors.
* @param pddX       <= array of second derivative vectors
* @param pPoleArray => poles of spline curve
* @param order      => spline order
* @param numParam   => number of (equally spaced) parameters to evaluate. AT LEAST 2 are evaluated.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_appendEvaluations
(
bvector<DPoint3d>*pX,
bvector<DVec3d>*pdX,
bvector<DVec3d>*pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        size_t      numParam
)
    {
    DPoint4d X, dX, ddX;
    DPoint3d Y;
    DVec3d dY, ddY;
    DVec3d tempX;
    double divW;
    double divW2;
    double divW4;
    double a;
    double w, dw, ddw;
    if (numParam < 2)
        numParam = 2;
    double df = 1.0 / (double)(numParam - 1);

    for (size_t i = 0; i < numParam; i++)
        {
        double f = i * df;
        bsiBezier_functionAndDerivativeExt ((double*)&X, (double*)&dX, (double*)&ddX,
                        const_cast<double*>((double const*)pPoleArray), order, 4, f);

        w = X.w;
        dw = dX.w;
        ddw = ddX.w;
        DoubleOps::SafeDivide (divW, 1.0, w, 0.0);
        divW2 = divW * divW;
        if (pX)
            {
            Y.Scale (*((DVec3d *)&X), divW);
            pX->push_back (Y);
            }
        if (pdX)
            {
            dY.SumOf(*((DVec3d *)&dX), divW, *((DVec3d*)&X), - dw * divW2);
            pdX->push_back (dY);
            }
        if (pddX)
            {
            divW4 = divW2 * divW2;
            a = -2.0 * w * dw * divW4;
            tempX.SumOf(*((DVec3d *) &ddX), w * divW2, *((DVec3d *)&X), - ddw * divW2);
            ddY.SumOf(tempX, *((DVec3d *) &dX), w * a, *((DVec3d *)&X), -dw * a);
            pddX->push_back (ddY);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
* @param pX         <= array of points on curve.
* @param pdX        <= array of tangent vectors.
* @param pddX       <= array of second derivative vectors
* @param pPoleArray => poles of spline curve
* @param order      => spline order
* @param pParam     => array of parameter values
* @param numParam   => number of points to evaluate.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint4dArrayExt
(
        DPoint4d    *pX,
        DPoint4d    *pdX,
        DPoint4d    *pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
)
    {
    int i;
    for (i = 0; i < numParam; i++)
        {
        bsiBezier_functionAndDerivativeExt
                        (
                        (double*)(pX+i),
                        (double*)(pdX+i),
                        (double*)(pddX+i),
                        const_cast<double*>((double const*)pPoleArray),
                        order,
                        4,
                        pParam[i]);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
* @param pX         <= array of points on curve.
* @param pdX        <= array of tangent vectors.
* @param pPoleArray => poles of spline curve
* @param order      => spline order
* @param pParam     => array of parameter values
* @param numParam   => number of points to evaluate.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint3dArray
(
        DPoint3d    *pX,
        DPoint3d    *pdX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
)
    {
    bsiBezierDPoint4d_evaluateDPoint3dArrayExt (pX, pdX, NULL, pPoleArray, order, pParam, numParam);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the xy distance from xx,yy to the normalized image of a point. Does NOT test
*   for zero weight.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  double     jmdlBezierDPoint4d_dist2XY
(
const   DPoint4d    *pPoint,
        double      xx,
        double      yy
)
    {
    double x, y;
    x = pPoint->x / pPoint->w;
    y = pPoint->y / pPoint->w;

    return (x - xx) * (x - xx) + (y - yy) * (y - yy);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the xy distance from xx,yy, zz to the normalized image of a point. Does NOT test
*   for zero weight.
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  double     jmdlBezierDPoint4d_dist2
(
const   DPoint4d    *pPoint,
        double      xx,
        double      yy,
        double      zz
)
    {
    double dx, dy, dz;
    dx = pPoint->x / pPoint->w - xx;
    dy = pPoint->y / pPoint->w - yy ;
    dz = pPoint->z / pPoint->w - zz;

    return dx * dx + dy * dy + dz * dz;
    }

/*---------------------------------------------------------------------------------**//**
* Upate the point, parameter, and distance to closest approach
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  void       jmdlBezierDPoint4d_updateClosePoint
(
        bool        *pIsFirstPoint,
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoint,
        double      param,
        double      xx,
        double      yy
)
    {

    if (   pPoint->w != 0.0)
        {
        double dist2 = jmdlBezierDPoint4d_dist2XY (pPoint, xx, yy);
        if (*pIsFirstPoint || dist2 < *pCloseDist2)
            {
            *pCloseDist2    = dist2;
            *pClosePoint    = *pPoint;
            *pCloseParam    = param;
            *pIsFirstPoint  = false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Upate the point, parameter, and distance to closest approach
*
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  void       jmdlBezierDPoint4d_updateClosePointXYZ
(
        bool        *pIsFirstPoint,
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoint,
        double      param,
        double      xx,
        double      yy,
        double      zz
)
    {

    if (   pPoint->w != 0.0)
        {
        double dist2 = jmdlBezierDPoint4d_dist2 (pPoint, xx, yy, zz);
        if (*pIsFirstPoint || dist2 < *pCloseDist2)
            {
            *pCloseDist2    = dist2;
            *pClosePoint    = *pPoint;
            *pCloseParam    = param;
            *pIsFirstPoint  = false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find the Bezier curve point closest to a space point, measuring in
* cartesian xy space only.  Only check points at or between the given parameters.
* @param pClosePoint <= xyzw of closest point
* @param pCloseParam <= parameter of closest point
* @param pCloseDist2 <= squared xy distance to closest point
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param xx         => x coordinate of space point
* @param yy         => y coordinate of space point
* @param s0         => minimum parameter to consider
* @param s1         => maximum parameter to consider
* @return true if a near point (possibly an endpoint) was computed.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_closestXYPoint
(
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoleArray,
        int         order,
        double      xx,
        double      yy,
        double      s0,
        double      s1
)
    {
    double rootArray[MAX_BEZIER_ORDER + 2];
    DPoint4d fixedPoint;
    double s, sMin, sMax;
    DPoint4d candidatePoint;
    int i, numRoot;

    bool    firstPoint;

    fixedPoint.Init( xx, yy, 0.0, 1.0);
    bsiBezierDPoint4d_allPerpendicularsFromDPoint4d
                (rootArray, NULL, &numRoot, MAX_BEZIER_ORDER, pPoleArray, order, &fixedPoint, 2);
    firstPoint = true;
    /* Remark: 'twould be nice to avoid full evaluation of these points in the (common)
        cases where s0 and s1 are endpoints */

    bsiBezier_functionAndDerivative ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s0);
    jmdlBezierDPoint4d_updateClosePoint (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                        &candidatePoint, s0, xx, yy);

    bsiBezier_functionAndDerivative ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s1);
    jmdlBezierDPoint4d_updateClosePoint (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                            &candidatePoint, s1, xx, yy);

    if (s0 < s1)
        {
        sMin = s0;
        sMax = s1;
        }
    else
        {
        sMin = s1;
        sMax = s0;
        }

    for (i = 0; i < numRoot; i++)
        {
        s = rootArray[i];
        if ( s > sMin && s < sMax)
            {
            bsiBezier_functionAndDerivative ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s);
            jmdlBezierDPoint4d_updateClosePoint (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                                &candidatePoint, s, xx, yy);
            }
        }

    return !firstPoint;

    }


/*---------------------------------------------------------------------------------**//**
* Find the Bezier curve point closest to a space point, measuring in
* cartesian xy space only.  Only check points at or between the given parameters.
* @param pClosePoint <= xyzw of closest point
* @param pCloseParam <= parameter of closest point
* @param pCloseDist2 <= squared xy distance to closest point
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param xx         => x coordinate of space point
* @param yy         => y coordinate of space point
* @param zz         => z coordinate of space point
* @param s0         => minimum parameter to consider
* @param s1         => maximum parameter to consider
* @return true if a near point (possibly an endpoint) was computed.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_closestPoint
(
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoleArray,
        int         order,
        double      xx,
        double      yy,
        double      zz,
        double      s0,
        double      s1
)
    {
    double rootArray[MAX_BEZIER_ORDER + 2];
    DPoint4d fixedPoint;
    double s, sMin, sMax;
    DPoint4d candidatePoint;
    int i, numRoot;

    bool    firstPoint;

    fixedPoint.Init( xx, yy, zz, 1.0);
    bsiBezierDPoint4d_allPerpendicularsFromDPoint4d
                (rootArray, NULL, &numRoot, MAX_BEZIER_ORDER, pPoleArray, order, &fixedPoint, 3);
    firstPoint = true;
    /* Remark: 'twould be nice to avoid full evaluation of these points in the (common)
        cases where s0 and s1 are endpoints */

    bsiBezier_functionAndDerivative ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s0);
    jmdlBezierDPoint4d_updateClosePointXYZ (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                        &candidatePoint, s0, xx, yy, zz);

    bsiBezier_functionAndDerivative ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s1);
    jmdlBezierDPoint4d_updateClosePointXYZ (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                            &candidatePoint, s1, xx, yy, zz);

    if (s0 < s1)
        {
        sMin = s0;
        sMax = s1;
        }
    else
        {
        sMin = s1;
        sMax = s0;
        }

    for (i = 0; i < numRoot; i++)
        {
        s = rootArray[i];
        if ( s > sMin && s < sMax)
            {
            bsiBezier_functionAndDerivative
                    ((double*)&candidatePoint, NULL, const_cast<double*>((double const*)pPoleArray), order, 4, s);
            jmdlBezierDPoint4d_updateClosePointXYZ (&firstPoint, pClosePoint, pCloseParam, pCloseDist2,
                                &candidatePoint, s, xx, yy, zz);
            }
        }

    return !firstPoint;

    }


/*---------------------------------------------------------------------------------**//**
* Conditionally add parameter and point to output arrays.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlBezierDPoint4d_saveParameterAndDPoint4d
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
        double      s
)
    {
    int i = *pNumOut;
    if (*pNumOut >= maxOut)
        return false;
    if (pParamArray)
        pParamArray[i] = s;
    if (pPointArray)
        bsiBezier_functionAndDerivative
                        (
                        (double*)&pPointArray[i],
                        NULL, const_cast<double*>((double const*)pPoleArray),
                        order,
                        4,
                        s);
    *pNumOut += 1;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier its perpendicular to the (moving) line from
* given fixed point.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pFixedPoint => dropping perpendicular from here.
* @param dimension  => 2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allPerpendicularsFromDPoint4d
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension
)
    {
    return bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt (pParamArray,
                pPointArray, pNumOut, maxOut, pPoleArray, order, pFixedPoint,
                workDimension, false);
    }

/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier its perpendicular to the (moving) line from
* given fixed point.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pFixedPoint => dropping perpendicular from here.
* @param dimension  => 2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension,
        bool        extend
)
    {
    DPoint3d tangentPoles[MAX_BEZIER_ORDER];
    DPoint3d eyePoles[MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    double fPoles[MAX_BEZIER_ORDER];
    int numRoot, i;
    int orderF;
    int orderTangent;
    int orderEye;

    if (   bsiBezierDPoint4d_pseudoTangent (tangentPoles, &orderTangent, MAX_BEZIER_ORDER,
                                    pPoleArray, order, workDimension)
        && bsiBezierDPoint4d_pseudoVectorFromDPoint4d
                                    (eyePoles, &orderEye, MAX_BEZIER_ORDER,
                                    pPoleArray, order, pFixedPoint, workDimension)
        && bsiBezier_dotProduct (fPoles, &orderF, MAX_BEZIER_ORDER,
                                    (double*)tangentPoles, orderTangent, 0, 3,
                                    (double*)eyePoles,     orderEye,     0, 3, workDimension)
        )
        {
        if (extend)
            bsiBezier_univariateRootsExt (root, &numRoot, fPoles, orderF);
        else
            {
            bsiBezier_univariateRoots (root, &numRoot, fPoles, orderF);
            bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, orderF, 0, 0.0, 0.0);
            bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, orderF, 1, 0.0, 0.0);
            }

        *pNumOut = 0;

        if (numRoot == orderF)
            return true;

        for (i = 0; i < numRoot; i++)
            {
            if (!jmdlBezierDPoint4d_saveParameterAndDPoint4d (pParamArray, pPointArray, pNumOut, maxOut,
                                    pPoleArray, order, root[i]))
                return false;
            }
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier intersects a plane.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pPlaneCoffs => plane equation
* @param workDimension  => unused.  (set z=0 in plane coefficients to get xy effects)
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allDPlane4dIntersections
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pPlaneCoffs,
        int         workDimension,
        bool        extend
)
    {
    double root[MAX_BEZIER_ORDER];
    double fPoles[MAX_BEZIER_ORDER];
    int numRoot;

    int i;

    for (i = 0; i < order; i++)
        fPoles[i] = pPoleArray[i].DotProduct (*pPlaneCoffs);

    if (extend)
        bsiBezier_univariateRootsExt (root, &numRoot, fPoles, order);
    else
        {
        bsiBezier_univariateRoots (root, &numRoot, fPoles, order);
        bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, order, 0, 0.0, 0.0);
        bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, order, 1, 0.0, 0.0);
        }

    *pNumOut = 0;

    if (numRoot == order)
        return true;

    for (i = 0; i < numRoot; i++)
        {
        if (!jmdlBezierDPoint4d_saveParameterAndDPoint4d (pParamArray, pPointArray, pNumOut, maxOut,
                                pPoleArray, order, root[i]))
            return false;
        }
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier tangent is small.  Points considered are
* (1) both endpoints and (2) all extrema of the (squared) tangent vector.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pFixedPoint => dropping perpendicular from here.
* @param dimension  => 2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
* @param relTol => relative tolerance to consider a tangent magnitude small, as compared
*               to large tangent magnitudes elsewhere on this bezier.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allNearCusps
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
        int         workDimension,
        double      relTol
)
    {
    DPoint3d tangentPoles[MAX_BEZIER_ORDER];
    double  rootArray[MAX_BEZIER_ORDER + 2];
    double   ttPoles[MAX_BEZIER_ORDER];
    double   dttPoles[MAX_BEZIER_ORDER];
    int numRoot;
    double sPrevious;
    int orderTangent;
    int orderTT;
    int orderDTT;
    double maxTT;
    double tolT;
    int i;
    //int numOut = 0;


    if (pNumOut)
        *pNumOut = 0;

    if (   bsiBezierDPoint4d_pseudoTangent (tangentPoles, &orderTangent, MAX_BEZIER_ORDER,
                        pPoleArray, order, workDimension)
        && bsiBezier_dotProduct (
                        ttPoles, &orderTT, MAX_BEZIER_ORDER,
                        (double*)tangentPoles, orderTangent, 0, 3,
                        (double*)tangentPoles, orderTangent, 0, 3,
                        workDimension)
        )
        {
        orderDTT = orderTT - 1;
        bsiBezier_derivativePoles (dttPoles, ttPoles, orderTT, 1);

        maxTT = DoubleOps::MaxAbs (ttPoles, orderTT);
        if (maxTT <= 0.0)
            return false;

        bsiBezier_univariateRoots (rootArray + 1, &numRoot, dttPoles, orderDTT);
        /* ignore totally flat function */
        if (numRoot == orderDTT)
            numRoot = 0;

        /* Force tests at end points */
        rootArray[0] = 0.0;
        rootArray[numRoot + 1] = 1.0;
        tolT = relTol * sqrt (maxTT);
        numRoot += 2;
        sPrevious = -1.0;

        *pNumOut = 0;
        for (i = 0; i < numRoot; i++)
            {
            double s = rootArray[i];
            double tt;
            double t;
            if (s > sPrevious)
                {
                bsiBezier_functionAndDerivative (&tt, NULL, ttPoles, orderTT, 1, s);
                t = sqrt (tt);
                if (t <= tolT)
                    if (!jmdlBezierDPoint4d_saveParameterAndDPoint4d (pParamArray, pPointArray, pNumOut, maxOut,
                                pPoleArray, order, s))
                        return false;
                }
            sPrevious = s;
            }
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier its tangent to the (moving) line from
* given fixed point.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pFixedPoint => dropping perpendicular from here.
* @param dimension  => 2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allTangentsFromDPoint4d
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension
)
    {
    return bsiBezierDPoint4d_allTangentsFromDPoint4dExt (pParamArray,
                pPointArray, pNumOut, maxOut, pPoleArray, order, pFixedPoint,
                workDimension, false);
    }


/*---------------------------------------------------------------------------------**//**
* Find the points where the bezier its tangent to the (moving) line from
* given fixed point.
* @param pPoleArray => array of curve poles
* @param order      => curve order (number of poles, one more than degree)
* @param pFixedPoint => dropping perpendicular from here.
* @param dimension  => 2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
* @param extend => true to use unbounded (extended) geometry when possible.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allTangentsFromDPoint4dExt
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension,
        bool        extend
)
    {
    DPoint4d derivativePoles[MAX_BEZIER_ORDER];

    DPoint3d rPoleArray[MAX_BEZIER_ORDER];
    DPoint3d tPoleArray[MAX_BEZIER_ORDER];
    DPoint3d t0PoleArray[MAX_BEZIER_ORDER];
    DPoint3d t1PoleArray[MAX_BEZIER_ORDER];
    DPoint3d qPoleArray[MAX_BEZIER_ORDER];
    double   coffArray[MAX_BEZIER_ORDER];
    double   rootArray[3][MAX_BEZIER_ORDER];
    double   uMin, uMax;
    int      numRoot[3];
    bool    degenerate[3];
    static double s_zeroComponentRelTol = 1.0e-14;

    int orderDerivative = order - 1;
    int orderR = order;
    int orderT = order + order - 2;
    int orderRcrossT = orderR + orderT - 1;
    double largestXYZComponent;
    double largestCurrComponent;
    int i, k, k1, k2, kMin;
    int currIndex[3];
    static double s_commonRootTolerance = 1.0e-6;

    *pNumOut = 0;

    /* tangency condition is the simultaneous vector condition
       (0,0,0) == R cross T
       where R = (X/X.w - A/A.w) is the vector from the fixed point to the curve
             T = (X' X.w - X'.w X) / X.w ^2 is the curve tangent
        and R, T are both the cartesian images.

        Multiplying through by A.w and X.w, R becomes
            A.w X.w R = A.w X - X.w A       (xyz only, w part vanishes)
            which for component i is a bezier with coefficients
                   R.i = A.w X.i - X.w A.i
        The numerator of T is also identically 0 in the weight component,
            and the numerator is a difference of products of beziers,
                T.i = X'.i X.w - X'.w X.i
    */

    if (orderRcrossT > MAX_BEZIER_ORDER || workDimension < 2 || workDimension > 3)
        return false;

    /* Lines are special .... */
    if (order <= 2)
        return true;

    bsiBezier_derivativePoles ((double *)derivativePoles, const_cast<double*>((double const*)pPoleArray), order, 4);

    /* Form R ... */
    for (i = 0; i < order; i++)
        {
        rPoleArray[i].x = pFixedPoint->w * pPoleArray[i].x - pPoleArray[i].w * pFixedPoint->x;
        rPoleArray[i].y = pFixedPoint->w * pPoleArray[i].y - pPoleArray[i].w * pFixedPoint->y;
        rPoleArray[i].z = pFixedPoint->w * pPoleArray[i].z - pPoleArray[i].w * pFixedPoint->z;
        }

    for (k = 0; k < 3; k++)
        {
        bsiBezier_univariateProduct
            (
            (double *)t0PoleArray, k, 3,
            (double *)derivativePoles, orderDerivative, k, 4,   /* X'.k */
            const_cast<double*>((double const*)pPoleArray), order, 3, 4                   /* X.w  */
            );
        bsiBezier_univariateProduct
            (
            (double *)t1PoleArray, k, 3,
            const_cast<double*>((double const*)pPoleArray), order, k, 4,                  /* X.k */
            (double *)derivativePoles, orderDerivative, 3, 4    /* X'.w  */
            );
        }

    bsiBezier_subtractPoles
                (
                (double *)tPoleArray,
                (double *)t0PoleArray,
                (double *)t1PoleArray,
                orderT,
                3
                );

    for (k = 0; k < 3; k++)
        {
        k1 = (k + 1) % 3;
        k2 = (k1 + 1) % 3;
        bsiBezier_univariateProduct
            (
            (double *)t0PoleArray, k, 3,
            (double *)rPoleArray, orderR, k1, 3,    /* R.k1 */
            (double *)tPoleArray, orderT, k2, 3     /* T.k2 */
            );
        bsiBezier_univariateProduct
            (
            (double *)t1PoleArray, k, 3,
            (double *)rPoleArray, orderR, k2, 3,    /* R.k2 */
            (double *)tPoleArray, orderT, k1, 3     /* T.k1  */
            );
        }

    bsiBezier_subtractPoles
                    (
                    (double *)qPoleArray,
                    (double *)t0PoleArray,
                    (double *)t1PoleArray,
                    orderRcrossT,
                    3
                    );

    largestXYZComponent = bsiDPoint3d_getLargestCoordinate (qPoleArray, orderRcrossT);

    for (k = 0; k < 3; k++)
        {
        bsiBezier_copyComponent (coffArray, 0, 1, (double*)qPoleArray, k, 3, orderRcrossT);
        largestCurrComponent = DoubleOps::MaxAbs (coffArray, orderRcrossT);
        if (largestCurrComponent < s_zeroComponentRelTol * largestXYZComponent)
            {
            degenerate[k] = true;
            }
        else
            {
            // CR #192021: handle roots at endpts
            if (fabs(coffArray[0]) < s_zeroComponentRelTol * largestXYZComponent)
                coffArray[0] = 0.0;
            if (fabs(coffArray[orderRcrossT - 1]) < s_zeroComponentRelTol * largestXYZComponent)
                coffArray[orderRcrossT - 1] = 0.0;
            bsiBezier_univariateRoots
                            (
                            &rootArray[k][0],
                            &numRoot[k],
                            coffArray,
                            orderRcrossT
                            );
            degenerate[k] = numRoot[k] == orderRcrossT;
            }
        if (degenerate[k])
            numRoot[k] = 0;
        }


    if (workDimension == 2)
        {
        /* Only look at the z component */
        for (k = 0; k < numRoot[2]; k++)
            {
            double u = rootArray[2][k];
            if (!jmdlBezierDPoint4d_saveParameterAndDPoint4d
                    (pParamArray, pPointArray, pNumOut, maxOut, pPoleArray, order, u))
                return false;
            }
        }
    else
        {
        /* The roots are sorted.  Sweep up for common roots. */
        currIndex[0] = currIndex[1] = currIndex[2] = 0;

        if (degenerate[0] && degenerate[1] && degenerate[2])
            {
            }
        else
            {
            int kFirst = 3;
            if (!degenerate[2])
                kFirst = 2;
            if (!degenerate[1])
                kFirst = 1;
            if (!degenerate[0])
                kFirst = 0;

            while (    *pNumOut < maxOut
                    && (degenerate[0] || currIndex[0] < numRoot[0])
                    && (degenerate[1] || currIndex[1] < numRoot[1])
                    && (degenerate[2] || currIndex[2] < numRoot[2]))
                {

                /* Find range of leftmost roots of non-degenerate axes ... */
                uMin = uMax = rootArray[kFirst][currIndex[0]];
                kMin = kFirst;
                for (k = kFirst; k < 2; k++)
                    {
                    if (!degenerate[k])
                        {
                        if (rootArray[k][currIndex[k]] < uMin)
                            {
                            kMin = k;
                            uMin = rootArray[k][currIndex[k]];
                            }
                        if (rootArray[k][currIndex[k]] > uMax)
                            uMax = rootArray[k][currIndex[k]];
                        }
                    }

                if (uMax > uMin + s_commonRootTolerance)
                    {
                    currIndex[kMin] += 1;
                    }
                else
                    {
                    if (!jmdlBezierDPoint4d_saveParameterAndDPoint4d
                            (pParamArray, pPointArray, pNumOut, maxOut, pPoleArray, order, uMin))
                        return false;
                    currIndex[0] += 1;
                    currIndex[1] += 1;
                    currIndex[2] += 1;
                    }
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Find the Bezier curve point closest to a space point.  Only check points at or
*                   between the given parameters.
* @param pClosePoint <= xyzw of closest point
* @param pCloseParam <= parameter of closest point
* @param pCloseDist2 <= squared xy distance to closest point
* @param pPoleArray => array of curve poles
* @param pWeightArray => array of curve weights.  May be null.
* @param order      => curve order (number of poles, one more than degree)
* @param xx         => x coordinate of space point
* @param yy         => y coordinate of space point
* @param zz         => z coordinate of space point
* @param s0         => minimum parameter to consider
* @param s1         => maximum parameter to consider
* @return true if a near point (possibly an endpoint) was computed.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint3d_closestPoint
(
        DPoint3d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint3d    *pPoleArray,
const   double      *pWeightArray,
        int         order,
        double      xx,
        double      yy,
        double      zz,
        double      s0,
        double      s1
)
    {
    DPoint4d closePoint;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int i;
    bool    boolstat;

    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;

    for (i = 0; i < order; i++)
        {
        poleArray[i].x = pPoleArray[i].x;
        poleArray[i].y = pPoleArray[i].y;
        poleArray[i].z = pPoleArray[i].z;
        poleArray[i].w = 1.0;
        }

    if (pWeightArray)
        {
        for (i = 0; i < order; i++)
            poleArray[i].w = pWeightArray[i];
        }

    boolstat = bsiBezierDPoint4d_closestPoint (&closePoint, pCloseParam, pCloseDist2, poleArray,
                                    order, xx, yy, zz, s0, s1);
    if (boolstat)
        {
        if (pClosePoint)
            boolstat = closePoint.GetProjectedXYZ (*pClosePoint);
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Find the Bezier curve point closest to a space point.  Only check points at or
*                   between the given parameters.
* @param pClosePoint <= xyzw of closest point
* @param pCloseParam <= parameter of closest point
* @param pCloseDist2 <= squared xy distance to closest point
* @param pPoleArray => array of curve poles
* @param pWeightArray => array of curve weights.  May be null.
* @param order      => curve order (number of poles, one more than degree)
* @param xx         => x coordinate of space point
* @param yy         => y coordinate of space point
* @param s0         => minimum parameter to consider
* @param s1         => maximum parameter to consider
* @return true if a near point (possibly an endpoint) was computed.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint3d_closestXYPoint
(
        DPoint3d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint3d    *pPoleArray,
const   double      *pWeightArray,
        int         order,
        double      xx,
        double      yy,
        double      s0,
        double      s1
)
    {
    DPoint4d closePoint;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int i;
    bool    boolstat;

    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;

    for (i = 0; i < order; i++)
        {
        poleArray[i].x = pPoleArray[i].x;
        poleArray[i].y = pPoleArray[i].y;
        poleArray[i].z = pPoleArray[i].z;
        poleArray[i].w = 1.0;
        }

    if (pWeightArray)
        {
        for (i = 0; i < order; i++)
            poleArray[i].w = pWeightArray[i];
        }

    boolstat = bsiBezierDPoint4d_closestXYPoint (&closePoint, pCloseParam, pCloseDist2, poleArray,
                                    order, xx, yy, s0, s1);
    if (boolstat)
        {
        if (pClosePoint)
            boolstat = closePoint.GetProjectedXYZ (*pClosePoint);
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the numerator P (a 1D Bezier polynomial) of the function representing
* the curvature perpendicular to the given axis of a (rational) Bezier curve.
* Pole coordinates corresponding to the given axis are ignored.
*
* @param pOutPoles  <= array of poles of P
* @param pInPoles   => array of curve poles
* @param order      => curve order
* @param axis       => coordinate to ignore (i.e., axis = 2 for xy-curvature)
* @bsimethod                                                    DavidAssaf      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlBezierDPoint4d_computeCurvatureNumerator
(
        double      *pOutPoles,
const   DPoint4d    *pInPoles,
        int         order,
        int         axis
)
    {
    /*
    The numerator of the curvature function for a 3D rational Bezier curve is
        Q(x,y,z,w) = P(x,y,w)^2 + P(x,z,w)^2 + P(y,z,w)^2,
    where
        P(a,b,c) = c(a'b" - a"b') + c'(a"b - ab") + c"(ab' - a'b).
    This utility routine computes the factor P of the term of Q that does not
    depend on coordinate axis (i.e., for the purpose of finding roots of Q).
    */

    int i, j, k, coord1, coord2;
    /* derivs 0,1,2 of input curve */
    DPoint4d    poleDerivs[3][MAX_BEZIER_ORDER];
    int         orderDeriv[3] = {order, order - 1, order - 2};
    /* components making up each Term of P (max length 2*order-2) */
    double      poleADB[MAX_BEZIER_ORDER];      /* a'b", a"b, ab' */
    double      poleDAB[MAX_BEZIER_ORDER];      /* a"b', ab", a'b */
    double      poleDiff[MAX_BEZIER_ORDER];     /* ADB - DAB */
    /* orderDiff[i] is order of ADB, DAB, Diff the i_th time around */
    int         orderDiff[3] =
                    {
                    orderDeriv[1] + orderDeriv[2] - 1,
                    orderDeriv[2] + orderDeriv[0] - 1,
                    orderDeriv[0] + orderDeriv[1] - 1
                    };
    /* terms of P: cDiff, c'Diff, c"Diff */
    double      poleTerm[MAX_BEZIER_ORDER];
    int         orderTerm = 3 * order - 5;      /* order of P too */

    /* cycle axis if needed & set coords accordingly */
    if (axis > 2)
        axis = axis % 3;
    else if (axis < 0)
        axis = 2 - (-1 - axis) % 3;
    coord1 = 1 << axis & 3;                     /* (axis + 1) mod 3 */
    coord2 = 1 << coord1 & 3;                   /* (axis + 2) mod 3 */

    /* fill derivative arrays */
    bsiBezier_copyPoles
        ((double *) poleDerivs[0], 0, const_cast<double*>((double const*)pInPoles), 0, order, 4);
    bsiBezier_derivativePoles
        ((double *) poleDerivs[1], const_cast<double*>((double const*)pInPoles), order, 4);
    bsiBezier_derivativePoles
        ((double *) poleDerivs[2], (double *) poleDerivs[1], order - 1, 4);

    memset (pOutPoles, 0, orderTerm * sizeof (double));

    /* loop once per Term of P */
    for (i = 0; i < 3; i++)
        {
        j = 1 << i & 03;                        /* (i + 1) mod 3 */
        k = 1 << j & 03;                        /* (i + 2) mod 3 */

        /* compute product a'b", a"b or ab' */
        bsiBezier_univariateProduct (poleADB, 0, 1,
                        (double *) poleDerivs[j], orderDeriv[j], coord1, 4,
                        (double *) poleDerivs[k], orderDeriv[k], coord2, 4
                        );
        /* compute product a"b', ab" or a'b */
        bsiBezier_univariateProduct (poleDAB, 0, 1,
                        (double *) poleDerivs[k], orderDeriv[k], coord1, 4,
                        (double *) poleDerivs[j], orderDeriv[j], coord2, 4
                        );
        /* compute difference of above polynomials */
        bsiBezier_subtractPoles (poleDiff, poleADB, poleDAB, orderDiff[i], 1);

        /* compute product of c, c' or c" with above polynomial */
        bsiBezier_univariateProduct (poleTerm, 0, 1,
                        (double *) poleDerivs[i], orderDeriv[i], 3, 4,
                        poleDiff, orderDiff[i], 0, 1);

        /* add above polynomial to running total */
        bsiBezier_addPolesInPlace (pOutPoles, poleTerm, orderTerm, 1);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Find the inflection points of a (rational) Bezier curve, ignoring the
* z-coordinate.  Caller ensures that both output arrays have length at
* least 3*order - 9.
*
* @param pInflectPoints <= array of inflection points (or null)
* @param pInflectParams <= array of parameters of inflection points (or null)
* @param pPoleArray     => array of curve poles
* @param order          => curve order (number of poles, one more than degree)
* @return number of isolated inflection points found or -1 if invalid order
* @bsimethod                                                    DavidAssaf      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_inflectionPointsXY
(
        DPoint4d    *pInflectPoints,
        double      *pInflectParams,
const   DPoint4d    *pPoleArray,
        int         order
)
    {
    /*
    Let x,y,w; x',y',w'; x",y",w" be derivatives #0,1,2 of univariate Bezier
    polynomials.  The parameter values of the inflection points of the
    rational Bezier curve parametrized by x,y,w are the roots within [0,1]
    of the polynomial

        P(x,y,w) = w(x'y" - x"y') + w'(x"y - xy") + w"(xy' - x'y),

    which in general (after exact degree reduction) has order 3*order-8.
    P is both the numerator of the derivative of the xy-tangent angle, and
    the numerator of the xy-curvature of the curve.
    */

    int     i, numRoot;
    int     orderP = 3 * order - 5;
    double  polesP[MAX_BEZIER_ORDER];
    double  rootsP[MAX_BEZIER_ORDER];

    /* do nothing if both output arrays are null */
    if (!pInflectPoints && !pInflectParams)
        return 0;

    /* quadratic, linear and constant Beziers have no inflection points */
    if (order < 4)
        return 0;

    /* disallow orders that are too big */
    if (orderP > MAX_BEZIER_ORDER)
        return -1;

    /* compute P(x,y,w) */
    jmdlBezierDPoint4d_computeCurvatureNumerator (polesP, pPoleArray, order, 2);

    /* find roots of P */
    bsiBezier_univariateRoots (rootsP, &numRoot, polesP, orderP);

    /*
    Fill optional output arrays with at most orderP - 1 items.
    bsiBezier_univariateRoots sets numRoot = orderP iff P identically 0.
    */
    if (numRoot > 0 && numRoot < orderP)
        {
        if (pInflectParams && pInflectPoints)
            for (i = 0; i < numRoot; i++)
                {
                pInflectParams[i] = rootsP[i];
                bsiBezier_functionAndDerivative
                    (
                    (double *) &pInflectPoints[i], NULL,
                    const_cast<double*>((double const*)pPoleArray), order, 4, rootsP[i]
                    );
                }

        else if (pInflectParams)
            for (i = 0; i < numRoot; i++)
                pInflectParams[i] = rootsP[i];

        else /* pInflectPoints */
            for (i = 0; i < numRoot; i++)
                bsiBezier_functionAndDerivative
                    (
                    (double *) &pInflectPoints[i], NULL,
                    const_cast<double*>((double const*)pPoleArray), order, 4, rootsP[i]
                    );

        return numRoot;
        }

    /* Found no inflection points or P identically zero */
    else
        return 0;
    }


/*---------------------------------------------------------------------------------**//**
* Find the inflection points of a (rational) 3D Bezier curve.
* Caller ensures that both output arrays have length at least 3*order - 9.
* If the poles are only 2D, it is more efficient to use
* <A HREF="#inflectionPointsXY">inflectionPointsXY</A>.
*
* @param pInflectPoints <= array of inflection points (or null)
* @param pInflectParams <= array of parameters of inflection points (or null)
* @param pPoleArray     => array of curve poles
* @param order          => curve order (number of poles, one more than degree)
* @return number of isolated inflection points found or -1 if invalid order
* @see #inflectionPointsXY
* @bsimethod                                                    DavidAssaf      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void         bsiBezierDPoint4d_inflectionPoints
(
bvector<GraphicsPoint> inflectionPoints,
bvector<DPoint4d> curvePoles
)
    {
    /*
    Let x,y,z,w; x',y',z',w'; x",y",z",w" be derivatives #0,1,2 of univariate
    Bezier polynomials.  The parameter values of the inflection points of the
    rational Bezier curve parametrized by x,y,z,w are the roots within [0,1]
    of the polynomial

        Q(x,y,z,w) = P(x,y,w)^2 + P(x,z,w)^2 + P(y,z,w)^2,

    where

        P(x,y,w) = w(x'y" - x"y') + w'(x"y - xy") + w"(xy' - x'y),

    which in general (after exact degree reduction) has order 3*order-8.
    Q is the numerator of the curvature of the curve.

    Our strategy is to find the roots of one of the terms (non-squared) of
    Q that doesn't vanish everywhere, and then evaluate the other terms at
    these roots to see if they vanish within tolerance.
    */

    int order = (int) curvePoles.size ();
    inflectionPoints.clear ();
    /* quadratic, linear and constant Beziers have no inflection points */
    if (order < 4)
        return;

    int     i, j, k, l, numRoot = 0, numRoot0 = 0, numRoot1 = 0;
    int     orderP = 3 * order - 5;
    double  polesP[3][MAX_BEZIER_ORDER];        /* poles of 3 term factors */
    double  rootsP[MAX_BEZIER_ORDER], rootsP0[MAX_BEZIER_ORDER], rootsP1[MAX_BEZIER_ORDER];
    double  tol = 1.0e-6;                       /* max of P^2 at a root */

    int     numUp, numDown, numZero;
    double  aMin, aMax;




    /* disallow orders that are too big */
    if (orderP > MAX_BEZIER_ORDER)
        return;

    /* compute P(x,y,w), P(x,z,w) and P(y,z,w) */
    for (i = 0; i < 3; i++)
        jmdlBezierDPoint4d_computeCurvatureNumerator
                (polesP[i], curvePoles.data (), order, i);

    /* find roots of last P that doesn't identically vanish */
    i = 2;
    numRoot = 0;
    do
        bsiBezier_univariateRoots (rootsP, &numRoot, polesP[i], orderP);
    while (numRoot == orderP && --i >= 0);

    if (i >= 0 && numRoot > 0 && numRoot < orderP)
        {
        if (i > 1)
            {
            /* evaluate the roots of P[1] */
            bsiBezier_univariateRoots (rootsP1, &numRoot1, polesP[1], orderP);
            bsiBezier_evaluateRangeAndShape (&aMin, &aMax, &numUp, &numDown, &numZero, polesP[1], orderP);
            if (numZero != orderP - 1)
                {
                for (j = k = 0; j < numRoot; j++)
                    {
                    for(l = 0; l < numRoot1; l++)
                        {
                        if (fabs(rootsP[j]-rootsP1[l]) < tol)
                            rootsP[k++] = rootsP[j];
                        }
                    }
                numRoot -= j - k;
                }
            }
        if (i)
            {
            /* evaluate the roots of P[0] */
            bsiBezier_univariateRoots (rootsP0, &numRoot0, polesP[0], orderP);
            bsiBezier_evaluateRangeAndShape (&aMin, &aMax, &numUp, &numDown, &numZero, polesP[0], orderP);
            if (numZero != orderP - 1)
                {
                for (j = k = 0; j < numRoot; j++)
                    {
                    for(l = 0; l < numRoot0; l++)
                        {
                        if (fabs(rootsP[j]-rootsP0[l]) < tol)
                            rootsP[k++] = rootsP[j];
                        }
                    }
                numRoot -= j - k;
                }
            }
        }
    else            /* Q identically zero or no roots found */
        return;

    /* Fill optional output arrays */
        for (i = 0; i < numRoot; i++)
            {
            double u = rootsP[i];
            DPoint4d xyzw;
            bsiBezier_functionAndDerivative
                (
                (double *) &xyzw, NULL,
                (double *) curvePoles.data (), order, 4, rootsP[i]
                );
            inflectionPoints.push_back (GraphicsPoint (xyzw, u));
            }

    }




/*---------------------------------------------------------------------------------**//**
* @param    pPointArray <= evaluated points
* @param    pDerivArray <= evaluated derivatives
* @param    pPoleArray  => master poles.
* @param    order       => curve order.
* @param    pParamArray       => parametric coordinates
* @param    numParam        => number of parameters, points, derivatives
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_evaluateArray
(
        DPoint4d    *pPointArray,
        DPoint4d    *pDerivArray,
const   DPoint4d    *pPoleArray,
        int         order,
const   double      *pParamArray,
        int         numParam
)
    {
    int i;

    for (i = 0; i < numParam; i++)
        {
        bsiBezier_functionAndDerivative
                    (
                    (double *)(pPointArray ? &pPointArray[i] : NULL),
                    (double *)(pDerivArray ? &pDerivArray[i] : NULL),
                    const_cast<double*>((double const*)pPoleArray),
                    order,
                    4,
                    pParamArray[i]);

        }
    }


/*---------------------------------------------------------------------------------**//**
* @param    pPointArray <= evaluated points
* @param    pDerivArray <= evaluated derivatives
* @param    pPoleArray  => master poles.
* @param    order       => curve order.
* @param    pParamArray       => parametric coordinates
* @param    numParam        => number of parameters, points, derivatives
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_evaluateArray
(
        DPoint3d    *pPointArray,
        DPoint3d    *pDerivArray,
const   DPoint3d    *pPoleArray,
        int         order,
const   double      *pParamArray,
        int         numParam
)
    {
    int i;

    for (i = 0; i < numParam; i++)
        {
        bsiBezier_functionAndDerivative
                    (
                    (double *)(pPointArray ? &pPointArray[i] : NULL),
                    (double *)(pDerivArray ? &pDerivArray[i] : NULL),
                    const_cast<double*>((double const*)pPoleArray),
                    order,
                    3,
                    pParamArray[i]);

        }
    }



/*---------------------------------------------------------------------------------**//**
* Single point evaluation of a bezier
*
* @param    pPoint <= evaluated point
* @param    pDeriv <= evaluated derivative
* @param    pPoleArray  => master poles.
* @param    order       => curve order.
* @param    param       => parametric coordinate
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_evaluateDPoint4d
(
        DPoint4d    *pPoint,
        DPoint4d    *pDeriv,
const   DPoint4d    *pPoleArray,
        int         order,
const   double      param
)
    {
    bsiBezier_functionAndDerivative
                (
                (double *)pPoint,
                (double *)pDeriv,
                const_cast<double*>((double const*)pPoleArray),
                order,
                4,
                param);
    }


/*---------------------------------------------------------------------------------**//**
* Single point evaluation of a bezier
*
* @param    pPoint <= evaluated point
* @param    pDeriv <= evaluated derivative
* @param    pPoleArray  => master poles.
* @param    order       => curve order.
* @param    param       => parametric coordinate
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_evaluateDPoint3d
(
        DPoint3d    *pPoint,
        DPoint3d    *pDeriv,
const   DPoint3d    *pPoleArray,
        int         order,
const   double      param
)
    {
    bsiBezier_functionAndDerivative
                (
                (double *)pPoint,
                (double *)pDeriv,
                const_cast<double*>((double const*)pPoleArray),
                order,
                3,
                param);
    }



/*---------------------------------------------------------------------------------**//**
* Finds the intersections, if any, of the given Bezier curve with the given
* plane.  The maximum number of intersections is order - 1.
*
* @param    pPointArray         <= coordinates of intersection points (or null)
* @param    pDerivArray         <= derivatives of intersection points (or null)
* @param    pParamArray         <= parameters of intersection points (or null)
* @param    pNumIntersection    <= number of intersections (or null)
* @param    pAllOn              <= true if curve is entirely within the plane (or null)
* @param    pPoleArray          => array of homogeneous Bezier poles
* @param    order               => curve order = # poles
* @param    pPlane              => homogeneous coordinates of the plane
* @bsimethod                                                    EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezierDPoint4d_intersectPlane
(
        DPoint4d    *pPointArray,
        DPoint4d    *pDerivArray,
        double      *pParamArray,
        int         *pNumIntersection,
        bool        *pAllOn,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pPlane
)
    {
    double altitudeArray[MAX_BEZIER_ORDER];
    double rootArray[MAX_BEZIER_ORDER];
    int numIntersection;
    int numRoot;
    bool    allOn = false;
    bool    funcStat = false;
    int i;

    numIntersection = 0;

    allOn = false;

    for (i = 0; i < order; i++)
        altitudeArray[i] = pPoleArray[i].DotProduct (*pPlane);

    if (bsiBezier_univariateRoots (rootArray, &numRoot, altitudeArray, order))
        {
        funcStat = true;
        if (numRoot == order)
            {
            allOn = true;
            }
        else
            {
            if (pParamArray)
                for (i = 0; i < numRoot; i++)
                    {
                    pParamArray[i] = rootArray[i];
                    }

            if (pPointArray || pDerivArray)
                bsiBezierDPoint4d_evaluateArray
                                (
                                pPointArray,
                                pDerivArray,
                                pPoleArray,
                                order,
                                rootArray,
                                numRoot
                                );

            numIntersection = numRoot;
            }
        }

    if (pNumIntersection)
        *pNumIntersection = numIntersection;

    if (pAllOn)
        *pAllOn = allOn;

    return funcStat;
    }

typedef struct
    {
    double tolerance;
    DPoint4dArrayHandler handlerFunc;
    void    *pHandlerContext;
    int     order;
    bool    unitWeight;
    } RecursiveStrokeContext;

typedef struct
    {
    int outputCounter;
    DPoint4dSubdivisionHandler testFunc;
    void    *pTestContext;
    DPoint4dSubdivisionHandler handlerFunc;
    void    *pHandlerContext;
    } GenericRecursiveStrokeContext;



/*---------------------------------------------------------------------------------**//**
* Test if all weights are near one.
* @param    pPoleArray  => poles.
* @param    order       => curve order.
* @param    tolerance   => tolerance for declaring non-unit weight.
*                           If a negative is passed, a small default is used.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_isUnitWeight
(
const   DPoint4d    *pPoleArray,
        int         order,
        double      tolerance
)
    {
    int i;
    if (tolerance < 0.0)
        tolerance = s_unitWeightRelTol;

    for (i = 0; i < order; i++)
        {
        if (fabs (pPoleArray[i].w - 1.0) > tolerance)
            return false;
        }

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* Find the maximum absolute deviation between any pole and the midpoint of the chord
*   between the two adjacent poles.  Deviation is measured as sum of absolute differences.
*
* @param    pPoleArray  => master poles.
* @param    order       => curve order.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_maxAbsMidpointDeviationXYZ
(
const   DPoint4d    *pPoleArray,
        int         order
)
    {
    DPoint4d point0, point1, point2;
    int i;
    double dMax, dCurr;
    if (order <= 2)
        return 0.0;

    point0 = pPoleArray[0];
    point1 = pPoleArray[1];
    dMax = 0.0;
    for (i = 2; i < order; i++, point0 = point1, point1 = point2)
        {
        point2 = pPoleArray[i];
        dCurr =   fabs (0.5 * (point0.x + point2.x) - point1.x)
                + fabs (0.5 * (point0.y + point2.y) - point1.y)
                + fabs (0.5 * (point0.z + point2.z) - point1.z);
        if (dCurr > dMax)
            dMax = dCurr;
        }

    return dMax;
    }


Public GEOMDLLIMPEXP int bsiBezierDPoint4d_estimateEdgeCount
(
const   DPoint4d    *pPoleArray,
        int         order,
        double      chordTol,
        double      angleTol,
        double      maxEdgeLength,
        bool        weightsAreAllOne
)
    {
    int edgeCount = order - 1;
    DPoint3d xyz[MAX_BEZIER_CURVE_ORDER];
    DVec3d   dxyz[MAX_BEZIER_CURVE_ORDER];

    // Get real points ...
    if (weightsAreAllOne)
        {
        for (int i = 0; i < order; i++)
            {
            xyz[i].x = pPoleArray[i].x;
            xyz[i].y = pPoleArray[i].y;
            xyz[i].z = pPoleArray[i].z;
            }
        }
    else
        {
        for (int i = 0; i < order; i++)
            pPoleArray[i].GetProjectedXYZ (xyz[i]);
        }

    double maxPolygonEdgeLength = 0.0;
    for (int i = 1; i < order; i++)
        {
        double a = dxyz[i-1].NormalizedDifference (xyz[i], xyz[i-1]);
        if (a > maxPolygonEdgeLength)
            maxPolygonEdgeLength = a;
        }

    double minCosine = 1.0;
    for (int i = 1; i < order - 1; i++)
        {
        double cosine = dxyz[i-1].DotProduct (dxyz[i]);
        if (cosine < minCosine)
            minCosine = cosine;
        }

    double maxPolygonAngle  = acos (minCosine);
    // Estimate both chord and angle errors by circular arc approximations
    if (chordTol > 0.0)
        {
        double s = sin (maxPolygonAngle * 0.5);
        double hMax = maxPolygonEdgeLength * s;
        if (hMax > chordTol)
            {
            static double constantMultiplier = 1.25;    // heuristic
            int n = (int) ceil (constantMultiplier * sqrt (hMax / chordTol));
            if (n > edgeCount)
                edgeCount = n;
            }
        }

    if (angleTol > 0.0 && maxPolygonAngle > angleTol)
        {
        int n = (order - 2) * (int)ceil (maxPolygonAngle / angleTol);
        if (n > edgeCount)
            edgeCount = n;
        }

    if (maxEdgeLength > 0 && maxPolygonEdgeLength > maxEdgeLength)
        {
        int n = (order - 1) * (int)ceil (maxPolygonEdgeLength / maxEdgeLength);
        if (n > edgeCount)
            edgeCount = n;
        }
    return edgeCount;
    }

//! Add a stroke point and (optional) tangent to the outputs.
static void AddStroke
(
bvector<DPoint3d>&points,
bvector<double> *params,
bvector<DVec3d> *derivatives,
double param,
double derivativeScale, // (order - 1) / paramStep 
DPoint4dCR point,   // Stroke point
DPoint4dCR pointA,
DPoint4dCR pointB
)
    {
    DPoint3d xyz;
    if (point.w != 0.0)
        {

        if (NULL != params)
            params->push_back (param);

        if (NULL == derivatives)
            {
            point.GetProjectedXYZ (xyz);
            points.push_back (xyz);
            }
        else
            {
            DPoint4d dX, ddX;
            
            dX.DifferenceOf (pointB, pointA);
            dX.Scale (derivativeScale);
            ddX.Zero ();
            auto tangents = DPoint4d::TryNormalizePointAndDerivatives (point, dX, ddX);
            points.push_back (tangents.Value ().origin);
            derivatives->push_back (tangents.Value ().vectorU);
            }
        }
    }


// Stroke from 0 to 1.  BUT ... map parameters to param0..param1
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_addStrokes
(
const   DPoint4d    *pPoleArray,
        int         order,
bvector<DPoint3d>&points,
bvector<double> *params,
bvector<DVec3d> *derivatives,
size_t numEdge,
bool includeStartPoint,
double param0,
double param1
)
    {
    DPoint4d poles[MAX_BEZIER_CURVE_ORDER];
    if (numEdge < 1)
        numEdge = 1;
    double paramStep = param1 - param0;
    double derivativeScale = (order - 1) / paramStep;
    double df = 1.0 / numEdge;
    double dp = (param1 - param0) / numEdge;
    if (includeStartPoint)
        AddStroke (points, params, derivatives, param0, derivativeScale, pPoleArray[0], pPoleArray[0], pPoleArray[1]);

    for (size_t i = 1; i < numEdge; i++)
        {
        memcpy (poles, pPoleArray, order * sizeof (DPoint4d));
        bsiBezierDPoint4d_subdivideRightInPlace (poles, NULL, order, i * df);
        AddStroke (points, params, derivatives, param0 + i * dp, derivativeScale,
                    poles[0], poles[0], poles[1]);
        }
    AddStroke (points, params, derivatives, param1, derivativeScale,
                    pPoleArray[order-1], pPoleArray[order-2], pPoleArray[order - 1]);

    }

/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, left part of subdivision
*       (top of triangle)
* @param pRightPoleArray <= poles for right of subdivision interval (bottom of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_subdivideLeftInPlace
(
DPoint4d    *pPoleArray,
DPoint4d    *pRightPoleArray,
    int     order,
    double  u
)
    {
    int i, j;
    double v = 1.0 - u;
    DPoint4d *pPoint0, *pPoint1;
    int iLast = order - 1;

    if (order > 0)
        {
        if (pRightPoleArray)
            pRightPoleArray[iLast] = pPoleArray[iLast];
        for (i = 1; i < order; i++)
            {
            for (j = order - 1; j >= i; j--)
                {
                pPoint1 = pPoleArray + j;
                pPoint0 = pPoint1 - 1;
                pPoint1->x = pPoint0->x * v + pPoint1->x * u;
                pPoint1->y = pPoint0->y * v + pPoint1->y * u;
                pPoint1->z = pPoint0->z * v + pPoint1->z * u;
                pPoint1->w = pPoint0->w * v + pPoint1->w * u;
                }
            if (pRightPoleArray)
                pRightPoleArray[iLast - i] = pPoleArray[iLast];
            }
        }
    }


/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, right part of subdivision
*       (bottom of triangle)
* @param pLeftPoleArray <= poles for left of subdivision interval (top of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_subdivideRightInPlace
(
DPoint4d    *pPoleArray,
DPoint4d    *pLeftPoleArray,
    int     order,
    double  u
)
    {
    int i, j;
    double v = 1.0 - u;
    int jLimit;
    DPoint4d *pPoint0, *pPoint1;

    if (order > 0)
        {
        if (pLeftPoleArray)
            pLeftPoleArray[0] = pPoleArray[0];
        for (i = 1; i < order; i++)
            {
            jLimit = order - i;
            for (j = 0; j < jLimit; j++)
                {
                pPoint0 = pPoleArray + j;
                pPoint1 = pPoint0 + 1;
                pPoint0->x = pPoint0->x * v + pPoint1->x * u;
                pPoint0->y = pPoint0->y * v + pPoint1->y * u;
                pPoint0->z = pPoint0->z * v + pPoint1->z * u;
                pPoint0->w = pPoint0->w * v + pPoint1->w * u;
                }
            if (pLeftPoleArray)
                pLeftPoleArray[i] = pPoleArray[0];
            }
        }
    }


/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, left part of subdivision
*       (top of triangle)
* @param pRightPoleArray <= poles for right of subdivision interval (bottom of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_subdivideLeftInPlace
(
DPoint3d    *pPoleArray,
DPoint3d    *pRightPoleArray,
    int     order,
    double  u
)
    {
    int i, j;
    double v = 1.0 - u;
    DPoint3d *pPoint0, *pPoint1;
    int iLast = order - 1;

    if (order > 0)
        {
        if (pRightPoleArray)
            pRightPoleArray[iLast] = pPoleArray[iLast];
        for (i = 1; i < order; i++)
            {
            for (j = order - 1; j >= i; j--)
                {
                pPoint1 = pPoleArray + j;
                pPoint0 = pPoint1 - 1;
                pPoint1->x = pPoint0->x * v + pPoint1->x * u;
                pPoint1->y = pPoint0->y * v + pPoint1->y * u;
                pPoint1->z = pPoint0->z * v + pPoint1->z * u;
                }
            if (pRightPoleArray)
                pRightPoleArray[iLast - i] = pPoleArray[iLast];
            }
        }
    }


/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, right part of subdivision
*       (bottom of triangle)
* @param pLeftPoleArray <= poles for left of subdivision interval (top of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_subdivideRightInPlace
(
DPoint3d    *pPoleArray,
DPoint3d    *pLeftPoleArray,
    int     order,
    double  u
)
    {
    int i, j;
    double v = 1.0 - u;
    int jLimit;
    DPoint3d *pPoint0, *pPoint1;

    if (order > 0)
        {
        if (pLeftPoleArray)
            pLeftPoleArray[0] = pPoleArray[0];
        for (i = 1; i < order; i++)
            {
            jLimit = order - i;
            for (j = 0; j < jLimit; j++)
                {
                pPoint0 = pPoleArray + j;
                pPoint1 = pPoint0 + 1;
                pPoint0->x = pPoint0->x * v + pPoint1->x * u;
                pPoint0->y = pPoint0->y * v + pPoint1->y * u;
                pPoint0->z = pPoint0->z * v + pPoint1->z * u;
                }
            if (pLeftPoleArray)
                pLeftPoleArray[i] = pPoleArray[0];
            }
        }
    }




/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, left part of subdivision
*       (top of triangle)
* @param pRightPoleArray <= poles for right of subdivision interval (bottom of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiBezierDPoint4d_subdivideToIntervalInPlace
(
DPoint4d    *pPoleArray,
    int     order,
    double  u0,
    double  u1
)
    {
    int k;
    DPoint4d point;
    if (order <= MAX_BEZIER_ORDER)
        {
        if (u0 == 0.0 && u1 == 1.0)
            {
            }
        else if (u0 == 1.0 && u1 == 0.0)
            {
            VectorOps<DPoint4d>::ReverseArrayInPlace (pPoleArray, order);
            }
        else if (u0 == u1)
            {
            /* Same parameter.  Explicitly copy it as a degenerate bezier. */
            bsiBezierDPoint4d_evaluateArray (&point, NULL, pPoleArray, order, &u0, 1);
            for (k = 0; k < order; k++)
                pPoleArray[k] = point;
            }
        else if (u0 > u1)
            {
            bsiBezierDPoint4d_subdivideToIntervalInPlace (pPoleArray, order, u1, u0);
            VectorOps<DPoint4d>::ReverseArrayInPlace (pPoleArray, order);
            }
        else
            {
            double reducedFraction;
            /* We know u0 < u1 */
            if (u0 <= 0.0)
                {
                if (u0 != 0.0)
                    bsiBezierDPoint4d_subdivideRightInPlace (pPoleArray, NULL, order, u0);
                if (u1 != 1.0)
                    {
                    reducedFraction = (u1 - u0) / (1.0 - u0);
                    bsiBezierDPoint4d_subdivideLeftInPlace (pPoleArray, NULL, order, reducedFraction);
                    }
                }
            else
                {
                if (u1 != 1.0)
                    bsiBezierDPoint4d_subdivideLeftInPlace (pPoleArray, NULL, order, u1);
                if (u0 != 0.0)
                    {
                    reducedFraction = u0 / u1;
                    bsiBezierDPoint4d_subdivideRightInPlace (pPoleArray, NULL, order, reducedFraction);
                    }
                }
            }
        }
    }


/**
* DeCasteljou subdivision.
* given interpolating fraction.
* @param pPoleArray <=> On input, full interval pole array. On output, left part of subdivision
*       (top of triangle)
* @param pRightPoleArray <= poles for right of subdivision interval (bottom of triangle).
* @param order => number of poles.
* @param u => interpolating parameter.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiBezierDPoint3d_subdivideToIntervalInPlace
(
DPoint3d    *pPoleArray,
    int     order,
    double  u0,
    double  u1
)
    {
    int k;
    DPoint3d point;
    if (order <= MAX_BEZIER_ORDER)
        {
        if (u0 == 0.0 && u1 == 1.0)
            {
            }
        else if (u0 == 1.0 && u1 == 0.0)
            {
            bsiDPoint3d_reverseArrayInPlace (pPoleArray, order);
            }
        else if (u0 == u1)
            {
            /* Same parameter.  Explicitly copy it as a degenerate bezier. */
            bsiBezierDPoint3d_evaluateArray (&point, NULL, pPoleArray, order, &u0, 1);
            for (k = 0; k < order; k++)
                pPoleArray[k] = point;
            }
        else if (u0 > u1)
            {
            bsiBezierDPoint3d_subdivideToIntervalInPlace (pPoleArray, order, u1, u0);
            bsiDPoint3d_reverseArrayInPlace (pPoleArray, order);
            }
        else
            {
            double reducedFraction;
            /* We know u0 < u1 */
            if (u0 <= 0.0)
                {
                if (u0 != 0.0)
                    bsiBezierDPoint3d_subdivideRightInPlace (pPoleArray, NULL, order, u0);
                if (u1 != 1.0)
                    {
                    reducedFraction = (u1 - u0) / (1.0 - u0);
                    bsiBezierDPoint3d_subdivideLeftInPlace (pPoleArray, NULL, order, reducedFraction);
                    }
                }
            else
                {
                if (u1 != 1.0)
                    bsiBezierDPoint3d_subdivideLeftInPlace (pPoleArray, NULL, order, u1);
                if (u0 != 0.0)
                    {
                    reducedFraction = u0 / u1;
                    bsiBezierDPoint3d_subdivideRightInPlace (pPoleArray, NULL, order, reducedFraction);
                    }
                }
            }
        }
    }


#include <Geom/newton.h>

static void GetSampleFractions (double *fraction, int &numOut, int order, int numInteriorEdge, int numExtrapolate, int maxOut)
    {
    double u0, du;
    static size_t sNumLines = 0;
    static size_t sNumCurves = 0;
    if (order < 3)
        {
        numOut = 2;
        u0 = 0.0;
        du = 1.0;
        sNumLines++;
        }
    else
        {
        numOut = 1 + numInteriorEdge + 2 * numExtrapolate;
        if (numOut > maxOut)
            {
            numOut = maxOut;
            }
        numInteriorEdge = numOut - 1 - 2 * numExtrapolate;
        du = 1.0 / numInteriorEdge;
        u0 = -numExtrapolate * du;
        sNumCurves++;
        }
    for (int i = 0; i < numOut; i++)
        fraction[i] =  u0 + i * du;
    }

static bool acceptFraction (double u)
    {
    return fabs (u - 0.5) <= 0.5 + Angle::SmallAngle ();
    }
/*---------------------------------------------------------------------------------**//**
Compute intersection of two bezier curves, starting with chordal approximations.
@param pPointA OUT computed points on curve A
@parma pParamA OUT parameters on curve A
@param pPointB OUT computed points on curve B
@parma pParamB OUT parameters on curve B
@param pNumIntersection OUT number of returned intersections
@param pNumExtra OUT number of extra intersections
@param maxIntersection OUT max returned (additional intersections are computed and
            counted in pNumExtra)
@param pA OUT curve A
@param orderA OUT order of curve A
@param pB OUT curve B
@param orderB OUT order of curve B

@param
@return true if sufficient storage to solve.
@bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezierDPoint4d_intersectXY_chordal
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      int       *pNumExtra,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB
)
    {
#define MAX_SAMPLE 75
    int numSampleA;
    int numSampleB;
#define MAX_SEGMENT_INTERSECTION 1024
    // Chordal form of each curve ...
    DPoint3d xyzA[MAX_SAMPLE];
    DPoint3d xyzB[MAX_SAMPLE];
    double   fractionA[MAX_SAMPLE];
    double   fractionB[MAX_SAMPLE];
    int i;
    // chordal intersections ...
    double segmentFractionA[MAX_SEGMENT_INTERSECTION];
    double segmentFractionB[MAX_SEGMENT_INTERSECTION];
    int    segmentIndexA[MAX_SEGMENT_INTERSECTION];
    int    segmentIndexB[MAX_SEGMENT_INTERSECTION];
    static int sNumInteriorEdge = 20;
    static int sNumExtrapolate  = 2;
    int numSegmentIntersection, numExtraSegmentIntersection;
    int numIntersection         = 0;
    int numExtraIntersection    = 0;


    static int sNum22 = 0;
    static int sNum2N = 0;
    static int sNumNN = 0;

    if (orderA == 2 && orderB == 2)
        sNum22++;
    else if (orderA == 2 || orderB == 2)
        sNum2N++;
    else
        sNumNN++;

    if (pNumIntersection)
        *pNumIntersection = 0;
    if (pNumExtra)
        *pNumExtra = 0;

    // Naive sampling in expanded range of bezier ..
    GetSampleFractions (fractionA, numSampleA, orderA, sNumInteriorEdge, sNumExtrapolate, MAX_SAMPLE);
    GetSampleFractions (fractionB, numSampleB, orderB, sNumInteriorEdge, sNumExtrapolate, MAX_SAMPLE);

    bsiBezierDPoint4d_evaluateDPoint3dArray (xyzA, NULL, pA, orderA,
                fractionA, numSampleA);
    bsiBezierDPoint4d_evaluateDPoint3dArray (xyzB, NULL, pB, orderB,
                fractionB, numSampleB);

    bsiDPoint3dArray_polylineIntersectXY
                (
                NULL, segmentIndexA, segmentFractionA,
                NULL, segmentIndexB, segmentFractionB,
                &numSegmentIntersection, &numExtraSegmentIntersection,
                maxIntersection,
                xyzA, numSampleA, false,
                xyzB, numSampleB, false);
    int numExternal = 0;
    if (numSegmentIntersection > 0)
        {
        Function_Bezier_FractionToXY bezierA(pA, orderA);
        Function_Bezier_FractionToXY bezierB(pB, orderB);
        NewtonIterationsRRToRR newton(1.0e-14);
        for (i = 0; i < numSegmentIntersection; i++)
            {
            int iA = segmentIndexA[i];
            double uA = fractionA[iA]
                    + segmentFractionA[i] * (fractionA[iA+1] - fractionA[iA]);
            int iB = segmentIndexB[i];
            double uB = fractionB[iB]
                    + segmentFractionB[i] * (fractionB[iB+1] - fractionB[iB]);

            if (newton.RunNewtonDifference (uA, uB, bezierA, bezierB))
                {
                if (!acceptFraction (uA) || !acceptFraction (uB))
                    {
                    numExternal++;
                    }
                else if (numIntersection < maxIntersection)
                    {
                    if (pParamA)
                        pParamA[numIntersection] = uA;
                    if (pPointA)
                        bsiBezierDPoint4d_evaluateDPoint4d (
                                &pPointA[numIntersection], NULL,
                                pA, orderA, uA);
                    if (pParamB)
                        pParamB[numIntersection] = uB;
                    if (pPointB)
                        bsiBezierDPoint4d_evaluateDPoint4d (
                                &pPointB[numIntersection], NULL,
                                pB, orderB, uB);
                    numIntersection++;
                    }
                else
                    {
                    numExtraIntersection++;
                    }
                }
            }
        }

    if (pNumIntersection)
        *pNumIntersection = numIntersection;
    if (pNumExtra)
        *pNumExtra = numExtraIntersection;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
Given {order} poles and {2*(order-1)} knots, apply blossoming to clamp the poles in place.
@remark Note that the number of knots at each side is only {order-1}.  The customary
extraneous knot is not required.  If you have an array knots[..] including the extra knot, the first "real" interval
starts with knots[1] rather than knots[0].
@param [in,out] pPoint on input, the unclamped poles. On output, bezier poles.
@param [in,out] pKnot unclamped knots
@bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiBezierDPoint4d_saturateKnotsInInterval
(
DPoint4d  *pPoint,
double    *pKnot,
int order,
bool &isNullInterval
)
    {
    bsiBezier_saturateKnotsInInterval ((double*)pPoint, 4, pKnot, order, isNullInterval);
    }

static double s_nullKnotInterval = 1.0e-8;
Public GEOMDLLIMPEXP bool bsiBezier_isNullKnotInterval (double a, double b)
    {
    return fabs (a - b) < s_nullKnotInterval * (1 + fabs (a) + fabs (b));
    }

Public GEOMDLLIMPEXP void bsiBezier_saturateKnotsInInterval
(
double *pPoleData,
size_t numPerPole,
double    *pKnot,
int order,
bool &isNullInterval
)
    {
    int degree = order - 1;
    double knot[MAX_BEZIER_ORDER];

    // knot indices at left and right of the interval ...
    int     kLeft = order - 2;
    int    kRight = kLeft + 1;
    int kLastKnot = 2 * order - 3;
    isNullInterval = true;
    if (kLastKnot < 1)
        return;
    for (int k = 0; k <= kLastKnot; k++)
        knot[k] = pKnot[k];

    isNullInterval = bsiBezier_isNullKnotInterval (pKnot[degree - 1], pKnot[degree]);

    double aLeft = knot[kLeft];
    double aRight = knot[kRight];
    int    j0, j1;  // POLE indices (always differ by exactly 1)
    int    k0, k2;    // KNOT indices.
                        
    while (knot[0] < aLeft)
        {
        for (k0 = 0; knot[k0] < aLeft; k0++)
            {
            j1 = k0 + 1;    // start index for knot, pole intervals are both k0.
            k2 = k0 + degree;
            double a0 = knot[k0];
            double a2 = knot[k2];
            if (a2 == a0)
                continue;
            double fraction1 = (aLeft - a0) / (a2 - a0);
            double fraction0 = 1.0 - fraction1;
            for (size_t i = 0; i < numPerPole; i++)
                pPoleData[k0*numPerPole + i] = fraction0 * pPoleData[k0*numPerPole + i] + fraction1 * pPoleData[j1*numPerPole + i];
            knot[k0] = knot[k0 + 1];
            }
        }

    // Right side
    // Knot layout [k2 kLeft k0]
    // From this end, knot and pole indices do not match.
    while (aRight < knot[kLastKnot])
        {
        for (k0 = kLastKnot, j0 = order - 1; aRight < knot[k0]; k0--, j0--)
            {
            k2 = k0 - degree;
            j1 = j0 - 1;
            double a0 = knot[k0];
            double a2 = knot[k2];
            if (a2 == a0)
                continue;
            double fraction1 = (aRight - a0) / (a2 - a0);
            double fraction0 = 1.0 - fraction1;
            for (size_t i = 0; i < numPerPole; i++)
                pPoleData[j0*numPerPole + i] = fraction0 * pPoleData[j0*numPerPole + i] + fraction1 * pPoleData[j1*numPerPole + i];
            knot[k0] = knot[k0 - 1];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
Insert a knot in arrays of poles and knots.  Both arrays are extended.
@param [in,out] pPoint pole array.
@param [in,out] pKnot knot array.
@param [in] numPoint number of poles.
@param [in] order spline order (one more than degree)
@param [in] numLeadingKnot In customary storage (with extraneous leading knot), equal to {order}.  
@param [in] leftPoleIndex index of leftmost pole of the {order} poles that apply.
@param [in] leftKnotIndex index of the left end of the knot interval.  {order-1}
                knots ENDING here are referenced as "left" of the interval. {order-1}
                knots BEGINNING at {leftKnot+1} are referenced.
@param [in] fraction fractional parameter within interval from knot {leftKnotIndex}
                to {leftKnotIndex+1}
@return true if the knot interval has nonzero length and all referenced knots are non-decreasing.
@bsimethod                                                    EarlinLutz      09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiBezierDPoint4d_insertKnot
(
bvector<DPoint4d> &poles,
bvector<double> &knots,
int        order,
size_t leftPoleIndex,
size_t leftKnotIndex,
double fraction
)
    {
    if (order < 2)
        return false;
    size_t degree = (size_t)order - 1;
    if (leftKnotIndex < degree - 1)
        return false;
    if (leftPoleIndex + order > poles.size ())
        return false;
    if (leftKnotIndex + degree > knots.size ())
        return false;
    double leftKnot = knots[leftKnotIndex];
    double rightKnot = knots[leftKnotIndex + 1];
    if (leftKnot >= rightKnot)
        return false;

    for (size_t k = leftKnotIndex - degree + 1;
                k <= leftKnotIndex + degree - 1;
                k++)
        {
        if (knots[k] > knots[k+1])
            return false;
        }

    double newKnot = leftKnot + fraction * (rightKnot - leftKnot);

    // insert space for pole, knot ..
    DPoint4d leftPole  = poles[leftPoleIndex];
    poles.insert (poles.begin () + leftPoleIndex + 1, leftPole);
    knots.insert (knots.begin () + leftKnotIndex + 1, newKnot);
    size_t i0 = leftPoleIndex + 1;
    size_t k0 = leftKnotIndex + 2 - order; // left end of first window
    size_t k1 = leftKnotIndex + 2;         // right end of first knot window
    for (size_t step = 0; step < degree; step++)
        {
        double edgeFraction = (newKnot - knots[k0 + step])
                / (knots[k1 + step] - knots[k0 + step]);
        poles[i0 + step].Interpolate (poles[i0 + step], edgeFraction, poles[i0 + step + 1]);
        }
    return true;
    }

Public GEOMDLLIMPEXP void bsiBezierDPoint4d_intersectDSegment4dXY
(
DPoint4dP bezierPoints,
double *bezierFractions,
DPoint4dP segmentPoints,
double *segmentFractions,
size_t &numOut,
size_t maxOut,
DPoint4dCP bezierPoles,
size_t order,
DSegment4dCR  segment,
bool extendSegment0,
bool extendSegment1
)
    {
    double epsilon = Angle::SmallAngle ();
    numOut = 0;
    DVec3d viewVector = DVec3d::From (0,0,-1);
    DVec3d tangent = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    DPoint4d planeCoffs;
    if (!planeCoffs.PlaneFromOriginAndVectors (segment.point[0], viewVector, tangent))
        return;
    double root[MAX_BEZIER_ORDER];
    double fPoles[MAX_BEZIER_ORDER];
    int numRoot;
    for (size_t i = 0; i < order; i++)
        fPoles[i] = planeCoffs.DotProduct (bezierPoles[i]);

    bsiBezier_univariateRoots (root, &numRoot, fPoles, (int)order);
    bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, (int)order, 0, 0.0, 0.0);
    bsiBezier_addRootNearEndpoint (root, &numRoot, fPoles, (int)order, 1, 0.0, 0.0);

    for (size_t i = 0; i < (size_t)numRoot && numOut < maxOut; i++)
        {
        DPoint4d curvePoint, segmentPoint;
        double segmentFraction;
        double curveFraction = root[i];
        bsiBezier_functionAndDerivative ((double*)&curvePoint, NULL, const_cast<double*>((double const*)bezierPoles), (int)order, 4, curveFraction);
        if (segment.ProjectPointUnboundedCartesianXYW (segmentPoint, segmentFraction, curvePoint))
            {
            if (!extendSegment0 && segmentFraction < -epsilon)
                continue;
            if (!extendSegment1 && segmentFraction > 1.0 + epsilon)
                continue;

            if (NULL != bezierPoints)
                bezierPoints[numOut] = curvePoint;
            if (NULL != bezierFractions)
                bezierFractions[numOut] = curveFraction;

            if (NULL != segmentPoints)
                segmentPoints[numOut] = segmentPoint;
            if (NULL != segmentFractions)
                segmentFractions[numOut] = segmentFraction;
            numOut++;
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE