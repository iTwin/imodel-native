/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_prop1.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Abstract base class for handling bezier curves and endpoints.
struct BezierHandler
{
GEOMAPI_VIRTUAL bool SelectPointsOnBezierCurve
    (
            double      *pParamArray,
            DPoint4d    *pPointArray,
            int         &numOut,
            int         maxOut,
    const   DPoint4d    *pPoleArray,
            int         order,
            bool        extend
    ) = 0;

GEOMAPI_VIRTUAL void HandleVertex
    (
    const   DPoint4d    *pVertex,
            int         primitiveIndex,
            double      primitiveFraction
    ) = 0;
};

typedef BezierHandler & BezierHandlerR;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
static     void    addRoots
(
        GraphicsPointArray  *pDest,
        int                     primitiveIndex,
const   DPoint4d                *pPoleArray,
        int                     order,
const   DConic4d                *pConic,
const   DPoint3d                *pTrigPoleArray,
        BezierHandlerR          handler,
        bool                    extend,
        bool                    testStart,
        bool                    testEnd
)
    {
    double paramArray[MAX_BEZIER_ORDER];
    DPoint4d pointArray[MAX_BEZIER_ORDER];
    DPoint3d trigArray[MAX_BEZIER_ORDER];
    int numPerp = 0;
    int i;
    if (handler.SelectPointsOnBezierCurve (paramArray, pointArray, numPerp, MAX_BEZIER_ORDER,
                    pPoleArray, order, extend))
        {

        if (pTrigPoleArray)
            bsiBezierDPoint3d_evaluateArray
                                (
                                trigArray, NULL,
                                pTrigPoleArray, order,
                                paramArray, numPerp
                                );

        for (i = 0; i < numPerp; i++)
            {
            double s, theta, fraction;
            DPoint4d finalPoint;
            s = paramArray[i];
            if (!pTrigPoleArray)
                {
                fraction = s;
                finalPoint = pointArray[i];
                }
            else
                {
                /* The point came from an conic.  Convert the bezier parameter
                        to angle, angle to fraction of conic,
                        and reevaluate the ellipse point (because the weighting
                        is different)
                */
                theta = atan2 (trigArray[i].y, trigArray[i].x);
                fraction = bsiDConic4d_angleParameterToFraction (pConic, theta);
                bsiDConic4d_angleParameterToDPoint4d (pConic, &finalPoint, theta);
                }

            jmdlGraphicsPointArray_addComplete (pDest,
                                finalPoint.x, finalPoint.y, finalPoint.z, finalPoint.w,
                                fraction, 0,primitiveIndex);
            jmdlGraphicsPointArray_markPoint (pDest);
            }
        }

    if (testStart)
        handler.HandleVertex  (&pPoleArray[0], primitiveIndex, 0.0);
    if (testEnd)
        handler.HandleVertex (&pPoleArray[order - 1], primitiveIndex, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* Generate bezier curve form of each source primitive.
* Pass bezier to handler function which returns (presumably after solving a polynomial)
* an array of interesting parameters.
* Save all the parameters in the destination array.
* Also pass true vertex points to test function
* @param pDest <= array to receive perpendicular points (with primitive and parameter in
*                       source array).
* @param pSource => array of path geoemtry.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    AddRootsPerPrimitive
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
        BezierHandlerR          handler,
        bool                    extend
)
    {
    int curr0, curr1;
    int readIndex;
    int curveType;
    size_t tailIndex;

    for (curr1 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &curr0, &curr1, NULL, NULL, &curveType, curr1);
        )
        {
        if (curveType == 0)
            {
            DSegment4d segment;
            DPoint4d poleArray[2];
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment))
                {
                poleArray[0] = segment.point[0];
                poleArray[1] = segment.point[1];
                addRoots
                    (pDest, curr0, poleArray, 2,
                    NULL, NULL, handler, extend,
                    true, true);
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            DConic4d conic, extendedConic;
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                                        NULL, NULL, NULL, NULL))
                {
                DPoint4d conicPoleArray[5];
                DPoint3d circlePoleArray[5];
                bool    testStart, testEnd;
                int numPole, numSpan;
                int i;
                extendedConic = conic;

            /* If extended geometry is requested, generate the bezier approximation
                over the whole circle.   However, pass only the original (partial)
                conic to addRoots, so it computes angular fractions relative to the
                trimmed range. */
                if (extend)
                    bsiDConic4d_makeFullSweep (&extendedConic);
                bsiDConic4d_getQuadricBezierPoles (&extendedConic,
                                    conicPoleArray,
                                    circlePoleArray,
                                    &numPole, &numSpan, 5);
                for (i = 0; i < numSpan; i++)
                    {
                    int k = 2 * i;
                    testStart = (i == 0);
                    testEnd   = (i == numSpan - 1);
                    addRoots (pDest, curr0, conicPoleArray + k, 3,
                                        &conic, circlePoleArray + k, handler, false,
                                        testStart, testEnd
                              );
                    }
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            if (readIndex = curr0,
                jmdlGraphicsPointArray_getBezier (pSource, &readIndex, poleArray, &order,
                                        MAX_BEZIER_ORDER))
                {
                addRoots (pDest, curr0, poleArray, order,
                                            NULL, NULL, handler, false,
                                            true, true);
                }
            }
        else if (pSource->IsBsplineCurve (curr0, tailIndex))
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            bool isNullInterval;
            double knot0, knot1;
            for (size_t spanIndex = 0;
                pSource->GetBezierSpanFromBsplineCurve (curr0, spanIndex, poleArray, order,
                        MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                {
                if (!isNullInterval)
                    {
                    addRoots (pDest, curr0, poleArray, order,
                                        NULL, NULL, handler, false,
                                        true, true);
                    }
                }
            }
        }
    }






// DCone3d setups ...
struct BezierHandler_DCone3dIntersections : BezierHandler
{
DCone3d mCone;
bool    mCapped;
BezierHandler_DCone3dIntersections (DCone3dCR cone, bool capped)
    {
    mCone = cone;
    mCapped = capped;
    }

bool SelectPointsOnBezierCurve
    (
            double      *pParamArray,
            DPoint4d    *pPointArray,
            int         &numOut,
            int         maxOut,
    const   DPoint4d    *pPoleArray,
            int         order,
            bool        extend
    ) override
    {
    double intersectionParam[MAX_BEZIER_ORDER];
    DPoint3d intersectionXYZ[MAX_BEZIER_ORDER];

    int numConeIntersection;
    int numCapIntersection;

    bsiDCone3d_intersectBezierCurve
                (
                &mCone,
                intersectionParam, intersectionXYZ, NULL,
                &numConeIntersection, &numCapIntersection, MAX_BEZIER_ORDER,
                pPoleArray, order, 2, 2);
    int n = numConeIntersection + numCapIntersection;
    numOut = n < maxOut ? n : maxOut;
    for (int i = 0; i < numOut; i++)
        {
        pParamArray[i] = intersectionParam[i];
        pPointArray[i].Init (intersectionXYZ[i], 1.0);
        }
    return numOut > 0;
    }

void HandleVertex
    (
    const   DPoint4d    *pVertex,
            int         primitiveIndex,
            double      primitiveFraction
    ) override
    {

    }
};

// DToroid3d setups ...
struct BezierHandler_DToroid3dIntersections : BezierHandler
{
DToroid3d mCone;
bool    mCapped;
BezierHandler_DToroid3dIntersections (DToroid3dCR torus, bool capped)
    {
    mCone = torus;
    mCapped = capped;
    }

bool SelectPointsOnBezierCurve
    (
            double      *pParamArray,
            DPoint4d    *pPointArray,
            int         &numOut,
            int         maxOut,
    const   DPoint4d    *pPoleArray,
            int         order,
            bool        extend
    ) override
    {
    double intersectionParam[MAX_BEZIER_ORDER];
    DPoint3d intersectionXYZ[MAX_BEZIER_ORDER];

    int numConeIntersection = 0;
    int numCapIntersection = 0;

    bsiDToroid3d_intersectBezierCurve
                (
                &mCone,
                intersectionParam, intersectionXYZ, NULL,
                &numConeIntersection, &numCapIntersection, MAX_BEZIER_ORDER,
                pPoleArray, order, 2, 1, false);

    int n = numConeIntersection + numCapIntersection;
    numOut = n < maxOut ? n : maxOut;
    for (int i = 0; i < numOut; i++)
        {
        pParamArray[i] = intersectionParam[i];
        pPointArray[i].Init (intersectionXYZ[i], 1.0);
        }
    return numOut > 0;
    }

void HandleVertex
    (
    const   DPoint4d    *pVertex,
            int         primitiveIndex,
            double      primitiveFraction
    ) override
    {

    }
};

/*---------------------------------------------------------------------------------**//**
* Intersect each primitive with a DCone3d.
* @param pDest <= array to receive intersection points, with primitive fraction parameter markup.
* @param pSource => array of path geoemtry.
* @param pCone => Cone to intersect
* @param pCapped => include cap intersections
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDCone3dIntersections
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
        DCone3dCP               pCone,
        bool                    capped
)
    {
    BezierHandler_DCone3dIntersections handler (*pCone, capped ? true : false);

    AddRootsPerPrimitive (pDest, pSource, handler, false);
    }

/*---------------------------------------------------------------------------------**//**
* Intersect each primitive with a DToroid3d.
* @param pDest <= array to receive intersection points, with primitive fraction parameter markup.
* @param pSource => array of path geoemtry.
* @param pToroid => Toroid to intersect
* @param pCapped => include cap intersections
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDToroid3dIntersections
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
        DToroid3dCP               pToroid,
        bool                    capped
)
    {
    BezierHandler_DToroid3dIntersections handler (*pToroid, capped ? true : false);

    AddRootsPerPrimitive (pDest, pSource, handler, false);
    }

END_BENTLEY_GEOMETRY_NAMESPACE