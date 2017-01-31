/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/triquad.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#ifdef DEBUG_TRIQUAD
END_BENTLEY_GEOMETRY_NAMESPACE
#include <toolsubs.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#endif

#define MAX_QUAD_POINT 36

typedef struct
    {
    DPoint2d uv[MAX_QUAD_POINT];
    double   weight[MAX_QUAD_POINT];
    int numPoint;
    } FF_Template;

typedef struct
    {
    DPoint3d point;
    DPoint3d vector;
    double exponent;
    } FF_DifferentialReceiver;

typedef void (*FF_ScalarIntegrand)
        (
        double *pF,
        DPoint3d *pX,
        DPoint3d *pU,
        DPoint3d *pV,
        int numPoint,
        void *pContext
        );

typedef int (*FF_1DTemplate)
    (
    double *,
    double *,
    double,
    double
    );

typedef struct
    {
    FF_Template *pTemplate;
    void *pIntegrandContext;
    FF_ScalarIntegrand func;
    double abstol;
    double reltol;
    double tolFactor;
    int maxDepth;
    double acceptedErrorSum;
    double failedErrorSum;
    int   numPoint;
    } FF_RecursionContext;

/*---------------------------------------------------------*//**
* @description Initialize a differential receiver patch.
* @param pXYZ IN point on patch.
* @param pNormal IN patch normal.
* @param exponent IN exponent for integration
*---------------------------------------------------------*/
static void ffDifferentialReceiver_init

(
FF_DifferentialReceiver *pR,
DPoint3dCP pXYZ,
DPoint3dCP pNormal,
double exponent
)
    {
    pR->point = *pXYZ;
    pR->vector = *pNormal;
    bsiDPoint3d_normalizeInPlace (&pR->vector);
    pR->exponent = exponent;
    }

/*---------------------------------------------------------*//**
* @description Initialize the fixed parts of a recursion context.
* @param pTemplate IN template containing weights and evaluation parameters
* @param func IN integrand function (callback)
* @param pIntegrationContext IN context for func.
* @param abstol IN absolute tolerance
* @param reltol IN relative tolerance
* @param tolFactor IN factor to apply to reduce abstol when moving to a deeper recursion.
* @param maxDepth IN maximum recursion depth.
*---------------------------------------------------------*/
static void ff_initRecursionContext

(
FF_RecursionContext *pRC,
FF_Template *pTemplate,
FF_ScalarIntegrand func,
void *pIntegrandContext,
double abstol,
double reltol,
double tolFactor,
int maxDepth
)
    {
    pRC->pTemplate = pTemplate;
    pRC->pTemplate = pTemplate;
    pRC->func = func;
    pRC->pIntegrandContext = pIntegrandContext;
    pRC->abstol = abstol;
    pRC->reltol = reltol;
    pRC->maxDepth = maxDepth;
    pRC->tolFactor = tolFactor;
    pRC->acceptedErrorSum = pRC->failedErrorSum = 0.0;
    pRC->numPoint = 0;
    }

/*---------------------------------------------------------*//**
* @description change the recursion context to absolute tolerance
*    which is the larger of (a) the current absolute tolerance and (b)
*    the current reltol times the estimated result.
* @param pRC IN OUT context whose tolerance is modified.
* @param estimatedResult IN an estimate of the (absolute value of) the result.
*       This is usually computed as a sum of absolute values of the integrand.
*---------------------------------------------------------*/
static void ff_resetToAbsoluteTolerance

(
FF_RecursionContext *pRC,
double estimatedResult
)
    {
    double tol0 = pRC->abstol;
    double tol1 = pRC->reltol * fabs (estimatedResult);
    pRC->reltol = 0.0;
    pRC->abstol = tol0 > tol1 ? tol0 : tol1;
    }

#ifdef CompileAll
/*---------------------------------------------------------*//**
* @description fill gauss weights for 1 dimensional 2 point Gaussian quadrature
* @param pXi OUT array of x values.
* @param pWi OUT array of weights
* @param a IN lower limit of integration
* @param b IN upper limit of integration
*---------------------------------------------------------*/
static int xw_gauss1D2

(
double *pXi,
double *pWi,
double a,
double b
)
    {
    double uu = sqrt(3.0)/ 6.0;
    double dx = b - a;
    pXi[0] = a + dx * (0.5 + uu);
    pXi[1] = a + dx * (0.5 - uu);
    pWi[0] = pWi[1] = 0.5 * dx;
    return 2;
    }
/*---------------------------------------------------------*//**
* @description fill gauss weights for 1 dimensional 3 point Gaussian quadrature
* @param pXi OUT array of x values.
* @param pWi OUT array of weights
* @param a IN lower limit of integration
* @param b IN upper limit of integration
*---------------------------------------------------------*/
static int xw_gauss1D3

(
double *pXi,
double *pWi,
double a,
double b
)
    {
    double x0 = 0.5 * (a + b);
    double dx = 0.5 * (b - a);

    pWi[0] = pWi[2] = 0.555555555555556 * dx;
    pWi[1] = 0.888888888888889 * dx;

    pXi[0] = x0 + 0.774596669241483 * dx;
    pXi[1] = x0;
    pXi[2] = x0 - 0.774596669241483 * dx;
    return 3;
    }
#endif

/*---------------------------------------------------------*//**
* @description fill gauss weights for 1 dimensional 4 point Gaussian quadrature
* @param pXi OUT array of x values.
* @param pWi OUT array of weights
* @param a IN lower limit of integration
* @param b IN upper limit of integration
*---------------------------------------------------------*/
static int xw_gauss1D4

(
double *pXi,
double *pWi,
double a,
double b
)
    {
    double x0 = 0.5 * (a + b);
    double dx = 0.5 * (b - a);

    pWi[0] = pWi[3] = 0.347854845137454 * dx;
    pWi[1] = pWi[2] = 0.652145154862546 * dx;

    pXi[0] = x0 - 0.861136311594053 * dx;
    pXi[1] = x0 - 0.339981043584856 * dx;
    pXi[2] = x0 + 0.339981043584856 * dx;
    pXi[3] = x0 + 0.861136311594053 * dx;

    return 4;
    }
#ifdef CompileAll
/*---------------------------------------------------------*//**
* @description fill gauss weights for 1 dimensional 3 point quadrature
*   (Simpson's rule)
* @param pXi OUT array of x values.
* @param pWi OUT array of weights
* @param a IN lower limit of integration
* @param b IN upper limit of integration
*---------------------------------------------------------*/
static int xw_simpson1D3

(
double *pXi,
double *pWi,
double a,
double b
)
    {
    double dx = b - a;

    pXi[0] = a;
    pXi[1] = 0.5 * (a + b);
    pXi[2] = b;

    pWi[0] = pWi[2] = dx / 6.0;
    pWi[1] = 2.0 * dx / 3.0;
    return 3;
    }
#endif
/*---------------------------------------------------------*//**
* @description Fill an integration template for a product rule integration on a rectangular
* patch.
* @param pTemplate OUT template of parameters and weights.
* @param weightFunc IN function to obtain weights in one direction.
* @param u0 IN u direction start parameter
* @param u1 IN u direction end parameter
* @param v0 IN v direction start parameter
* @param v1 IN v direction end parameter
*---------------------------------------------------------*/
static void ffTemplate_initProductTemplate

(
FF_Template *pTemplate,
FF_1DTemplate weightFunc,
double u0,
double u1,
double v0,
double v1
)
    {
    double ui[MAX_QUAD_POINT], wi[MAX_QUAD_POINT];
    double vj[MAX_QUAD_POINT], wj[MAX_QUAD_POINT];
    int i, j, k;
    int ni, nj;
    ni = weightFunc (ui, wi, u0, u1);
    nj = weightFunc (vj, wj, v0, v1);
    k = 0;
    for (i = 0; i < ni; i++)
        for (j = 0; j < nj; j++)
            {
            pTemplate->uv[k].x = ui[i];
            pTemplate->uv[k].y = vj[j];
            pTemplate->weight[k] = wi[i] * wj[j];
            k++;
            }
    pTemplate->numPoint = ni * nj;
    }


/*---------------------------------------------------------*//**
* @description diffuse surface form factor integrand for
*       differential receiver point.
* @param pF OUT array of integrand values
* @param pX IN array of points for evaluation
* @param pdXdu IN array of u-direction tangential derivatives
* @param pdXdv IN array of v-direction tangential derivatives
* @param numPoint IN number of points
* @param pContext IN receiver patch description.
*---------------------------------------------------------*/
static void cb_diffuseIntegrand

(
double *pF,
DPoint3dP pX,
DPoint3dP pdXdu,
DPoint3dP pdXdv,
int numPoint,
FF_DifferentialReceiver *pContext
)
    {
    DPoint3d vectorR;
    double RdotW, rr, RdotUV;
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorR, &pX[i], &pContext->point);
        RdotW = bsiDPoint3d_dotProduct (&vectorR, &pContext->vector);
        RdotUV = bsiDPoint3d_tripleProduct (&vectorR, &pdXdu[i], &pdXdv[i]);
        rr = bsiDPoint3d_dotProduct (&vectorR, &vectorR);
        pF[i] = RdotW * RdotUV / (rr * rr);
        }
    }

/*---------------------------------------------------------*//**
* @description specular surface form factor integrand for
*       differential receiver point.
* @param pF OUT array of integrand values
* @param pX IN array of points for evaluation
* @param pdXdu IN array of u-direction tangential derivatives
* @param pdXdv IN array of v-direction tangential derivatives
* @param numPoint IN number of points
* @param pContext IN receiver patch description.
*---------------------------------------------------------*/
static void cb_specularIntegrand

(
double  *pF,
DPoint3dP pX,
DPoint3dP pdXdu,
DPoint3dP pdXdv,
int numPoint,
FF_DifferentialReceiver *pContext
)
    {
    DPoint3d vectorR;
    double RdotW, RdotUV, rr, r;
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorR, &pX[i], &pContext->point);
        RdotW = bsiDPoint3d_dotProduct (&vectorR, &pContext->vector);
        RdotUV = bsiDPoint3d_tripleProduct (&vectorR, &pdXdu[i], &pdXdv[i]);
        rr = bsiDPoint3d_dotProduct (&vectorR, &vectorR);
        r = sqrt (rr);
        pF[i] = pow (RdotW / r, pContext->exponent) * RdotUV / (rr * r);
        }
    }

/*---------------------------------------------------------*//**
* @description Compute point, tangents, and weights for integration.
* @param pGT IN template, contains counts, parameters, and weights.
* @param pX00 IN 00 point of bilinear patch
* @param pX10 IN 10 point of bilinear patch
* @param pX01 IN 01 point of bilinear patch
* @param pX11 IN 11 point of bilinear patch
* @param pXEval OUT ARRAY of points for evaluation
* @param pdXduEval OUT ARRAY of tangential derivatives
* @param pdXdvEval OUT ARRAY of tangential derivatives
* @param pWEval OUT ARRAY of weights
*---------------------------------------------------------*/
static int computeIntegrationPointsOnBilinearPatch

(
FF_Template *pGT,
DPoint3dP pX00,
DPoint3dP pX10,
DPoint3dP pX01,
DPoint3dP pX11,
DPoint3dP pXEval,
DPoint3dP pdXduEval,
DPoint3dP pdXdvEval,
double   *pWEval
)
    {
    int i;
    double u, v;
    DPoint3d vectorU;
    DPoint3d vectorV;
    DPoint3d vectorW;
    DPoint3d point11;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, pX10, pX00);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorV, pX01, pX00);
    bsiDPoint3d_addDPoint3dDPoint3d (&point11, pX10, &vectorV);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorW, pX11, &point11);

    for (i = 0; i < pGT->numPoint; i++)
        {
        u = pGT->uv[i].x;
        v = pGT->uv[i].y;
        bsiDPoint3d_add3ScaledDPoint3d (&pXEval[i],
                        pX00,
                        &vectorU, u,
                        &vectorV, v,
                        &vectorW, u * v
                        );
        bsiDPoint3d_addScaledDPoint3d (&pdXduEval[i], &vectorU, &vectorW, v);
        bsiDPoint3d_addScaledDPoint3d (&pdXdvEval[i], &vectorV, &vectorW, u);
        pWEval[i] = pGT->weight[i];
        }
    return pGT->numPoint;
    }

/*---------------------------------------------------------*//**
* @description Compute midside and midpatch points of a quadrilateral patch
* @param pXm0 OUT bottom midside, between pX00 and pX10
* @param pX0m OUT left midside, between pX00 and pX01
* @param pXmm OUT midspatch
* @param pX1m OUT right midside, between pX10 and pX11
* @param pX1m OUT top midside, between pX01 and pX11
* @param pX00 IN 00 point of quadrilateral
* @param pX10 IN 10 point of quadrilateral
* @param pX01 IN 01 point of quadrilateral
* @param pX11 IN 11 point of quadrilateral
*---------------------------------------------------------*/
static void computePatchMidpoints

(
DPoint3dP pXm0,
DPoint3dP pX0m,
DPoint3dP pXmm,
DPoint3dP pX1m,
DPoint3dP pXm1,
DPoint3dCP pX00,
DPoint3dCP pX10,
DPoint3dCP pX01,
DPoint3dCP pX11
)
    {
    bsiDPoint3d_interpolate (pXm0, pX00, 0.5, pX10);
    bsiDPoint3d_interpolate (pX0m, pX00, 0.5, pX01);
    bsiDPoint3d_interpolate (pX1m, pX10, 0.5, pX11);
    bsiDPoint3d_interpolate (pXm1, pX01, 0.5, pX11);
    bsiDPoint3d_interpolate (pXmm, pXm0, 0.5, pXm1);
    }

/*---------------------------------------------------------*//**
* @description Evaluate and sum integrands in a single patch.
* @param pRC IN recursion context
* @param pResult OUT computed sum.  (May be NULL)
* @param pAbsResult OUT computed sum using absolute values of integrand. (May be NULL)
* @param pX00 IN 00 point of quadrilateral
* @param pX10 IN 10 point of quadrilateral
* @param pX01 IN 01 point of quadrilateral
* @param pX11 IN 11 point of quadrilateral
*---------------------------------------------------------*/
static void ff_sumInSinglePatch

(
FF_RecursionContext *pRC,
double *pResult,
double *pAbsResult,
DPoint3dP pX00,
DPoint3dP pX10,
DPoint3dP pX01,
DPoint3dP pX11
)
    {
    DPoint3d xyz[MAX_QUAD_POINT];
    DPoint3d vectorU[MAX_QUAD_POINT];
    DPoint3d vectorV[MAX_QUAD_POINT];
    double weight [MAX_QUAD_POINT];
    double funcValue[MAX_QUAD_POINT];
    int i;
    double sum;

    int numPoint = computeIntegrationPointsOnBilinearPatch
                (
                pRC->pTemplate, pX00, pX10, pX01, pX11,
                xyz, vectorU, vectorV, weight
                );
    pRC->func (funcValue, xyz, vectorU, vectorV, numPoint, pRC->pIntegrandContext);

    if (pResult)
        {
        sum = 0.0;
        for (i = 0; i < numPoint; i++)
            {
            sum += weight[i] * funcValue[i];
            }
        *pResult = sum;
        }

    if (pAbsResult)
        {
        sum = 0.0;
        for (i = 0; i < numPoint; i++)
            {
            sum += fabs (weight[i] * funcValue[i]);
            }
        *pAbsResult = sum;
        }
    }


/*---------------------------------------------------------*//**
* @description Check termination conditions
* @param pRC IN recursion context
* @param parentSum IN integral estimate from parent depth
* @param localSum IN integral estimate at current depth
* @param toleranceFactor IN depth-specific factor applied to abstol of recursion context.
* @param depth IN current depth of integration.
*---------------------------------------------------------*/
static bool    ff_terminate

(
FF_RecursionContext *pRC,
double parentSum,
double localSum,
double toleranceFactor,
int depth
)
    {
    double errorHere = fabs (parentSum - localSum);
    if (errorHere <= toleranceFactor * pRC->abstol + fabs(localSum) * pRC->reltol)
        {
        pRC->acceptedErrorSum += errorHere;
        return true;
        }

    if (depth >= pRC->maxDepth)
        {
        pRC->failedErrorSum += errorHere;
        return true;
        }
    /* Allow the caller to recurse .... */
    return false;
    }
#ifdef DEBUG_TRIQUAD
static void indent (int depth)
    {
    while (depth-- > 0)
        printf ("  ");
    }
#endif

/*---------------------------------------------------------*//**
* @description Body of recursive integration
* @param pRC IN recursion context.
* @param pX00 IN 00 point of quadrilateral
* @param pX10 IN 10 point of quadrilateral
* @param pX01 IN 01 point of quadrilateral
* @param pX11 IN 11 point of quadrilateral
* @param parentSum IN integral estimated over this patch at prior level of recursion.
* @param toleranceFactor IN depth-specific factor applied to abstol of recursion context.
* @param depth IN current depth of integration.
*---------------------------------------------------------*/
static double ff_recurse

(
FF_RecursionContext *pRC,
DPoint3dP pX00,
DPoint3dP pX10,
DPoint3dP pX01,
DPoint3dP pX11,
double parentSum,
double toleranceFactor,
int depth
)
    {
    DPoint3d Ybottom, Yleft, Ymid, Yright, Ytop;
    double f00, f10, f01, f11, mySum;

    computePatchMidpoints
                (
                &Ybottom, &Yleft, &Ymid, &Yright, &Ytop,
                pX00, pX10, pX01, pX11
                );

    ff_sumInSinglePatch (pRC, &f00, NULL, pX00, &Ybottom, &Yleft, &Ymid);
    ff_sumInSinglePatch (pRC, &f10, NULL, &Ybottom, pX10, &Ymid, &Yright);
    ff_sumInSinglePatch (pRC, &f01, NULL, &Yleft, &Ymid, pX01, &Ytop);
    ff_sumInSinglePatch (pRC, &f11, NULL, &Ymid, &Yright, &Ytop, pX11);
    pRC->numPoint += 4 * pRC->pTemplate->numPoint;

    mySum = f00 + f10 + f01 + f11;

#ifdef DEBUG_TRIQUAD_ALL
    indent (depth);
    printf ("(00 %lg)\n", f00);

    indent (depth);
    printf ("(10 %lg)\n", f10);

    indent (depth);
    printf ("(01 %lg)\n", f01);

    indent (depth);
    printf ("(11 %lg)\n", f11);

    indent (depth);
    printf ("   (parent %lg local %lg err %lg)\n", parentSum, mySum, fabs (mySum - parentSum));
#endif


    if (!ff_terminate (pRC, parentSum, mySum, toleranceFactor, depth))
        {
        depth++;
        toleranceFactor *= pRC->tolFactor;
        f00 = ff_recurse (pRC, pX00, &Ybottom, &Yleft, &Ymid,
                        f00, toleranceFactor, depth);
        f10 = ff_recurse (pRC, &Ybottom, pX10, &Ymid, &Yright,
                        f10, toleranceFactor, depth);
        f01 = ff_recurse (pRC, &Yleft, &Ymid, pX01, &Ytop,
                        f01, toleranceFactor, depth);
        f11 = ff_recurse (pRC, &Ymid, &Yright, &Ytop, pX11,
                        f11, toleranceFactor, depth);
        mySum = f00 + f10 + f01 + f11;
        }
    return mySum;
    }

/*---------------------------------------------------------*//**
* @description Top level driver for recursive integration.over a quadrilateral
* @param pGT IN template of integration weights, in 0..1 parameter space in each direction.
* @param func IN function to compute a vector of integrand values
* @param pIntegrationContext IN context pointer for integrand function
* @param pX00 IN 00 point of quadrilateral
* @param pX10 IN 10 point of quadrilateral
* @param pX01 IN 01 point of quadrilateral
* @param pX11 IN 11 point of quadrilateral
* @param reltol IN relative tolerance
* @param maxDepth IN maximum permittted depth
*---------------------------------------------------------*/
static double ff_recursionDriver

(
FF_Template *pGT,
FF_ScalarIntegrand func,
void *pIntegrandContext,
DPoint3dP pX00,
DPoint3dP pX10,
DPoint3dP pX01,
DPoint3dP pX11,
double reltol,
int maxDepth
)
    {
    FF_RecursionContext rc;
    double fTop, fTopAbs, fResult;
    double tolFactor = 0.5;
    ff_initRecursionContext
                (
                &rc, pGT,
                func, pIntegrandContext,
                0.0,
                reltol,
                tolFactor,
                maxDepth);

    ff_sumInSinglePatch (&rc, &fTop, &fTopAbs, pX00, pX10, pX01, pX11);
#ifdef DEBUG_TRIQUAD_ALL
    printf ("(top %lg)\n", fTop);
#endif
    ff_resetToAbsoluteTolerance (&rc, fTopAbs);
    fResult = ff_recurse (&rc, pX00, pX10, pX01, pX11, fTop, 1.0, 0);
#ifdef DEBUG_TRIQUAD
    printf ("(function calls %d\n)", rc.numPoint);
#endif
    return fResult;
    }

/*---------------------------------------------------------*//**
* @description Compute the form factor for a specular quadrilateral reflecting to a differential receiver,
* @remark This integrates over the entire reflector without regard to visibility.
* @param pXYZTri IN 4 quadrilateral vertices, in CCW order.
* @param pXYZReceiver IN receiver point
* @param pReceiverNormal IN surface normal at receiver
* @param exponent IN specular exponent to be applied to the cosine term of the integrand.
* @param relTol IN relative tolerance for result.
*---------------------------------------------------------*/
static double ff_specularQuad

(
DPoint3dP pXYZQuad,
DPoint3dP pXYZReceiver,
DPoint3dP pReceiverNormal,
double exponent,
double relTol
)
    {
    FF_Template myTemplate;
    FF_DifferentialReceiver receiver;
    ffDifferentialReceiver_init (&receiver, pXYZReceiver, pReceiverNormal, exponent);
    ffTemplate_initProductTemplate (&myTemplate, xw_gauss1D4, 0.0, 1.0, 0.0, 1.0);
    return ff_recursionDriver (&myTemplate, (FF_ScalarIntegrand)cb_specularIntegrand, &receiver,
                                &pXYZQuad[0], &pXYZQuad[1], &pXYZQuad[3], &pXYZQuad[2],
                                relTol, 10);
    }

/*---------------------------------------------------------*//**
* @description Compute the form factor for a diffuse quadrilateral reflecting to a differential receiver,
* @remark This integrates over the entire reflector without regard to visibility.
* @param pXYZTri IN 4 quadrilateral vertices, in CCW order.
* @param pXYZReceiver IN receiver point
* @param pReceiverNormal IN surface normal at receiver
* @param relTol IN relative tolerance for result.
*---------------------------------------------------------*/
static double ff_diffuseQuad

(
DPoint3dP pXYZQuad,
DPoint3dP pXYZReceiver,
DPoint3dP pReceiverNormal,
double relTol
)
    {
    FF_Template myTemplate;
    FF_DifferentialReceiver receiver;
    ffDifferentialReceiver_init (&receiver, pXYZReceiver, pReceiverNormal, 1.0);
    ffTemplate_initProductTemplate (&myTemplate, xw_gauss1D4, 0.0, 1.0, 0.0, 1.0);
    return ff_recursionDriver (&myTemplate, (FF_ScalarIntegrand)cb_diffuseIntegrand, &receiver,
                                &pXYZQuad[0], &pXYZQuad[1], &pXYZQuad[3], &pXYZQuad[2],
                                relTol, 10);
    }
#ifdef CompileAll
/*---------------------------------------------------------*//**
* @description Compute the form factor for a diffuse triangle reflecting to a differential receiver,
* @remark This integrates over the entire reflector without regard to visibility.
* @param pXYZTri IN 3 triangle vertices.
* @param pXYZReceiver IN receiver point
* @param pReceiverNormal IN surface normal at receiver
* @param relTol IN relative tolerance for result.
*---------------------------------------------------------*/
static double ff_diffuseTri

(
DPoint3dP pXYZTri,
DPoint3dP pXYZReceiver,
DPoint3dP pReceiverNormal,
double relTol
)
    {
    DPoint3d xyzQuad[4];
    xyzQuad[0] = pXYZTri[0];
    xyzQuad[1] = pXYZTri[1];
    xyzQuad[2] = pXYZTri[2];
    xyzQuad[3] = pXYZTri[0];
    return ff_diffuseQuad (xyzQuad, pXYZReceiver, pReceiverNormal, relTol);
    }
#endif
/*---------------------------------------------------------*//**
* @description Compute the form factor for a specular triangle reflecting to a differential receiver,
* @remark This integrates over the entire reflector without regard to visibility.
* @param pXYZTri IN triangle vertices, in CCW order.
* @param pXYZReceiver IN receiver point
* @param pReceiverNormal IN surface normal at receiver
* @param exponent IN specular exponent to be applied to the cosine term of the integrand.
* @param relTol IN relative tolerance for result.
*---------------------------------------------------------*/
static double ff_specularTri

(
DPoint3dP pXYZTri,
DPoint3dP pXYZReceiver,
DPoint3dP pReceiverNormal,
double   exponent,
double relTol
)
    {
    DPoint3d xyzQuad[4];
    xyzQuad[0] = pXYZTri[0];
    xyzQuad[1] = pXYZTri[1];
    xyzQuad[2] = pXYZTri[2];
    xyzQuad[3] = pXYZTri[0];
    if (exponent == 1.0)
        return ff_diffuseQuad (xyzQuad, pXYZReceiver, pReceiverNormal, relTol);
    else
        return ff_specularQuad (xyzQuad, pXYZReceiver, pReceiverNormal, relTol, exponent);
    }

/*---------------------------------------------------------*//**
* @description adjust an index into cyclic range.
*---------------------------------------------------------*/
static int cyclicIndex

(
int i,
int n
)
    {
    if (i >= n)
        {
        do
            {
            i -= n;
            } while (i >= n);
        }
    else if (i < 0)
        {
        do
            {
            i += n;
            } while (i < 0);
        }
    return i;
    }

/*---------------------------------------------------------*//**
* @description Interpolate between points specified as CYCLIC indices into a polygon.
* @param pXYZOut OUT interpolated point
* @param altitude IN altitude at interpolated point.
* @param pXYZArray IN polygon points.
* @param pAltitudeArray IN altitudes for interpolation
* @param numEdge IN number of edges in polygon
* @param i0 IN index of first vertex.  Adjusted cyclically into range.
* @param i1 IN index of second vertex.  Adjusted cyclically into range
*---------------------------------------------------------*/
static void getDPoint3dCyclic

(
DPoint3dP pXYZOut,
DPoint3dCP pXYZArray,
int i,
int n
)
    {
    *pXYZOut = pXYZArray[cyclicIndex (i, n)];
    }
/*---------------------------------------------------------*//**
* @description Interpolate between points specified as CYCLIC indices into a polygon.
* @param pXYZOut OUT interpolated point
* @param altitude IN altitude at interpolated point.
* @param pXYZArray IN polygon points.
* @param pAltitudeArray IN altitudes for interpolation
* @param numEdge IN number of edges in polygon
* @param i0 IN index of first vertex.  Adjusted cyclically into range.
* @param i1 IN index of second vertex.  Adjusted cyclically into range
*---------------------------------------------------------*/
static void interpolateIndexedCyclicEdge

(
DPoint3dP pXYZOut,
double altitude,
DPoint3dCP pXYZArray,
const double *pAltitudeArray,
int numEdge,
int i0,
int i1
)
    {
    double f;
    i0 = cyclicIndex (i0, numEdge);
    i1 = cyclicIndex (i1, numEdge);
    bsiTrig_safeDivide (&f, altitude - pAltitudeArray[i0], pAltitudeArray[i1] - pAltitudeArray[i0], 0.0);
    bsiDPoint3d_interpolate (pXYZOut, &pXYZArray[i0], f, &pXYZArray[i1]);
    }

/*---------------------------------------------------------*//**
* @description Compute the form factor for a specular triangle reflecting to a differential receiver,
* @remark The triangle is evaluated to reorient towards the receiver and split at the
*           reciever horizon.   Any receiver on the plane of the triangle returns 0.
* @param pXYZTri IN triangle vertices, in CCW order.
* @param pXYZReceiver IN receiver point
* @param pReceiverNormal IN surface normal at receiver
* @param exponent IN specular exponent to be applied to the cosine term of the integrand.
* @param relTol IN relative tolerance for result.
*---------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsiFormFactor_specularDPoint3dTriangle

(
DPoint3dP pXYZTri,
DPoint3dP pXYZReceiver,
DPoint3dP pReceiverNormal,
double   exponent,
double reltol
)
    {
    double altitude[3];
    double distanceSum;
    double receiverAltitude;
    int i;
    int numPositive = 0;
    int numNegative = 0;
    int lastPositive = -1;
    int lastNegative = -1;
    double result = 0.0;
    double resultSign = 1.0;
    static double s_altitudeReltol = 1.0e-5;
    DPoint3d xyzArray[4];
    DPoint3d triangleNormal;

    bsiDPoint3d_crossProduct3DPoint3d (&triangleNormal, &pXYZTri[0], &pXYZTri[1], &pXYZTri[2]);
    bsiDPoint3d_normalizeInPlace (&triangleNormal);
    distanceSum = 0.0;
    for (i = 0; i < 3; i++)
        distanceSum += bsiDPoint3d_distance (pXYZReceiver, &pXYZTri[i]);

    receiverAltitude = bsiDPoint3d_dotDifference (&pXYZTri[0], pXYZReceiver, (DVec3d*) &triangleNormal);

    /* On plane of triangle? */
    if (fabs (receiverAltitude) < s_altitudeReltol * distanceSum)
        return 0.0;

    resultSign = 1.0;
    if (receiverAltitude < 0.0)
        resultSign = -1.0;

    for (i = 0; i < 3; i++)
        {
        altitude[i] = bsiDPoint3d_dotDifference (&pXYZTri[i], pXYZReceiver, (DVec3d*) pReceiverNormal);
        if (altitude[i] > 0.0)
            {
            lastPositive= i;
            numPositive++;
            }
        else
            {
            lastNegative = i;
            numNegative++;
            }
        }

    switch (numPositive)
        {
        case 0:
            /* The triangle is entirely below the horizon */
            result = 0.0;
            break;
        case 1:
            /* Split off the (one) triangle above the horizon */
            xyzArray[0] = pXYZTri[lastPositive];
            interpolateIndexedCyclicEdge (&xyzArray[1], 0.0, pXYZTri, altitude, 3, lastPositive, lastPositive+1);
            interpolateIndexedCyclicEdge (&xyzArray[2], 0.0, pXYZTri, altitude, 3, lastPositive, lastPositive+2);
            result = ff_specularTri (xyzArray, pXYZReceiver, pReceiverNormal, exponent, reltol);
            break;
        case 2:
            /* Split off the (one) triangle above the horizon */
            interpolateIndexedCyclicEdge (&xyzArray[0], 0.0, pXYZTri, altitude, 3, lastNegative, lastNegative + 1);
            getDPoint3dCyclic (&xyzArray[1], pXYZTri, lastNegative + 1, 3);
            getDPoint3dCyclic (&xyzArray[2], pXYZTri, lastNegative + 2, 3);
            interpolateIndexedCyclicEdge (&xyzArray[3], 0.0, pXYZTri, altitude, 3, lastNegative, lastNegative + 2);
            result = ff_specularQuad (xyzArray, pXYZReceiver, pReceiverNormal, exponent, reltol);
            break;
        case 3:
            /* All at once */
            result = ff_specularTri (pXYZTri, pXYZReceiver, pReceiverNormal, exponent, reltol);
            break;
        }

    return resultSign * result;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
