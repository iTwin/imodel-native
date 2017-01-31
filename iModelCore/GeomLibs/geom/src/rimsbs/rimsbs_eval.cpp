/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/rimsbs/rimsbs_eval.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define IS_CURVEID(__id__) ((__id__) >= 0)

static bool    mappedCartesianDerivatives
(
MSBsplineCurve *pCurve,
DPoint3d *pXYZ,
int numDerivativesRequested,
double s,
double ds
)
    {
#define MAX_DERIVATIVE 10
    DPoint3d homogeneousPoles[10];
    double   dWeights[10];
    DPoint3d cDeriv[10];

    if (numDerivativesRequested == 0)
        {
        bspcurv_evaluateCurvePoint (&pXYZ[0], NULL, pCurve, s);
        }
    else if (numDerivativesRequested == 1)
        {
        bspcurv_evaluateCurvePoint (&pXYZ[0], &pXYZ[1], pCurve, s);
        }
    else if (numDerivativesRequested <= 3)
        {
        int numEval = numDerivativesRequested;
        if (numEval > 3)
            numEval = 3;
        if (SUCCESS != bspcurv_computeDerivatives (homogeneousPoles, dWeights, pCurve, 3, s, false))
            return false;

        if (pCurve->rational)
            {
            bspcurv_chainRule (cDeriv, homogeneousPoles, dWeights);
            memcpy (pXYZ, cDeriv, (1 + numEval) * sizeof(DPoint3d));
            }
        else
            {
            memcpy (pXYZ, homogeneousPoles, (1 + numEval) * sizeof(DPoint3d));
            }
        }
    else
        return false;

    jmdlRIMSBS_applyScalePowers (pXYZ, numDerivativesRequested, ds);
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_initPartialEllipse                               |
|                                                                       |
| Author:   EarlinLutz                               9/19/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     jmdlRIMSBS_initPartialEllipse
(
      DEllipse3d    *pInstance,
const DEllipse3d    *pParentEllipse,
      double        s0,
      double        s1
)
    {
    /* Copy changed angular parts to local vars to allow pInstance==pParentEllipse */
    double parentStart = pParentEllipse->start;
    double parentSweep = pParentEllipse->sweep;
    *pInstance = *pParentEllipse;
    pInstance->start = parentStart + parentSweep * s0;
    pInstance->sweep = (s1 - s0) * parentSweep;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMappedCurveRange
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_getMappedCurveRange
(
RIMSBS_Context  *pContext,
DRange3d        *pRange,
RIMSBS_CurveId  curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                DEllipse3d partialEllipse = *pEllipse;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, pEllipse, s0, s1);
                bsiDEllipse3d_getRange (&partialEllipse, pRange);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_getMappedCurveRange (pContext, pRange, pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = jmdlRIMSBS_getMappedCurveRange (pContext,
                                    pRange, parentCurveId, t0, t1);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
		*pRange = partialCurve.GetRange ();
                bspcurv_freeCurve (&partialCurve);
                myResult = true;
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getCurveRange                                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_getCurveRange
(
RIMSBS_Context  *pContext,
DRange3d        *pRange,
RIMSBS_CurveId  curveId
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                bsiDEllipse3d_getRange (pEllipse, pRange);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    myResult = jmdlRIMSBS_getCurveRange (pContext, pRange, pInterval->partialCurveId);
                else
                    myResult = jmdlRIMSBS_getMappedCurveRange (pContext,
                                pRange,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1
                                );
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
		*pRange = pCurve->GetRange ();
                myResult = true;
                break;
                }
            }
        }
    return  myResult;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_applyScalePowers                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void    jmdlRIMSBS_applyScalePowers
(
DPoint3d        *pXYZ,
int             numDerivatives,
double          scale
)
    {
    int i;
    double a;
    a = scale;
    for (i = 1; i <= numDerivatives; i++)
        {
        bsiDPoint3d_scaleInPlace (pXYZ + i, a);
        a *= scale;
        }
    }
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_scaleDPoint3dArrayInPlace                        |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void    jmdlRIMSBS_scaleDPoint3dArrayInPlace
(
DPoint3d        *pXYZ,
int             n,
double          scale
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        bsiDPoint3d_scaleInPlace (pXYZ + i, scale);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateMappedDerivatives                        |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_evaluateMappedDerivatives
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    double ds = s1 - s0;
    double s;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                double scale = ds * pEllipse->sweep;
                s = s0 + param * ds;
                bsiDEllipse3d_evaluateDerivativeArray
                        (
                        pEllipse,
                        pXYZ,
                        numDerivatives,
                        bsiDEllipse3d_fractionToAngle (pEllipse, s)
                        );

                jmdlRIMSBS_applyScalePowers (pXYZ, numDerivatives, scale);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_evaluateMappedDerivatives
                                    (
                                    pContext,
                                    pXYZ,
                                    numDerivatives,
                                    param,
                                    pInterval->partialCurveId,
                                    s0,
                                    s1
                                    );
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;

                    myResult = jmdlRIMSBS_evaluateMappedDerivatives
                                    (
                                    pContext,
                                    pXYZ,
                                    numDerivatives,
                                    param,
                                    parentCurveId,
                                    t0,
                                    t1
                                    );

                    jmdlRIMSBS_applyScalePowers (pXYZ, numDerivatives, dt);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                s = s0 + param * ds;
                myResult = mappedCartesianDerivatives (pCurve, pXYZ, numDerivatives, s, ds);
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateDerivatives                              |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_evaluateDerivatives
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                bsiDEllipse3d_evaluateDerivativeArray
                        (
                        pEllipse,
                        pXYZ,
                        numDerivatives,
                        bsiDEllipse3d_fractionToAngle (pEllipse, param)
                        );
                jmdlRIMSBS_applyScalePowers (pXYZ, numDerivatives, pEllipse->sweep);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_evaluateDerivatives (pContext, pXYZ, numDerivatives, param, pInterval->partialCurveId);
                    }
                else
                    {
                    myResult = jmdlRIMSBS_evaluateMappedDerivatives
                                (
                                pContext,
                                pXYZ,
                                numDerivatives,
                                param,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                myResult = mappedCartesianDerivatives (pCurve, pXYZ, numDerivatives, param, 1.0);
                break;
                }

            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateMapped                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_evaluateMapped
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    double ds = s1 - s0;
    double s;
    int i;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                double scale = ds * pEllipse->sweep;
                for (i = 0; i < nParam; i++)
                    {
                    s = s0 + pParam[i] * ds;
                    bsiDEllipse3d_evaluateDerivatives
                            (
                            pEllipse,
                            pXYZ ? &pXYZ[i] : NULL,
                            pTangent ? &pTangent[i] : NULL,
                            NULL,
                            bsiDEllipse3d_fractionToAngle (pEllipse, s)
                            );
                    }
                if (pTangent)
                    jmdlRIMSBS_scaleDPoint3dArrayInPlace (pTangent, nParam, scale);

                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_evaluateMapped
                                    (
                                    pContext,
                                    pXYZ,
                                    pTangent,
                                    pParam,
                                    nParam,
                                    pInterval->partialCurveId,
                                    s0,
                                    s1
                                    );
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = jmdlRIMSBS_evaluateMapped
                                    (
                                    pContext,
                                    pXYZ,
                                    pTangent,
                                    pParam,
                                    nParam,
                                    parentCurveId,
                                    t0,
                                    t1
                                    );
                    if (pTangent)
                        jmdlRIMSBS_scaleDPoint3dArrayInPlace (pTangent, nParam, dt);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                for (i = 0; i < nParam; i++)
                    {
                    s = s0 + pParam[i] * ds;
                    bspcurv_evaluateCurvePoint (
                                    pXYZ     ? pXYZ + i     : NULL,
                                    pTangent ? pTangent + i : NULL,
                                    pCurve,
                                    s);
                    }

                if (pTangent)
                    jmdlRIMSBS_scaleDPoint3dArrayInPlace (pTangent, nParam, ds);
                myResult = true;
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluate                                         |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_evaluate
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    int i;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                for (i = 0; i < nParam; i++)
                    {
                    bsiDEllipse3d_evaluateDerivatives
                            (
                            pEllipse,
                            pXYZ ? &pXYZ[i] : NULL,
                            pTangent ? &pTangent[i] : NULL,
                            NULL,
                            bsiDEllipse3d_fractionToAngle (pEllipse, pParam[i])
                            );
                    }
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    myResult = jmdlRIMSBS_evaluate (pContext, pXYZ, pTangent, pParam, nParam, pInterval->partialCurveId);
                else
                    myResult = jmdlRIMSBS_evaluateMapped
                                (
                                pContext,
                                pXYZ,
                                pTangent,
                                pParam,
                                nParam,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1);
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                for (i = 0; i < nParam; i++)
                    bspcurv_evaluateCurvePoint (
                                    pXYZ     ? pXYZ + i     : NULL,
                                    pTangent ? pTangent + i : NULL,
                                    pCurve,
                                    pParam[i]);
                myResult = true;
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getCurveInterval                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getCurveInterval
(
RIMSBS_Context  *pContext,
RG_CurveId      *pParentCurveId,
double          *pStartFraction,
double          *pEndFraction,
RG_CurveId      curveId,
bool            reversed
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    *pParentCurveId = RIMSBS_NULL_CURVE_ID;

    *pStartFraction = 0.0;
    *pEndFraction   = 1.0;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                *pParentCurveId = pInterval->parentId;
                *pStartFraction = pInterval->s0;
                *pEndFraction   = pInterval->s1;
                myResult = true;
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                *pParentCurveId = curveId;
                *pStartFraction = 0.0;
                *pEndFraction   = 1.0;
                myResult = true;
                break;
                }
            }
        }
    if (myResult && reversed)
        {
        double temp = *pStartFraction;
        *pStartFraction = *pEndFraction;
        *pEndFraction = temp;
        }
    return  myResult;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getDEllipse3d                                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getDEllipse3d
(
RIMSBS_Context  *pContext,
DEllipse3d      *pEllipse,
RG_CurveId      curveId,
bool            reversed
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (curveId != RG_NULL_CURVEID
        && jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pOriginalEllipse = (DEllipse3d *)desc.pGeometryData;
                *pEllipse = *pOriginalEllipse;
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (jmdlRIMSBS_getDEllipse3d (pContext, pEllipse, pInterval->partialCurveId, false))
                    {
                    myResult = true;
                    }
                else if (jmdlRIMSBS_getDEllipse3d (pContext, pEllipse, pInterval->parentId, false))
                    {
                    double theta0 = bsiDEllipse3d_fractionToAngle (pEllipse, pInterval->s0);
                    double theta1 = bsiDEllipse3d_fractionToAngle (pEllipse, pInterval->s1);
                    bsiDEllipse3d_setLimits (pEllipse, theta0, theta1);
                    myResult = true;
                    }
                break;
                }
            case RIMSBS_CurveChain:
                {
                RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct *)desc.pGeometryData;
                RG_CurveId primaryCurveId = pChain->m_primaryCurveId;
                myResult = jmdlRIMSBS_getDEllipse3d (pContext, pEllipse, primaryCurveId, false);
                break;
                }
            }
        }
    if (myResult && reversed)
        bsiDEllipse3d_initReversed (pEllipse, pEllipse);
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMappedMSBsplineCurve                          |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getMappedMSBsplineCurve
(
RIMSBS_Context  *pContext,
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                DEllipse3d partialEllipse = *pEllipse;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, pEllipse, s0, s1);
                bspconv_convertDEllipse3dToCurve (pCurve, &partialEllipse);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                /* Recurse to a (remapped) part of the referenced curve */
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId)
                    && (myResult = jmdlRIMSBS_getMappedMSBsplineCurve (pContext, pCurve, pInterval->partialCurveId, s0, s1))
                    )
                    {
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    myResult = jmdlRIMSBS_getMappedMSBsplineCurve (pContext, pCurve, pInterval->parentId, t0, t1);
                    }
                break;
                }
            case RIMSBS_CurveChain:
                {
                RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct *)desc.pGeometryData;
                RG_CurveId primaryCurveId = pChain->m_primaryCurveId;
                myResult = jmdlRIMSBS_getMappedMSBsplineCurve (pContext, pCurve, primaryCurveId, s0, s1);
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pStoredCurve = (MSBsplineCurve *)desc.pGeometryData;
                StatusInt status = ERROR;
                if (s0 == 0.0 && s1 == 1.0)
                    {
                    status = bspcurv_copyCurve (pCurve, pStoredCurve);
                    }
                else
                    {
                    status = bspcurv_segmentCurve (pCurve, pStoredCurve, s0, s1);
                    }
                myResult = status == SUCCESS;
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMSBsplineCurve                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getMSBsplineCurve
(
RIMSBS_Context  *pContext,
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
bool            reversed
)
    {
    double s0 = reversed ? 1.0 : 0.0;
    double s1 = reversed ? 0.0 : 1.0;
    return jmdlRIMSBS_getMappedMSBsplineCurve (pContext, pCurve, curveId, s0, s1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_sweptPopertiesMapped                             |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_sweptPopertiesMapped
(
RIMSBS_Context  *pContext,
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint,
int             curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                DEllipse3d partialEllipse = *pEllipse;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, pEllipse, s0, s1);
                bsiDEllipse3d_xySweepProperties (&partialEllipse, pArea, pAngle, pPoint);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_sweptPopertiesMapped (pContext, pArea, pAngle, pPoint, pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = jmdlRIMSBS_sweptPopertiesMapped
                                    (
                                    pContext,
                                    pArea,
                                    pAngle,
                                    pPoint,
                                    parentCurveId,
                                    t0,
                                    t1
                                    );
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                double area = 0.0, angle = 0.0, dArea = 0.0, dAngle = 0.0;
                static double relTol_angle = 1.0e-6;
                static double relTol_area  = 1.0e-8;
                if (    (!pArea  || SUCCESS == bspcurv_sweptArea (&area, &dArea, s0, s1, pCurve, pPoint, relTol_area))
                    &&  (!pAngle || SUCCESS == bspcurv_sweptAngle (&angle, &dAngle, s0, s1, pCurve, pPoint, relTol_angle)))
                    {
                    if (pArea)
                        *pArea = area;
                    if (pAngle)
                        *pAngle = angle;
                    myResult = true;
                    }
                break;
                }

            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_sweptProperties                                  |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_sweptProperties
(
RIMSBS_Context  *pContext,
RG_EdgeData     *pEdgeData,
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    int curveId = jmdlRGEdge_getCurveId (pEdgeData);

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                bsiDEllipse3d_xySweepProperties (pEllipse, pArea, pAngle, pPoint);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    // hmmm..  we'd normally recurse to ourself, but the curveId arg is hidden in the EdgeData, so we have 
                    // to recurse on the mapped evaluator with whole interval.
                    myResult = jmdlRIMSBS_sweptPopertiesMapped (pContext, pArea, pAngle, pPoint, pInterval->partialCurveId, 0.0, 1.0);
                    }
                else
                    myResult = jmdlRIMSBS_sweptPopertiesMapped
                                (
                                pContext,
                                pArea,
                                pAngle,
                                pPoint,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1
                                );
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                double area = 0.0, angle = 0.0, dArea, dAngle;
                static double relTol_angle = 1.0e-6;
                static double relTol_area  = 1.0e-8;
                if (    (!pArea  || SUCCESS == bspcurv_sweptArea (&area, &dArea, 0.0, 1.0, pCurve, pPoint, relTol_area))
                    &&  (!pAngle || SUCCESS == bspcurv_sweptAngle (&angle, &dAngle, 0.0, 1.0, pCurve, pPoint, relTol_angle)))
                    {
                    if (pArea)
                        *pArea = area;
                    if (pAngle)
                        *pAngle = angle;
                    myResult = true;
                    }
                break;
                }
            }
        }
    if (myResult && jmdlRGEdge_isReversed (pEdgeData))
        {
        if (pArea)
            *pArea =    -(*pArea);
        if (pAngle)
            *pAngle = -(*pAngle);
        }

    return  myResult;
    }

static RG_CurveId jmdlRIMSBS_resolveThroughPartialCurve
(
RIMSBS_Context          *pContext,
RG_CurveId              curveId
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, curveId))
        {
        if (header.type == RIMSBS_CurveInterval)
            {
            RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)header.pGeometryData;
            if (IS_CURVEID(pInterval->partialCurveId))
                return pInterval->partialCurveId;
            }
        }
    return curveId;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCurveIntersection                           |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_curveCurveIntersection
(
RIMSBS_Context          *pContext,          /* => general context */
RG_Header               *pRG,               /* => receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
RG_EdgeData             *pEdgeData0,        /* => segment edge data */
RG_EdgeData             *pEdgeData1         /* => curve edge data */
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc0, desc1;
    int curve0Id = jmdlRIMSBS_resolveThroughPartialCurve (pContext, jmdlRGEdge_getCurveId (pEdgeData0));
    int curve1Id = jmdlRIMSBS_resolveThroughPartialCurve (pContext, jmdlRGEdge_getCurveId (pEdgeData1));
    int type0, type1;


    if (   jmdlRIMSBS_getElementHeader (pContext, &desc0, curve0Id)
        && jmdlRIMSBS_getElementHeader (pContext, &desc1, curve1Id)
       )
        {
        type0 = desc0.type;
        type1 = desc1.type;

        if (curve0Id == curve1Id)
            {
            if (type0 == RIMSBS_MSBsplineCurve)
                {
                MSBsplineCurve *pCurve0 = (MSBsplineCurve *)desc0.pGeometryData;
                jmdlRIMSBS_MSBsplineCurveSelfIntersection
                        (
                        pContext,
                        pRG,
                        pIntersections,
                        pEdgeData0,
                        pCurve0
                        );
                myResult = true;
                }
            else if (type0 == RIMSBS_DEllipse3d)
                {
                jmdlRIMSBS_checkClosedEdge (pContext, pRG, pIntersections, pEdgeData0);
                myResult = true;
                }
            else
                {
                myResult = true;
                }
            }
        else if  (type0 == RIMSBS_DEllipse3d && type1 == RIMSBS_DEllipse3d)
            {
            DEllipse3d *pEllipse0 = (DEllipse3d *)desc0.pGeometryData;
            DEllipse3d *pEllipse1 = (DEllipse3d *)desc1.pGeometryData;
            jmdlRIMSBS_ellipseEllipseIntersection
                    (
                    pContext,
                    pRG,
                    pIntersections,
                    pEdgeData0,
                    pEllipse0,
                    pEdgeData1,
                    pEllipse1
                    );
            myResult = true;
            }

        else if  (type0 == RIMSBS_MSBsplineCurve && type1 == RIMSBS_MSBsplineCurve)
            {
            MSBsplineCurve *pCurve0 = (MSBsplineCurve *)desc0.pGeometryData;
            MSBsplineCurve *pCurve1 = (MSBsplineCurve *)desc1.pGeometryData;
            jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection
                    (
                    pContext,
                    pRG,
                    pIntersections,
                    pEdgeData0,
                    pCurve0,
                    pEdgeData1,
                    pCurve1
                    );
            myResult = true;
            }
        else if  (type0 == RIMSBS_DEllipse3d && type1 == RIMSBS_MSBsplineCurve)
            {
            DEllipse3d *pEllipse0 = (DEllipse3d *)desc0.pGeometryData;
            MSBsplineCurve *pCurve1 = (MSBsplineCurve *)desc1.pGeometryData;
            jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
                    (
                    pContext,
                    pRG,
                    pIntersections,
                    pEdgeData1,
                    pCurve1,
                    pEdgeData0,
                    pEllipse0,
                    NULL
                    );
            myResult = true;
            }
        else if  (type0 == RIMSBS_MSBsplineCurve && type1 == RIMSBS_DEllipse3d)
            {
            MSBsplineCurve *pCurve0 = (MSBsplineCurve *)desc0.pGeometryData;
            DEllipse3d *pEllipse1 = (DEllipse3d *)desc1.pGeometryData;
            jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
                    (
                    pContext,
                    pRG,
                    pIntersections,
                    pEdgeData0,
                    pCurve0,
                    pEdgeData1,
                    pEllipse1,
                    NULL
                    );
            myResult = true;
            }
        else
            {
            myResult = false;
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentCurveIntersectionMapped                   |
|                                                                       |
| Author:   EarlinLutz                               9/19/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_segmentCurveIntersectionMapped
(
RIMSBS_Context          *pContext,          /* => general context */
RG_Header               *pRG,               /* => receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
RG_EdgeData             *pEdgeData0,        /* => segment edge data */
RG_EdgeData             *pEdgeData1,        /* => curve edge data, known to be mapped */
int                     parentCurveId,
double                  s0,
double                  s1
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, parentCurveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;

                DEllipse3d partialEllipse = *pEllipse;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, pEllipse, s0, s1);

                jmdlRIMSBS_segmentEllipseIntersection
                        (
                        pContext,
                        pRG,
                        pIntersections,
                        pEdgeData0,
                        pEdgeData1,
                        &partialEllipse
                        );
                myResult = true;
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
                jmdlRIMSBS_segmentMSBSplineCurveIntersection
                            (pContext, pRG, pIntersections, pEdgeData0, pEdgeData1, &partialCurve);
                bspcurv_freeCurve (&partialCurve);
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped
                                (
                                pContext,
                                pRG,
                                pIntersections,
                                pEdgeData0,
                                pEdgeData1,
                                pInterval->partialCurveId,
                                s0,
                                s1
                                );
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped
                                (
                                pContext,
                                pRG,
                                pIntersections,
                                pEdgeData0,
                                pEdgeData1,
                                pInterval->parentId,
                                t0,
                                t1
                                );
                    }
                }
                break;
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentCurveIntersection                         |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_segmentCurveIntersection
(
RIMSBS_Context          *pContext,          /* => general context */
RG_Header               *pRG,               /* => receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
RG_EdgeData     *pEdgeData0,                /* => segment edge data */
RG_EdgeData     *pEdgeData1                 /* => curve edge data */
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    int curveId = jmdlRGEdge_getCurveId (pEdgeData1);

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                jmdlRIMSBS_segmentEllipseIntersection
                        (
                        pContext,
                        pRG,
                        pIntersections,
                        pEdgeData0,
                        pEdgeData1,
                        pEllipse
                        );
                myResult = true;
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                jmdlRIMSBS_segmentMSBSplineCurveIntersection
                            (pContext, pRG, pIntersections, pEdgeData0, pEdgeData1, pCurve);
                break;
                }
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID (pInterval->partialCurveId))
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped (pContext, pRG, pIntersections,
                                pEdgeData0, pEdgeData1, pInterval->partialCurveId, 0.0, 1.0);
                else
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped
                                (
                                pContext,
                                pRG,
                                pIntersections,
                                pEdgeData0,
                                pEdgeData1,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1
                                );
                break;
                }
            }
        }
    return  myResult;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getClosestXYPointOnMappedCurve                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getClosestXYPointOnMappedCurve
(
RIMSBS_Context  *pContext,
double   *pMinParam,        /* => parameter at closest approach point */
double   *pMinDistSquared,  /* => squard distance to closest approach point */
DPoint3d *pMinPoint,        /* => closest approach point */
DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
DPoint3d *pPoint,           /* => space point */
RG_CurveId  curveId,        /* => curve identifier */
double    s0,               /* => start of active interval */
double    s1                /* => end param for active interval */
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;
    double param;
    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                DEllipse3d partialEllipse = *pEllipse;
                double theta;
                partialEllipse.start = pEllipse->start + pEllipse->sweep * s0;
                partialEllipse.sweep = (s1 - s0) * pEllipse->sweep;
                if (bsiDEllipse3d_closestPointXYBounded (&partialEllipse,
                                        &theta, pMinDistSquared, pMinPoint, pPoint)
                    )
                    {
                    param = bsiTrig_normalizeAngleToSweep (
                                            theta,
                                            partialEllipse.start,
                                            partialEllipse.sweep);
                    if (pMinParam)
                        *pMinParam = param;
                    if (pMinTangent)
                        {
                        DPoint3d angularTangent;
                        bsiDEllipse3d_evaluateDerivatives (&partialEllipse,
                                            NULL, &angularTangent, NULL, theta);
                        bsiDPoint3d_scale (pMinTangent, &angularTangent, partialEllipse.sweep);
                        }
                    myResult = true;
                    }
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_getClosestXYPointOnMappedCurve (pContext,
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = jmdlRIMSBS_getClosestXYPointOnMappedCurve (pContext,
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                parentCurveId, t0, t1);
                    }

                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                double ds = s1 - s0;
                DPoint3d xyz[2];
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                myResult = bspcurv_closestXYPoint (
                            pMinPoint, &param, pMinDistSquared,
                            pCurve, s0, s1,
                            pPoint->x, pPoint->y);
                if (pMinParam)
                    *pMinParam = (param - s0) / ds;
                if (myResult && pMinTangent)
                    {
                    jmdlRIMSBS_evaluateDerivatives (pContext, xyz, 1, param, curveId);
                    *pMinTangent = xyz[1];
                    bsiDPoint3d_scale (pMinTangent, pMinTangent, ds);
                    }
                break;
                }
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getClosestXYPointOnCurve                         |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getClosestXYPointOnCurve
(
RIMSBS_Context  *pContext,
double   *pMinParam,        /* => parameter at closest approach point */
double   *pMinDistSquared,  /* => squard distance to closest approach point */
DPoint3d *pMinPoint,        /* => closest approach point */
DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
DPoint3d *pPoint,           /* => space point */
RG_CurveId  curveId         /* => curve identifier */
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc;

    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveId))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                double theta, param;
                if (bsiDEllipse3d_closestPointXYBounded (pEllipse,
                                        &theta, pMinDistSquared, pMinPoint, pPoint)
                    )
                    {
                    param = bsiTrig_normalizeAngleToSweep (
                                            theta,
                                            pEllipse->start,
                                            pEllipse->sweep);
                    if (pMinParam)
                        *pMinParam = param;

                    if (pMinTangent)
                        {
                        DPoint3d angularTangent;
                        bsiDEllipse3d_evaluateDerivatives (pEllipse,
                                            NULL, &angularTangent, NULL, theta);
                        bsiDPoint3d_scale (pMinTangent, &angularTangent, pEllipse->sweep);
                        }
                    }
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)desc.pGeometryData;
                if (IS_CURVEID(pInterval->partialCurveId))
                    myResult = jmdlRIMSBS_getClosestXYPointOnCurve (pContext,
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->partialCurveId);
                else 
                    myResult = jmdlRIMSBS_getClosestXYPointOnMappedCurve (pContext,
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1
                                );
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                double param;
                DPoint3d xyz[2];
                myResult = bspcurv_closestXYPoint (
                            pMinPoint, &param, pMinDistSquared,
                            pCurve, 0.0, 1.0,
                            pPoint->x, pPoint->y);
                if (pMinParam)
                    *pMinParam = param;
                if (myResult && pMinTangent)
                    {
                    jmdlRIMSBS_evaluateDerivatives (pContext, xyz, 1, param, curveId);
                    *pMinTangent = xyz[1];
                    }
                break;
                }

            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_appendAllCurveSamplePoints                       |
|                                                                       |
| Author:   EarlinLutz                               12/10/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void    jmdlRIMSBS_appendAllCurveSamplePoints
(
RIMSBS_Context  *pContext,
EmbeddedDPoint3dArray *pXYZArray
)
    {
    RIMSBS_ElementHeader desc;
    static int s_numPerEllipse = 4;
    int curveIndex;
    DPoint3d point;
    int i;

    for (curveIndex = 0;
        jmdlRIMSBS_getElementHeader (pContext, &desc, curveIndex);
        curveIndex++)
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                double df = pEllipse->sweep / (double)(s_numPerEllipse - 1);
                double f;
                for (i = 0; i < s_numPerEllipse; i++)
                    {
                    f = df * i;
                    bsiDEllipse3d_evaluateDerivatives (pEllipse,
                                        &point, NULL, NULL,
                                        bsiDEllipse3d_fractionToAngle (pEllipse, f));
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &point);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                int n = pCurve->params.numPoles;
                if (pCurve->rational)
                    {
                    for (i = 0; i < n; i++)
                        {
                        if (bsiDPoint3d_safeDivide (&point, &pCurve->GetPoleCP ()[i], pCurve->weights[i]))
                            jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &point);
                        }
                    }
                else
                    {
                    jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXYZArray, pCurve->GetPoleCP (), n);
                    }
                break;
                }
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_transformCurve                                   |
|                                                                       |
| Author:   EarlinLutz                               12/10/01           |
|                                                                       |
| Transform a single curve.  No action for subcurves -- only parents    |
| can be transformed!!!                                                 |
+----------------------------------------------------------------------*/
Public bool            jmdlRIMSBS_transformCurve
(
RIMSBS_Context          *pContext,
int                     curveIndex,
const Transform         *pTransform
)
    {
    RIMSBS_ElementHeader desc;
    bool    bResult = false;
    if (jmdlRIMSBS_getElementHeader (pContext, &desc, curveIndex))
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)desc.pGeometryData;
                bsiTransform_multiplyDPoint3dInPlace (pTransform, &pEllipse->center);
                bsiTransform_multiplyDPoint3dByMatrixPartInPlace (pTransform, &pEllipse->vector0);
                bsiTransform_multiplyDPoint3dByMatrixPartInPlace (pTransform, &pEllipse->vector90);
                bResult = true;
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)desc.pGeometryData;
                pCurve->TransformCurve (*pTransform);
                bResult = true;
                break;
                }
            }
        }
    return bResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_transformCurve                                   |
|                                                                       |
| Author:   EarlinLutz                               12/10/01           |
|                                                                       |
| Transform all curves in place.                                        |
+----------------------------------------------------------------------*/
Public void            jmdlRIMSBS_transformAllCurves
(
RIMSBS_Context          *pContext,
const Transform         *pTransform
)
    {
    RIMSBS_ElementHeader desc;
    int curveIndex;

    for (curveIndex = 0;
        jmdlRIMSBS_getElementHeader (pContext, &desc, curveIndex);
        curveIndex++)
        {
        jmdlRIMSBS_transformCurve (pContext, curveIndex, pTransform);
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE