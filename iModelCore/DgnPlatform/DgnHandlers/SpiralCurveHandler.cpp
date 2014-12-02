/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/SpiralCurveHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

// These identifier are replicated in mstn/pubinc/3dtools.h and the spiral handlers.
// Assume that if SPIRALTYPE_Clothoid is undefined all the rest are.

/*----------------------------------------------------------------------+
|                                                                       |
|   Spiral Types                                                        |
|                                                                       |
+----------------------------------------------------------------------*/
#define     SPIRALTYPE_Clothoid             0
#define     SPIRALTYPE_Archimedes           1
#define     SPIRALTYPE_Logarithmic          2

/*----------------------------------------------------------------------+
|                                                                       |
|   Spiral Creaton Modes                                                |
|                                                                       |
+----------------------------------------------------------------------*/
#define     SPIRALMODE_Points               0
#define     SPIRALMODE_Sweep                1
#define     SPIRALMODE_Angle                2
#define     SPIRALMODE_Degree               3
#define     SPIRALMODE_Tangent              4
#define     SPIRALMODE_Length               5

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSpiralData::TransitionSpiralData ()
    {
    m_stAngle           = 0.0;
    m_endAngle          = 0.0;
    m_stRadius          = 0.0;
    m_endRadius         = 0.0;
    m_fractionA         = 0.0;
    m_fractionB         = 1.0;
    m_spiralType        = SPIRALTYPE_Clothoid;
    m_spiralCreateMode  = SPIRALMODE_Points;

    m_rMatrix.InitIdentity ();
    m_origin.Zero ();

    memset (m_reserved, 0, sizeof (m_reserved));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TransitionSpiralData::TransitionSpiralData (DSpiral2dBaseCR spiral, TransformCR frame, double fractionA, double fractionB)
    {
    m_stAngle   = Angle::RadiansToDegrees (spiral.mTheta0);
    m_endAngle  = Angle::RadiansToDegrees (spiral.mTheta1);

    m_stRadius  = TransitionSpiralData::RadiusFromCurvature (spiral.mCurvature0);
    m_endRadius = TransitionSpiralData::RadiusFromCurvature (spiral.mCurvature1);

    m_fractionA = fractionA;
    m_fractionB = fractionB;

    m_spiralCreateMode = SPIRALMODE_Points; // Does this matter?!?

    if (NULL != dynamic_cast <DSpiral2dClothoid const*> (&spiral))
        m_spiralType = SPIRALTYPE_TransitionClothoid;
    else if (NULL != dynamic_cast <DSpiral2dBloss const*> (&spiral))
        m_spiralType = SPIRALTYPE_TransitionBloss;
    else if (NULL != dynamic_cast <DSpiral2dBiQuadratic const*> (&spiral))
        m_spiralType = SPIRALTYPE_TransitionBiQuadratic;
    else if (NULL != dynamic_cast <DSpiral2dCosine const*> (&spiral))
        m_spiralType = SPIRALTYPE_TransitionCosine;
    else if (NULL != dynamic_cast <DSpiral2dSine const*> (&spiral))
        m_spiralType = SPIRALTYPE_TransitionSine;
    else
        m_spiralType = 0; // ERROR: Create will fail...

    frame.GetMatrix (m_rMatrix);
    frame.GetTranslation (m_origin);

    memset (m_reserved, 0, sizeof (m_reserved));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::GetMetrics
(
double &bearingRadians0,
double &radius0,
double &bearingRadians1,
double &radius1,
double &startFraction,
double &endFraction
)
    {
    bearingRadians0 = bsiTrig_degreesToRadians (m_stAngle);
    bearingRadians1 = bsiTrig_degreesToRadians (m_endAngle);
    radius0 = m_stRadius;
    radius1 = m_endRadius;
    startFraction = m_fractionA;
    endFraction   = m_fractionB;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::SetMetrics
(
double bearingRadians0,
double radius0,
double bearingRadians1,
double radius1,
double startFraction,
double endFraction
)
    {
    m_stAngle  = bsiTrig_radiansToDegrees (bearingRadians0 );
    m_endAngle = bsiTrig_radiansToDegrees (bearingRadians1);
    m_stRadius = radius0;
    m_endRadius = radius1;
    m_fractionA = startFraction;
    m_fractionB = endFraction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::SweepRadiansFromRadiiAndLength
(
double radius0,
double radius1,
double length
)
    {
    double k0 = TransitionSpiralData::CurvatureFromRadius (radius0);
    double k1 = TransitionSpiralData::CurvatureFromRadius (radius1);
    return 0.5 * (k0 + k1) * length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::SetAngle (double newVal, bool    bStart)
    {
    double oldVal = m_stAngle;
    if (bStart)
        {
        m_stAngle = newVal;
        return oldVal;
        }
    else
        {
        oldVal = m_endAngle;
        m_endAngle = newVal;
        return oldVal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::ElementFractionToSpiralFraction (double elementFraction) const
    {
    return m_fractionA + elementFraction * (m_fractionB - m_fractionA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::SpiralFractionToElementFraction (double elementFraction) const
    {
    double spiralFraction;
    bsiTrig_safeDivide (&spiralFraction, elementFraction - m_fractionA, m_fractionB - m_fractionA, m_fractionA);
    return spiralFraction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::ElementToSpiralScale () const
    {
    return m_fractionB - m_fractionA;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::SpiralToElementScale() const
    {
    double s;
    bsiTrig_safeDivide (&s, 1.0, m_fractionB - m_fractionA, 0.0);
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::GetAngle (bool bStart) const
    {
    if (bStart)
        return m_stAngle;
    else
        return m_endAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::SetRadius (double newVal, bool bStart)
    {
    double oldVal = m_stRadius;
    if (bStart)
        {
        m_stRadius = newVal;
        return oldVal;
        }
    else
        {
        oldVal = m_endRadius;
        m_endRadius = newVal;
        return oldVal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::GetRadius (bool bStart) const
    {
    if (bStart)
        return m_stRadius;
    else
        return m_endRadius;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::SetSpiralType (int type)
    {
    m_spiralType = type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
int TransitionSpiralData::GetSpiralType () const
    {
    return m_spiralType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::SetCreateMode (int mode)
    {
    m_spiralCreateMode = mode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
int TransitionSpiralData::GetCreateMode () const
    {
    return m_spiralCreateMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d TransitionSpiralData::SetOrigin (DPoint3dCR newPt)
    {
    DPoint3d oldPt = m_origin;
    m_origin = newPt;
    return oldPt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d TransitionSpiralData::GetOrigin () const
    {
    return m_origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix TransitionSpiralData::SetMatrix (RotMatrixCR newMatrix)
    {
    RotMatrix oldMatrix = m_rMatrix;
    m_rMatrix = newMatrix;
    return oldMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::SetExtraData (double *pData, int count)
    {
    if (count > EXTRA_DATA_COUNT)
        count = EXTRA_DATA_COUNT;
    for (int i = 0; i < count; i++)
        m_reserved[i] = pData[i];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::GetExtraData (double *pData, int &actualDataCount, int maxOut)
    {
    actualDataCount = EXTRA_DATA_COUNT;
    if (maxOut > EXTRA_DATA_COUNT)
        maxOut = EXTRA_DATA_COUNT;
    if (NULL != pData)
        {
        for (int i = 0; i < maxOut; i++)
            pData[i] = m_reserved[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix TransitionSpiralData::GetMatrix () const
    {
    return m_rMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
Transform TransitionSpiralData::GetTransform ()
    {
    Transform T;
    T.initFrom (&m_rMatrix, &m_origin);
    return T;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::LocalToWorld (DPoint3dP pXYZ, DVec2dCP pUV, int n) const
    {
    Transform transform;
    transform.initFrom (&m_rMatrix, &m_origin);
    for (int i = 0; i < n; i++)
        {
        bsiTransform_multiplyComponents (&transform, &pXYZ[i], pUV[i].x, pUV[i].y, 0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::LocalToWorldAsVector (DVec3dP pXYZ, DVec2dCP pUV, int n) const
    {
    for (int i = 0; i < n; i++)
        {
        bsiRotMatrix_multiplyComponents (&m_rMatrix, &pXYZ[i], pUV[i].x, pUV[i].y, 0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DSpiral2dBaseP TransitionSpiralData::GetDSpiral2d () const
    {
    DSpiral2dBaseP pSpiral = NULL;

    switch (GetSpiralType ())
        {
        case SPIRALTYPE_TransitionClothoid:
            {
            pSpiral = new DSpiral2dClothoid ();
            break;
            }
        case SPIRALTYPE_TransitionBloss:
            {
            pSpiral = new DSpiral2dBloss ();
            break;
            }
        case SPIRALTYPE_TransitionBiQuadratic:
            {
            pSpiral = new DSpiral2dBiQuadratic ();
            break;
            }
        case SPIRALTYPE_TransitionCosine:
            {
            pSpiral = new DSpiral2dCosine ();
            break;
            }
        case SPIRALTYPE_TransitionSine:
            {
            pSpiral = new DSpiral2dSine ();
            break;
            }
        }

    if (NULL == pSpiral)
        return NULL;

    double stAngle = GetAngle (true) * (msGeomConst_pi / 180.0);
    double endAngle = GetAngle (false) * (msGeomConst_pi / 180.0);
    double stCurvature = TransitionSpiralData::CurvatureFromRadius (GetRadius (true));
    double endCurvature = TransitionSpiralData::CurvatureFromRadius (GetRadius (false));

    pSpiral->SetBearingAndCurvatureLimits (stAngle, stCurvature, endAngle, endCurvature);
    
    return pSpiral;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::GetInterval (double &spiralFractionA, double &spiralFractionB) const
    {
    spiralFractionA = m_fractionA;
    spiralFractionB = m_fractionB;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::SetInterval (double spiralFractionA, double spiralFractionB)
    {
    m_fractionA = spiralFractionA;
    m_fractionB = spiralFractionB;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void TransitionSpiralData::DropSpiral (DSpiral2dBaseP spiral) const
    {
    if (spiral)
        delete spiral;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::CurvatureFromRadius (double radius)
    {
    if (fabs (radius) < 1.0e-8)
        return 0;

    return 1.0 / radius;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::RadiusFromCurvature (double curvature)
    {
    if (fabs (curvature) < 1.0e-8)
        return 0;

    return 1.0 / curvature;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::DegreeFromRadius (double radius)
    {
    if (fabs (radius) > 1.0e-8)
        return radius * 100.0 / (msGeomConst_pi / 180.0);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chen.Ping                       12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double TransitionSpiralData::RadiusFromDegree (double degree)
    {
    return degree * (msGeomConst_pi / 180.0) / 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SpiralCurveHandler::_OnTransform (EditElementHandleR eeh, TransformInfoCR tinfo)
    {
    TransitionSpiralData  data;

    if (SUCCESS != GetData (eeh, data))
        return ERROR;

    double      radians0, rad0, radians1, rad1, fractionA, fractionB;
    DPoint3d    appliedTranslation;
    DPoint3d    spiralOrigin = data.GetOrigin ();
    RotMatrix   spiralMatrix = data.GetMatrix ();
    Transform   appliedTransform = *tinfo.GetTransform ();
    RotMatrix   rawAppliedMatrix, descaledAppliedMatrix, newSpiralMatrix;

    data.GetMetrics (radians0, rad0, radians1, rad1, fractionA, fractionB);
    appliedTransform.GetMatrix (rawAppliedMatrix);
    appliedTransform.GetTranslation (appliedTranslation);

    double      det = rawAppliedMatrix.Determinant ();
    double      meanScale = pow (fabs (det), 1.0 / 3.0);

    // If smash transform, let it alone ..
    if (meanScale == 0.0)
        meanScale = 1.0;

    double      a = 1.0 / meanScale;

    rad0 *= meanScale;
    rad1 *= meanScale;

    descaledAppliedMatrix.ScaleColumns (rawAppliedMatrix, a, a, a);
    appliedTransform.Multiply (spiralOrigin);
    newSpiralMatrix.InitProduct (descaledAppliedMatrix, spiralMatrix);

    data.SetMetrics (radians0, rad0, radians1, rad1, fractionA, fractionB);
    data.SetMatrix (newSpiralMatrix);
    data.SetOrigin (spiralOrigin);

    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
    if (BSPLINE_STATUS_Success != SpiralCurveHandler::CreateSpiralCurveElement (newEeh, &eeh, data, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
        return ERROR;

    newEeh.ScheduleWriteXAttribute (SpiralXAHandler::GetXAttrId (), 0, sizeof (data), &data);

    return eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SpiralCurveHandler::_OnFenceStretch (EditElementHandleR eeh, TransformInfoCR tInfo, FenceParamsP fp, FenceStretchFlags options)
    {
    if (SUCCESS != T_Super::_OnFenceStretch (eeh, tInfo, fp, options))
        return ERROR;

    // Stretch invalidate spiral data, delete it and drop handler to that we just have a normal bspline curve element...
#ifdef WIP_VANCOUVER_MERGE // spiral
    ElementHandlerManager::RemoveHandlerFromElement (eeh);
#endif
    eeh.ScheduleDeleteXAttribute (SpiralXAHandler::GetXAttrId (), 0);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SpiralCurveHandler::_GetDescription (ElementHandleCR eh, WStringR descr, UInt32 desiredLength)
    {
    TransitionSpiralData data;

    if (SUCCESS == GetData (eh, data))
        {
        switch (data.GetSpiralType ())
            {
            case SPIRALTYPE_TransitionClothoid:
#ifdef WIP_VANCOUVER_MERGE // spiral
                descr.assign (g_dgnHandlersResources->GetString (IDS_SubType_SpiralClothoid));
                #endif
                descr = L"Clothoid";
                return;

            case SPIRALTYPE_TransitionBloss:
#ifdef WIP_VANCOUVER_MERGE // spiral
                descr.assign (g_dgnHandlersResources->GetString (IDS_SubType_SpiralBloss));
                #endif
                descr = L"Bloss";
                return;

            case SPIRALTYPE_TransitionBiQuadratic:
#ifdef WIP_VANCOUVER_MERGE // spiral
                descr.assign (g_dgnHandlersResources->GetString (IDS_SubType_SpiralBiQuadratic));
                #endif
                descr = L"Bi-quadratic";
                return;

            case SPIRALTYPE_TransitionCosine:
#ifdef WIP_VANCOUVER_MERGE // spiral
                descr.assign (g_dgnHandlersResources->GetString (IDS_SubType_SpiralCosine));
                #endif
                descr = L"Cosine";
                return;

            case SPIRALTYPE_TransitionSine:
#ifdef WIP_VANCOUVER_MERGE // spiral
                descr.assign (g_dgnHandlersResources->GetString (IDS_SubType_SpiralSine));
                #endif
                descr = L"Sine";
                return;
            }
        }

    T_Super::_GetDescription (eh, descr, desiredLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpiralCurveHandler::GetData (ElementHandleCR eh, TransitionSpiralData& data)
    {
    ElementHandle::XAttributeIter  xa (eh, SpiralXAHandler::GetXAttrId ());

    if (!xa.IsValid ())
        {
        assert (false);
        memset (&data, 0, sizeof (data));

        return ERROR;
        }

    data = *(TransitionSpiralData*) xa.PeekData ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus SpiralCurveHandler::SetData (EditElementHandleR eeh, TransitionSpiralData const& data)
    {
    EditElementHandle newEeh;
    BSplineStatus status;
    if (BSPLINE_STATUS_Success == (status = CreateSpiralCurveElement (newEeh, &eeh, data, eeh.GetElementCP()->Is3d(), *eeh.GetDgnModelP())))
        eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpiralCurveHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    if (BSPLINE_CURVE_ELM != eh.GetLegacyType())
        return ERROR;

    TransitionSpiralData  data;

    if (SUCCESS != GetData (eh, data))
        return ERROR;

    DSpiral2dBaseP  spiral;

    if (NULL == (spiral = data.GetDSpiral2d ()))
        return ERROR;

    double      fractionA, fractionB;
    Transform   frame = data.GetTransform ();
    
    data.GetInterval (fractionA, fractionB);

    curves = CurveVector::Create (eh.GetElementCP()->ToBspline_curve().flags.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateSpiral (*spiral, frame, fractionA, fractionB));
    data.DropSpiral (spiral);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpiralCurveHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    switch (path.HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            {
            MSBsplineCurveCP      bCurve = path.front ()->GetProxyBsplineCurveCP ();
            EditElementHandle     newEeh;

            // Make it easy to drop spiral to a normal bspline curve (ex. drag manipulator)...
            if (BSPLINE_STATUS_Success != BSplineCurveHandler::CreateBSplineCurveElement (newEeh, &eeh, *bCurve, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
                return ERROR;

#ifdef WIP_VANCOUVER_MERGE // spiral
            ElementHandlerManager::RemoveHandlerFromElement (newEeh);
#endif
BeAssert(false);
            newEeh.ScheduleDeleteXAttribute (SpiralXAHandler::GetXAttrId (), 0);

            return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            DSpiral2dPlacementCP  sp = path.front ()->GetSpiralPlacementCP ();
            TransitionSpiralData  data (*sp->spiral, sp->frame, sp->fractionA, sp->fractionB);
            EditElementHandle     newEeh;

            // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
            if (BSPLINE_STATUS_Success != SpiralCurveHandler::CreateSpiralCurveElement (newEeh, &eeh, data, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
                return ERROR;

            return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus SpiralCurveHandler::CreateSpiralCurveElement (EditElementHandleR eeh, ElementHandleCP templateEh, TransitionSpiralData const& data, bool is3d, DgnModelR modelRef)
    {
    switch (data.GetSpiralType ())
        {
        case SPIRALTYPE_TransitionClothoid:
        case SPIRALTYPE_TransitionBloss:
        case SPIRALTYPE_TransitionBiQuadratic:
        case SPIRALTYPE_TransitionCosine:
        case SPIRALTYPE_TransitionSine:
            break;

        default:
            return BSPLINE_STATUS_BadSpiralDefinition;
        }

    DSpiral2dBaseP  spiral;

    if (NULL == (spiral = data.GetDSpiral2d ()))
        return BSPLINE_STATUS_BadSpiralDefinition;

    DPoint3d    origin = data.GetOrigin ();
    RotMatrix   rMatrix = data.GetMatrix ();
    double      fractionA, fractionB;
    
    data.GetInterval (fractionA, fractionB);

    BSplineStatus       status = BSPLINE_STATUS_BadSpiralDefinition;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr ();

    if (SUCCESS == bspcurv_curveFromDSpiral2dBaseInterval (curve.get (), spiral, fractionA, fractionB, &origin, &rMatrix))
        {
        curve->display.curveDisplay = true;

        if (BSPLINE_STATUS_Success == (status = BSplineCurveHandler::CreateBSplineCurveElement (eeh, templateEh, *curve, is3d, modelRef)))
            {
            if (BSPLINE_CURVE_ELM != eeh.GetLegacyType())
                {
                eeh.Invalidate ();
                status = BSPLINE_STATUS_BadSpiralDefinition; // Create function can return a complex chain if > MAX_POLES...
                }
            else
                {
#ifdef WIP_VANCOUVER_MERGE // spiral
                ElementHandlerManager::AddHandlerToElement (eeh, ElementHandlerXAttribute (SpiralCurveHandler::GetElemHandlerId (), MISSING_HANDLER_PERMISSION_All_));
#endif
BeAssert(false);
                eeh.ScheduleWriteXAttribute (SpiralXAHandler::GetXAttrId (), 0, sizeof (data), &data);
                }
            }
        }

    data.DropSpiral (spiral);

    return status;
    }

#ifdef WIP_EC_EXTENSION

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//            EC properties
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Autogenerated map for class Spiral
static const ElementECDelegate::MapEntry s_BESMAP_Spiral[] = 
    {
    BESINDEX_Spiral_StartRadius, L"StartRadius", NULL_MAP,
    BESINDEX_Spiral_EndRadius, L"EndRadius", NULL_MAP,
    BESINDEX_Spiral_SweepAngle, L"SweepAngle", NULL_MAP,
    BESINDEX_Spiral_StartBearing, L"StartBearing", NULL_MAP,
    BESINDEX_Spiral_EndBearing, L"EndBearing", NULL_MAP,
    ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpiralCurveECData : IECPerDelegateData
    {
private:
    TransitionSpiralData        m_data;
    SpiralCurveECData (TransitionSpiralData const& data) : m_data(data)     { }
public:
    TransitionSpiralData&       GetData()                                   { return m_data; }
    static SpiralCurveECData*   Create (TransitionSpiralData const& data)   { return new SpiralCurveECData (data); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpiralCurveECDelegate : ElementECDelegate
    {
private:
    SpiralCurveECDelegate() { }

    virtual const MapEntry*         _GetMap() const override            { return s_BESMAP_Spiral; }
    virtual bool                    _AttachToInstance (DelegatedElementECInstanceCR instance) const override;
    virtual ECN::ECObjectsStatus    _GetValue (ECN::ECValueR v, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool, UInt32) const override;
    virtual ECN::ECObjectsStatus    _SetValue (ECN::ECValueCR v, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool, UInt32) const override;
    virtual ECN::ECObjectsStatus    _Commit (DelegatedElementECInstanceR instance) const override;
    virtual bool                    _IsNullValue (DelegatedElementECInstanceCR instance, UInt32 propIdx) const override { return false; }
public:
    static SpiralCurveECDelegate* Create()                          { return new SpiralCurveECDelegate(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpiralCurveECDelegate::_AttachToInstance (DelegatedElementECInstanceCR instance) const
    {
    ElementHandleCR eh = instance.GetElementHandle();
    TransitionSpiralData data;
    if (SUCCESS == SpiralCurveHandler::GetData (eh, data))
        {
        instance.SetPerDelegateData (*this, SpiralCurveECData::Create (data));
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus SpiralCurveECDelegate::_GetValue (ECN::ECValueR v, DelegatedElementECInstanceCR instance, UInt32 propIdx, bool, UInt32) const
    {
    SpiralCurveECData* ecdata = static_cast<SpiralCurveECData*> (instance.GetPerDelegateData (*this));
    TransitionSpiralData const& data = ecdata->GetData();

    switch (propIdx)
        {
    case BESINDEX_Spiral_StartRadius:               v.SetDouble (data.GetRadius (true)); break;
    case BESINDEX_Spiral_EndRadius:                 v.SetDouble (data.GetRadius (false)); break;
    case BESINDEX_Spiral_SweepAngle:                v.SetDouble (data.GetAngle (false)-data.GetAngle (true)); break;     
    case BESINDEX_Spiral_StartBearing:              v.SetDouble (data.GetAngle (true)); break;
    case BESINDEX_Spiral_EndBearing:                v.SetDouble (data.GetAngle (false)); break;
    default:                                        return ECN::ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECN::ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus SpiralCurveECDelegate::_SetValue (ECN::ECValueCR v, DelegatedElementECInstanceR instance, UInt32 propIdx, bool, UInt32) const
    {
    SpiralCurveECData* ecdata = static_cast<SpiralCurveECData*> (instance.GetPerDelegateData (*this));
    TransitionSpiralData& data = ecdata->GetData();

    switch (propIdx)
        {
    case BESINDEX_Spiral_StartRadius:               data.SetRadius (v.GetDouble(), true); break;
    case BESINDEX_Spiral_EndRadius:                 data.SetRadius (v.GetDouble(), false); break;
    case BESINDEX_Spiral_EndBearing:                data.SetAngle (v.GetDouble(), false); break;
    case BESINDEX_Spiral_SweepAngle:                data.SetAngle (v.GetDouble() + data.GetAngle(true), false); break;
    case BESINDEX_Spiral_StartBearing:
        {
        double stAngle  = data.GetAngle (true),
               endAngle = data.GetAngle (false),
               inVal    = v.GetDouble();
        data.SetAngle (inVal, true);
        data.SetAngle (inVal + endAngle - stAngle, false);
        }
        break;
    default:
        return ECN::ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECN::ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus SpiralCurveECDelegate::_Commit (DelegatedElementECInstanceR instance) const
    {
    EditElementHandleR eeh = *instance.GetWipElementHandle();
    SpiralCurveECData* ecdata = static_cast<SpiralCurveECData*> (instance.GetPerDelegateData (*this));
    return SUCCESS == SpiralCurveHandler::SetData (eeh, ecdata->GetData()) ? ECN::ECOBJECTS_STATUS_Success : ECN::ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpiralCurveECExtension : DelegatedElementECExtension
    {
private:
    SpiralCurveECExtension() { }

    virtual void _GetECClasses (T_ECClassCPVector& classes) const override
        {
        classes.push_back (LookupECClass (DGN_ELEMENT_SCHEMA, L"SpiralCurveElement"));
        }
    virtual ElementECDelegatePtr _SupplyDelegate (ECN::ECClassCR ecClass) const
        {
        return ecClass.GetName().Equals (L"Spiral") ? SpiralCurveECDelegate::Create() : NULL;
        }
public:
    static ElementECExtension* Create() { return new SpiralCurveECExtension(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECExtension& SpiralCurveHandler::CreateElementECExtension()
    {
    return *SpiralCurveECExtension::Create();
    }

#endif