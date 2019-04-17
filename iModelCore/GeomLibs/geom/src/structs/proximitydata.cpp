/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* Initialize proximity test data for xy distance testing.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiProximityData_init

(
ProximityDataP pProx,
DPoint3dCP pTestPoint,
int         index,
double      param
)
    {
    pProx->closeIndex = index;
    pProx->closeParam = param;
    bsiDPoint4d_setComponents (&pProx->closePoint, 0.0, 0.0, 0.0, 1.0);
    pProx->testPoint = *pTestPoint;
    pProx->dataValid = false;
    }


/*---------------------------------------------------------------------------------**//**
* Update proximity test data with a new, possibly closer, test point.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiProximityData_testXY

(
ProximityDataP pProx,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
)
    {
    double newDistanceSquared;
    if (pProx && pNewPoint->RealDistanceSquaredXY (&newDistanceSquared, pProx->testPoint))
        {
        if (!pProx->dataValid || newDistanceSquared < pProx->closeDistanceSquared)
            {
            pProx->closeIndex = newIndex;
            pProx->closeParam = newParam;
            pProx->closePoint = *pNewPoint;
            pProx->closeDistanceSquared = newDistanceSquared;
            pProx->dataValid = true;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Update proximity test data with a new, possibly closer, test point.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiProximityData_test

(
ProximityDataP pProx,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
)
    {
    double newDistanceSquared;
    if (pProx && bsiDPoint4d_realDistanceSquaredDPoint3d (pNewPoint, &newDistanceSquared, &pProx->testPoint))
        {
        if (!pProx->dataValid || newDistanceSquared < pProx->closeDistanceSquared)
            {
            pProx->closeIndex = newIndex;
            pProx->closeParam = newParam;
            pProx->closePoint = *pNewPoint;
            pProx->closeDistanceSquared = newDistanceSquared;
            pProx->dataValid = true;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Update proximity test data with a new, possibly closer, test point.  Do the
* distance test on a different point from the one stored.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiProximityData_testProxy

(
ProximityDataP pProx,
DPoint4dCP pProxyPoint,
DPoint4dCP pNewPoint,
double    newParam,
int       newIndex
)
    {
    double newDistanceSquared;
    if (pProx && bsiDPoint4d_realDistanceSquaredDPoint3d (pProxyPoint, &newDistanceSquared, &pProx->testPoint))
        {
        if (!pProx->dataValid || newDistanceSquared < pProx->closeDistanceSquared)
            {
            pProx->closeIndex = newIndex;
            pProx->closeParam = newParam;
            pProx->closePoint = *pNewPoint;
            pProx->closeDistanceSquared = newDistanceSquared;
            pProx->dataValid = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Copy from proximity data structure to output args.
* (Assumes proximity data is initialized to copyable values even if dataValid
*   flag is false)
* @return dataValid flag from proximtity data.
* @indexVerb
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiProximityData_unload

(
ProximityDataP pProx,
int       *pCloseIndex,
double    *pCloseParam,
DPoint4dP pClosePoint,
DPoint3dP pClosePoint3d,
double    *pMinDistSquared
)
    {
    if (pProx->dataValid)
        {

        if (pCloseIndex)
            *pCloseIndex = pProx->closeIndex;

        if (pCloseParam)
            *pCloseParam = pProx->closeParam;

        if (pClosePoint)
            *pClosePoint = pProx->closePoint;

        if (pClosePoint3d)
            pProx->closePoint.GetProjectedXYZ (*pClosePoint3d);

        if (pMinDistSquared)
            *pMinDistSquared = pProx->closeDistanceSquared;

        }

    return pProx->dataValid;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
