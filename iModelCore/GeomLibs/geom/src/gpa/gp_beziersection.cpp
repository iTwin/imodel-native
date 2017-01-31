/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_beziersection.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct
    {
    const double *pAij;
    const double *pBij;
    double fa;
    double fb;
    int         iCount;
    int         jCount;
    int         iStride;
    int         jStride;
    GraphicsPointArray *pCollector;
    double      du;
    double      dv;
    } LevelSurfaceContext;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     orientedHyperbolaCoefficients
(
double *pf,
double *pa,
double *pb,
double *pc,
double  f00,
double  f10,
double  f01,
double  f11
)
    {
    *pf = f00;
    *pa = f10 - f00;
    *pb = f01 - f00;
    *pc = f00 + f11 - f01 - f10;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     orientedHyperbolaIntersections
(
double  *pU,
double  *pV,
int     *pNumOut,
double f0,
double a0,
double b0,
double c0,
double f1,
double a1,
double b1,
double c1
)
    {
    double cc = f1 * a0 - f0 * a1;
    double bb = f1 * c0 - c1 * f0 + b1 * a0 - a1 * b0;
    double aa = b1 * c0 - b0 * c1;
    double vv[2];
    double relTol = bsiTrig_smallAngle ();
    int numRoot = bsiMath_solveQuadratic (vv, aa, bb, cc);
    int i;
    double u, v;
    *pNumOut = 0;
    for (i = 0; i < numRoot; i++)
        {
        v = vv[i];
        if (v < 0.0 && v > -relTol)
            v = 0.0;
        if (v > 1.0 && v < 1.0 + relTol)
            v = 1.0;

        if (    0.0 <= v && v <= 1.0)
            {
            double alpha0 = - (f0 + b0 * v);
            double beta0  =  a0 + c0 * v;
            double alpha1 = - (f1 + b1 * v);
            double beta1  =  a1 + c1 * v;
            bool    ok = false;
            if (fabs(beta1) > fabs (beta0))
                ok = bsiTrig_safeDivide (&u, alpha1, beta1, 0.0);
            else
                ok = bsiTrig_safeDivide (&u, alpha0, beta0, 0.0);
            if (ok && 0.0 <= u && u <= 1.0)
                {
                pU[*pNumOut] = u;
                pV[*pNumOut] = v;
                *pNumOut += 1;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        levelSurface_getAB
(
LevelSurfaceContext *pContext,
        double      *pA,
        double      *pB,
        int         i,
        int         j,
        bool        shiftAltitude
)
    {
    if (    0 <= i
        &&  i < pContext->iCount
        &&  0 <= j
        &&  j < pContext->jCount)
        {
        if (pA)
            {
            *pA = pContext->pAij[i * pContext->iStride + j * pContext->jStride];
            if (shiftAltitude)
                *pA -= pContext->fa;
            }
        if (pB)
            {
            *pB = pContext->pBij[i * pContext->iStride + j * pContext->jStride];
            if (shiftAltitude)
                *pB -= pContext->fb;
            }
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     runNewtonRaphson
(
LevelSurfaceContext *pContext,
    double          *pU,
    double          *pV
)
    {
    double u = *pU;
    double v = *pV;
    double f0, f1;
    double g0, g1;
    double f0u, f0v, f1u, f1v;
    double du, dv;
    int counter = 0;
    static double s_uvTol = 1.0e-10;
    for (counter = 0; counter < 8; counter++)
        {
        bsiBezier_evaluateTensorProduct (&f0, &f0u, &f0v, NULL,
                        pContext->pAij, pContext->iCount, pContext->iStride,
                                        pContext->jCount, pContext->jStride,
                                        1, u, v);
        g0 = f0 - pContext->fa;
        bsiBezier_evaluateTensorProduct (&f1, &f1u, &f1v, NULL,
                        pContext->pBij, pContext->iCount, pContext->iStride,
                                        pContext->jCount, pContext->jStride,
                                        1, u, v);
        g1 = f1 - pContext->fb;
        if (bsiSVD_solve2x2 (&du, &dv,f0u, f0v, f1u, f1v, g0, g1))
            {
#ifdef NOISY
            printf(" Newton Step u:(%le-%le)   v:(%le-%le)\n",
                            u, du, v, dv);
#endif
            if (fabs (du) > pContext->du)
                du = (du > 0.0 ? 1.0 : -1.0) * pContext->dv;
            if (fabs (dv) > pContext->dv)
                du = (dv > 0.0 ? 1.0 : -1.0) * pContext->dv;
            u -= du;
            v -= dv;
            if (fabs (du) < s_uvTol && fabs (dv) < s_uvTol)
                {
                *pU = u;
                *pV = v;
                return;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     levelSurface_commonRootsOnSquare
(
LevelSurfaceContext *pContext,
        int         i0,
        int         j0,
        bool        doNewtonRaphson
)
    {
    double f[2][2];
    double g[2][2];
    /* Hyperbola coefficients */
    double f0, f1, a0, a1, b0, b1, c0, c1;
    int i, j;
    int numIntersection;
    double uu[2], vv[2];
    double fMin, fMax;

    for (j = 0; j < 2; j++)
        for (i = 0; i < 2; i++)
            if (!levelSurface_getAB (pContext, &f[i][j], &g[i][j], i0 + i, j0 + j, true))
                return;
    bsiDoubleArray_minMax (&fMin, &fMax, (const double *)f, 4);
    if (fMin > 0.0 || fMax < 0.0)
        return;
    bsiDoubleArray_minMax (&fMin, &fMax, (const double *)g, 4);
    if (fMin > 0.0 || fMax < 0.0)
        return;

    orientedHyperbolaCoefficients (&f0, &a0, &b0, &c0, f[0][0], f[1][0], f[0][1], f[1][1]);
    orientedHyperbolaCoefficients (&f1, &a1, &b1, &c1, g[0][0], g[1][0], g[0][1], g[1][1]);
    orientedHyperbolaIntersections (uu, vv, &numIntersection, f0, a0, b0, c0, f1, a1, b1, c1);
    for (i = 0; i < numIntersection; i++)
        {
        double u = (i0 + uu[i]) * pContext->du;
        double v = (j0 + vv[i]) * pContext->dv;
        if (doNewtonRaphson)
            runNewtonRaphson (pContext, &u, &v);
        jmdlGraphicsPointArray_addComponents (pContext->pCollector, u, v, 0.0, 1.0);
        jmdlGraphicsPointArray_markPoint (pContext->pCollector);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Clip segment to 0..1 in x,y (weighted).  Save residual in graphics point array.
* Sounds simple, looks ugly because we have to anticipate the homogeneous line has
* all the negative weight possibilities.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     levelSurface_clipAndSaveSegmentInUnitSquare
(
LevelSurfaceContext *pContext,
        Transform    *pTransform,
const   DPoint4d    *pStart,
const   DPoint4d    *pEnd
)
    {
    DSegment4d segment;
    static DPoint4d planeVec[2][2] =
        {
            { {1.0, 0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, -1.0} },
            { {0.0, 1.0, 0.0, 0.0}, {0.0, 1.0, 0.0, -1.0} },
        };
    double h0, h1, lambda[5];
    int numLambda = 0;
    int numError;
    int direction, side, k0, k1;
    DPoint4d midHomogeneous;
    DPoint3d midCartesian;
    double midLambda;
    segment.point[0] = *pStart;
    segment.point[1] = *pEnd;
    for (direction = 0; direction < 2; direction++)
        {
        numError = 0;
        for (side = 0; side < 2; side++)
            {
            h0 = bsiDPoint4d_dotProduct (&segment.point[0], &planeVec[direction][side]);
            h1 = bsiDPoint4d_dotProduct (&segment.point[1], &planeVec[direction][side]);
            if (!bsiTrig_safeDivide (&lambda[numLambda], -h0, h1 - h0, 0.0))
                numError++;
            else
                numLambda++;
            }
        }

    if (numLambda > 1)
        {
        double delta;
        bsiDoubleArray_sort (lambda, numLambda, true);
        /* Check the interval from each lambda to its successor, including the
            cyclic interval from max to min (by way of infinity) */
        delta = lambda[numLambda-1] - lambda[0];
        lambda[numLambda] = lambda[numLambda - 1] + delta;
        for (k0 = 0; k0 < numLambda; k0++)
            {
            k1 = k0 + 1;
            midLambda = 0.5 * (lambda[k0] + lambda[k1]);

            bsiDSegment4d_fractionParameterToDPoint4d (&segment, &midHomogeneous, midLambda);
            if (   bsiDPoint4d_normalize (&midHomogeneous, &midCartesian)
                && midCartesian.x >= 0.0
                && midCartesian.y >= 0.0
                && midCartesian.x <= 1.0
                && midCartesian.y <= 1.0)
                {
                DSegment4d clipped;
                bsiDSegment4d_fractionParameterToDPoint4d (&segment, &clipped.point[0], lambda[k0]);
                bsiDSegment4d_fractionParameterToDPoint4d (&segment, &clipped.point[1], lambda[k1]);
                bsiTransform_multiplyDPoint4dArray (pTransform, clipped.point, clipped.point, 2);
                jmdlGraphicsPointArray_addDSegment4d (pContext->pCollector, &clipped, false);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     levelSurface_levelSurfaceOnSquare
(
LevelSurfaceContext *pContext,
        int         i0,
        int         j0
)
    {
    double f[2][2];
    double f0, a, b, c, fMin, fMax;
    int typeCode;
    double u0, v0;
    int i, j;
    DConic4d fullConic, clippedConic;
    Transform transform;
    DRange3d clipRange;

    for (j = 0; j < 2; j++)
        for (i = 0; i < 2; i++)
            if (!levelSurface_getAB (pContext, &f[i][j], NULL, i0 + i, j0 + j, true))
                return;

    u0 = i0 * pContext->du;
    v0 = j0 * pContext->dv;

    bsiDoubleArray_minMax (&fMin, &fMax, (const double *)f, 4);
    if (fMin > 0.0 || fMax < 0.0)
        return;

    orientedHyperbolaCoefficients (&f0, &a, &b, &c, f[0][0], f[1][0], f[0][1], f[1][1]);
    typeCode = bsiDConic4d_classifyImplicitXYW (&fullConic, 0.0, c, a, 0.0, b, f0);

    bsiTransform_initFromRowValues
                (&transform,
                pContext->du, 0.0,      0.0, u0,
                0.0, pContext->dv,      0.0, v0,
                0.0, 0.0,               1.0, pContext->fa
                );

    if (typeCode == 0)
        {
        double  clipAngle[20];
        int     clipInOut[20];
        int numClip;
        int i;
        bsiDPoint3d_setXYZ (&clipRange.low, 0.0, 0.0, 0.0);
        bsiDPoint3d_setXYZ (&clipRange.high, 1.0, 1.0, 0.0);
        bsiDConic4d_intersectDRange3d
                    (&fullConic, clipAngle, clipInOut, &numClip, 20, &clipRange, true, true, false);
        bsiDConic4d_applyTransform (&fullConic, &transform, &fullConic);
        clippedConic = fullConic;
        for (i = 0; i < numClip - 1; i++)
            {
            if (clipInOut[i] != 0)
                {
                clippedConic.start = clipAngle[i];
                clippedConic.sweep = clipAngle[i+1] - clipAngle[i];
                jmdlGraphicsPointArray_addDConic4d (pContext->pCollector, &clippedConic);
                }
            }
        }
    else if (typeCode == 1)
        {
        levelSurface_clipAndSaveSegmentInUnitSquare (pContext, &transform, &fullConic.center, &fullConic.vector0);
        levelSurface_clipAndSaveSegmentInUnitSquare (pContext, &transform, &fullConic.center, &fullConic.vector90);
        }
    else if (typeCode == 2)
        {
        levelSurface_clipAndSaveSegmentInUnitSquare (pContext, &transform, &fullConic.center, &fullConic.vector0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     levelSurface_initContext
(
LevelSurfaceContext *pContext,
const   double      *pAij,
        double      fa,
const   double      *pBij,
        double      fb,
        int         iCount,
        int         iStride,
        int         jCount,
        int         jStride,
GraphicsPointArray *pCollector
)
    {
    pContext->pAij = pAij;
    pContext->fa = fa;
    pContext->pBij = pBij;
    pContext->fb = fb;
    pContext->iCount = iCount;
    pContext->jCount = jCount;
    pContext->iStride = iStride;
    pContext->jStride = jStride;
    pContext->du = 1.0 / (iCount - 1);
    pContext->dv = 1.0 / (jCount - 1);
    pContext->pCollector = pCollector;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addBilinearCommonRoots
(
        GraphicsPointArray  *pCollector,
const   double *pAij,
        double fa,
const   double *pBij,
        double fb,
        int     iCount,
        int     iStride,
        int     jCount,
        int     jStride
)
    {
    int i, j;

    LevelSurfaceContext context;
    levelSurface_initContext
                    (
                    &context,
                    pAij, fa, pBij, fb,
                    iCount, iStride, jCount, jStride,
                    pCollector
                    );

    for (i = 1; i < iCount; i++)
        {
        for (j = 1; j < jCount; j++)
            {
            levelSurface_commonRootsOnSquare (&context, i-1, j-1, false);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addBezierCommonRoots
(
        GraphicsPointArray  *pCollector,
const   double *pAij,
        double fa,
const   double *pBij,
        double fb,
        int     iCount,
        int     iStride,
        int     jCount,
        int     jStride
)
    {
    int i, j;

    LevelSurfaceContext context;
    levelSurface_initContext
                    (
                    &context,
                    pAij, fa, pBij, fb,
                    iCount, iStride, jCount, jStride,
                    pCollector
                    );

    for (i = 1; i < iCount; i++)
        {
        for (j = 1; j < jCount; j++)
            {
            levelSurface_commonRootsOnSquare (&context, i-1, j-1, true);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addBezierLevelSurface
(
        GraphicsPointArray  *pCollector,
const   double *pAij,
        int     iCount,
        int     iStride,
        int     jCount,
        int     jStride,
        double  zPlane
)
    {
    int i, j;

    LevelSurfaceContext context;
    levelSurface_initContext (&context, pAij, zPlane, NULL, 0.0, iCount, iStride, jCount, jStride, pCollector);

    for (i = 1; i < iCount; i++)
        {
        for (j = 1; j < jCount; j++)
            {
            levelSurface_levelSurfaceOnSquare (&context, i-1, j-1);
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE