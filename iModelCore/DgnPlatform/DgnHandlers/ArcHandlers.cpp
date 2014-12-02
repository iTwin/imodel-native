/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ArcHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

static double   s_axisRatioTolerance = 1.0e-6;

/*=================================================================================**//**
* class for converting DgnElement <--> DEllipse3d
* @bsiclass                                                     Keith.Bentley   02/05
+===============+===============+===============+===============+===============+======*/
struct AhEllipse : DEllipse3d
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
AhEllipse (ElementHandleCR thisElm, TransformCP pTransform = NULL)
    {
    DgnElementCP el = thisElm.GetElementCP();
    switch (el->GetLegacyType())
        {
        case ELLIPSE_ELM:
            {
            static double s_startDegrees = 0.0;//-90.0;
            double startRadians = Angle::DegreesToRadians (s_startDegrees);
            if (DisplayHandler::Is3dElem (el))
                {
                Ellipse_3d const*   arc = &el->ToEllipse_3d();
                this->initFromDGNFields3d (&arc->origin, const_cast <double *> (arc->quat), NULL, NULL, arc->primary, arc->secondary, &startRadians, NULL);
                }
            else
                {
                 Ellipse_2d const*   arc = &el->ToEllipse_2d();
                 this->initFromDGNFields2d (&arc->origin, &arc->rotationAngle, NULL, arc->primary, arc->secondary, &startRadians, NULL, 0.0);
                 }
            }
            break;

        case ARC_ELM:
            if (DisplayHandler::Is3dElem (el))
                {
                Arc_3d const*   arc = &el->ToArc_3d();
                this->initFromDGNFields3d (&arc->origin, const_cast <double *> (arc->quat), NULL, NULL, arc->primary, arc->secondary, &arc->startAngle, &arc->sweepAngle);
                }
            else
                {
                Arc_2d const*   arc = &el->ToArc_2d();
                this->initFromDGNFields2d (&arc->origin, &arc->rotationAngle, NULL, arc->primary, arc->secondary, &arc->startAngle, &arc->sweepAngle, 0.0);
                }
            break;

        default:
            BeAssert (0);
        }

    if (pTransform != NULL)
        productOf (pTransform, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Earlin.Lutz      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            ToElement (EditElementHandleR thisElm)
    {
    DgnElementP  el = thisElm.GetElementP();

    if (DisplayHandler::Is3dElem (el))
        {
        DEllipse3d majorMinorEllipse;
        majorMinorEllipse.initWithPerpendicularAxes (this);

        DVec3d xAxis, yAxis, zAxis;
        double r0  = xAxis.normalize(&majorMinorEllipse.vector0);
        double r90 = yAxis.normalize(&majorMinorEllipse.vector90);

        if (r0 < s_axisRatioTolerance * r90)
            {
            // x axis is garbage, y is good ...
            yAxis.getNormalizedTriad (&zAxis, &xAxis, &yAxis);
            r0 = 0.0;
            }
        else if (r90 < s_axisRatioTolerance * r0)
            {
            // x axis is garbage, y is good ...
            xAxis.getNormalizedTriad (&yAxis, &zAxis, &xAxis);
            r90 = 0.0;
            }
        else
            {
            // The usual case.  x,y are perpendicular unit vectors ....
            zAxis.normalizedCrossProduct(&xAxis, &yAxis);
            }

        RotMatrix matrix;
        matrix.initFromColumnVectors (&xAxis, &yAxis, &zAxis);
        matrix.squareAndNormalizeColumns (&matrix, 0, 1);

        if (el->GetLegacyType() == ELLIPSE_ELM)
            {
            Ellipse_3d*   ellipse = &el->ToEllipse_3dR();
            ellipse->origin = majorMinorEllipse.center;
            matrix.getQuaternion (ellipse->quat, true);
            ellipse->primary   = r0;
            ellipse->secondary = r90;
            }
        else
            {
            Arc_3d*  arc = &el->ToArc_3dR();
            arc->origin = majorMinorEllipse.center;
            matrix.getQuaternion (arc->quat, true);
            arc->primary    = r0;
            arc->secondary  = r90;
            arc->startAngle = majorMinorEllipse.start;
            arc->sweepAngle = majorMinorEllipse.sweep;
            }
        }
    else
        {
        DEllipse3d xyEllipse = *this;
        xyEllipse.center.z   = 0.0;
        xyEllipse.vector0.z  = 0.0;
        xyEllipse.vector90.z = 0.0;

        DEllipse3d xyMajorMinorEllipse;
        xyMajorMinorEllipse.initWithPerpendicularAxes (&xyEllipse);

        double r0  = xyMajorMinorEllipse.vector0.magnitude();
        double r90 = xyMajorMinorEllipse.vector90.magnitude();

        // Reverse left handed coordinate system
        if (xyMajorMinorEllipse.vector0.crossProductXY (&xyMajorMinorEllipse.vector90) < 0.0)
            {
            xyMajorMinorEllipse.vector90.negate();
            xyMajorMinorEllipse.start = -xyMajorMinorEllipse.start;
            xyMajorMinorEllipse.sweep = -xyMajorMinorEllipse.sweep;
            }

        // Use the longer axis for angle calculation
        double thetaX = r0 > r90 ? bsiTrig_atan2 (xyMajorMinorEllipse.vector0.y, xyMajorMinorEllipse.vector0.x)
                                 : bsiTrig_atan2 (-xyMajorMinorEllipse.vector90.x, xyMajorMinorEllipse.vector90.y);

        if (el->GetLegacyType() == ELLIPSE_ELM)
            {
            Ellipse_2d*   ellipse = &el->ToEllipse_2dR();
            ellipse->origin.x = xyMajorMinorEllipse.center.x;
            ellipse->origin.y = xyMajorMinorEllipse.center.y;
            ellipse->rotationAngle = thetaX;
            ellipse->primary   = r0;
            ellipse->secondary = r90;
            }
        else
            {
            Arc_2d*  arc = &el->ToArc_2dR();
            arc->origin.x = xyMajorMinorEllipse.center.x;
            arc->origin.y = xyMajorMinorEllipse.center.y;
            arc->rotationAngle = thetaX;
            arc->primary    = r0;
            arc->secondary  = r90;
            arc->startAngle = xyMajorMinorEllipse.start;
            arc->sweepAngle = xyMajorMinorEllipse.sweep;
            }
        }
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArcHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArcHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation)
    {
    DgnElementCP el = elHandle.GetElementCP ();

    if (Is3dElem (el))
        orientation.InitTransposedFromQuaternionWXYZ ( el->ToArc_3d().quat);
    else
        orientation.InitFromAxisAndRotationAngle(2,  el->ToArc_2d().rotationAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipseHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation)
    {
    DgnElementCP el = elHandle.GetElementCP ();

    if (Is3dElem (el))
        orientation.InitTransposedFromQuaternionWXYZ ( el->ToEllipse_3d().quat);
    else
        orientation.InitFromAxisAndRotationAngle(2,  el->ToEllipse_2d().rotationAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EllipseHandler::_OnTransform (EditElementHandleR thisElm, TransformInfoCR trans)
    {
    StatusInt   status = T_Super::_OnTransform (thisElm, trans);
    if (status != SUCCESS)
        return status;

    AhEllipse  ellipse (thisElm);
    ellipse.productOf (trans.GetTransform(), &ellipse);
    ellipse.ToElement(thisElm);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EllipseHandler::_OnFenceClip
(
ElementAgendaP      inside,
ElementAgendaP      outside,
ElementHandleCR     elemHandle,
FenceParamsP        fp,
FenceClipFlags      options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ArcHandler::_OnTransform (EditElementHandleR thisElm, TransformInfoCR trans)
    {
    StatusInt   status = T_Super::_OnTransform (thisElm, trans);
    if (status != SUCCESS)
        return status;

    AhEllipse  ellipse (thisElm);
    ellipse.productOf (trans.GetTransform(), &ellipse);
    ellipse.ToElement(thisElm);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ArcHandler::_OnFenceStretch
(
EditElementHandleR  thisElm,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    DPoint3d    startPt, endPt;
    AhEllipse   ellipse (thisElm);

    ellipse.fractionParameterToDerivatives (&startPt, NULL, NULL, 0.0);
    ellipse.fractionParameterToDerivatives (&endPt, NULL, NULL, 1.0);

    bool        instart = fp->PointInside (startPt);
    bool        inend   = fp->PointInside (endPt);

    if (instart && inend)
        return _OnTransform (thisElm, transform);

    DVec3d      distance;
    AhEllipse   result = ellipse;

    transform.GetTransform()->getTranslation (&distance);

    if (instart)
        {
        if (!bsiDEllipse3d_translateEndPoint (&result, &ellipse, &distance, 0))
            return ERROR;
        }
    else if (inend)
        {
        if (!bsiDEllipse3d_translateEndPoint (&result, &ellipse, &distance, 1))
            return ERROR;
        }
    else
        {
        return SUCCESS;
        }

    result.ToElement (thisElm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ArcHandler::_OnFenceClip
(
ElementAgendaP      inside,
ElementAgendaP      outside,
ElementHandleCR     elemHandle,
FenceParamsP        fp,
FenceClipFlags      options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipticArcBaseHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EllipticArcBaseHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipticArcBaseHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipticArcBaseHandler::_GetTransformOrigin (ElementHandleCR thisElm, DPoint3dR origin)
    {
    AhEllipse  ellipse (thisElm);

    origin = ellipse.center;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipticArcBaseHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    curves = CurveVector::Create (ELLIPSE_ELM == eh.GetLegacyType() ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
    DEllipse3d ellipse = AhEllipse (eh);
    curves->push_back (ICurvePrimitive::CreateArc (ellipse));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipticArcBaseHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != path.HasSingleCurvePrimitive ())
        return ERROR;

    DEllipse3d          ellipse = *path.front ()->GetArcCP ();
    BentleyStatus       status = ERROR;
    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElement, Create methods uses SetElementDescr...
    if (path.IsClosedPath ())
        status = EllipseHandler::CreateEllipseElement (newEeh, &eeh, ellipse, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ());
    else
        status = ArcHandler::CreateArcElement (newEeh, &eeh, ellipse, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ());

    if (SUCCESS == status)
        status = (BentleyStatus) eeh.ReplaceElement (newEeh.GetElementCP ());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipseHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_ELLIPSE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipseHandler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    AhEllipse   ellipse (el);

    if (ellipse.isCircular ())
        descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_Circle));
    else
        descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_ELLIPSE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArcHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_ARC_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArcHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;
    eeh.GetElementCP ()->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Arc_3d) / 2);

    elm.ToArc_3dR().startAngle  = eeh.GetElementCP ()->ToArc_2d().startAngle;
    elm.ToArc_3dR().sweepAngle  = eeh.GetElementCP ()->ToArc_2d().sweepAngle;
    elm.ToArc_3dR().primary     = eeh.GetElementCP ()->ToArc_2d().primary;
    elm.ToArc_3dR().secondary   = eeh.GetElementCP ()->ToArc_2d().secondary;

    DataConvert::RotationToQuaternion (elm.ToArc_3dR().quat, eeh.GetElementCP ()->ToArc_2d().rotationAngle);
    DataConvert::Points2dTo3d (&elm.ToArc_3dR().origin, &eeh.GetElementCP ()->ToArc_2d().origin, 1, elevation);

    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArcHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    double      r0  = eeh.GetElementCP ()->ToArc_3d().primary;
    double      r90 = eeh.GetElementCP ()->ToArc_3d().secondary;
    double      startAngle = eeh.GetElementCP ()->ToArc_3d().startAngle;
    double      sweepAngle = eeh.GetElementCP ()->ToArc_3d().sweepAngle;
    double      endAngle = startAngle + sweepAngle;
    RotMatrix   rMatrix;

    rMatrix.InitTransposedFromQuaternionWXYZ ( eeh.GetElementCP ()->ToArc_3d().quat);

    // see if the transformation has reduced the arc to a line
    if (fabs (r0) < s_axisRatioTolerance * fabs (r90))
        {
        double      min, max;
        DSegment3d  segment;

        min = sin (startAngle);
        max = sin (endAngle);

        if (min > max)
            std::swap (min, max);

        if (in_span (msGeomConst_piOver2, startAngle, sweepAngle))
            max = 1.0;

        if (in_span (msGeomConst_pi + msGeomConst_piOver2, startAngle, sweepAngle))
            min = -1.0;

        segment.point[0].x = eeh.GetElementCP ()->ToArc_3d().origin.x + min * rMatrix.form3d[0][1] * r90;
        segment.point[0].y = eeh.GetElementCP ()->ToArc_3d().origin.y + min * rMatrix.form3d[1][1] * r90;
        segment.point[1].x = eeh.GetElementCP ()->ToArc_3d().origin.x + max * rMatrix.form3d[0][1] * r90;
        segment.point[1].y = eeh.GetElementCP ()->ToArc_3d().origin.y + max * rMatrix.form3d[1][1] * r90;
        segment.point[0].z = segment.point[1].z = 0.0;

        EditElementHandle   tmpEeh;

        LineHandler::CreateLineElement (tmpEeh, &eeh, segment, false, *eeh.GetDgnModelP ());

        eeh.ReplaceElementDescr (tmpEeh.ExtractElementDescr().get());
        }
    else if (fabs (r90) < s_axisRatioTolerance * fabs (r0))
        {
        double      min, max;
        DSegment3d  segment;

        min = cos (startAngle);
        max = cos (endAngle);

        if (min > max)
            std::swap (min, max);

        if (in_span (0.0, startAngle, sweepAngle))
            max = 1.0;

        if (in_span (msGeomConst_pi, startAngle, sweepAngle))
            min = -1.0;

        segment.point[0].x = eeh.GetElementCP ()->ToArc_3d().origin.x + min * rMatrix.form3d[0][0] * r0;
        segment.point[0].y = eeh.GetElementCP ()->ToArc_3d().origin.y + min * rMatrix.form3d[1][0] * r0;
        segment.point[1].x = eeh.GetElementCP ()->ToArc_3d().origin.x + max * rMatrix.form3d[0][0] * r0;
        segment.point[1].y = eeh.GetElementCP ()->ToArc_3d().origin.y + max * rMatrix.form3d[1][0] * r0;
        segment.point[0].z = segment.point[1].z = 0.0;

        EditElementHandle   tmpEeh;

        LineHandler::CreateLineElement (tmpEeh, &eeh, segment, false, *eeh.GetDgnModelP ());

        eeh.ReplaceElementDescr (tmpEeh.ExtractElementDescr().get());
        }
    else
        {
        DgnV8ElementBlank   elm;

        eeh.GetElementCP ()->CopyTo (elm);

        if (rMatrix.form3d[2][2] < 0.0)
            {
            elm.ToArc_2dR().startAngle = -eeh.GetElementCP ()->ToArc_3d().startAngle;
            elm.ToArc_2dR().sweepAngle = -eeh.GetElementCP ()->ToArc_3d().sweepAngle;
            }
        else
            {
            elm.ToArc_2dR().startAngle = eeh.GetElementCP ()->ToArc_3d().startAngle;
            elm.ToArc_2dR().sweepAngle = eeh.GetElementCP ()->ToArc_3d().sweepAngle;
            }

        elm.ToArc_2dR().primary        = eeh.GetElementCP ()->ToArc_3d().primary;
        elm.ToArc_2dR().secondary      = eeh.GetElementCP ()->ToArc_3d().secondary;
        elm.ToArc_2dR().rotationAngle  = Angle::Atan2 (rMatrix.form3d[1][0], rMatrix.form3d[0][0]);

        DataConvert::Points3dTo2d (&elm.ToArc_2dR().origin, &eeh.GetElementCP ()->ToArc_3d().origin, 1);

        elm.SetSizeWordsNoAttributes(sizeof (Arc_2d) / 2);

        ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

        eeh.ReplaceElement (&elm);
        }

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipseHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Ellipse_3d) / 2);

    elm.ToEllipse_3dR().primary     = eeh.GetElementCP ()->ToEllipse_2d().primary;
    elm.ToEllipse_3dR().secondary   = eeh.GetElementCP ()->ToEllipse_2d().secondary;

    DataConvert::RotationToQuaternion (elm.ToEllipse_3dR().quat, eeh.GetElementCP ()->ToEllipse_2d().rotationAngle);
    DataConvert::Points2dTo3d (&elm.ToEllipse_3dR().origin, &eeh.GetElementCP ()->ToEllipse_2d().origin, 1, elevation);

    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipseHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    double      r0  = eeh.GetElementCP ()->ToEllipse_3d().primary;
    double      r90 = eeh.GetElementCP ()->ToEllipse_3d().secondary;
    RotMatrix   rMatrix;

    rMatrix.InitTransposedFromQuaternionWXYZ ( eeh.GetElementCP ()->ToEllipse_3d().quat);

    // see if the transformation has reduced the ellipse to a line
    if (fabs (r0) < s_axisRatioTolerance * fabs (r90))
        {
        DSegment3d  segment;

        segment.point[0].x = eeh.GetElementCP ()->ToEllipse_3d().origin.x + rMatrix.form3d[0][1] * r90;
        segment.point[0].y = eeh.GetElementCP ()->ToEllipse_3d().origin.y + rMatrix.form3d[1][1] * r90;
        segment.point[1].x = eeh.GetElementCP ()->ToEllipse_3d().origin.x - rMatrix.form3d[0][1] * r90;
        segment.point[1].y = eeh.GetElementCP ()->ToEllipse_3d().origin.y - rMatrix.form3d[1][1] * r90;
        segment.point[0].z = segment.point[1].z = 0.0;

        EditElementHandle   tmpEeh;

        LineHandler::CreateLineElement (tmpEeh, &eeh, segment, false, *eeh.GetDgnModelP ());

        eeh.ReplaceElementDescr (tmpEeh.ExtractElementDescr().get());
        }
    else if (fabs (r90) < s_axisRatioTolerance * fabs (r0))
        {
        DSegment3d  segment;

        segment.point[0].x = eeh.GetElementCP ()->ToEllipse_3d().origin.x + rMatrix.form3d[0][0] * r0;
        segment.point[0].y = eeh.GetElementCP ()->ToEllipse_3d().origin.y + rMatrix.form3d[1][0] * r0;
        segment.point[1].x = eeh.GetElementCP ()->ToEllipse_3d().origin.x - rMatrix.form3d[0][0] * r0;
        segment.point[1].y = eeh.GetElementCP ()->ToEllipse_3d().origin.y - rMatrix.form3d[1][0] * r0;
        segment.point[0].z = segment.point[1].z = 0.0;

        EditElementHandle   tmpEeh;

        LineHandler::CreateLineElement (tmpEeh, &eeh, segment, false, *eeh.GetDgnModelP ());

        eeh.ReplaceElementDescr (tmpEeh.ExtractElementDescr().get());
        }
    else
        {
        DgnV8ElementBlank   elm;

        eeh.GetElementCP ()->CopyTo (elm);

        DataConvert::Points3dTo2d (&elm.ToEllipse_2dR().origin, &eeh.GetElementCP ()->ToEllipse_3d().origin, 1);

        elm.ToEllipse_2dR().rotationAngle = Angle::Atan2 (rMatrix.form3d[1][0], rMatrix.form3d[0][0]);
        elm.ToEllipse_2dR().primary       = eeh.GetElementCP ()->ToEllipse_3d().primary;
        elm.ToEllipse_2dR().secondary     = eeh.GetElementCP ()->ToEllipse_3d().secondary;

        elm.SetSizeWordsNoAttributes(sizeof (Ellipse_2d) / 2);

        ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

        eeh.ReplaceElement (&elm);
        }

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ArcHandler::Extract
(
DPoint3dP       start_pt,
DPoint3dP       end_pt,
double*         start,
double*         sweep,
double*         x1,
double*         x2,
RotMatrixP      trans,
double*         rot,
DPoint3dP       center,
ElementHandleCR    eh
)
    {
    DgnElementCR elm = *eh.GetElementCP ();
    RotMatrix   tmpTrans;
    DPoint3d    tmpCenter;
    double      tmpStart, tmpSweep, tmpX1, tmpX2, tmpRot;

    if (!start) start = &tmpStart;
    if (!sweep) sweep = &tmpSweep;
    if (!x1) x1 = &tmpX1;
    if (!x2) x2 = &tmpX2;
    if (!trans) trans = &tmpTrans;
    if (!rot) rot = &tmpRot;
    if (!center) center = &tmpCenter;

    if (ELLIPSE_ELM == elm.ToArc_2d().GetLegacyType())
        {
        EllipseHandler::Extract (start_pt, x1, x2, trans, rot, center, eh);
        *start=0.0;
        *sweep=msGeomConst_2pi;
        if (end_pt && start_pt)
            *end_pt = *start_pt;

        return SUCCESS;
        }
    else if (ARC_ELM == elm.GetLegacyType())
        {
        *start = elm.ToArc_2d().startAngle;
        *sweep = elm.ToArc_2d().sweepAngle;

        *x1 = elm.ToArc_2d().primary;
        *x2 = elm.ToArc_2d().secondary;

        if (elm.Is3d())
            {
            *center = elm.ToArc_3d().origin;
            trans->InitTransposedFromQuaternionWXYZ ( elm.ToArc_3d().quat);
            }
        else
            {
            center->x = elm.ToArc_2d().origin.x;
            center->y = elm.ToArc_2d().origin.y;
            center->z = 0.0;
            *rot = elm.ToArc_2d().rotationAngle;
            trans->InitFromAxisAndRotationAngle(2,  *rot);
            }

        if (end_pt)
            {
            end_pt->x = *x1 * cos(*start + *sweep);
            end_pt->y = *x2 * sin(*start + *sweep);
            end_pt->z = 0.0;

            trans->Multiply(*end_pt);
            bsiDVec3d_addInPlace ((DVec3dP)end_pt, (DVec3dCP) center);
            }

        if (start_pt)
            {
            start_pt->x = *x1 * cos (*start);
            start_pt->y = *x2 * sin (*start);
            start_pt->z = 0.0;

            trans->Multiply(*start_pt);

            bsiDVec3d_addInPlace ((DVec3dP)start_pt, (DVec3dCP) center);
            }

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    05/86
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus arc_create
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCP      center,                 /* => center of ellipse            */
double          axis1,                  /* => primary axis                 */
double          axis2,                  /* => secondary axis                       */
double*         pRotationAngle,         /* => rotation angle (if NULL == rMatrix)    */
RotMatrixCP     rMatrix,                /* => rotation transformation              */
double          start,                  /* => start angle */
double          sweep,                  /* => sweep angle */
bool            is3d,                   /* => 3D */
DgnModelR    modelRef
)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, ARC_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof (Arc_3d));
        ElementUtil::SetRequiredFields (out, ARC_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    RotMatrix   tmpRMatrix;

    if ((!is3d && NULL != pRotationAngle) || NULL == rMatrix)
        {
        tmpRMatrix.InitFromAxisAndRotationAngle(2,  NULL == pRotationAngle ? 0.0 : *pRotationAngle);
        rMatrix = &tmpRMatrix;
        }

    out.ToArc_2dR().primary     = axis1;
    out.ToArc_2dR().secondary   = axis2;
    out.ToArc_2dR().startAngle  = start;
    out.ToArc_2dR().sweepAngle  = sweep;

    int         elmSize;

    if (is3d)
        {
        /* set the quaternion here */
        rMatrix->GetQuaternion(out.ToArc_3dR().quat, true);

        out.ToArc_3dR().origin = *center;
        elmSize = sizeof (Arc_3d);
        }
    else
        {
        out.ToArc_2dR().rotationAngle = (NULL == pRotationAngle) ? rMatrix->ColumnXAngleXY () : *pRotationAngle;

        DVec3d      xVec, yVec;

        bsiRotMatrix_getColumn (rMatrix, &xVec, 0);
        bsiRotMatrix_getColumn (rMatrix, &yVec, 1);

        if (bsiDVec3d_crossProductXY (&xVec, &yVec) < 0.0)
           {
           out.ToArc_2dR().startAngle *= -1.0;
           out.ToArc_2dR().sweepAngle *= -1.0;
           }

        out.ToArc_2dR().origin.init (center);
        elmSize = sizeof (Arc_2d);
        }

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ArcHandler::CreateArcElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCR      center,                 // => center of ellipse
double          axis1,                  // => primary axis
double          axis2,                  // => secondary axis
double          rotationAngle,          // => rotation angle
double          start,                  // => start angle
double          sweep,                  // => sweep angle
bool            is3d,
DgnModelR    modelRef
)
    {
    return arc_create (eeh, templateEh, &center, axis1, axis2, &rotationAngle, NULL, start, sweep, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ArcHandler::CreateArcElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCR      center,                 // => center of ellipse
double          axis1,                  // => primary axis
double          axis2,                  // => secondary axis
RotMatrixCR     rMatrix,                // => rotation transformation
double          start,                  // => start angle
double          sweep,                  // => sweep angle
bool            is3d,
DgnModelR    modelRef
)
    {
    return arc_create (eeh, templateEh, &center, axis1, axis2, NULL, &rMatrix, start, sweep, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ArcHandler::CreateArcElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DEllipse3dCR        ellipse,
bool                is3d,
DgnModelR        modelRef
)
    {
    DPoint3d    center;
    RotMatrix   rMatrix;
    double      r0, r90, start, sweep;

    ellipse.getScaledRotMatrix (&center, &rMatrix, &r0, &r90, &start, &sweep);

    return arc_create (eeh, templateEh, &center, r0, r90, NULL, &rMatrix, start, sweep, is3d, modelRef);
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
static ReprojectStatus      replaceFromTwoPoints
(
EditElementHandleR                  source,
DPoint3dP                           points,
DPoint3dCP                          center,
double                              primary,
IGeoCoordinateReprojectionHelper&   reprojectionHelper
)
    {
    // if we get here, we know that the arc either has identical start and end points, or has all points on it colinear.
    DgnElementP      el      = source.GetElementP();
    bool            is3d    = DisplayHandler::Is3dElem (el);
    DPoint3d        transformedPoints[3];

    // put the center in points[1] and transform all three points.
    points[1] = *center;
    reprojectionHelper.ReprojectPoints (transformedPoints, NULL, NULL, points, 3);

    // Is it a point (0-length primary axis) ?
    if (abs (primary) < mgds_fc_epsilon)
        {
        DgnElementP  outEl = (DgnElementP) _alloca (sizeof (Arc_3d) + 2000);  // don't need full 120K element.

        // just change the center, retaining all other parameters.
        el->CopyTo (*outEl);

        if (is3d)
            outEl->ToArc_3d().origin = transformedPoints[1];
        else
            outEl->ToArc_2d().origin.init (&transformedPoints[1]);

        source.ReplaceElement (outEl);

        return REPROJECT_Success;
        }

    // sweep is either very small or 2*pi. Compute arc from 2 points and center, and then substitute original sweep.
    DEllipse3d  arcRepresentation;
    if (!arcRepresentation.InitFromArcCenterStartEnd (transformedPoints[1], transformedPoints[0], transformedPoints[2]))
        return REPROJECT_DataError;

    EditElementHandle  tmpEeh;

    if (SUCCESS != ArcHandler::CreateArcElement (tmpEeh, &source, arcRepresentation, is3d, *reprojectionHelper.GetDestinationDgnModel()))
        return REPROJECT_DataError;

    DgnElementP  newEl       = tmpEeh.GetElementP();

    // make sure sweep stays the same.
    if (is3d)
        newEl->ToArc_3d().sweepAngle = el->ToArc_3d().sweepAngle;
    else
        newEl->ToArc_2d().sweepAngle = el->ToArc_2d().sweepAngle;

    if (SUCCESS != source.ReplaceElementDescr (tmpEeh.ExtractElementDescr()))
        return REPROJECT_DataError;

    return REPROJECT_Success;
    }
#endif

#define MAX_ARC_STROKE_POINTS 31

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus     ArcHandler::_OnGeoCoordinateReprojection (EditElementHandleR  source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (reprojectionHelper.ShouldStroke (source, settings->StrokeArcs()))
        return GeoCoordinateReprojectionStrokeElement (source, reprojectionHelper, inChain);

    DgnElementCP     el      = source.GetElementCP();
    bool            is3d    = Is3dElem (el);
    ReprojectStatus status  = REPROJECT_Success;
    // Relative error tolerance between pure point transform and linearized ellipse transform.
    static double s_relErrorTol = 1.0e-3;
    double      strokeFraction[MAX_ARC_STROKE_POINTS];

    // Preferred transformation:
    // 1) Reproject 3 keypoints on the ellipse.
    // 2) From 3point-to-3point determine a linear transformation.
    // 3) Apply that linear transformation to the whole ellipse.
    // 4) Due to keypoint selection, we are assured exact match of:
    //     a) partial arc: start middle end
    //     b) full: start, 1/3 and 2/3
    //
    // As a check on how much the reprojection deviates from the linearization,
    //   do point-by-point reprojection of points at {6?} degree increments.
    // If max error, as a fraction of mean ellipse radius, is large, drop back to linestring.
    AhEllipse   ellipse (source, NULL);
    bool        isFull = ellipse.isFullEllipse ();

    double targetFraction[3];
    targetFraction[0] = 0.0;
    if (isFull)
        {
        targetFraction[1] = 1.0 / 3.0;
        targetFraction[2] = 2.0 / 3.0;
        }
    else
        {
        targetFraction[1] = 0.5;
        targetFraction[2] = 1.0;
        }

    DPoint3d pointA[3]; // original start + either (middle angle, end angle) or (1/3, 2/3)
    for (int iPoint = 0; iPoint < 3; iPoint++)
        {
        ellipse.fractionParameterToPoint (&pointA[iPoint], targetFraction[iPoint]);
        }

    int numStrokeEdges = (int) (0.999 + (double)(MAX_ARC_STROKE_POINTS - 1) * fabs (ellipse.sweep / msGeomConst_2pi));
    if (numStrokeEdges > MAX_ARC_STROKE_POINTS - 1)
        numStrokeEdges = MAX_ARC_STROKE_POINTS - 1;
    if (numStrokeEdges < 2)
        numStrokeEdges = 2;

    int         numStrokePoints = numStrokeEdges + 1;
    double      strokeStep = 1.0 / (double) numStrokeEdges;
    DPoint3d    strokePointA[MAX_ARC_STROKE_POINTS];
    for (int iEdge = 0; iEdge <= numStrokeEdges; iEdge++)
        {
        strokeFraction[iEdge] = iEdge * strokeStep;
        ellipse.fractionParameterToPoint (&strokePointA[iEdge], strokeFraction[iEdge]);
        }

    // Reproject pointA's to pointB's..
    DPoint3d pointB[3];
    status = reprojectionHelper.ReprojectPoints (pointB, NULL, NULL, pointA, 3);

    // stroke strokePointA's to strokePointB's
    DPoint3d            strokePointB[MAX_ARC_STROKE_POINTS];
    ReprojectStatus     pointStatus = reprojectionHelper.ReprojectPoints (strokePointB, NULL, NULL, strokePointA, numStrokePoints);

    // Apply linear transform to middle and end of arc (or 1/3+2/3 points of full)
    DVec3d vectorAU, vectorAV, vectorAW;
    DVec3d vectorBU, vectorBV, vectorBW;
    DTransform3d frameA, frameB, frameAInverse;

    // frame A is the original ellipse
    vectorAU.DifferenceOf (pointA[1], pointA[0]);
    vectorAV.differenceOf (&pointA[2], &pointA[0]);
    vectorAW.geometricMeanCrossProduct (&vectorAU, &vectorAV);
    frameA.initFromOriginAndVectors (&pointA[0], &vectorAU, &vectorAV, &vectorAW);

    // frame D is the reprojections of original ellipse points
    vectorBU.differenceOf (&pointB[1], &pointB[0]);
    vectorBV.differenceOf (&pointB[2], &pointB[0]);
    vectorBW.geometricMeanCrossProduct (&vectorBU, &vectorBV);
    frameB.initFromOriginAndVectors (&pointB[0], &vectorBU, &vectorBV, &vectorBW);

    if (REPROJECT_Success == status && REPROJECT_Success == pointStatus && frameAInverse.inverseOf (&frameA))
        {
        DEllipse3d ellipseA1, ellipseB;
        ellipseA1.productOf (&frameAInverse, &ellipse);
        ellipseB.productOf (&frameB, &ellipseA1);

        double eMax = 0.0;
        for (int iPoint = 0; iPoint < numStrokePoints; iPoint++)
            {
            double      error;
            DPoint3d    testPoint;
            ellipseB.fractionParameterToPoint (&testPoint, strokeFraction[iPoint]);
            error = testPoint.distance (&strokePointB[iPoint]);
            if (error > eMax)
                eMax = error;
            }
        double refDistance  = vectorBW.magnitude ();
        double relError     = eMax / refDistance;
        EditElementHandle  replacement;
        if ( (relError < s_relErrorTol) && (SUCCESS == CreateArcElement (replacement, &source, ellipseB, is3d, *source.GetDgnModelP())) )
            {
            source.ReplaceElementDescr (replacement.ExtractElementDescr().get());
            return REPROJECT_Success;
            }
        }

    // Arc did not transform cleanly -- stroke and output linestring.
    status = GeoCoordinateReprojectionReplaceElement (source, strokePointB, numStrokePoints, inChain, false);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus EllipseHandler::_OnGeoCoordinateReprojection (EditElementHandleR  source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (reprojectionHelper.ShouldStroke (source, settings->StrokeEllipses()))
        return GeoCoordinateReprojectionStrokeElement (source, reprojectionHelper, inChain);
    else
        return T_Super::_OnGeoCoordinateReprojection (source, reprojectionHelper, inChain);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipseHandler::Extract
(
DPoint3dP       sf_pt,        /* <= start/finish point of ellipse     */
double*         x1,           /* <= major axis of ellipse             */
double*         x2,           /* <= minor axis of ellipse             */
RotMatrixP      trans,        /* <= ellipse transformation            */
double*         rot,          /* <= rotation angle (2d only)          */
DPoint3dP       center,       /* <= center point of ellipse           */
ElementHandleCR    eh
)
    {
    DgnElementCR elm = *eh.GetElementCP ();

    if (ELLIPSE_ELM != elm.GetLegacyType())
        return ERROR;

    RotMatrix tmpTrans;
    DPoint3d  tmpCenter;
    double    tmpX1, tmpRot;

    Ellipse_2d const& ellipse_2d = elm.ToEllipse_2d();
    Ellipse_3d const& ellipse_3d = elm.ToEllipse_3d();

    if (!x1) x1 = &tmpX1;
    if (!trans) trans = &tmpTrans;
    if (!rot) rot = &tmpRot;
    if (!center) center = &tmpCenter;

    *x1 = ellipse_2d.primary;

    if (x2)
        *x2 = ellipse_2d.secondary;

    if (ellipse_2d.Is3d())
        {
        *center = ellipse_3d.origin;
        trans->InitTransposedFromQuaternionWXYZ ( ellipse_3d.quat);
        }
    else
        {
        center->x = ellipse_2d.origin.x;
        center->y = ellipse_2d.origin.y;
        center->z = 0.0;
        *rot = ellipse_2d.rotationAngle;
        trans->InitFromAxisAndRotationAngle(2,  *rot);
        }

    if (sf_pt)
        {
        sf_pt->x = *x1;
        sf_pt->y = 0.0;
        sf_pt->z = 0.0;
        trans->Multiply(*sf_pt);
        bsiDVec3d_addInPlace ((DVec3dP)sf_pt, (DVec3dCP) center);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    05/86
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus ellipse_create
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCP      center,                 /* => center of ellipse            */
double          axis1,                  /* => primary axis                 */
double          axis2,                  /* => secondary axis                       */
double*         pRotationAngle,         /* => rotation angle (if NULL == rMatrix)    */
RotMatrixCP     rMatrix,                /* => rotation transformation              */
bool            is3d,                   /* => 3D */
DgnModelR    modelRef
)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, ELLIPSE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof (Ellipse_3d));
        ElementUtil::SetRequiredFields (out, ELLIPSE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    RotMatrix   tmpRMatrix;

    if ((!is3d && NULL != pRotationAngle) || NULL == rMatrix)
        {
        tmpRMatrix.InitFromAxisAndRotationAngle(2,  NULL == pRotationAngle ? 0.0 : *pRotationAngle);
        rMatrix = &tmpRMatrix;
        }

    out.ToEllipse_2dR().primary     = axis1;
    out.ToEllipse_2dR().secondary   = axis2;

    int         elmSize;

    if (is3d)
        {
        /* set the quaternion here */
        rMatrix->GetQuaternion(out.ToEllipse_3dR().quat, true);

        out.ToEllipse_3dR().origin = *center;
        elmSize = sizeof (Ellipse_3d);
        }
    else
        {
        out.ToEllipse_2dR().rotationAngle = (NULL == pRotationAngle) ? rMatrix->ColumnXAngleXY () : *pRotationAngle;

        out.ToEllipse_2dR().origin.init (center);
        elmSize = sizeof (Ellipse_2d);
        }

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipseHandler::CreateEllipseElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCR      center,                 // => center of ellipse
double          axis1,                  // => primary axis
double          axis2,                  // => secondary axis
double          rotationAngle,          // => rotation angle
bool            is3d,
DgnModelR    modelRef
)
    {
    return ellipse_create (eeh, templateEh, &center, axis1, axis2, &rotationAngle, NULL, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipseHandler::CreateEllipseElement
(
EditElementHandleR eeh,
ElementHandleCP    templateEh,
DPoint3dCR      center,                 // => center of ellipse
double          axis1,                  // => primary axis
double          axis2,                  // => secondary axis
RotMatrixCR     rMatrix,                // => rotation transformation
bool            is3d,
DgnModelR    modelRef
)
    {
    return ellipse_create (eeh, templateEh, &center, axis1, axis2, NULL, &rMatrix, is3d, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   EllipseHandler::CreateEllipseElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DEllipse3dCR        ellipse0,
bool                is3d,
DgnModelR        modelRef
)
    {
    DEllipse3d  ellipse1 = DEllipse3d::FromCopyWithPositiveSweep (ellipse0); // Sign of sweep denotes normal direction...
    DPoint3d    center;
    RotMatrix   rMatrix;
    double      r0, r90;

    ellipse1.getScaledRotMatrix (&center, &rMatrix, &r0, &r90, NULL, NULL);

    return ellipse_create (eeh, templateEh, &center, r0, r90, NULL, &rMatrix, is3d, modelRef);
    }

