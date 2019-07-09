/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <mutex>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Pascal's triangle table                                             |
|                                                                       |
+----------------------------------------------------------------------*/
// Binomial coefficients stored as double are exact up to 59.
// But we have to go higher.  We have to deal with curves of order 26,
//  we expect products up to 3 times that.
#define MAX_BINOMIAL_ORDER 80
#define MAX_PASCAL_COFF ((MAX_BINOMIAL_ORDER + 1) * (MAX_BINOMIAL_ORDER + 2) / 2)

static double *s_pascalRowAddress[MAX_BINOMIAL_ORDER+1];
static double s_pascal[MAX_PASCAL_COFF];

/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Initialize the pascal's triangle
*
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   jmdlBezier_initializePascal
(
)
    {
    int row;
    int i;
    double *pPrev, *pCurr;
    pPrev = s_pascalRowAddress[0] = s_pascal;
    s_pascal[0] = 1.0;

    for (row = 1; row <= MAX_BINOMIAL_ORDER; row++)
        {
        pCurr = s_pascalRowAddress[row] = pPrev + row - 1;
        pCurr[row] = 1.0;
        for (i = 1; i < row; i++)
            pCurr[i] = pPrev[i-1] + pPrev[i];
        pPrev = pCurr;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Get a pointer to a row of pascal's triangle.
* The row has row+1 entries.
* Hence it has coefficients for the row=order bezier polynomials.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  double         *jmdlBezier_getPascalRow
(
int row
)
    {
    if (row < 0 || row > MAX_BINOMIAL_ORDER)
        return NULL;
    static std::once_flag s_ignoreListOnceFlag;
    std::call_once(s_ignoreListOnceFlag, jmdlBezier_initializePascal);

    return s_pascalRowAddress[row];
    }


/*---------------------------------------------------------------------------------**//**
* Get the binomial coefficient iCn.  These coefficients are exact up to degree 59
*   (That's an IEEE double fact!!!)
* @param i => index within row (0..n)
* @param n => row index. (apex row with singleton 1 entry is row zero.)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiBezier_getBinomialCoefficient
(
int i,
int n
)
    {
    double *pRow = jmdlBezier_getPascalRow (n);
    if (pRow && i >= 0 && i <= n)
            return pRow[i];
    return 0.0;
    }



/*---------------------------------------------------------------------------------**//**
* Convert coefficients of a univariate Bernstein-Bezier polynomial to
* coefficients in the Power basis.
*
* @param    pPowCoff    <= coefficients in Power basis
* @param    pBezCoff    => coefficients in Bernstein-Bezier basis
* @param    order       => order of the polynomial
* @bsimethod                                                    DavidAssaf      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_convertBezierToPower
(
double      *pPowCoff,
double      *pBezCoff,
int         order
)
    {
    int i,j;
    double *pPascal, *pPascal2;
    double sum;

    // zeroth basis functions (i.e. constant term) have same coefficient
    pPowCoff[0] = pBezCoff[0];

    pPascal = jmdlBezier_getPascalRow (order - 1);

    // compute all remaining coefficients
    for (i = 1; i < order; i++)
        {
        sum = 0.0;
        pPascal2 = jmdlBezier_getPascalRow (i);

        for (j = 0; j <= i; j++)
            if ( (i+j) & 01)        /* i+j odd */
                sum -= pPascal2[j] * pBezCoff[j];
            else
                sum += pPascal2[j] * pBezCoff[j];

        pPowCoff[i] = sum * pPascal[i];
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the bezier coefficients for a bezier in which the interval [0,1] corresponds
* to the positive half of the real line for a power basis polynomial.
* The mapping from u to x is x = u / (1-u).
*
* @param    pBezCoff    => coefficients in Bernstein-Bezier basis
* @param    pPowCoff    <= coefficients in Power basis
* @param    order       => order of the polynomial
* @bsimethod                                                    DavidAssaf      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_mapPositiveRealsToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
)
    {
    int i;
    int last = order - 1;
    double *pPascal;

    pBezCoff[0]     = pPowCoff[0];
    pBezCoff[last]  = pPowCoff[last];

    pPascal = jmdlBezier_getPascalRow (order - 1);

    // compute all remaining coefficients
    for (i = 1; i < last; i++)
        {
        pBezCoff[i] = pPowCoff[i] / pPascal[i];
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the bezier coefficients for a bezier in which the interval [0,1] corresponds
* to the negative half of the real line for a power basis polynomial.
* The mapping from u to x is x = -u / (1-u).
*
* @param    pBezCoff    => coefficients in Bernstein-Bezier basis
* @param    pPowCoff    <= coefficients in Power basis
* @param    order       => order of the polynomial
* @bsimethod                                                    DavidAssaf      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_mapNegativeRealsToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
)
    {
    int i;
    double s = -1.0;
    double *pPascal;

    pBezCoff[0]     = pPowCoff[0];

    pPascal = jmdlBezier_getPascalRow (order - 1);

    for (i = 1, s = -1.0; i < order; i++, s = -s)
        {
        pBezCoff[i] = s * pPowCoff[i] / pPascal[i];
        }
    }


/*---------------------------------------------------------------------------------**//**
* Convert coefficients of a univariate polynomial in the Power basis to
* coefficients in the Bernstein-Bezier basis, with identity mapping between
* x and u.
*
* @param    pPowCoff    <= coefficients in Bernstein-Bezier basis
* @param    pBezCoff    => coefficients in Power basis
* @param    order       => order of the polynomial
* @bsimethod                                                    DavidAssaf      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_convertPowerToBezier
(
double      *pBezCoff,
double      *pPowCoff,
int         order
)
    {
    int i,j;
    double *pPascal, *pPascal2;
    double sum;
    int degree = order - 1;

    // zeroth basis functions (i.e. constant term) have same coefficient
    pBezCoff[0] = pPowCoff[0];

    pPascal = jmdlBezier_getPascalRow (degree);

    // compute all but first and last coefficients
    for (i = 1; i < degree; i++)
        {
        sum = pPowCoff[0];      // constant term is always an addend
        pPascal2 = jmdlBezier_getPascalRow (i);

        for (j = 1; j <= i; j++)
            sum += pPascal2[j] / pPascal[j] * pPowCoff[j];

        pBezCoff[i] = sum;
        }

    // last BB-coefficient is sum of P-coefficients
    for (i = 0, sum = 0.0; i < order; i++)
            sum += pPowCoff[i];

    pBezCoff[degree] = sum;
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the Bernstein-Bezier Basis Functions for a given order.
*
* @param    pBi     <= array of order basis function values.
* @param    u       => parameter value for evaluation.
* @param    order   => polynomial order (degree + 1)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_evaluateBasisFunctions
(
double      *pBi,
int         order,
double      u
)
    {
    double uTo[MAX_BEZIER_ORDER];
    double vTo[MAX_BEZIER_ORDER];
    double *pC;
    int    i;
    double v = 1.0 - u;
    double uu, vv;
    int    degree = order - 1;

    switch (order)              /* low order cases explicitly given for speed */
        {
        case 1:
            pBi[0] = 1.0;
            break;
        case 2:
            pBi[1] = u;
            pBi[0] = v;
            break;
        case 3:
            pBi[2] = u * u;
            pBi[1] = 2.0 * u * v;
            pBi[0] = v * v;
            break;
        case 4:
            uu = u * u;
            vv = v * v;
            pBi[3] = uu * u;
            pBi[2] = 3.0 * uu * v;
            pBi[1] = 3.0 * u * vv;
            pBi[0] = vv * v;
            break;
        /* For higher orders, use definition of Bernstein polynomials,
           instead of the slower recurrence formula. */

        /* For example, if m = order, then the recurrence-based Piegl-Tiller
           Alg. A1.3 (p.20) uses
                (m^2 + m)/2 adds
                (m^2 + m)   multiplies
           while the below code (including initializing Pascal) only uses
                (m^2 + m)/2 adds
                4m          multiplies.
        */
        default:
            uTo[0] = vTo[0] = uu = vv = 1.0;
            for (i = 1; i <= order; i++)
                {
                uTo[i] = uu = u * uu;
                vTo[i] = vv = v * vv;
                }
            pC = jmdlBezier_getPascalRow (degree);
            for (i = 0; i < order; i++)
                {
                pBi[i] = uTo[i] * vTo[degree - i] * pC[i];
                }
        }
    }


Public GEOMDLLIMPEXP void bsiBezier_evaluateDerivativeBasisFunctions
(
double      *pdBi,
int         order,
double      u
)
    {
    if (order > 1)
        {
        // derivatives for {order} are difference of primaries at {order-1}
        bsiBezier_evaluateBasisFunctions (pdBi, order - 1, u);
        pdBi[order - 1] = pdBi[order - 2];
        for (int i = order - 2; i > 0; i--)
            pdBi[i] = pdBi[i-1] - pdBi[i];
        pdBi[0] = - pdBi[0];
        double a = (double)(order-1);
        for (int i = 0; i < order; i++)
            pdBi[i] *= a;
        }
    else if (order == 1)
        pdBi[0] = 0.0;
    }



/*---------------------------------------------------------------------------------**//**
* Evaluate a Bezier curve by summing the (vector of) Bernstein basis function values.
*
* @param    pPoint          <= evaluated point
* @param    pPoles          => array of order poles
* @param    order           => polynomial order (degree + 1)
* @param    numComponent    => number of components of each pole vector.
* @param    u               => parameter value for evaluation.
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_evaluate
(
double      *pPoint,
double      *pPoles,
int         order,
int         numComponent,
double      u
)
    {
    double basisFunc[MAX_BEZIER_ORDER];
    int i, j;
    double *pP;
    double f;

    bsiBezier_evaluateBasisFunctions (basisFunc, order, u);

    /* Use definition of Bezier curve in terms of the Bernstein basis instead of
       using the deCasteljau algorithm for sake of speed:
       Indeed, on each component, deCasteljau uses
                (m^2 + m)/2     adds
                 m^2 + m        multiplies
       but below code increments only m adds and m mults to initialization steps.
       Thus for m > 4 we use a total of
                (m^2 + 3m)/2    adds
                5m              multiplies,
       which for m > 4 does not exceed the number of operations of deCasteljau.
       Also, since both evaluation algorithms use the Bernstein basis for polynomials,
       both enjoy the same numerical stability.
    */

    for (j = 0; j < numComponent; j++)
        {
        f = 0.0;
        pP = pPoles + j;
        for (i = 0; i < order; i++, pP += numComponent)
            f += basisFunc[i] * (*pP);
        *pPoint++ = f;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate a Bezier curve by summing the (vector of) Bernstein basis function values.
*
* @param    pX      <= evaluated point
* @param    pdXdu   <= partial derivative wrt u
* @param    pdXdv   <= partial derivative wrt v
* @param    pd2Xdudv <= mixed partial.
* @param    pPoles      => array of order poles
* @param    orderU      => number of u poles
* @param    strideU     => stride between u poles
* @param    orderV      => number of v poles
* @param    strideV     => stride between v poles
* @param    numComponet     => number of components per pole.
* @param    u           => u parameter for evaluation
* @param    v           => v parameter for evaluation
* @bsimethod                                                    EarlinLutz      04/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_evaluateTensorProduct
(
double      *pX,
double      *pdXdu,
double      *pdXdv,
double      *pd2Xdudv,
const double      *pPoles,
int         orderU,
int         strideU,
int         orderV,
int         strideV,
int         numComponent,
double      u,
double      v
)
    {
    double Bu[MAX_BEZIER_ORDER];
    double Budu[MAX_BEZIER_ORDER];
    double Bv[MAX_BEZIER_ORDER];
    double Bvdv[MAX_BEZIER_ORDER];
    double au[MAX_BEZIER_ORDER];
    double f, a;
    const double *pCurr0, *pCurr;
    int   jumpU = numComponent * strideU;
    int   jumpV = numComponent * strideV;
    int orderUm1 = orderU - 1;
    int orderVm1 = orderV - 1;

    int i, j, k;
    bsiBezier_evaluateBasisFunctions (Bu, orderU, u);
    bsiBezier_evaluateBasisFunctions (Bv, orderV, v);

    if (pX)
        {
        /* Only need the surface. */
        for (k = 0; k < numComponent; k++)
            {
            f = 0.0;
            pCurr0 = pPoles + k;
            pCurr = pCurr0;
            for (j = 0; j < orderV; j++, pCurr0 += jumpV)
                {
                a = 0.0;
                pCurr = pCurr0;
                for (i = 0; i < orderU; i++, pCurr += jumpU)
                    a += Bu[i] * (*pCurr);
                f += Bv[j] * a;
                }
            pX[k] = f;
            }
        }

    if (pdXdu)
        {
        bsiBezier_evaluateBasisFunctions (Budu, orderUm1, u);
        for (k = 0; k < numComponent; k++)
            {
            f = 0.0;
            pCurr0 = pPoles + k;
            pCurr = pCurr0;
            for (j = 0; j < orderV; j++, pCurr0 += jumpV)
                {
                a = 0.0;
                pCurr = pCurr0;
                for (i = 0; i < orderUm1; i++, pCurr += jumpU)
                    a += Budu[i] * (pCurr[jumpU] - pCurr[0]);
                f += Bv[j] * a;
                }
            pdXdu[k] = f * orderUm1;
            }
        }

    if (pdXdv)
        {
        bsiBezier_evaluateBasisFunctions (Bvdv, orderVm1, v);
        for (k = 0; k < numComponent; k++)
            {
            f = 0.0;
            pCurr0 = pPoles + k;
            pCurr = pCurr0;
            for (i = 0; i < orderU; i++, pCurr0 += jumpU)
                {
                a = 0.0;
                pCurr = pCurr0;
                for (j = 0; j < orderVm1; j++, pCurr += jumpV)
                    a += Bvdv[j] * (pCurr[jumpV] - pCurr[0]);
                f += Bu[i] * a;
                }
            pdXdv[k] = f * orderVm1;
            }
        }
    if (pd2Xdudv)
        {
        if (!pdXdv)
            bsiBezier_evaluateBasisFunctions (Bvdv, orderVm1, v);
        if (!pdXdu)
            bsiBezier_evaluateBasisFunctions (Budu, orderUm1, u);

        for (k = 0; k < numComponent; k++)
            {
            f = 0.0;
            pCurr0 = pPoles + k;
            pCurr = pCurr0;
            for (i = 0; i < orderUm1; i++, pCurr0 += jumpU)
                {
                a = 0.0;
                pCurr = pCurr0;
                for (j = 0; j < orderVm1; j++, pCurr += jumpV)
                    a += Bv[j] * (pCurr[jumpV] - pCurr[0]);
                au[i] = a;
                }

            for (i = 0; i < orderUm1; i++)
                f += Budu[i] * (au[i+1] - au[i]);

            pd2Xdudv[k] = f * orderVm1 * orderUm1;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the poles of the derivative of a Bezier curve
*
* @param    pDeriv          <= array of order - 1 (degree) poles of the derivative
* @param    pPoles          => array of order poles
* @param    order           => number of poles in function
* @param    numComponent    => number of components of each pole vector.
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_derivativePoles
(
double      *pDeriv,
double      *pPoles,
int         order,
int         numComponent
)
    {
    int i;
    int j;
    int offset;
    double *pD;
    double *pP;
    double degree = order - 1;
    double a = (double) degree;
    for (j = 0; j < numComponent; j++)
        {
        pD = pDeriv + j;
        pP = pPoles  + j;
        for (i = 0; i < degree; i++)
            {
            offset = i * numComponent;
            pD[offset] = a * (pP[offset + numComponent] - pP[offset]);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the poles of the integral
*
* @param    pIntegral       <= array of order + 1 poles of the integral.
* @param    *pPole0         => constant of integration. (First pole of integral, numComponent entries)
* @param    h               => x extent of full interval.
* @param    pPoles          => array of (order) poles of input bezier
* @param    inOrder         => number of input
* @param    numComponent    => number of components of each pole vector.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_integralPoles
(
double      *pIntegral,
double      *pPole0,
double      h,
double      *pPoles,
int         order,
int         numComponent
)
    {
    int i;
    int j;
    double s;
    int offset;
    double *pI;
    double *pP;
    //double degree = order - 1;
    double a = h / (double) order;

    for (j = 0; j < numComponent; j++)
        {
        pI = pIntegral + j;
        pP = pPoles  + j;
        s = *pI = pPole0[j];
        pI += numComponent;
        offset = 0;
        for (i = offset = 0; i < order; i++, offset += numComponent)
            {
            s += pP[offset] * a;
            pI[offset] = s;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the poles of the integral
*
* @param    pIntegral       <= array of order + 1 poles of the integral.
* @param    f0              => constant of integration.
* @param    h               => x extent of full interval.
* @param    pPoles          => array of (order) poles of input bezier
* @param    inOrder         => number of input
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_univariateIntegralPoles
(
double      *pIntegral,
double      f0,
double      h,
double      *pPoles,
int         order
)
    {
    bsiBezier_integralPoles (pIntegral, &f0, h, pPoles, order, 1);
    }

/*---------------------------------------------------------------------------------**//**
* Raise the degree by 1.  May be called with the same array, with output occupying
* the input pole memory plus one additional position.
*
* @param    pB          <= order + 1 poles
* @param    pA          => order poles
* @param    order           => input curve order
* @param    numComponent    => number of components of each pole vector.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_raiseDegree
(
double          *pB,
const double    *pA,
int         order,
int         numComponent
)
    {
    int i;
    int j;
    double *pBi, a1, a0;
    const double *pAi;
    double s = 1.0 / (double) order;

    for (j = 0; j < numComponent; j++)
        {
        /* Work backwards to allow overwrite. */
        pBi = pB + order * numComponent + j;
        pAi = pA + (order - 1) * numComponent + j;
        /* Final poles match .. */
        a1 = *pBi = *pAi;
        for (i = order - 1; i > 0; i--)
            {
            pAi -= numComponent;
            pBi -= numComponent;
            a0 = *pAi;
            *pBi = a0 + (order - i) * s * (a1 - a0);
            a1 = a0;
            }
        /* And first poles match */
        pB[j] = pA[j];
        }
    }


/*---------------------------------------------------------------------------------**//**
* Repeated degree raising in  place.
*
* @param    pB              <=> poles
* @param    inOrder         => input curve order
* @param    outOrder        => output curve order
* @param    numComponent    => number of components of each pole vector.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_raiseDegreeInPlace
(
double          *pB,
int             orderIn,
int             orderOut,
int             numComponent
)
    {
    int currOrder;
    for (currOrder = orderIn; currOrder < orderOut; currOrder++)
            bsiBezier_raiseDegree (pB, pB, currOrder, numComponent);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Reduce degree of a univariate bezier, optionally working right to left or left to right.
* Expect to call this twice and compare results to see if degree is actually reducible!!
*   Do NOT call with inplace!!!
* @param pA <= reduced degree coefficients.
* @param pB => higher degree coefficients.
* @param orderB => order of B.
* @param reverseDirection => true to sweep from right to left.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlBezier_univariateReduceDegreeDirectional
(
      double    *pA,
const double    *pB,
      int       orderB,
      bool      reverseDirection
)
    {
    int dB = orderB - 1;                /* DEGREE of B */
    //int orderA = orderB - 1;
    double *pCB  = jmdlBezier_getPascalRow (dB);
    int dA = dB - 1;            /* DEGREE of A */
    double *pCA  = jmdlBezier_getPascalRow (dA);
    int i;

    if (!pCA || !pCB)
        return false;

    /* Shift base pointers to component */

    if (reverseDirection)
        {
        pA[dA] = pB[dB];
        for (i = dA; i > 0; i--)
            pA[i-1] = (pCB[i] * pB[i] - pCA[i] * pA[i])
                                        / pCA[i-1];
        }
    else
        {
        pA[0] = pB[0];
        for (i = 1; i <= dA; i++)
            pA[i] = (pCB[i] * pB[i] - pCA[i-1] * pA[i-1])
                                        / pCA[i];
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Reduce degree of a univariate bezier, optionally working right to left or left to right.
* Expect to call this twice and compare results to see if degree is actually reducible!!
*   Do NOT call with inplace!!!
* @param pA <= reduced degree coefficients.
* @param pB => higher degree coefficients.
* @param orderB => order of B.
* @param numCoff => number of coefficients per pole.
* @param reverseDirection => true to sweep from right to left.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezier_reduceDegreeDirectional
(
      double    *pA,
const double    *pB,
      int       orderB,
      int       numCoff,
      bool      reverseDirection
)
    {
    int iA, iB, j;
    int k;
    int orderA = orderB - 1;
    if (orderA < 1 || orderB < 1)
        return false;
    double *pAk = (double*)_alloca (orderA * sizeof (double));
    double *pBk = (double*)_alloca (orderB * sizeof (double));

    if (!pAk || !pBk)
        return false;

    for (k = 0; k < numCoff; k++)
        {
        for (iB = 0, j = k; iB < orderB; iB++, j += numCoff)
            pBk[iB] = pB[j];
        jmdlBezier_univariateReduceDegreeDirectional (pAk, pBk, orderB, reverseDirection);
        for (iA = 0, j = k; iA < orderA; iA++, j += numCoff)
            pA[j] = pAk[iA];
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Reduce degree of a univariate bezier, working in both directions and averaging results.
* @param pA <= reduced degree coefficients.
* @param pB => higher degree coefficients.
* @param orderB => order of B.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlBezier_univariateReduceDegreeAverage
(
      double    *pA,
      double    *pError,
const double    *pB,
      int       orderB
)
    {
    double A0[MAX_BEZIER_ORDER];
    double A1[MAX_BEZIER_ORDER];
    int i;
    int orderA = orderB - 1;
    int degreeA = orderA - 1;
    bool stat = false;
    double error = 0.0;

    if (   jmdlBezier_univariateReduceDegreeDirectional (A0, pB, orderB, false)
        && jmdlBezier_univariateReduceDegreeDirectional (A1, pB, orderB, true))
        {
        double scale = 1.0 / degreeA;
        for (i = 0; i < orderA; i++)
            {
            error += fabs (A0[i] - A1[i]);
            pA[i] = ((degreeA - i) * A0[i] + i * A1[i]) * scale;
            }
        stat = true;
        }
    if (pError)
        *pError = error;
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* Reduce degree of a univariate bezier, working in both directions and averaging results.
* @param pA <= reduced degree coefficients.
* @param pB => higher degree coefficients.
* @param orderB => order of B.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezier_reduceDegree
(
      double    *pA,
      double    *pError,
const double    *pB,
      int       orderB,
      int       numComponent
)
    {
    double AA[MAX_BEZIER_ORDER];
    double BB[MAX_BEZIER_ORDER];
    double totalError = 0.0, error = 0.0;
    int j;
    int orderA = orderB - 1;
    bool stat = true;
    for (j = 0; stat && j < numComponent; j++)
        {
        bsiBezier_copyComponent (BB, 0, 1, pB, j, numComponent, orderB);
        stat = jmdlBezier_univariateReduceDegreeAverage (AA, &error, BB, orderB);
        if (!stat)
            break;
        totalError += error;
        bsiBezier_copyComponent (pA, j, numComponent, AA, 0, 1, orderA);
        }

    if (pError)
        *pError = totalError;
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the sum of coefficients scaled by the binomial coefficients and alternating signs
@param pSignedSum OUT sum of signed coefficients times binomial coefficients times (-1)^i.
            (This is the leading coefficient of the standard basis form of the polynomial)
@param pAbsUm OUT sum of absolute values of coefficients times binomial coeffients.
        (This is a useful indication of size of coefficient, to determine of the signed sum is
            close to zero in a relative sense.)
@param pB IN coefficients
@param orderB order of polynomial
@param numComponent number of components per pole.  Also the number of components of the output
        signed sum and abs sum.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiBezier_sumScaledCoefficients
(
      double    *pSignedSum,
      double    *pAbsSum,
const double    *pB,
      int       orderB,
      int       numComponent
)
    {
    int i, j, k;
    const double *pCB  = jmdlBezier_getPascalRow (orderB - 1);
    double aa;
    double s;
    if (pSignedSum)
        {
        for (k = 0; k < numComponent; k++)
            {
            aa = 0.0;
            for (i = 0, j = k, s = 1.0; i < orderB; i++, j += numComponent, s = -s)
                aa += s * pCB[i] * pB[j];
            pSignedSum[k] = aa;
            }
        }

    if (pAbsSum)
        {
        for (k = 0; k < numComponent; k++)
            {
            aa = 0.0;
            for (i = 0, j = k; i < orderB; i++, j += numComponent)
                aa += fabs (pCB[i] * pB[j]);
            pAbsSum[k] = aa;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Return orderB-2 coefficients of the (Bezier form) coefficients of the polynomial
* remaining after the following reductions:
* 1) Subtract the straight line between endpoints.
* 2) factor out (1-u)*u from the deviation.
*
* (OR: Return the residual after extraction of the leading (linear)part of the
*       symmetric basis {u, 1-u, u(1-u)u, u(1-u)(1-u), ... U^k u, U^k (1-u)}
* where the k'th pair of basis functions is U^k=u^k (1-u)^k=(u(1-u))^k.
*
* @param pA <= orderB - 2 reduced degree coefficients.
* @param pB => higher degree coefficients.
* @param orderB => order of B.
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezier_symmetricRemainder
(
      double    *pA,
const double    *pB,
      int       orderB,
      int       numComponent
)
    {

    double AA[MAX_BEZIER_ORDER];
    double BB[MAX_BEZIER_ORDER];
    double b0, b1;
    const double *pCB, *pCA;
    double scale;
    int i,j,k;
    int orderA = orderB - 2;
    bool stat = true;

    if (orderB <= 2)
        return false;

    pCB  = jmdlBezier_getPascalRow (orderB - 1);
    pCA  = jmdlBezier_getPascalRow (orderA - 1);
    scale = 1.0 / (double)(orderB - 1);
    for (j = 0; stat && j < numComponent; j++)
        {
        bsiBezier_copyComponent (BB, 0, 1, pB, j, numComponent, orderB);
        b0 = BB[0] * scale;
        b1 = BB[orderB - 1] * scale;
        for (k = 0; k < orderA; k++)
            {
            i = k + 1;
            AA[k] = (BB[i] - (b0 * (orderB - 1 - i) + b1 * i)) * pCB[k+1] / pCA[k];
            }
        bsiBezier_copyComponent (pA, j, numComponent, AA, 0, 1, orderA);
        }

    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* Copy (all coordinates of) poles
*
* @param    pDest           => destination pole array
* @param    iDest           => pole index within destination
* @param    pSource         => source bezier
* @param    iSource         => index of pole in source
* @param    numCopy         => number of poles to copy
* @param    numComponent    => number of components per pole
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_copyPoles
(
double      *pDest,
int         iDest,
double      *pSource,
int         iSource,
int         numCopy,
int         numComponent
)
    {
    memcpy (pDest + iDest * numComponent,
            pSource + iSource * numComponent,
            numCopy * numComponent * sizeof (double));
    }


/*---------------------------------------------------------------------------------**//**
* Copy one component from a multicomponent source to a multicomponent destination.
*
* @param    pDest               => destination pole array
* @param    jDest               => destination component index
* @param    numComponentDest    => number of components in the destination
* @param    pSource             => source pole array
* @param    jSource             => source component index
* @param    numComponentSource  => number of components in the source
* @param    numCopy             => number of doubles to copy (a.k.a. order)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_copyComponent
(
double      *pDest,
int         jDest,
int         numComponentDest,
const double      *pSource,
int         jSource,
int         numComponentSource,
int         numCopy
)
    {
    int i;
    pDest += jDest;
    pSource += jSource;
    for (i = 0; i < numCopy; i++, pSource += numComponentSource, pDest += numComponentDest)
        *pDest = *pSource;
    }


/*---------------------------------------------------------------------------------**//**
* Form a linear combination of two poles (from the same Bezier curve).
*
* @param    pDest           <=> destination pole array
* @param    iDest           => pole index within destination
* @param    pSource         => source bezier
* @param    iSource0        => index of first point in source
* @param    u0              => scale factor for first point
* @param    iSource1        => index of second point in source
* @param    u1              => scale factor for second point
* @param    numComponent    => number of components per pole
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_addScaledPoles
(
double      *pDest,
int         iDest,
double      *pSource,
int         iSource0,
double      scale0,
int         iSource1,
double      scale1,
int         numComponent
)
    {
    int i;
    double *pD = pDest + iDest * numComponent;
    double *p0 = pSource + iSource0 * numComponent;
    double *p1 = pSource + iSource1 * numComponent;
    for (i = 0; i < numComponent; i++)
        pD[i] = p0[i] * scale0 + p1[i] * scale1;
    }


Public GEOMDLLIMPEXP bool bsiBezier_evaluateUnivariate
(
double      *pP,
double      *pD,
double      *pPoles,
int         order,
double      u
)
    {
    double v = 1.0 - u;
    double v2, v3, v4, v5;
    if (order < 1)
        return false;
    if (order == 1)
        {
        *pP = pPoles[0];
        if (pD)
            *pD = 0.0;
        return true;
        }
    else if (order == 2)
        {
        *pP = v * pPoles[0] + u * pPoles[1];
        if (pD)
            *pD = pPoles[1] - pPoles[0];
        return true;
        }
    else if (order == 3)
        {
        v2 = v * v;
        *pP =          pPoles[0] * v2
            + u * (2.0 * pPoles[1] * v
            + u *          pPoles[2]);
        if (pD)
            *pD = 2.0 * ( v * (pPoles[1] - pPoles[0])
                        + u * (pPoles[2] - pPoles[1]));
        return true;
        }
    else if (order == 4)
        {
        v2 = v * v;
        v3 = v2 * v;
        *pP =          pPoles[0] * v3
            + u * (3.0 * pPoles[1] * v2
            + u *  (3.0 * pPoles[2] * v
            + u *          pPoles[3]));
        if (pD)
            *pD = 3.0 * (           (pPoles[1] - pPoles[0]) * v2
                        + u * (2.0 * (pPoles[2] - pPoles[1]) * v
                        + u *          (pPoles[3] - pPoles[2])));
        return true;
        }
    else if (order == 5)
        {
        v2 = v * v;
        v3 = v2 * v;
        v4 = v3 * v;
        *pP =          pPoles[0] * v4
            + u * (4.0 * pPoles[1] * v3
            + u *  (6.0 * pPoles[2] * v2
            + u *   (4.0 * pPoles[3] * v
            + u *            pPoles[4])));
        if (pD)
            *pD = 4.0 * (           (pPoles[1] - pPoles[0]) * v3
                        + u * (3.0 * (pPoles[2] - pPoles[1]) * v2
                        + u *  (3.0 * (pPoles[3] - pPoles[2]) * v
                        + u *           (pPoles[4] - pPoles[3]))));
        return true;
        }
    else if (order == 6)
        {
        v2 = v * v;
        v3 = v2 * v;
        v4 = v3 * v;
        v5 = v4 * v;
        *pP =          pPoles[0] * v5
            + u * (5.0 * pPoles[1] * v4
            + u *  (10.0 * pPoles[2] * v3
            + u *   (10.0 * pPoles[3] * v2
            + u *     (5.0 * pPoles[4] * v
            + u *             pPoles[5]))));
        if (pD)
            *pD = 5.0 * (           (pPoles[1] - pPoles[0]) * v4
                        + u * (4.0 * (pPoles[2] - pPoles[1]) * v3
                        + u *  (6.0 * (pPoles[3] - pPoles[2]) * v2
                        + u *    (4.0 * (pPoles[4] - pPoles[3]) * v
                        + u *             (pPoles[5] - pPoles[4])))));
        return true;
        }

    double vn[MAX_BINOMIAL_ORDER];
    double *pPascal = jmdlBezier_getPascalRow (order - 1);
    vn[0] = 1.0;
    for (int i = 1; i < order; i++)
        vn[i] = vn[i-1] * v;
    double f = vn[order - 1] * pPoles[0];
    double un = u;
    for (int i = 1, j = order - 2; i < order - 1; i++, un *= u, j--)
        f += un * vn[j] * pPascal [i] * pPoles[i];
    f += un * pPoles[order - 1];
    *pP = f;

    if (NULL != pD)
        {
        int derivativeOrder = order - 1;
        pPascal = jmdlBezier_getPascalRow (derivativeOrder - 1);
        un = u;
        f = (pPoles[1] - pPoles[0]) * vn[derivativeOrder - 1];
        for (int i = 1, j = derivativeOrder - 2; i < derivativeOrder - 1; i++, un *= u, j--)
            f += un * vn[j] * pPascal [i] * (pPoles[i+1] - pPoles[i]);
        f += un * (pPoles[derivativeOrder] - pPoles[derivativeOrder - 1]);
        *pD = derivativeOrder * f;        
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the function value and first derivative using deCasteljau algorithm.
*
* @param    pP              <= function value at u
* @param    pD              <= derivative at u
* @param    pK              <= second derivative at u
* @param    pPoles          => control polygon
* @param    order           => number of poles
* @param    numComponent    => number of components per pole
* @param    u               => parameter value
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_functionAndDerivativeExt
(
double      *pP,
double      *pD,
double      *pK,
double      *pPoles,
int         order,
int         numComponent,
double      u
)
    {
    /* OUCH!! We really don't want max component here!!! */
#define MAX_COMPONENT 6
    double bCoff[MAX_COMPONENT * MAX_BEZIER_ORDER];
    int i, j, k;
    int    degree = order - 1;
    double a = (double) degree;
    double b = (double) degree * (degree - 1);
    double v = 1.0 - u;
    int bottom, bottom1;


    bsiBezier_copyPoles
            (bCoff, 0, pPoles, 0, order, numComponent);

    if (numComponent == 1)
        {
        if (pK && degree < 2)
            {
            /* zero out second derivative, suppress the pointer */
            *pK = 0.0;
            pK = NULL;
            }

        for (j = 1; j <= degree; j++)
            {
            if (j == degree)
                {
                if (pD)
                    *pD = a * (bCoff[j] - bCoff[j-1]);
                }
            else if (j == degree - 1)
                {
                if (pK)
                    *pK = b * ((bCoff[j+1] - bCoff[j]) - (bCoff[j] - bCoff[j-1]));
                }

            for (k = degree; k >= j; k--)
                bCoff[k] = v * bCoff[k-1] + u * bCoff[k];
            }

        *pP = bCoff[degree];
        }
    else    /* numComponent > 1 */
        {
        if (pK && degree < 2)
            {
            /* zero out second derivative, suppress the pointer */
            memset (pK, 0, numComponent * sizeof(double));
            pK = NULL;
            }

        for (i = 0; i < numComponent; i++)
            {
            bottom = i + degree * numComponent;
            bottom1 = bottom - numComponent;

            for (j = i + numComponent; j <= bottom; j += numComponent)
                {
                if (j == bottom)
                    {
                    if (pD)
                        *pD++ = a * (bCoff[j] - bCoff[j - numComponent]);
                    }
                else if (j == bottom1)
                    {
                    double a0 = bCoff[j - numComponent];
                    double a1 = bCoff[j];
                    double a2 = bCoff[j + numComponent];
                    if (pK)
                        *pK++ = b * ((a2 - a1) - (a1 - a0));
                    }

                for (k = bottom; k >= j; k -= numComponent)
                    bCoff[k] = v * bCoff[k-numComponent] + u * bCoff[k];
                }

            *pP++ = bCoff[bottom];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute the function value and first derivative using deCasteljau algorithm.
*
* @param    pP              <= function value at u
* @param    pD              <= derivative at u
* @param    pPoles          => control polygon
* @param    order           => number of poles
* @param    numComponent    => number of components per pole
* @param    u               => parameter value
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_functionAndDerivative
(
double      *pP,
double      *pD,
double      *pPoles,
int         order,
int         numComponent,
double      u
)
    {
    if (numComponent == 1)
        bsiBezier_evaluateUnivariate (pP, pD, pPoles, order, u);
    else
        bsiBezier_functionAndDerivativeExt (pP, pD, NULL, pPoles, order, numComponent, u);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the control points of the two polygons for curves from 0..u and u..1
*
* @param    pLeft           <= left polygon control points
* @param    pRight          <= right polygon control points
* @param    pPoles          => full polygon control points
* @param    order           => number of poles
* @param    numComponent    => number of components per pole
* @param    u               => subdivision parameter
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_subdivisionPolygons
(
double      *pLeft,
double      *pRight,
double      *pPoles,
int         order,
int         numComponent,
double      u
)
    {
    int     i;
    int     degree = order - 1;
    double  v = 1.0 - u;
    double  *pL, *pLbottom, *pLtop, *pLprev;
    double  *pR, *pRbottom;


    /* See Piegl-Tiller page 24, figure Table 1.3 for deCasteljau triangle.
        The left polygon poles are the upper fringe of the triangle.
        The right polygon poles are the lower fringe of the triangle.
        Initialize pLeft along the left edge (vertical column with order entries)
        of the triangle.  Write successive columns (each with one less entry than
        the previous) in place in the tail of pLeft.  After writing column i,
        entries 0 through i in pLeft are the first i+1 poles of the left
        polygon; the last entry in pLeft after writing column i is pole
        degree-i of the right polygon.
    */


    /* write zeroth column of each coordinate */
    bsiBezier_copyPoles (pLeft, 0, pPoles, 0, order, numComponent);

    if (numComponent == 1)
        {
        pRbottom = pRight + degree;
        pLbottom = pLeft + degree;

        pR      = pRbottom;     /* last pole of right polygon       */
        *pR--   = *pLbottom;    /*   = last pole of mother polygon  */

        /* write column *pLtop - *pLeft from bottom up */
        for (pLtop = pLeft + 1; pLtop <= pLbottom; pLtop++)
            {
            for (pL = pLbottom, pLprev = pL - 1; pL >= pLtop; pL--, pLprev--)
                *pL = v * (*pLprev) + u * (*pL);

            *pR-- = *pLbottom;
            }
        }
    else    /* numComponent > 1 */
        {
        pRbottom = pRight + degree * numComponent;
        pLbottom = pLeft + degree * numComponent;

        for (i = 0; i < numComponent; i++)
            {
            pR   = pRbottom;            /* last pole of right polygon       */
            *pR  = *pLbottom;           /*   = last pole of mother polygon  */
            pR  -= numComponent;

            /* write next column from bottom up */
            for (
                pLtop = pLeft + i + numComponent;
                pLtop <= pLbottom;
                pLtop += numComponent
                )
                {
                for (
                    pL = pLbottom, pLprev = pL - numComponent;
                    pL >= pLtop;
                    pL -= numComponent, pLprev -= numComponent
                    )
                    *pL = v * (*pLprev) + u * (*pL);

                *pR = *pLbottom;
                pR -= numComponent;
                }

            pRbottom++;     /* set for next         */
            pLbottom++;     /*   coord of the poles */
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the (univariate) control points of the dot product of two vector bezier functions.
*
* @param    pAB             <= orderAB poles
* @param    pOrderAB        <= order of dot product
* @param    maxOrderAB      => permitted order of product.
* @param    pA              => control points for bezier B
* @param    orderA          => number of control points for bezier A
* @param    firstComponentA => first component index to use in A
* @param    numComponentA   => number of components in bezier A (may be more than components used in
*                                   dot product)
* @param    pB              => control points for bezier B
* @param    orderB          => number of control points for bezier B
* @param    firstComponentB => first component index to use in B
* @param    numVectorComponent => number of vector components
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_dotProduct
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
double      *pA,
int         orderA,
int         firstComponentA,
int         numComponentA,
double      *pB,
int         orderB,
int         firstComponentB,
int         numComponentB,
int         numVectorComponent
)
    {
    double work[MAX_BEZIER_ORDER];
    int i, k;
    int    orderAB = orderA + orderB - 1;
    *pOrderAB = 0;
    if (orderAB > MAX_BEZIER_ORDER || orderAB > maxOrderAB)
        return false;
    if (numVectorComponent <= 0)
        return false;

    *pOrderAB = orderAB;
    bsiBezier_univariateProduct (  pAB, 0, 1,
                                    pA, orderA, firstComponentA, numComponentA,
                                    pB, orderB, firstComponentB, numComponentB);
    for (k = 1; k < numVectorComponent; k++)
        {
        bsiBezier_univariateProduct
                                    (
                                    work, 0, 1,
                                    pA, orderA, firstComponentA + k, numComponentA,
                                    pB, orderB, firstComponentB + k, numComponentB);
        for (i = 0; i < orderAB; i++)
            pAB[i] += work[i];
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the (univariate) control points of the cross product of two components
* of vector functions, i.e the difference A[iA0] * B[iB1] - A[iA1] * B[iB0]
*
* (Typically, A and B will be addressed similarly, so iA0==iB0==i0 and iA1==iB1==i1, in the
*  usual cyclic combinations of (iAB, i0, i1) of (0,1,2), (1,2,0), and (2,0,1)).
*
* @param    pAB             <= orderAB poles
* @param    pOrderAB        <= order of cross product
* @param    iAB             => component index in AB
* @param    numComponentAB  => number of components in poles of fAB
* @param    maxOrderAB      => permitted order of product.
* @param    pA              => control points for bezier B
* @param    orderA          => number of control points for bezier A
* @param    iA0             => first component index to use in A
* @param    iA1             => second component index to use in A
* @param    numComponentA   => number of components in  poles of A
* @param    pB              => control points for bezier B
* @param    orderB          => number of control points for bezier B
* @param    iAB             => first component index to use in B
* @param    iAB             => second component index to use in B
* @param    numComponentB   => number of components in poles of B
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_crossProductComponent
(
double      *pAB,
int         *pOrderAB,
int         iAB,
int         numComponentAB,
int         maxOrderAB,
double      *pA,
int         orderA,
int         iA0,
int         iA1,
int         numComponentA,
double      *pB,
int         orderB,
int         iB0,
int         iB1,
int         numComponentB
)
    {
    double product01[MAX_BEZIER_ORDER];
    double product10[MAX_BEZIER_ORDER];
    double crossProduct[MAX_BEZIER_ORDER];
    int    orderAB = orderA + orderB - 1;
    *pOrderAB = 0;
    if (orderAB > MAX_BEZIER_ORDER || orderAB > maxOrderAB)
        return false;

    *pOrderAB = orderAB;
    bsiBezier_univariateProduct (  product01, 0, 1,
                                    pA, orderA, iA0, numComponentA,
                                    pB, orderB, iB1, numComponentB);
    bsiBezier_univariateProduct (  product10, 0, 1,
                                    pA, orderA, iA1, numComponentA,
                                    pB, orderB, iB0, numComponentB);
    bsiBezier_subtractPoles (crossProduct, product01, product10, orderAB, 1);
    bsiBezier_copyComponent (pAB, iAB, numComponentAB, crossProduct, 0, 1, orderAB);
    return true;
    }

static bool       bsiBezier_fastUnivariateProduct
(
double      *pAB,
int         componentAB,
int         numComponentAB,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
)
    {
    //int degreeA = orderA - 1;
    //int degreeB = orderB - 1;
    //int degreeAB = degreeA + degreeB;
    //int orderAB  = degreeAB + 1;
    static double div3 = 1.0 / 3.0;
    static double div4 = 0.25;
    static double div5 = 1.0 / 5.0;
    static double div6 = 1.0 / 6.0;
    static double div10 = 1.0 / 10.0;
    static double div15 = 1.0 / 15.0;
    static double div20 = 1.0 / 20.0;
    if (orderA == 1)
        {
        if (orderB == 1)
            {
            pAB[componentAB] = pA[componentA] * pB[componentB];
            return true;
            }
        else if (orderB == 2)
            {
            pAB[componentAB] = pA[componentA] * pB[componentB];
            pAB[componentAB + numComponentAB] = pA[componentA] * pB[componentB + numComponentB];
            return true;
            }
        else if (orderB == 3)
            {
            pAB[componentAB] = pA[componentA] * pB[componentB];
            pAB[componentAB + numComponentAB] = pA[componentA] * pB[componentB + numComponentB];
            pAB[componentAB + 2 * numComponentAB] = pA[componentA] * pB[componentB + 2 * numComponentB];
            return true;
            }
        else
            {
            double a0 = pA[componentA];
            for (int i = 0, iAB = componentAB, iB = componentB; i < orderB;
                    i++, iB += numComponentB, iAB += numComponentAB
                    )
                pAB[iAB] = a0 * pB[iB];
            return true;
            }
        }
    else if (orderA == 2)
        {
        if (orderB == 1)
            {
            pAB[componentAB] = pA[componentA] * pB[componentB];
            pAB[componentAB + numComponentAB] = pA[componentA + numComponentA] * pB[componentB];
            return true;
            }
        else if (orderB == 2)
            {
            pAB[componentAB] = pA[componentA] * pB[componentB];
            pAB[componentAB + numComponentAB] = 
                    0.5 * (  pA[componentA] * pB[componentB + numComponentB]
                          +  pA[componentA + numComponentA] * pB[componentB]);
            pAB[componentAB + 2* numComponentAB] = pA[componentA + numComponentA] * pB[componentB + numComponentB];
            return true;
            }
        else if (orderB == 3)
            {
            double a0 = pA[componentA];
            double a1 = pA[componentA + numComponentA];
            double b0 = pB[componentB];
            double b1 = 2.0 * pB[componentB + numComponentB];
            double b2 = pB[componentB + 2 * numComponentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = (a1 * b0 + a0 * b1) * div3;
            pAB[componentAB + 2 * numComponentAB] = (a1 * b1 + a0 * b2) * div3;
            pAB[componentAB + 3 * numComponentAB] = a1 * b2;
            return true;
            }            
        else if (orderB == 4)
            {
            double a0 = pA[componentA];
            double a1 = pA[componentA + numComponentA];
            double b0 = pB[componentB];
            double b1 = 3.0 * pB[componentB + numComponentB];
            double b2 = 3.0 * pB[componentB + 2 * numComponentB];
            double b3 = pB[componentB + 3 * numComponentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = (a0 * b1 + a1 * b0) * div4;
            pAB[componentAB + 2 * numComponentAB] = (a0 * b2 + a1 * b1) * div6;
            pAB[componentAB + 3 * numComponentAB] = (a0 * b3 + a1 * b2) * div4;
            pAB[componentAB + 4 * numComponentAB] = a1 * b3;
            return true;
            }            

        }
    else if (orderA == 3)
        {
        double a0 = pA[componentA];
        double a1 = 2.0 * pA[componentA + numComponentA];
        double a2 = pA[componentA + 2 * numComponentA];
        if (orderB == 1)
            {
            double b0 = pB[componentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = (a1 * b0) * 0.5;
            pAB[componentAB + 2 * numComponentAB] = a2 * b0;
            return true;
            }
        else if (orderB == 2)
            {
            double b0 = pB[componentB];
            double b1 = pB[componentB + numComponentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = (a1 * b0 + a0 * b1) * div3;
            pAB[componentAB + 2 * numComponentAB] = (a2 * b0 + a1 * b1) * div3;
            pAB[componentAB + 3 * numComponentAB] = a2 * b1;
            return true;
            }
        else if (orderB == 3)
            {
            double b0 = pB[componentB];
            double b1 = 2.0 * pB[componentB + numComponentB];
            double b2 = pB[componentB + 2 * numComponentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = (a1 * b0 + a0 * b1) * 0.25;
            pAB[componentAB + 2 * numComponentAB] = (a2 * b0 + a1 * b1 + a0 * b2) * div6;
            pAB[componentAB + 3 * numComponentAB] = (a2 * b1 + a1 * b2) * 0.25;
            pAB[componentAB + 4 * numComponentAB] = a2 * b2;
            return true;
            }
        else if (orderB == 4)
            {
            double b0 = pB[componentB];
            double b1 = 3.0 * pB[componentB + numComponentB];
            double b2 = 3.0 * pB[componentB + 2 * numComponentB];
            double b3 = pB[componentB + 3 * numComponentB];
            pAB[componentAB] =                       a0 * b0;
            pAB[componentAB + numComponentAB] =     (a0 * b1 + a1 * b0          ) * div5;
            pAB[componentAB + 2 * numComponentAB] = (a0 * b2 + a1 * b1 + a2 * b0) * div10;
            pAB[componentAB + 3 * numComponentAB] = (a0 * b3 + a1 * b2 + a2 * b1) * div10;
            pAB[componentAB + 4 * numComponentAB] = (          a1 * b3 + a2 * b2) * div5;
            pAB[componentAB + 5 * numComponentAB] =                      a2 * b3;
            return true;
            }
        }
    else if (orderA == 4)
        {
        double a0 = pA[componentA];
        double a1 = 3.0 * pA[componentA + numComponentA];
        double a2 = 3.0 * pA[componentA + 2 * numComponentA];
        double a3 = pA[componentA + 3 * numComponentA];
        if (orderB == 1)
            {
            double b0 = pB[componentB];
            pAB[componentAB] = a0 * b0;
            pAB[componentAB + numComponentAB] = a1 * b0 * div3;
            pAB[componentAB + 2 * numComponentAB] = a2 * b0 * div3;
            pAB[componentAB + 3 * numComponentAB] = a3 * b0;
            return true;
            }
        else if (orderB == 2)
            {
            double b0 = pB[componentB];
            double b1 = pB[componentB + numComponentB];
            pAB[componentAB]                      =  a0 * b0;
            pAB[componentAB + numComponentAB]     = (a0 * b1 + a1 * b0                    ) * div4;
            pAB[componentAB + 2 * numComponentAB] = (        + a1 * b1 + a2 * b0          ) * div6;
            pAB[componentAB + 3 * numComponentAB] = (                    a2 * b1 + a3 * b0) * div4;
            pAB[componentAB + 4 * numComponentAB] =                                a3 * b1;
            return true;
            }
        else if (orderB == 3)
            {
            double b0 = pB[componentB];
            double b1 = 2.0 * pB[componentB + numComponentB];
            double b2 = pB[componentB + 2 * numComponentB];
            pAB[componentAB]                      =  a0 * b0;
            pAB[componentAB + numComponentAB]     = (a0 * b1 + a1 * b0               ) * div5;
            pAB[componentAB + 2 * numComponentAB] = (a0 * b2 + a1 * b1 + a2 * b0     ) * div10;
            pAB[componentAB + 3 * numComponentAB] = (          a1 * b2 + a2 * b1 + a3 * b0) * div10;
            pAB[componentAB + 4 * numComponentAB] = (                    a2 * b2 + a3 * b1) * div5;
            pAB[componentAB + 5 * numComponentAB] =                                a3 * b2;
            return true;
            }
        else if (orderB == 4)
            {
            double b0 = pB[componentB];
            double b1 = 3.0 * pB[componentB + numComponentB];
            double b2 = 3.0 * pB[componentB + 2 * numComponentB];
            double b3 = pB[componentB + 3 * numComponentB];
            pAB[componentAB]                     =  a0 * b0;
            pAB[componentAB + numComponentAB]     = (a0 * b1 + a1 * b0               ) * div6;
            pAB[componentAB + 2 * numComponentAB] = (a0 * b2 + a1 * b1 + a2 * b0     ) * div15;
            pAB[componentAB + 3 * numComponentAB] = (a0 * b3 + a1 * b2 + a2 * b1 + a3 * b0) * div20;
            pAB[componentAB + 4 * numComponentAB] = (          a1 * b3 + a2 * b2 + a3 * b1) * div15;
            pAB[componentAB + 5 * numComponentAB] = (                    a2 * b3 + a3 * b2) * div6;
            pAB[componentAB + 6 * numComponentAB] =                                a3 * b3;
            return true;
            }
        }
    return false;
    }

#ifdef TIMING_DATA
OPTIMIZED CODE "full product"
Univariate products (count 1000000) (orderA 1)(orderB 1) (time 33.000)
Univariate products (count 1000000) (orderA 1)(orderB 2) (time 34.000)
Univariate products (count 1000000) (orderA 1)(orderB 3) (time 39.000)
Univariate products (count 1000000) (orderA 1)(orderB 4) (time 60.000)
Univariate products (count 1000000) (orderA 2)(orderB 1) (time 40.000)
Univariate products (count 1000000) (orderA 2)(orderB 2) (time 44.000)
Univariate products (count 1000000) (orderA 2)(orderB 3) (time 63.000)
Univariate products (count 1000000) (orderA 2)(orderB 4) (time 77.000)
Univariate products (count 1000000) (orderA 3)(orderB 1) (time 51.000)
Univariate products (count 1000000) (orderA 3)(orderB 2) (time 69.000)
Univariate products (count 1000000) (orderA 3)(orderB 3) (time 72.000)
Univariate products (count 1000000) (orderA 3)(orderB 4) (time 121.000)
Univariate products (count 1000000) (orderA 4)(orderB 1) (time 70.000)
Univariate products (count 1000000) (orderA 4)(orderB 2) (time 74.000)
Univariate products (count 1000000) (orderA 4)(orderB 3) (time 102.000)
Univariate products (count 1000000) (orderA 4)(orderB 4) (time 141.000)

OPTIMIZED CODE "fast products"
Univariate products (count 1000000) (orderA 1)(orderB 1) (time 19.000)
Univariate products (count 1000000) (orderA 1)(orderB 2) (time 22.000)
Univariate products (count 1000000) (orderA 1)(orderB 3) (time 23.000)
Univariate products (count 1000000) (orderA 1)(orderB 4) (time 25.000)
Univariate products (count 1000000) (orderA 2)(orderB 1) (time 23.000)
Univariate products (count 1000000) (orderA 2)(orderB 2) (time 29.000)
Univariate products (count 1000000) (orderA 2)(orderB 3) (time 31.000)
Univariate products (count 1000000) (orderA 2)(orderB 4) (time 33.000)
Univariate products (count 1000000) (orderA 3)(orderB 1) (time 26.000)
Univariate products (count 1000000) (orderA 3)(orderB 2) (time 28.000)
Univariate products (count 1000000) (orderA 3)(orderB 3) (time 31.000)
Univariate products (count 1000000) (orderA 3)(orderB 4) (time 35.000)
Univariate products (count 1000000) (orderA 4)(orderB 1) (time 26.000)
Univariate products (count 1000000) (orderA 4)(orderB 2) (time 31.000)
Univariate products (count 1000000) (orderA 4)(orderB 3) (time 34.000)
Univariate products (count 1000000) (orderA 4)(orderB 4) (time 39.000)

DEBUG CODE "full products"
Univariate products (count 1000000) (orderA 1)(orderB 1) (time 53.000)
Univariate products (count 1000000) (orderA 1)(orderB 2) (time 64.000)
Univariate products (count 1000000) (orderA 1)(orderB 3) (time 85.000)
Univariate products (count 1000000) (orderA 1)(orderB 4) (time 114.000)
Univariate products (count 1000000) (orderA 2)(orderB 1) (time 81.000)
Univariate products (count 1000000) (orderA 2)(orderB 2) (time 96.000)
Univariate products (count 1000000) (orderA 2)(orderB 3) (time 136.000)
Univariate products (count 1000000) (orderA 2)(orderB 4) (time 207.000)
Univariate products (count 1000000) (orderA 3)(orderB 1) (time 113.000)
Univariate products (count 1000000) (orderA 3)(orderB 2) (time 139.000)
Univariate products (count 1000000) (orderA 3)(orderB 3) (time 221.000)
Univariate products (count 1000000) (orderA 3)(orderB 4) (time 231.000)
Univariate products (count 1000000) (orderA 4)(orderB 1) (time 152.000)
Univariate products (count 1000000) (orderA 4)(orderB 2) (time 210.000)
Univariate products (count 1000000) (orderA 4)(orderB 3) (time 227.000)
Univariate products (count 1000000) (orderA 4)(orderB 4) (time 290.000)

DEBUG "fast"
Univariate products (count 1000000) (orderA 1)(orderB 1) (time 21.000)
Univariate products (count 1000000) (orderA 1)(orderB 2) (time 27.000)
Univariate products (count 1000000) (orderA 1)(orderB 3) (time 33.000)
Univariate products (count 1000000) (orderA 1)(orderB 4) (time 40.000)
Univariate products (count 1000000) (orderA 2)(orderB 1) (time 25.000)
Univariate products (count 1000000) (orderA 2)(orderB 2) (time 34.000)
Univariate products (count 1000000) (orderA 2)(orderB 3) (time 36.000)
Univariate products (count 1000000) (orderA 2)(orderB 4) (time 42.000)
Univariate products (count 1000000) (orderA 3)(orderB 1) (time 30.000)
Univariate products (count 1000000) (orderA 3)(orderB 2) (time 40.000)
Univariate products (count 1000000) (orderA 3)(orderB 3) (time 49.000)
Univariate products (count 1000000) (orderA 3)(orderB 4) (time 52.000)
Univariate products (count 1000000) (orderA 4)(orderB 1) (time 41.000)
Univariate products (count 1000000) (orderA 4)(orderB 2) (time 42.000)
Univariate products (count 1000000) (orderA 4)(orderB 3) (time 55.000)
Univariate products (count 1000000) (orderA 4)(orderB 4) (time 54.000)
#endif


/*---------------------------------------------------------------------------------**//**
* Compute the (1-D) control points of the product of two univariate Bezier curves.
* Only component componentAB is computed in the numComponentAB-dimensional output array.
* Returns true if product curve's order is within allowable range.
*
* @param    pAB             <= (orderA - 1) + (orderB - 1) + 1 control points of product.
* @param    componentAB     => component to compute in product
* @param    numComponentAB  => number of components in product
* @param    pA              => control points for bezier B
* @param    orderA          => number of control points for bezier B
* @param    componentA      => component index within bezier A
* @param    numComponentA   => number of components in bezier A
* @param    pB              => control points for bezier B
* @param    orderB          => number of control points for bezier B
* @param    componentB      => component index within bezier B
* @param    numComponentB   => number of components in bezier B
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateProduct
(
double      *pAB,
int         componentAB,
int         numComponentAB,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
)
    {
    static bool sTryFast = true;
    if (sTryFast && bsiBezier_fastUnivariateProduct (
            pAB, componentAB, numComponentAB,
            pA, orderA, componentA, numComponentA,
            pB, orderB, componentB, numComponentB))
        return true;

    int k;
    int i, j;
    int degreeA = orderA - 1;
    int degreeB = orderB - 1;
    int degreeAB = degreeA + degreeB;
    int orderAB  = degreeAB + 1;

    double *pCA  = jmdlBezier_getPascalRow (degreeA);
    double *pCB  = jmdlBezier_getPascalRow (degreeB);
    double *pCAB = jmdlBezier_getPascalRow (degreeAB);

    if (orderAB > MAX_BEZIER_ORDER)
        return false;

    static int sNum1 = 0;
    static int sNum2 = 0;
    static int sNum3 = 0;
    static int sNumX = 0;
    if (degreeAB == 1)
        sNum1++;
    else if (degreeAB == 2)
        sNum2++;
    else if (degreeAB == 3)
        sNum3++;
    else
        sNumX++;
    
    /* The product of a Bezier curve of degree d with poles a_i
       and a Bezier curve of degree e with poles b_j is a Bezier
       curve of degree d+e with poles c_k given by the below sum
       over all nonnegative integers i and j which add to k:

                            a_i b_j dCi eCj
            c_k =   sum     --------------- ,
                    i+j=k       (d+e)Ck

       where xCy = x!/(y!(x-y)!) are the binomial coefficients
       (found in the row of Pascal's triangle with index x).

       Alternately, the combination of coefficients i and j from the
        two factors produces a value
                    a_i b_j dCi eCj
        which is of degree i+j, hence is to be added to term i+j of the product
            coefficients.
        After all of these have been accumulated, divide each product term
            by (e+d)C(i+j)
    */

    for (k = 0; k < orderAB; k++)
        pAB[k * numComponentAB + componentAB] = 0.0;

    for (i = 0; i < orderA; i++)
        {
        for (j = 0; j < orderB; j++)
            {
            k = i + j;
            pAB[k * numComponentAB + componentAB]
                += pCA[i]* pCB[j]
                         * pA[i * numComponentA + componentA]
                         * pB[j * numComponentB + componentB];
            }
        }

    for (k = 0; k < orderAB; k++)
        pAB[k * numComponentAB + componentAB] /= pCAB[k];

    return  true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the (1-D) control points of the product of two univariate Bezier curves.
* Only component componentAB is computed in the numComponentAB-dimensional output array.
* Returns true if product curve's order is within allowable range.
*
* @param    pAB             <= (orderA - 1) + (orderB - 1) + 1 control points of product.
* @param    *pOrderAB       <= order of product.
* @param    maxOrderAB      => allowed order of product.
* @param    componentAB     => component to compute in product
* @param    numComponentAB  => number of components in product
* @param    pA              => control points for bezier B
* @param    orderA          => number of control points for bezier B
* @param    componentA      => component index within bezier A
* @param    numComponentA   => number of components in bezier A
* @param    pB              => control points for bezier B
* @param    orderB          => number of control points for bezier B
* @param    componentB      => component index within bezier B
* @param    numComponentB   => number of components in bezier B
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_univariateProductExt
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
int         componentAB,
int         numComponentAB,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
)
    {
    int orderAB = orderA + orderB - 1;
    if  (  orderAB <= maxOrderAB
        && bsiBezier_univariateProduct
                    (
                    pAB, componentAB, numComponentAB,
                    pA, orderA, componentA, numComponentA,
                    pB, orderB, componentB, numComponentB
                    )
        )
        {
        *pOrderAB = orderAB;
        return true;
        }
    else
        {
        *pOrderAB = 0;
        return false;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute the (1-D) control points of the product of two univariate Bezier curves.
* Accumulate the product to another polynomial of the same order.
* Only component componentAB is computed in the numComponentAB-dimensional output array.
* Returns true if product curve's order is within allowable range.
*
* @param    pAB             <=> (orderA - 1) + (orderB - 1) + 1 control points of accumulating
*                                   product.
* @param    componentAB     => component to compute in product
* @param    numComponentAB  => number of components in product
* @param    coff            => scalar multiplier to apply as to accumulated terms.
* @param    pA              => control points for bezier B
* @param    orderA          => number of control points for bezier B
* @param    componentA      => component index within bezier A
* @param    numComponentA   => number of components in bezier A
* @param    pB              => control points for bezier B
* @param    orderB          => number of control points for bezier B
* @param    componentB      => component index within bezier B
* @param    numComponentB   => number of components in bezier B
* @bsimethod                                                    EarlinLutz      08/98
* @bsimethod                                                    DavidAssaf      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiBezier_accumulateUnivariateProduct
(
double      *pAB,
int         componentAB,
int         numComponentAB,
double      coff,
double      *pA,
int         orderA,
int         componentA,
int         numComponentA,
double      *pB,
int         orderB,
int         componentB,
int         numComponentB
)
    {
    double productPoles[MAX_BEZIER_ORDER];
    int productOrder = orderA + orderB - 1;
    if (    productOrder > MAX_BEZIER_ORDER
        ||  !bsiBezier_univariateProduct (productPoles, 0, 1,
                    pA, orderA, componentA, numComponentA,
                    pB, orderB, componentB, numComponentB)
        )
        {
        return false;
        }
    else
        {
        int i;
        double *pDest = pAB + componentAB;
        for (i = 0; i < productOrder; i++, pDest += numComponentAB)
            {
            *pDest += coff * productPoles[i];
            }
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute the control points of the product of a single bezier basis function B_i^n
* multiplying a multicomponent (curve) bezier (i.e. poles of a curve).
*
* @param    pAB             <= (orderA - 1) + (orderB - 1) + 1 control points of product.
* @param    pOrderAB        <= order of product
* @param    maxOrderAB      => max poles allowed in the product
* @param    pA              => control points for bezier A
* @param    orderA          => order of polynomials in A
* @param    numComponent    => number of components in A and AB.
* @param    coffB           => numeric coefficient for the basisi function.
* @param    indexB          => power of u in B_i^n
* @param    orderB          => order of B
* @bsimethod                                                    EarlinLutz      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezier_basisProduct
(
double      *pAB,
int         *pOrderAB,
int         maxOrderAB,
const double *pA,
int         orderA,
int         numComponent,
const double coffB,
int         indexB,
int         orderB
)
    {
    int degreeA = orderA - 1;
    int degreeB = orderB - 1;
    int degreeAB = degreeA + degreeB;
    int orderAB = degreeAB + 1;
    double scale[MAX_BEZIER_ORDER];
    double *pCA  = jmdlBezier_getPascalRow (degreeA);
    double *pCB  = jmdlBezier_getPascalRow (degreeB);
    double *pCAB = jmdlBezier_getPascalRow (degreeAB);
    int lastABIndex = orderAB - 1;
    const double *pAi;
    double *pABi;
    int i, j;

    if (!pCA || !pCB || !pCAB || orderAB > maxOrderAB || indexB < 0 || indexB >= orderB
        || orderA >= MAX_BEZIER_ORDER)
        {
        if (pOrderAB)
            *pOrderAB = 0;
        return false;
        }

    if (pOrderAB)
        *pOrderAB = orderAB;
    /* Precomponet per-component scale factors. */
    for (i = 0; i < orderA; i++)
        {
        scale[i] = coffB * pCA[i] * pCB[indexB] / pCAB[lastABIndex - i];
        }

    /* Apply scales with component loop outside.  Do this right to left to allow
        in-place products. */
    if (numComponent == 1)
        {
        for (i = orderA - 1; i >= 0; i--)
            pAB[lastABIndex - i] = pA[i] * scale[i];
        }
    else
        {
        for (j = 0; j < numComponent; j++)
            {
            pAi = pA + j + i * numComponent;
            pABi = pAB + j + (lastABIndex - i) * numComponent;
            for (i = orderA - 1; i >= 0;
                i--, pAi -= numComponent, pABi -= numComponent)
                {
                *pABi = (*pA) * scale[i];
                }
            }
        }
    /* Fill zeros at left */
    memset (pAB, 0, sizeof (double) * numComponent * degreeB);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the poles of the (Bezier curve) difference of two Bezier curves of
* shared order.
*
* @param    pDiff           <= control points for bezier A - bezier B
* @param    pA              => control points for bezier A
* @param    pB              => control points for bezier B
* @param    order           => number of control points in A, B, AB
* @param    numComponent    => number of components per control point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_subtractPoles
(
double      *pDiff,
double      *pA,
double      *pB,
int         order,
int         numComponent
)
    {
    int i;
    int n = order * numComponent;

    for (i = 0; i < n; i++)
        pDiff[i] = pA[i] - pB[i];
    }


/*---------------------------------------------------------------------------------**//**
* Zero out poles of a bezier.
* @param order => number of poles.
* @param numComponent => number of components per pole.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_zeroPoles
(
double      *pA,
int         order,
int         numComponent
)
    {
    memset (pA, 0, order * numComponent * sizeof (double));
    }



/*---------------------------------------------------------------------------------**//**
* Add the poles of polynomial pA to those of pSum, replacing pSum.
*
* @param    pSum            <=> control points for sum.
* @param    pA              => control points for bezier A
* @param    order           => number of control points in A, B, AB
* @param    numComponent    => number of components per control point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_addPolesInPlace
(
double      *pSum,
double      *pA,
int         order,
int         numComponent
)
    {
    int i;
    int n = order * numComponent;

    for (i = 0; i < n; i++)
        pSum[i] += pA[i];
    }

Public GEOMDLLIMPEXP void       bsiBezier_addPoles
(
double      *pSum,
double      *pA,
double      scaleA,
double      *pB,
double      scaleB,
int         order,
int         numComponent
)
    {
    int i;
    int n = order * numComponent;

    for (i = 0; i < n; i++)
        pSum[i] = pA[i] * scaleA + pB[i] * scaleB;
    }


/*---------------------------------------------------------------------------------**//**
* Add the poles of polynomial pA to those of pB, replacing pSum.
*
* @param    pSum            <=> control points for sum.
* @param    pA              => control points for bezier A
* @param    pA              => control points for bezier B
* @param    order           => number of control points in A, B, sum
* @param    numComponent    => number of components per control point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_addPoles
(
double      *pSum,
const double *pA,
const double *pB,
int         order,
int         numComponent
)
    {
    int i;
    int n = order * numComponent;

    for (i = 0; i < n; i++)
        pSum[i] = pA[i] + pB[i];
    }



/*---------------------------------------------------------------------------------**//**
* Subtract the poles of polynomial pA from those of pSum, replacing pSum.
*
* @param    pSum            <=> control points for sum.
* @param    pA              => control points for bezier A
* @param    order           => number of control points in each bezier
* @param    numComponent    => number of components per control point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_subtractPolesInPlace
(
double      *pSum,
double      *pA,
int         order,
int         numComponent
)
    {
    int i;
    int n = order * numComponent;

    for (i = 0; i < n; i++)
        pSum[i] -= pA[i];
    }


/*---------------------------------------------------------------------------------**//**
* Add a constant vector to all poles of a Bezier curve.
*
* @param    pDiff           <= control points for bezier + delta
* @param    pA              => control points for bezier
* @param    pDelta          => constant delta to apply (array must have numComponent entries)
* @param    order           => number of control points
* @param    numComponent    => number of components per control point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void       bsiBezier_addConstant
(
double      *pDiff,
double      *pA,
double      *pDelta,
int         order,
int         numComponent
)
    {
    int i;
    int component;
    double a;
    double *pCurr;

    for (component = 0; component < numComponent; component++)
        {
        a = pDelta[component];

        for (i = 0, pCurr = pA + component; i < order; i++, pCurr += numComponent)
            (*pCurr) += a;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Return the product of two entries in a pascal's triangle row.
* @param i  => first index in row, 0<=i<=n
* @param j  => second index in row, 0<=j<=n
* @param n  => row index.  Each row has n+1 entries.
* @return iCn * jCn
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiBezier_pascalProductInRow
(
int         i,
int         j,
int         n
)
    {
    double *pC  = jmdlBezier_getPascalRow (n);
    return pC[i] * pC[j];
    }



/*---------------------------------------------------------------------------------**//**
* Fill a dense, row-major matrix with values
*       A[i][j] = iCm * jCn / ( (i+j)C(m+n) * (m + n + 1)
*   for 0 < i < (m+1) and 0 < j < (n+1)
*
* @param pA => matrix pointer.   Filled with (m+1)*(n+1) values.
* @param maxA => number of doubles the A can hold.
* @param rowOrder => number of rows of A.
* @param colOrder => number of columns of A.
* @return true if all dimensions are non-negative and dimensional
*       limits are satisfied.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezier_pascalRegressionMatrix
(
double      *pA,
int         maxA,
int         rowOrder,
int         colOrder
)
    {
#define AA(ii,jj) pA[(jj) + rowOrder * (ii)]
    int i, j;
    double scale;
    int m = rowOrder - 1;
    int n = colOrder - 1;

    double *pCm = jmdlBezier_getPascalRow (m);
    double *pCn = jmdlBezier_getPascalRow (n);
    double *pCmn = jmdlBezier_getPascalRow (m + n);
    bool boolstat = false;

    if (   rowOrder
        && colOrder > 0
        && pCm
        && pCn
        && pCmn
        && rowOrder * colOrder <= maxA)
        {
        boolstat = true;
        scale = 1.0 / (m + n + 1);
        for (i = 0; i <= m; i++)
            {
            for (j = 0; j <= n; j++)
                {
                AA(i,j) = scale * pCm[i] * pCn[j] / pCmn[i + j];
                }
            }
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Compute control points for univariate interpolation.
* @param pA <= array of poles.
* @param pY => array of function values.
* @return true if poles computed.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiBezier_univariateInterpolationPoles
(
double          *pA,
const double    *pY,
int             n
)
    {
    int i;
    double y1, y2, z1, z2;
    double aa[MAX_BEZIER_ORDER], bb[MAX_BEZIER_ORDER];
    double aaa[MAX_BEZIER_ORDER], bbb[MAX_BEZIER_ORDER], work[MAX_BEZIER_ORDER];
    bool stat = false;

    if (n < 3)
        {
        for (i = 0; i < n; i++)
            pA[i] = pY[i];
        stat = true;
        }
    else if (n == 3)
        {
        pA[0] = pY[0];
        pA[2] = pY[2];
        y1 = 0.5 * (pY[0] + pY[2]);
        z1 = pY[1] - y1;
        pA[1] = y1 + 2.0 * z1;
        stat = true;
        }
    else if (n == 4)
        {
        double a = 1.0 / 3.0;
        pA[0] = pY[0];
        pA[3] = pY[3];
        y1 = (2.0 * pY[0] + pY[3]) * a;
        y2 = (pY[0] + 2.0 * pY[3]) * a;
        z1 = pY[1] - y1;
        z2 = pY[2] - y2;
        pA[1] = y1 + 1.5 * (2.0 * z1 - z2);
        pA[2] = y2 + 1.5 * (2.0 * z2 - z1);
        stat = true;
        }
    else if (n <= MAX_BEZIER_ORDER)
        {
        /* Recurse to lower degree, form convex combination */
        int m = n - 1;
        double *pCm = jmdlBezier_getPascalRow (m-1);
        double *pCn = jmdlBezier_getPascalRow (n-1);
        /* Form the lower order interpolant to the prefix and suffixes ... */
        if (   pCm
            && pCn
            && bsiBezier_univariateInterpolationPoles (aa, pY, m)
            && bsiBezier_univariateInterpolationPoles (bb + 1, pY + 1, m)
           )
            {
            /* Use deCastlejou to squeeze the intervals by factor n/(n+1)
                towards respective ends: */
            double aPriorIntervals = n - 2;
            double fb = 1.0 / aPriorIntervals;
            double fa = m * fb;

            bsiBezier_subdivisionPolygons (aaa, work, aa, m, 1, fa);
            bsiBezier_subdivisionPolygons (work, bbb + 1, bb + 1, m, 1, -fb);

            /* Put the Y values on a grid of n points.
                aaa interpolates m=n-1 of them, from 0 to m-1.
                bbb interpolates m=n-1 of them from 1 to m.
                Hence (1-u)*aaa(u) + u*bbb(u) interpolates all of them
                The order-n Beziers (1-u)*aaa(u) and u*bbb(u) are available
                    by revising the coefficients of aaa and bbb: */
            aa[m] = bb[0] = 0.0;
            for (i = 1; i < m; i++)
                {
                aa[i] = aaa[i] * pCm[i] / pCn[i];
                bb[i] = bbb[i] * pCm[i-1] / pCn[i];
                }
            /* And the sum of the order-n product Beziers is the output: */
            for (i = 0; i < n;i++)
                {
                pA[i] = aa[i] + bb[i];
                }
            stat = true;
            }
        }
    return stat;
    }

// Each table gives the weight applied to f(0), f'(0), f''(0) etc in hermite integration.
// f(1), f'(1) etc use same weights, negated for odd derivatives.
static double s_hermiteCoffs0[1] =
    {
    0.5
    };

static double s_hermiteCoffs1[2] =
    {
    0.5,
    0.0833333333333333
    };

static double s_hermiteCoffs2[3] =
    {
    0.5,
    0.1,
    0.00833333333333333
    };

static double s_hermiteCoffs3[4] =
    {
    0.5,
    0.107142857142857,
    0.0119047619047619,
    0.000595238095238095
    };

static double s_hermiteCoffs4[5] =
    {
    0.5,
    0.111111111111111,
    0.0138888888888889,
    0.000992063492063492,
    3.30687830687831E-05
    };

static double s_hermiteCoffs5[6] =
    {
    0.5,
    0.113636363636364,
    0.0151515151515152,
    0.00126262626262626,
    6.31313131313131E-05,
    1.5031265031265E-06
    };

double s_hermiteCoffs6[7]
    =
    {
    0.5,
    0.115384615384615,
    0.016025641025641,
    0.00145687645687646,
    8.74125874125874E-05,
    3.23750323750324E-06,
    5.78125578125578E-08
    };
// Ragged array of hermite integration coefficients.
static double * s_hermiteIntegralTable[7] =
    {
    s_hermiteCoffs0,
    s_hermiteCoffs1,
    s_hermiteCoffs2,
    s_hermiteCoffs3,
    s_hermiteCoffs4,
    s_hermiteCoffs5,
    s_hermiteCoffs6
    };

/*---------------------------------------------------------------------------------**//**
Integrate a hermite polynomial.
@param h IN interval size.  The bezier parameterizes the interval from 0 to 1.
@param fa IN array of function, first derivative, etc, at start of interval.
@param fb IN array of function, first derivative, etc, at end of interval.
@param numDerivative IN number of derivatives.  If larger than 6, only first 6 are used.  0 is valid.
@return true if poles computed.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiQuadrature_hermiteIntegral
(
double h,
double fa[],
double fb[],
int numDerivative
)
    {
    if (h == 0.0)
        return 0.0;

    if (numDerivative < 0)
        numDerivative = 0;
    if (numDerivative == 0)
        return 0.5 * h * (fa[0] + fb[0]);

    if (numDerivative > 6)
        numDerivative = 6;

    double *pCoffs = s_hermiteIntegralTable[numDerivative];

    if (numDerivative == 1)
        return h * (0.5 * (fa[0] + fb[0])
                   +  h * pCoffs[1] * ( fa[1] - fb[1]));

    if (numDerivative == 2)
        return h * (0.5 * (fa[0] + fb[0])
                   +  h * (pCoffs[1] * ( fa[1] - fb[1])
                           + h * pCoffs[2] * (fa[2] + fb[2])));

    double sum = 0.0;
    double hk = 1.0;
    for (int k = 0; k <= numDerivative; k++)
        {
        hk *= h;
        if (k & 0x01)
            sum += hk * (fa[k] - fb[k]) * pCoffs[k];
        else
            sum += hk * (fa[k] + fb[k]) * pCoffs[k];
        }
    return sum;
    }

// fill bezier coefficient for a univariate hermite curve fit.
static int bsiBezier_univariateHermiteFitPoles_fromPascalTriangle
(
double *pBezCoffs,
double h,
double fa[],
double fb[],
int numDerivative
)
    {
    //   1  0  0  0  0
    //   1  1  0  0  0
    //   1  2  1  0  0
    //   1  3  3  1  0
    //   1  4  6  4  1
    // To Do: make a bezier that has derivative f[k](0) = 1, other derivatives 0..K zero at (0), and all 0 at (1).
    // Bezcoffs from 0 forward are order (2(K+1)), coffs 0..K-1 in column k, divide by (order)(order-1)(order-K)
    // Bezcoffs from 1 backwards are 0.
    // All scale by h^k * fa[k].
    // At (1), flip direction and multiply by -1^k
    // Add all such beziers together, scaled by

    int order = 2 * (numDerivative + 1);
    double ah = 1.0;
    double bh = 1.0;
    double fdiv = 1.0;
    for (int i = 0; i < order; i++)
        pBezCoffs[i] = 0.0;

    for (int k = 0; k <= numDerivative; k++)
        {
        double a0 = fa[k] * ah * fdiv;
        double a1 = fb[k] * bh * fdiv;
        for (int row = k; row <= numDerivative; row++)
            {
            double c = bsiBezier_getBinomialCoefficient (k, row);
            pBezCoffs[row]          += c * a0;
            pBezCoffs[order-1-row]    += c * a1;
            }
        fdiv /= (order - 1 - k);
        ah *= h;
        bh *= -h;
        }
    return 2 * (numDerivative + 1);
    }


/*---------------------------------------------------------------------------------**//**
Compute control points for a hermite curve fit.
@param pBezCoffs OUT computed poles
@param h IN interval size.  The bezier parameterizes the interval from 0 to 1.
@param fa IN array of function, first derivative, etc, at start of interval.
@param fb IN array of function, first derivative, etc, at end of interval.
@param numDerivative IN number of derivatives.
@return true if poles computed.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiBezier_univariateHermiteFitPoles
(
double *pBezCoffs,
double h,
double fa[],
double fb[],
int numDerivative
)
    {
#ifdef UNROLL_HERMITE_FIT
    if (numDerivative <= 0)
        {
        pBezCoffs[0] = fa[0];
        pBezCoffs[1] = fb[0];
        return 2;
        }
    if (numDerivative == 1)
        {
        double c1 = 1.0 / 3.0;
        pBezCoffs[0] = fa[0];
        pBezCoffs[1] = fa[0] + h * c1 * fa[1];
        pBezCoffs[2] = fb[0] - h * c1 * fb[1];
        pBezCoffs[3] = fb[0];
        return 4;
        }

    if (numDerivative == 2)
        {
        double c1 = 1.0 / 5.0;
        double c2 = 2.0 / 5.0;

        double d2 = 1.0 / 20.0;
        pBezCoffs[0] = fa[0];
        pBezCoffs[1] = fa[0] + h * c1 * fa[1];
        pBezCoffs[2] = fa[0] + h * (c2 * fa[1] + h * d2 * fa[2]);
        pBezCoffs[3] = fb[0] - h * (c2 * fb[1] - h * d2 * fb[2]);
        pBezCoffs[4] = fb[0] - h * c1 * fb[1];
        pBezCoffs[5] = fb[0];
        return 6;
        }

    if (numDerivative == 3)
        {
        double c1 = 1.0 / 7.0;
        double c2 = 2.0 / 7.0;
        double c3 = 3.0 / 7.0;

        double d2 = 1.0 / 42.0;
        double d3 = 3.0 / 42.0;

        double e3 = 1.0 / 210.0;

        pBezCoffs[0] = fa[0];
        pBezCoffs[1] = fa[0] + h *  c1 * fa[1];
        pBezCoffs[2] = fa[0] + h * (c2 * fa[1] + h *  d2 * fa[2]);
        pBezCoffs[3] = fa[0] + h * (c3 * fa[1] + h * (d3 * fa[2] + h * e3 * fa[3]));
        pBezCoffs[4] = fb[0] - h * (c3 * fb[1] - h * (d3 * fb[2] - h * e3 * fb[3]));
        pBezCoffs[5] = fb[0] - h * (c2 * fb[1] - h *  d2 * fb[2]);
        pBezCoffs[6] = fb[0] - h *  c1 * fb[1];
        pBezCoffs[7] = fb[0];
        return 8;
        }

    if (numDerivative == 4)
        {
        double c1 = 1.0 / 9.0;
        double c2 = 2.0 / 9.0;
        double c3 = 3.0 / 9.0;
        double c4 = 4.0 / 9.0;

        double d2 = 1.0 / 72.0;
        double d3 = 3.0 / 72.0;
        double d4 = 6.0 / 72.0;

        double e3 = 1.0 / 504.0;
        double e4 = 4.0 / 504.0;

        double f4 = 1.0 / 3024.0;

        pBezCoffs[0] = fa[0];
        pBezCoffs[1] = fa[0] + h *  c1 * fa[1];
        pBezCoffs[2] = fa[0] + h * (c2 * fa[1] + h *  d2 * fa[2]);
        pBezCoffs[3] = fa[0] + h * (c3 * fa[1] + h * (d3 * fa[2] + h *  e3 * fa[3]));
        pBezCoffs[4] = fa[0] + h * (c4 * fa[1] + h * (d4 * fa[2] + h * (e4 * fa[3] + h * f4 * fa[4])));

        pBezCoffs[5] = fb[0] - h * (c4 * fb[1] - h * (d4 * fb[2] - h * (e4 * fb[3] - h * f4 * fb[4])));
        pBezCoffs[6] = fb[0] - h * (c3 * fb[1] - h * (d3 * fb[2] - h *  e3 * fb[3]));
        pBezCoffs[7] = fb[0] - h * (c2 * fb[1] - h *  d2 * fb[2]);
        pBezCoffs[8] = fb[0] - h *  c1 * fb[1];
        pBezCoffs[9] = fb[0];
        return 10;
        }
#endif
    return bsiBezier_univariateHermiteFitPoles_fromPascalTriangle (pBezCoffs, h, fa, fb, numDerivative);
    }


/*---------------------------------------------------------------------------------**//**
 Return the "direct coefficient" product of a linear polynomial and a bezier.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void directMultiplyLinear
(
double *pOut,
double *pLine,
double *pIn,
int    inOrder
)
    {
    double a = pLine[0];
    for (int i = 0; i < inOrder; i++)
       pOut[i] = a * pIn[i];
    pOut[inOrder] = 0.0;

    a = pLine[1];
    for (int i = 0; i < inOrder; i++)
        pOut[i+1] += a * pIn[i];
    }

/*---------------------------------------------------------------------------------**//**
 Return the "direct coefficient" product of a linear polynomial and a bezier.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void directCombinationLinear
(
double *pOut,
double *pLineA,
double *pInA,
double *pLineB,
double *pInB,
int    inOrder
)
    {
    double a = pLineA[0];
    double b = pLineB[0];
    for (int i = 0; i < inOrder; i++)
       pOut[i] = a * pInA[i] + b * pInB[i];
    pOut[inOrder] = 0.0;

    a = pLineA[1];
    b = pLineB[1];
    for (int i = 0; i < inOrder; i++)
        pOut[i+1] += a * pInA[i] + b * pInB[i];
    }

static void raiseBasisSetOrder
(
double *pNew,
double *pOld,
int     oldOrder,
double *pCompleteKnots,
int     knotIndex0
)
    {
    int newOrder = oldOrder + 1;
    double lineA[2], lineB[2];
    for (int i = 0; i < newOrder; i++)
        {
        // The weighting line pair covers x0..x3, with x1..x2 as the live interval
        int    ix0 = knotIndex0 - oldOrder + i;
        // x coordinates of "left" side of active parts of weight functions ...
        double x0A = pCompleteKnots[ix0];
        double x0B = pCompleteKnots[ix0+1];
        double x1 = pCompleteKnots[knotIndex0];
        double x2 = pCompleteKnots[knotIndex0 + 1];
        double x3A = pCompleteKnots[ix0 + oldOrder];
        double x3B = pCompleteKnots[ix0 + oldOrder + 1];
        double hA = x3A - x0A;
        double hB = x3B - x0B;

        if (hA != 0.0)
            {
            lineA[0] = (x1 - x0A) / hA;
            lineA[1] = (x2 - x0A) / hA;
            }
        else
            lineA[0] = lineA[1] = 0.0;

        if (hB != 0.0)
            {
            lineB[0] = (x3B - x1) / hB;
            lineB[1] = (x3B - x2) / hB;
            }
        else
            lineB[0] = lineB[1] = 0.0;

        if (i == 0)
            {
            directMultiplyLinear (pNew, lineB, pOld, oldOrder);
            }
        else if (i < oldOrder)
            {
            directCombinationLinear (pNew + i * newOrder,
                    lineA, pOld + (i-1) * oldOrder,
                    lineB, pOld + i * oldOrder,
                    oldOrder);
            }
        else
            {
            directMultiplyLinear (pNew + i * newOrder,
                    lineA, pOld + (i - 1) * oldOrder, oldOrder);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
 Return the bezier polynomials of the nurbs basis over a single interval of a nurbs.
@param pBasisCoffs OUT array of order * order doubles.   Each block of order doubles
        are the bezier coefficient for a basis function over the interval.
    The bezier independent variable is a local 0..1 space within
        pCompleteKnots[knotIndex0] < t < pCompleteKnots[knotIndex0 + 1]
 The bezier curve over the interval is a linear combination of the control points times
    these beziers,
        P(u} = P[K]*bezier[0] + P[K+1] * bezier[1] + .....
    where knot0 = K + order - 1,   i.e.    K = knot0 - order + 1
@param
@return true if basis completed.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool bsiNurbs_singleIntervalBasisFunctions
(
double *pBasisPolys,
double *pCompleteKnots,
int knotIndex0,
int order
)
    {
    double coffs[2][MAX_BEZIER_ORDER*MAX_BEZIER_ORDER];
    static int sBaseOrder = 1;
    if (order < 1)
        return false;
    if (order == 1)
        {
        pBasisPolys[0] = 1.0;
        return true;
        }
    if (order == 2 && sBaseOrder > 1)
        {
        pBasisPolys[0] = 1.0;
        pBasisPolys[1] = 0.0;
        pBasisPolys[2] = 0.0;
        pBasisPolys[3] = 1.0;
        return true;
        }

    if (order > MAX_BEZIER_ORDER)
        return false;
    int baseOrder;
    if (sBaseOrder == 1)
        {
        coffs[0][0] = 1.0;
        baseOrder = 1;
        }
    else
        {
        // Start recursion with order 2 in working coffs "0"
        coffs[0][0] = 1.0;
        coffs[0][1] = 0.0;
        coffs[0][2] = 0.0;
        coffs[0][3] = 1.0;
        baseOrder = 2;
        }
    int currSelect = 0;
    // coffs[currSelect] is a set of k-1 polynomials of order k-1
    // coffs[nextSelect] is a set of k polynomials of order k.
    for (int k = baseOrder + 1; k <= order; k++)
        {
        int nextSelect = 1 - currSelect;
        double *pA = coffs[currSelect];
        double *pB = coffs[nextSelect];
        raiseBasisSetOrder (pB, pA, k - 1, pCompleteKnots, knotIndex0);
        currSelect = nextSelect;
        }

    int basisPoly, k;
    double *pA = &coffs[currSelect][0];
    for (int i = 0; i < order; i++)
        {
        double pascalCoff = bsiBezier_getBinomialCoefficient (i, order - 1);
        double q = 1.0 / pascalCoff;

        for (basisPoly = 0, k = i; basisPoly < order; basisPoly++, k += order)
            {
            pBasisPolys[k] = q * pA[k];
            }
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
 Return the bezier polynomial for a single span of a nurbs curve.
@param pBezierControlPoints OUT numerator control points for bezier curve.
@param pBezierWeights OUT denominator control points
@param pNURBSControlPoints OUT numerator control points for NURBS curve
@param pNURBSWeights OUT denominator control points for NURBS curve.  May be NULL
@param pCompleteKnots IN
@param pCompleteKnots IN complete knot vector, i.e. in clamped end case end knot has muliplicity = order
@return true if basis completed.
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool bsiNurbs_singleIntervalAsBezier
(
double *pBezierControlPoints,
double *pBezierWeights,
double *pNURBSControlPoints,
double *pNURBSWeights,
int    numComponent,
int    pointIndex0,
double *pCompleteKnots,
int order
)
    {
    double basisCoffs[MAX_BEZIER_CURVE_ORDER*MAX_BEZIER_CURVE_ORDER];
    if (order < 1 || order > MAX_BEZIER_CURVE_ORDER)
        return false;
    if (numComponent < 1)
        return false;

    int knotIndex0 = pointIndex0 + order - 1;
    if (!bsiNurbs_singleIntervalBasisFunctions (basisCoffs, pCompleteKnots, knotIndex0, order))
        return false;

    if (pNURBSControlPoints != NULL && pBezierControlPoints != NULL && numComponent > 0)
        {
        double *pY0 = pNURBSControlPoints + pointIndex0 * numComponent;
        for (int i = 0; i < order; i++)
            {
            double *pQ = pBezierControlPoints + i * numComponent;
            double *pY, *pB;
            int j;
            memset (pQ, 0, numComponent * sizeof (double));
            for (j = 0,
                    pY = pY0,
                    pB = basisCoffs + i;
                    j < order;
                    j++,
                pY += numComponent,
                pB += order)
                {
                double b = *pB;
                for (int k = 0; k < numComponent; k++)
                    {
PUSH_MSVC_IGNORE(6385)
                    pQ[k] += pY[k] * b;
POP_MSVC_IGNORE
                    }
                }
            }
        }

    if (pNURBSWeights != NULL && pBezierWeights != NULL)
        {
        double *pW = pNURBSWeights + pointIndex0;
        for (int i = 0; i < order; i++)
            {
            double w = 0.0;
            for (int j = 0; j < order; j++)
                {
                w += pW[j] * basisCoffs[i + j * order];
                }
            pBezierWeights[i] = w;
            }
        }

    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE