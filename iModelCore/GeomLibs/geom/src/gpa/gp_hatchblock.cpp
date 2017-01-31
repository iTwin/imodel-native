/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_hatchblock.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

extern void mdlUtil_sortDoubles (double *, int, int);
static int hatchBlock_roundUp
(
double value
)
    {
    return (int)ceil (value);
    }


static int hatchBlock_roundDown
(
double value
)
    {
    return (int)floor (value);
    }


static bool    inverseRationalInterpolate
(
double *pLambda,
double x0,
double w0,
double x1,
double w1,
double xx,
double ww
)
    {
    double a0 = ww * x0 - xx * w0;
    double a1 = ww * x1 - xx * w1;
    return bsiTrig_safeDivide (pLambda, -a0, a1-a0, 0.0);
    }


typedef struct
    {
    DMap4d map;
    double y0[2];
    double dy;
    int numLevel;
    GraphicsPointArray *pBlockDef;
    } HatchBlockContext;

static void    registerCrossing
(
HatchBlockContext *pContext,
DPoint4d    *pPoint0,
DPoint4d    *pPoint1,
double      yk,
int         bottomParity,
int         topParity
)
    {
    DPoint3d point0, point1;
    double xmin, xmax;
    if (   bsiDPoint4d_normalize (pPoint0, &point0)
        && bsiDPoint4d_normalize (pPoint1, &point1)
        )
        {
        xmin = point0.x < point1.x ? point0.x : point1.x;
        xmax = point0.x > point1.x ? point0.x : point1.x;
        jmdlGraphicsPointArray_addComplete
                    (
                    pContext->pBlockDef,
                    xmin, yk, 0.0, 1.0,
                    xmax - xmin, bottomParity, topParity);
        }
    }
static void    addBlock
(
GraphicsPointArray *pGPA,
double x0,
double y0,
double dx
)
    {
    jmdlGraphicsPointArray_addComplete (pGPA, x0, y0, 0.0, 1.0, dx, 0, 0);
    }


/*---------------------------------------------------------------------------------**//**
* "Process" function for DSegment4d hatch block boundaries.
* conic and dispatch to more generic integrators.  (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbHatchBlockDSegment4d
(
        HatchBlockContext           *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DSegment4d                  *pSegment
)
    {
    DSegment4d localSegment;
    double kk[4];
    double minKK, maxKK;
    int minK, maxK, k;
    double yBase, yTop;
    double lambdaBase, lambdaTop;
    DPoint4d basePoint, topPoint;
    bsiDSegment4d_transformDMatrix4d (&localSegment, &pContext->map.M0, pSegment);
    kk[0] = (localSegment.point[0].y - localSegment.point[0].w * pContext->y0[0])
                    / (localSegment.point[0].w * pContext->dy);
    kk[1] = (localSegment.point[1].y - localSegment.point[1].w * pContext->y0[0])
                    / (localSegment.point[1].w * pContext->dy);
    kk[2] = (localSegment.point[0].y - localSegment.point[0].w * pContext->y0[1])
                    / (localSegment.point[0].w * pContext->dy);
    kk[3] = (localSegment.point[1].y - localSegment.point[1].w * pContext->y0[1])
                    / (localSegment.point[1].w * pContext->dy);
    bsiDoubleArray_minMax (&minKK, &maxKK, kk, 4);
    minK = hatchBlock_roundDown (minKK);
    maxK = hatchBlock_roundUp   (maxKK);

    for (k = minK; k <= maxK; k++)
        {
        yBase = pContext->y0[0] + k * pContext->dy;
        yTop  = pContext->y0[1] + k * pContext->dy;

        if (   inverseRationalInterpolate
                        (
                        &lambdaBase,
                        localSegment.point[0].y, localSegment.point[1].y,
                        localSegment.point[0].w, localSegment.point[1].w,
                        yBase, 1.0
                        )
            && inverseRationalInterpolate
                        (
                        &lambdaTop,
                        localSegment.point[0].y, localSegment.point[0].w,
                        localSegment.point[1].y, localSegment.point[1].w,
                        yTop, 1.0
                        )
            )
            {
            bsiDSegment4d_fractionParameterToDPoint4d (&localSegment, &basePoint, lambdaBase);
            bsiDSegment4d_fractionParameterToDPoint4d (&localSegment, &topPoint,  lambdaTop );
            /* Ugh... let's not worry about homogeneous wraparound. */
            if (    (lambdaBase < 0.0 && lambdaTop < 0.0)
                ||  (lambdaBase > 1.0 && lambdaTop > 1.0))
                {
                /* Bogus - not a real crossing */
                }
            else if (  lambdaBase >= 0.0 && lambdaBase <= 1.0
                    && lambdaTop  >= 0.0 && lambdaTop  >= 1.0
                    )
                {
                /* The line cuts all the way across the block */
                registerCrossing (pContext, &basePoint, &topPoint, yBase, 1, 1);
                }
            else if (  lambdaBase >= 0.0 && lambdaBase <= 1.0)
                {
                if (lambdaTop > 1.0)
                    registerCrossing (pContext, &basePoint, &topPoint, yBase, 0, 1);
                else if (lambdaTop < 0.0)
                    registerCrossing (pContext, &basePoint, &topPoint, yBase, 1, 0);
                }
            else if (  lambdaTop >= 0.0 && lambdaTop <= 1.0)
                {
                if (lambdaBase > 1.0)
                    registerCrossing (pContext, &basePoint, &topPoint, yBase, 0, 1);
                else if (lambdaBase < 0.0)
                    registerCrossing (pContext, &basePoint, &topPoint, yBase, 1, 0);
                }
            else
                {
                /* Both lambdas must be outside 0..1? */
                registerCrossing (pContext, &basePoint, &topPoint, yBase, 0, 0);
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for conic area integrals.  Repackage the
* conic and dispatch to more generic integrators.  (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbHatchBlockDConic4d
(
        HatchBlockContext           *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DConic4d                    *pConic
)
    {
    //DConic4d localConic;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for BezierDPoint4d area integrals.  Repackage the
* bezier and dispatch to more generic integrators. (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbHatchBlockBezierDPoint4d
(
        HatchBlockContext           *pContext,
        TaggedBezierDPoint4d        &bezier
)
    {
    //DPoint4d localPole[MAX_BEZIER_CURVE_ORDER];
    if (bezier.m_order <= MAX_BEZIER_CURVE_ORDER)
        {
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Convert list of blocked ranges to list of free ranges.
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extractFreeBlocks
(
GraphicsPointArray      *pOut,
GraphicsPointArray      *pIn
)
    {
    double oldY;
    double newY;
    double oldX1;
    double newX1;
    double newX0;
    GraphicsPoint *pInBuffer;
    int baselineParity;
    int numIn = jmdlGraphicsPointArray_getCount (pIn);
    int i;

    jmdlGraphicsPointArray_sortByYThenX (pIn);
    pInBuffer = jmdlGraphicsPointArray_getPtr (pIn, 0);

    if (numIn <= 0)
        return;
    oldY = pInBuffer[0].point.y - 10000.0;
    oldX1 = pInBuffer[0].point.x;
    baselineParity = 0;
    for (i = 0; i < numIn; i++)
        {
        newY = pInBuffer[i].point.y;
        newX0 = pInBuffer[i].point.x;
        if (newY != oldY)
            {
            oldY = newY;
            oldX1 = newX0;
            baselineParity = 0;
            }
        newX1 = pInBuffer[i].point.x + pInBuffer[i].a;
        if (newX0 > oldX1)
            {
            /* The gap from prior to current block has not geometry.
                Check parity of prior geometry and output if "inside" */
            if (baselineParity & 0x01)
                addBlock (pOut, oldX1, newY, newX0 - oldX1);
            }
        baselineParity += pInBuffer[i].mask;
        if (newX1 > oldX1)
            oldX1 = newX1;
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* Generate cross for a range of uniformly spaced y cuts, restricted so that the entire
* space vertically above each base line is clear of boundary geometry.
*
*
* Each output graphics point describes a hatch block.  The outpoint point is of the form
*     (x,y,z,w,a) = (x0, y0, 0,1,dx)
* where x0,y0 is a hatch block start in local coordinates and dx is the length of
* the hatch block base line.
*
* Output data is in the local coordinates of the transform -- multiply by the transform
* to get back to world.
*
* @param pCollector         => array of hatch blocks
* @param pMap               => transform from hatch space back to world.
*                               M0 is local to world, M1 is world to local.
* @param y0                 => first baseline y coordinate.
* @param y1                 => last baseline y coordinates.
* @param dy                 => thickness of hatch blocks.  Typically, but not
*                               necessarily, this is smaller than (y1 - y0)/numBaseline
* @param numBaseline        => number of baselines to compute.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addTransformedHatchBlocks
(
        GraphicsPointArray      *pCollector,
GraphicsPointArrayCP pBoundary,
const   DMap4d                      *pMap,
        double                      y0,
        double                      y1,
        double                      hy,
        int                         numBaseline
)
    {
    HatchBlockContext params;

    params.y0[0] = y0;
    params.y0[1] = y0 + hy;
    params.dy = (y1 - y0) / hy;

    params.map = *pMap;
    params.pBlockDef = jmdlGraphicsPointArray_grab ();

    jmdlGraphicsPointArray_processPrimitives
                    (
                    &params,
                    pBoundary,
                    (GPAFunc_DSegment4d)cbHatchBlockDSegment4d,
                    (GPAFunc_DConic4d)cbHatchBlockDConic4d,
                    (GPAFunc_BezierDPoint4dTagged)cbHatchBlockBezierDPoint4d
                    );

    extractFreeBlocks (pCollector, params.pBlockDef);
    jmdlGraphicsPointArray_drop (params.pBlockDef);
    }