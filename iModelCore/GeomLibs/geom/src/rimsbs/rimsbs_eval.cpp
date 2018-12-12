/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/rimsbs/rimsbs_eval.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    return pContext->TryGetMappedCurveRange (*pRange, curveId, s0, s1);
    }

bool    RIMSBS_Context::TryGetMappedCurveRange
(
DRange3dR       range,
RIMSBS_CurveId  curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                DEllipse3d partialEllipse = arc;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, &arc, s0, s1);
                partialEllipse.GetRange (range);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = TryGetMappedCurveRange (range, pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = TryGetMappedCurveRange (range, parentCurveId, t0, t1);
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
		        range = partialCurve.GetRange ();
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
    return pContext->TryGetRange (curveId, *pRange);
    }

bool RIMSBS_Context::TryGetRange (int curveIndex, DRange3dR range)
    {
    if (IsValidCurveIndex (curveIndex))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveIndex);
        switch (desc.type)
            {
            case RIMSBS_MSBsplineCurve:
            case RIMSBS_DEllipse3d:
                {
                return desc.curve->GetRange (range);
                }
            case RIMSBS_CurveInterval:
                {
                if (IS_CURVEID (desc.m_partialCurve.partialCurveId))
                    return TryGetRange (desc.m_partialCurve.partialCurveId, range);
                else
                    return jmdlRIMSBS_getMappedCurveRange (this,
                                &range,
                                desc.m_partialCurve.parentId,
                                desc.m_partialCurve.s0,
                                desc.m_partialCurve.s1
                                );
                }
            }
        }
    range.Init ();
    return false;
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
        pXYZ[ i].Scale (a);
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
        pXYZ[ i].Scale (scale);
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
    return pContext->TryEvaluateMappedDerivatives (pXYZ, numDerivatives, param, curveId, s0, s1);
    }

bool RIMSBS_Context::TryEvaluateMappedDerivatives
(
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    double ds = s1 - s0;
    double s;

    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                double scale = ds * arc.sweep;
                s = s0 + param * ds;
                arc.Evaluate (
                        pXYZ,
                        numDerivatives,
                        arc.FractionToAngle (s)
                        );

                jmdlRIMSBS_applyScalePowers (pXYZ, numDerivatives, scale);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = TryEvaluateMappedDerivatives
                                    (
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

                    myResult = TryEvaluateMappedDerivatives
                                    (
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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
    return pContext->TryEvaluateDerivatives (pXYZ, numDerivatives, param, curveId);
    }

bool RIMSBS_Context::TryEvaluateDerivatives
(
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId
)
    {
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);

                arc.Evaluate
                        (
                        pXYZ,
                        numDerivatives,
                        arc.FractionToAngle (param)
                        );
                jmdlRIMSBS_applyScalePowers (pXYZ, numDerivatives, arc.sweep);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = TryEvaluateDerivatives (pXYZ, numDerivatives, param, pInterval->partialCurveId);
                    }
                else
                    {
                    myResult = TryEvaluateMappedDerivatives
                                (
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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
    return pContext->TryEvaluateMapped (pXYZ, pTangent, pParam, nParam, curveId, s0, s1);
    }

bool        RIMSBS_Context::TryEvaluateMapped
(
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    double ds = s1 - s0;
    double s;
    DPoint3d xyzA;
    DVec3d vectorA1;
    DVec3d vectorA2;
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                double scale = ds * arc.sweep;
                for (int i = 0; i < nParam; i++)
                    {
                    s = s0 + pParam[i] * ds;
                    arc.Evaluate
                            (
                            pXYZ ? pXYZ[i] : xyzA,
                            pTangent ? ((DVec3d*)pTangent)[i] : vectorA1,
                            vectorA2,
                            arc.FractionToAngle (s)
                            );
                    }
                if (pTangent)
                    jmdlRIMSBS_scaleDPoint3dArrayInPlace (pTangent, nParam, scale);

                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    myResult = TryEvaluateMapped (
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
                    myResult = TryEvaluateMapped (
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
                for (int i = 0; i < nParam; i++)
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
    return pContext->TryEvaluate (pXYZ, pTangent, pParam, nParam, curveId);
    }

bool RIMSBS_Context::TryEvaluate
(
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId
)
    {
    bool    myResult = false;
    DPoint3d pointA;
    DVec3d vectorA1, vectorA2;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                for (int i = 0; i < nParam; i++)
                    {
                    arc.Evaluate (
                            pXYZ ? pXYZ[i] : pointA,
                            pTangent ? ((DVec3d*)pTangent)[i] : vectorA1,
                            vectorA2,
                            arc.FractionToAngle (pParam[i])
                            );
                    }
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    myResult = TryEvaluate (pXYZ, pTangent, pParam, nParam, pInterval->partialCurveId);
                else
                    myResult = jmdlRIMSBS_evaluateMapped
                                (
                                this,
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
                for (int i = 0; i < nParam; i++)
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
    return pContext->TryGetCurveInterval (pParentCurveId, pStartFraction, pEndFraction, curveId, reversed);
    }

bool RIMSBS_Context::TryGetCurveInterval
(
RG_CurveId      *pParentCurveId,
double          *pStartFraction,
double          *pEndFraction,
RG_CurveId      curveId,
bool            reversed
)
    {
    *pParentCurveId = RIMSBS_NULL_CURVE_ID;
    *pStartFraction = 0.0;
    *pEndFraction   = 1.0;

    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
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
    return pContext->TryGetResolvedArc (*pEllipse, curveId, reversed);
    }

bool RIMSBS_Context::TryGetResolvedArc
(
DEllipse3dR      arc,
RG_CurveId      curveId,
bool            reversed
)
    {
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                myResult = TryGetArc (curveId, arc);
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (TryGetResolvedArc (arc, pInterval->partialCurveId, false))
                    {
                    myResult = true;
                    }
                else if (TryGetResolvedArc (arc, pInterval->parentId, false))
                    {
                    double theta0 = arc.FractionToAngle (pInterval->s0);
                    double theta1 = arc.FractionToAngle (pInterval->s1);
                    arc.SetLimits (theta0, theta1);
                    myResult = true;
                    }
                break;
                }
            case RIMSBS_CurveChain:
                {
                RG_CurveId primaryCurveId = desc.m_chainData.m_primaryCurveId;
                myResult = TryGetResolvedArc (arc, primaryCurveId, false);
                break;
                }
            }
        }
    if (myResult && reversed)
        arc = DEllipse3d::FromReversed (arc);
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMappedMSBsplineCurve                          |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool jmdlRIMSBS_getMappedMSBsplineCurve
(
RIMSBS_Context  *pContext,
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    return pContext->TryGetMappedMSBsplineCurve (pCurve, curveId, s0, s1);
    }

bool RIMSBS_Context::TryGetMappedMSBsplineCurve
(
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                DEllipse3d partialEllipse = arc;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, &arc, s0, s1);
                bspconv_convertDEllipse3dToCurve (pCurve, &partialEllipse);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                /* Recurse to a (remapped) part of the referenced curve */
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId)
                    && (myResult = jmdlRIMSBS_getMappedMSBsplineCurve (this, pCurve, pInterval->partialCurveId, s0, s1))
                    )
                    {
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    myResult = jmdlRIMSBS_getMappedMSBsplineCurve (this, pCurve, pInterval->parentId, t0, t1);
                    }
                break;
                }
            case RIMSBS_CurveChain:
                {
                RG_CurveId primaryCurveId = desc.m_chainData.m_primaryCurveId;
                myResult = jmdlRIMSBS_getMappedMSBsplineCurve (this, pCurve, primaryCurveId, s0, s1);
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pStoredCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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
    return pContext->TrySweptPropertiesMapped (pArea, pAngle, pPoint, curveId, s0, s1);
    }

bool        RIMSBS_Context::TrySweptPropertiesMapped
(
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint,
int             curveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                DEllipse3d partialEllipse = arc;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, &arc, s0, s1);
                double area1, angle1;
                DPoint3d point1;
                partialEllipse.XySweepProperties (
                        pArea ? *pArea : area1,
                        pAngle ? *pAngle : angle1,
                        pPoint ? *pPoint : point1);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_sweptPopertiesMapped (this, pArea, pAngle, pPoint, pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = jmdlRIMSBS_sweptPopertiesMapped
                                    (
                                    this,
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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
    return pContext->TrySweptProperties (pEdgeData, pArea, pAngle, pPoint);
    }

bool RIMSBS_Context::TrySweptProperties
(
RG_EdgeData     *pEdgeData,
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint
)
    {
    bool    myResult = false;
    int curveId = jmdlRGEdge_getCurveId (pEdgeData);
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                double area1, angle1;
                DPoint3d point1;
                arc.XySweepProperties (
                        pArea ? *pArea : area1,
                        pAngle ? *pAngle : angle1,
                        pPoint ? *pPoint : point1);
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    {
                    // hmmm..  we'd normally recurse to ourself, but the curveId arg is hidden in the EdgeData, so we have 
                    // to recurse on the mapped evaluator with whole interval.
                    myResult = jmdlRIMSBS_sweptPopertiesMapped (this, pArea, pAngle, pPoint, pInterval->partialCurveId, 0.0, 1.0);
                    }
                else
                    myResult = jmdlRIMSBS_sweptPopertiesMapped
                                (
                                this,
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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

RG_CurveId RIMSBS_Context::ResolveThroughPartialCurve (RG_CurveId curveId)
    {
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        if (desc.type == RIMSBS_CurveInterval)
            {
            RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
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
    return pContext->TryCurveCurveIntersection (pRG, pIntersections, pEdgeData0, pEdgeData1);
    }

bool RIMSBS_Context::TryCurveCurveIntersection
(
RG_Header               *pRG,               /* => receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
RG_EdgeData             *pEdgeData0,        /* => segment edge data */
RG_EdgeData             *pEdgeData1         /* => curve edge data */
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader desc0, desc1;
    int curve0Id = ResolveThroughPartialCurve (jmdlRGEdge_getCurveId (pEdgeData0));
    int curve1Id = ResolveThroughPartialCurve (jmdlRGEdge_getCurveId (pEdgeData1));
    int type0, type1;


    if (   IsValidCurveIndex (curve0Id)
        && IsValidCurveIndex (curve1Id)
        )
        {
        RIMSBS_ElementHeader &desc0 = GetElementR (curve0Id);
        RIMSBS_ElementHeader &desc1 = GetElementR (curve1Id);
        type0 = desc0.type;
        type1 = desc1.type;

        if (curve0Id == curve1Id)
            {
            if (type0 == RIMSBS_MSBsplineCurve)
                {
                MSBsplineCurveP pCurve0 = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curve0Id));
                jmdlRIMSBS_MSBsplineCurveSelfIntersection
                        (
                        this,
                        pRG,
                        pIntersections,
                        pEdgeData0,
                        pCurve0
                        );
                myResult = true;
                }
            else if (type0 == RIMSBS_DEllipse3d)
                {
                jmdlRIMSBS_checkClosedEdge (this, pRG, pIntersections, pEdgeData0);
                myResult = true;
                }
            else
                {
                myResult = true;
                }
            }
        else if  (type0 == RIMSBS_DEllipse3d && type1 == RIMSBS_DEllipse3d)
            {
            DEllipse3d arc0, arc1;
            TryGetArc (curve0Id, arc0);
            TryGetArc (curve1Id, arc1);
            jmdlRIMSBS_ellipseEllipseIntersection
                    (
                    this,
                    pRG,
                    pIntersections,
                    pEdgeData0,
                    &arc0,
                    pEdgeData1,
                    &arc1
                    );
            myResult = true;
            }

        else if  (type0 == RIMSBS_MSBsplineCurve && type1 == RIMSBS_MSBsplineCurve)
            {
            MSBsplineCurveP pCurve0 = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curve0Id));
            MSBsplineCurveP pCurve1 = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curve1Id));
            jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection
                    (
                    this,
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
            DEllipse3d arc0;
            TryGetArc (curve0Id, arc0);
            MSBsplineCurveP pCurve1 = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curve1Id));
            jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
                    (
                    this,
                    pRG,
                    pIntersections,
                    pEdgeData1,
                    pCurve1,
                    pEdgeData0,
                    &arc0,
                    NULL
                    );
            myResult = true;
            }
        else if  (type0 == RIMSBS_MSBsplineCurve && type1 == RIMSBS_DEllipse3d)
            {
            DEllipse3d arc1;
            TryGetArc (curve1Id, arc1);
            MSBsplineCurveP pCurve0 = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curve0Id));
            jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
                    (
                    this,
                    pRG,
                    pIntersections,
                    pEdgeData0,
                    pCurve0,
                    pEdgeData1,
                    &arc1,
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
    return pContext->TrySegmentCurveIntersectionMapped (pRG, pIntersections, pEdgeData0, pEdgeData1, parentCurveId, s0, s1);
    }
bool RIMSBS_Context::TrySegmentCurveIntersectionMapped
(
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
    if (IsValidCurveIndex (parentCurveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (parentCurveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (parentCurveId, arc);

                DEllipse3d partialEllipse = arc;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, &arc, s0, s1);

                jmdlRIMSBS_segmentEllipseIntersection
                        (
                        this,
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
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (parentCurveId));
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
                jmdlRIMSBS_segmentMSBSplineCurveIntersection
                            (this, pRG, pIntersections, pEdgeData0, pEdgeData1, &partialCurve);
                bspcurv_freeCurve (&partialCurve);
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped
                                (
                                this,
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
                                this,
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
    return pContext->TrySegmentCurveIntersection (pRG, pIntersections, pEdgeData0, pEdgeData1);
    }
bool RIMSBS_Context::TrySegmentCurveIntersection
(
RG_Header               *pRG,               /* => receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
RG_EdgeData     *pEdgeData0,                /* => segment edge data */
RG_EdgeData     *pEdgeData1                 /* => curve edge data */
)
    {
    bool    myResult = false;
    int curveId = jmdlRGEdge_getCurveId (pEdgeData1);
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                jmdlRIMSBS_segmentEllipseIntersection
                        (
                        this,
                        pRG,
                        pIntersections,
                        pEdgeData0,
                        pEdgeData1,
                        &arc
                        );
                myResult = true;
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
                jmdlRIMSBS_segmentMSBSplineCurveIntersection
                            (this, pRG, pIntersections, pEdgeData0, pEdgeData1, pCurve);
                break;
                }
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID (pInterval->partialCurveId))
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped (this, pRG, pIntersections,
                                pEdgeData0, pEdgeData1, pInterval->partialCurveId, 0.0, 1.0);
                else
                    myResult = jmdlRIMSBS_segmentCurveIntersectionMapped
                                (
                                this,
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
    return pContext->TryGetClosestXYPointOnMappedCurve (pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint, curveId, s0, s1);
    }
bool RIMSBS_Context::TryGetClosestXYPointOnMappedCurve
(
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
    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        DPoint3d pointA;
        DVec3d vectorA2;
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                DEllipse3d partialEllipse = arc;
                double theta;
                partialEllipse.start = arc.start + arc.sweep * s0;
                partialEllipse.sweep = (s1 - s0) * arc.sweep;
                double minSquaredA;
                DPoint3d minPointA;
                if (partialEllipse.ClosestPointXYBounded (
                                        theta,
                                        pMinDistSquared ? *pMinDistSquared : minSquaredA,
                                        pMinPoint ? *pMinPoint : minPointA,
                                        *pPoint)
                    )
                    {
                    double param = bsiTrig_normalizeAngleToSweep (
                                            theta,
                                            partialEllipse.start,
                                            partialEllipse.sweep);
                    if (pMinParam)
                        *pMinParam = param;
                    if (pMinTangent)
                        {
                        DVec3d angularTangent;
                        partialEllipse.Evaluate (
                                            pointA, angularTangent, vectorA2, theta);
                        pMinTangent->Scale (angularTangent, partialEllipse.sweep);
                        }
                    myResult = true;
                    }
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID(pInterval->partialCurveId))
                    {
                    myResult = TryGetClosestXYPointOnMappedCurve (
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->partialCurveId, s0, s1);
                    }
                else
                    {
                    double dt = pInterval->s1 - pInterval->s0;
                    double t0 = pInterval->s0 + s0 * dt;
                    double t1 = pInterval->s0 + s1 * dt;
                    int parentCurveId = pInterval->parentId;
                    myResult = TryGetClosestXYPointOnMappedCurve (
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                parentCurveId, t0, t1);
                    }

                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
                double ds = s1 - s0;
                DPoint3d xyz[2];
                double param;
                myResult = bspcurv_closestXYPoint (
                            pMinPoint, &param, pMinDistSquared,
                            pCurve, s0, s1,
                            pPoint->x, pPoint->y);
                if (pMinParam)
                    *pMinParam = (param - s0) / ds;
                if (myResult && pMinTangent)
                    {
                    jmdlRIMSBS_evaluateDerivatives (this, xyz, 1, param, curveId);
                    *pMinTangent = xyz[1];
                    pMinTangent->Scale (*pMinTangent, ds);
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
    return pContext->TryGetClosestXYPointOnCurve (pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint, curveId);
    }

bool        RIMSBS_Context::TryGetClosestXYPointOnCurve
(
double   *pMinParam,        /* => parameter at closest approach point */
double   *pMinDistSquared,  /* => squard distance to closest approach point */
DPoint3d *pMinPoint,        /* => closest approach point */
DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
DPoint3d *pPoint,           /* => space point */
RG_CurveId  curveId         /* => curve identifier */
)

    {
    bool    myResult = false;
    DPoint3d xyzA;
    DVec3d vectorA2;

    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                double theta, param;
                double minSquaredA;
                DPoint3d minPointA;
                if (arc.ClosestPointXYBounded (
                                        theta,
                                        pMinDistSquared ? *pMinDistSquared : minSquaredA,
                                        pMinPoint ? *pMinPoint : minPointA,
                                        *pPoint)
                    )
                    {
                    param = bsiTrig_normalizeAngleToSweep (
                                            theta,
                                            arc.start,
                                            arc.sweep);
                    if (pMinParam)
                        *pMinParam = param;

                    if (pMinTangent)
                        {
                        DVec3d angularTangent;
                        arc.Evaluate (xyzA, angularTangent, vectorA2, theta);
                        pMinTangent->Scale (angularTangent, arc.sweep);
                        }
                    }
                myResult = true;
                break;
                }

            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                if (IS_CURVEID(pInterval->partialCurveId))
                    myResult = TryGetClosestXYPointOnCurve (
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->partialCurveId);
                else 
                    myResult = TryGetClosestXYPointOnMappedCurve (
                                pMinParam, pMinDistSquared, pMinPoint, pMinTangent, pPoint,
                                pInterval->parentId,
                                pInterval->s0,
                                pInterval->s1
                                );
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
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
                    jmdlRIMSBS_evaluateDerivatives (this, xyz, 1, param, curveId);
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
    return pContext->AppendAllCurveSamplePoints (*pXYZArray);
    }

void RIMSBS_Context::AppendAllCurveSamplePoints (bvector<DPoint3d> &xyzArray)
    {
    RIMSBS_ElementHeader desc;
    static int s_numPerEllipse = 4;
    DPoint3d point;
    for (auto &desc : m_geometry)
        {
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                if (desc.curve->TryGetArc (arc))
                    {
                    double df = arc.sweep / (double)(s_numPerEllipse - 1);
                    double f;
                    for (int i = 0; i < s_numPerEllipse; i++)
                        {
                        f = df * i;
                        arc.Evaluate (point, arc.FractionToAngle (f));
                        xyzArray.push_back (point);
                        }
                    }
                break;
                }

            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveCP pCurve = desc.curve->GetBsplineCurveCP ();
                if (nullptr != pCurve)
                    {
                    int n = pCurve->params.numPoles;
                    for (int k = 0; k < n; k++)
                        xyzArray.push_back (pCurve->GetUnWeightedPole (k));
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
    return pContext->TryTransformCurve (curveIndex, *pTransform);
    }

bool RIMSBS_Context::TryTransformCurve (int curveIndex, TransformCR transform)
    {
    bool    bResult = false;
    if (IsValidCurveIndex (curveIndex))
        {
        auto &desc = GetElementR (curveIndex);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
            case RIMSBS_MSBsplineCurve:
                bResult = desc.curve->TransformInPlace (transform);
            }
        }
    return bResult;
    }

void RIMSBS_Context::TransformAllCurves (TransformCR transform)
    {
    for (size_t i = 0; i < m_geometry.size (); i++)
        TryTransformCurve ((int)i, transform);
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
    pContext->TransformAllCurves (*pTransform);
    }
END_BENTLEY_GEOMETRY_NAMESPACE