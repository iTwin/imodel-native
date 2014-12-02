/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ConeHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define TOLERANCE_ChordAngle            .1
#define TOLERANCE_ChordLen              1000

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_CONE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          ConeHandler::ExtractTopRadius (ElementHandleCR eh)
    {
    return eh.GetElementCP ()->ToCone_3d().radius_1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          ConeHandler::ExtractBottomRadius (ElementHandleCR eh)
    {
    return eh.GetElementCP ()->ToCone_3d().radius_2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCP      ConeHandler::ExtractTopCenter (ElementHandleCR eh)
    {
    return &eh.GetElementCP ()->ToCone_3d().center_1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCP      ConeHandler::ExtractBottomCenter (ElementHandleCR eh)
    {
    return &eh.GetElementCP ()->ToCone_3d().center_2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh)
    {
    rMatrix.InitTransposedFromQuaternionWXYZ ( eh.GetElementCP ()->ToCone_3d().quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ConeHandler::ExtractCapFlag (ElementHandleCR eh)
    {
    return (!eh.GetElementCP ()->ToCone_3d().b.surf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConeHandler::GetConeData (ElementHandleCR source, RotMatrixP rMatrix, DPoint3dP center0, DPoint3dP center1, double* r0, double* r1, bool* capped)
    {
    if (rMatrix)
        ExtractRotation (*rMatrix, source);

    if (center0)
        *center0 = *ExtractTopCenter (source);
    
    if (center1)
        *center1 = *ExtractBottomCenter (source);
    
    if (r0)
        *r0 = ExtractTopRadius (source);
    
    if (r1)
        *r1 = ExtractBottomRadius (source);

    if (capped)
        *capped = ExtractCapFlag (source);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConeHandler::SetConeData (EditElementHandleR eeh, RotMatrixCR rMatrix, DPoint3dCR center0, DPoint3dCR center1, double r0, double r1, bool capped)
    {
    EditElementHandle  newEeh;

    // NOTE: In case eeh is component use ReplaceElement, Create methods uses SetElementDescr...
    if (SUCCESS != ConeHandler::CreateConeElement (newEeh, &eeh, r0, r1, center0, center1, rMatrix, capped, *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElement (newEeh.GetElementCP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConeHandler::_GetSolidPrimitive (ElementHandleCR eh, ISolidPrimitivePtr& primitive)
    {
    bool        capped;
    double      r0, r1;
    DPoint3d    center0, center1;
    RotMatrix   axes;

    if (SUCCESS != GetConeData (eh, &axes, &center0, &center1, &r0, &r1, &capped))
        return ERROR;

    DgnConeDetail  detail (center1, center0, axes, r1, r0, capped);
    
    primitive = ISolidPrimitive::CreateDgnCone (detail);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConeHandler::_SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive)
    {
    DgnConeDetail  detail;

    if (!primitive.TryGetDgnConeDetail (detail))
        return ERROR;

    bool        capped;
    double      radiusA, radiusB;
    DPoint3d    centerA, centerB;
    RotMatrix   rMatrix;

    if (!detail.IsCircular (centerA, centerB, rMatrix, radiusA, radiusB, capped))
        return ERROR;

    return SetConeData (eeh, rMatrix, centerB, centerA, radiusB, radiusA, capped);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      ConeHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    SnapPathP       snap = context->GetSnapPath ();
    SnapMode    snapMode = context->GetSnapMode ();
    bool            forceHot = false;
    DPoint3d        hitPoint;
    DSegment3d      seg;
    ElementHandle   eh (snap->GetPathElem (snapPathIndex));

    seg.point[0] = *ExtractTopCenter (eh);
    seg.point[1] = *ExtractBottomCenter (eh);

    switch (snapMode)
        {
#if defined (CREATE_DEPRECATED_CUSTOM_ASSOC)
        // NOTE: In SS3 Sam created a 1 byte custom keypoint data linkage which is too small and triggers an assert in insertNewLinkage.
        //       Should be ok to change size since it's not checked in _EvaluateCustomKeypoint...
        //       Unless it becomes an issue it's better to stop creating these and let the new TopologyCurveAssociation generator create the associations.
        case SnapMode::Origin:
            {
            byte  customData[4];

            memset (customData, 0, sizeof (customData));
            customData[0] = 'b';

            context->ElmLocalToWorld (seg.point[0]);
            context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), seg.point[0], true, false, sizeof (customData), customData);

            return SnapStatus::Success;
            }

        case SnapMode::NearestKeypoint:
            {
            SnapStatus  snapStatus = context->DoDefaultDisplayableSnap (snapPathIndex);

            if (SnapStatus::Success == snapStatus)
                {
                byte  customData[4];

                memset (customData, 0, sizeof (customData));

                if (seg.point[0].IsEqual (snap->GetSnapPoint ()))
                    {
                    customData[0] = 'b';
                    snap->SetCustomKeypoint (sizeof (customData), customData);
                    }
                else if (seg.point[1].IsEqual (snap->GetSnapPoint ()))
                    {
                    customData[0] = 't';
                    snap->SetCustomKeypoint (sizeof (customData), customData);
                    }
                }

            return snapStatus;
            }
#else
        case SnapMode::Origin:
            {
            hitPoint = seg.point[0];
            forceHot = true;
            break;
            }
#endif

        case SnapMode::MidPoint:
            {
            GeomDetailCR  detail = snap->GetGeomDetail ();

            // NOTE: if we're doing "midpoint" snapping, and hit is a segment...use it's midpoint.
            if (HitGeomType::Segment == detail.GetGeomType ())
                return context->DoDefaultDisplayableSnap (snapPathIndex);

            seg.FractionParameterToPoint (hitPoint, 0.5);
            break;
            }

        case SnapMode::Center:
            {
            GeomDetailCR  detail = snap->GetGeomDetail ();

            // NOTE: if we're doing "center" snapping, and hit is ellipse/point use it's center.
            if (HitGeomType::Arc == detail.GetGeomType () || HitGeomType::Point == detail.GetGeomType ())
                return context->DoDefaultDisplayableSnap (snapPathIndex);

            seg.FractionParameterToPoint (hitPoint, 0.5);
            forceHot = true;
            break;
            }

        case SnapMode::Bisector:
            {
            seg.FractionParameterToPoint (hitPoint, 0.5);
            forceHot = true;
            break;
            }

        default:
            return context->DoDefaultDisplayableSnap (snapPathIndex);
        }

    // NOTE: Point from extracted element data is in local coords!
    context->ElmLocalToWorld (hitPoint);
    context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), hitPoint, forceHot, false);

    return SnapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2007
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_EvaluateCustomKeypoint
(
ElementHandleCR elHandle,
DPoint3dP       outPointP,
byte*           customKeypointData
)
    {
    // NOTE: Evaluate an existing custom assoc, we don't create these anymore in Vancouver (see CREATE_DEPRECATED_CUSTOM_ASSOC)...
    DSegment3d  seg;

    seg.point[0] = *ExtractTopCenter (elHandle);
    seg.point[1] = *ExtractBottomCenter (elHandle);

    char const* c = (char const*)customKeypointData;
    *outPointP = (*c == 'b')? seg.point[0]: seg.point[1];

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    DgnElementCP elm = elHandle.GetElementCP ();

    origin.interpolate (&elm->ToCone_3d().center_1, 0.5, &elm->ToCone_3d().center_2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR rMatrix)
    {
    ConeHandler::ExtractRotation (rMatrix, elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    ISolidPrimitivePtr  primitive;

    if (SUCCESS != _GetSolidPrimitive (thisElm, primitive))
        return;

    context.GetIDrawGeom().DrawSolidPrimitive (*primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_OnTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    StatusInt   status;

    if (SUCCESS != (status = T_Super::_OnTransform (elemHandle, trans)))
        return status;

    BeAssert (CONE_ELM == elemHandle.GetLegacyType());

    RotMatrix   rMatrix;
    double      r1, r2;
    DPoint3d    center1, center2;

    ConeHandler::GetConeData (elemHandle, &rMatrix, &center1, &center2, &r1, &r2, NULL);

    // scale the center point
    ( trans.GetTransform())->Multiply(center1);
    ( trans.GetTransform())->Multiply(center2);

    double      primary, secondary, rotation, start = 0.0, rtmp = 0.0;
    RotMatrix   rmSave = rMatrix; // Save for 2nd call to LegacyMath::TMatrix::TransformArc...

    primary = secondary = r2;
    LegacyMath::TMatrix::TransformArc (&primary, &secondary, &rMatrix, &rotation, &start, &rtmp, trans.GetTransform(), true);
    r2 = (primary+secondary)/2.0;

    primary = secondary = r1;
    LegacyMath::TMatrix::TransformArc (&primary, &secondary, &rmSave, &rotation, &start, &rtmp, trans.GetTransform(), true);
    r1 = (primary+secondary)/2.0;

    Cone_3d*    cone = &elemHandle.GetElementP()->ToCone_3dR();

    cone->center_1 = center1;
    cone->center_2 = center2;
    cone->radius_1 = r1;
    cone->radius_2 = r2;

    if (fabs (r2) >= fabs (r1))
        rMatrix.GetQuaternion (cone->quat, true);
    else
        rmSave.GetQuaternion (cone->quat, true);


    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_ApplyTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    StatusInt   status;
    bool        wasHandled;

    if (SUCCESS != (status = ElementUtil::NonUniformScaleAsBsplineSurf (wasHandled, elemHandle, trans)) || wasHandled)
        return status;

    return T_Super::_ApplyTransform (elemHandle, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConeHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    EditElementHandle  cellEeh;

    NormalCellHeaderHandler::CreateOrphanCellElement (cellEeh, L"From Cone", true, *eeh.GetDgnModelP ());

    double      r1, r2;
    DPoint3d    center1, center2;
    RotMatrix   rMatrix;

    ConeHandler::GetConeData (eeh, &rMatrix, &center1, &center2, &r1, &r2, NULL);

    EditElementHandle  tmpEeh;

    if (fabs (r1) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (tmpEeh, NULL, center1, r1, r1, rMatrix, true, *eeh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eeh);

        NormalCellHeaderHandler::AddChildElement (cellEeh, tmpEeh);
        }

    if (fabs (r2) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (tmpEeh, NULL, center2, r2, r2, rMatrix, true, *eeh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eeh);

        NormalCellHeaderHandler::AddChildElement (cellEeh, tmpEeh);
        }

    int         nProfiles;
    double      angles[4];

    if (true) // always have a direction...
        {
        RotMatrix   coneToView;

        nProfiles = 2;
        coneToView.InitProduct(flattenTrans, rMatrix);
        extract_coneAngles (angles, &coneToView, NULL, NULL, 0.0, NULL);
        }
    else
        {
        nProfiles = 4;

        for (int i=0; i<nProfiles; i++)
            angles[i] = (double) i * msGeomConst_piOver2;
        }

    for (int i=0; i<nProfiles; i++)
        {
        DSegment3d  profile;

        get_ellpnt (&profile.point[0], angles[i], r1, r1, &rMatrix, &center1);
        get_ellpnt (&profile.point[1], angles[i], r2, r2, &rMatrix, &center2);

        if (SUCCESS == LineHandler::CreateLineElement (tmpEeh, NULL, profile, true, *eeh.GetDgnModelP ()))
            {
            ElementPropertiesSetter::ApplyTemplate (tmpEeh, eeh);

            NormalCellHeaderHandler::AddChildElement (cellEeh, tmpEeh);
            }
        }

    NormalCellHeaderHandler::AddChildComplete (cellEeh);
    eeh.ReplaceElementDescr (cellEeh.ExtractElementDescr().get());

    eeh.GetHandler().ConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                 Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_OnFenceClip
(
ElementAgendaP      inside,
ElementAgendaP      outside,
ElementHandleCR     eh,
FenceParamsP        fp,
FenceClipFlags      options
)
    {
    double      r1, r2;
    DPoint3d    center1, center2;
    RotMatrix   rMatrix;

    ConeHandler::GetConeData (eh, &rMatrix, &center1, &center2, &r1, &r2, NULL);

    EditElementHandle  tmpEeh;

    if (fabs (r1) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (tmpEeh, NULL, center1, r1, r1, rMatrix, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eh);

        tmpEeh.GetHandler().FenceClip (inside, outside, tmpEeh, fp, options);
        }

    if (fabs (r2) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (tmpEeh, NULL, center2, r2, r2, rMatrix, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eh);

        tmpEeh.GetHandler().FenceClip (inside, outside, tmpEeh, fp, options);
        }

    double      angles[2];
    RotMatrix   coneToView;

    coneToView.InitProduct(*( fp->GetTransform ()), rMatrix);
    extract_coneAngles (angles, &coneToView, NULL, NULL, 0.0, NULL);

    DSegment3d  profile;

    get_ellpnt (&profile.point[0], angles[0], r1, r1, &rMatrix, &center1);
    get_ellpnt (&profile.point[1], angles[0], r2, r2, &rMatrix, &center2);

    if (SUCCESS == LineHandler::CreateLineElement (tmpEeh, NULL, profile, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eh);

        tmpEeh.GetHandler().FenceClip (inside, outside, tmpEeh, fp, options);
        }

    get_ellpnt (&profile.point[0], angles[1], r1, r1, &rMatrix, &center1);
    get_ellpnt (&profile.point[1], angles[1], r2, r2, &rMatrix, &center2);

    if (SUCCESS == LineHandler::CreateLineElement (tmpEeh, NULL, profile, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (tmpEeh, eh);

        tmpEeh.GetHandler().FenceClip (inside, outside, tmpEeh, fp, options);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    MSElementDescr  *edP = elemHandle.GetElementDescrP();

    if (fp->PointInside (edP->Element().ToCone_3d().center_1))
        transform.GetTransform()->Multiply(*(&edP->ElementR().ToCone_3dR().center_1));

    if (fp->PointInside (edP->Element().ToCone_3d().center_2))
        transform.GetTransform()->Multiply(*(&edP->ElementR().ToCone_3dR().center_2));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConeHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Solids & geometry.GetOptions ()))
        return ERROR;

    if (DropGeometry::SOLID_Surfaces == geometry.GetSolidsOptions () && !ConeHandler::ExtractCapFlag (eh))
        return ERROR; // Already a surface...

    double      r1, r2;
    DPoint3d    center1, center2;
    RotMatrix   rMatrix;

    ConeHandler::GetConeData (eh, &rMatrix, &center1, &center2, &r1, &r2, NULL);

    EditElementHandle  newEeh;

    if (DropGeometry::SOLID_Surfaces == geometry.GetSolidsOptions ())
        {
        if (SUCCESS != ConeHandler::CreateConeElement (newEeh, &eh, r1, r2, center1, center2, rMatrix, false, *eh.GetDgnModelP ()))
            return ERROR;

        dropGeom.Insert (newEeh);

        if (fabs (r1) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (newEeh, NULL, center1, r1, r1, rMatrix, true, *eh.GetDgnModelP ()))
            {
            ElementPropertiesSetter::ApplyTemplate (newEeh, eh);

            dropGeom.Insert (newEeh);
            }

        if (fabs (r2) > 1.0e-12 && SUCCESS == EllipseHandler::CreateEllipseElement (newEeh, NULL, center2, r2, r2, rMatrix, true, *eh.GetDgnModelP ()))
            {
            ElementPropertiesSetter::ApplyTemplate (newEeh, eh);

            dropGeom.Insert (newEeh);
            }

        return SUCCESS;
        }

    for (int i=0; i<4; i++)
        {
        double      angle = (double) i * msGeomConst_piOver2;
        DSegment3d  profile;

        get_ellpnt (&profile.point[0], angle, r1, r1, &rMatrix, &center1);
        get_ellpnt (&profile.point[1], angle, r2, r2, &rMatrix, &center2);

        if (SUCCESS == LineHandler::CreateLineElement (newEeh, NULL, profile, true, *eh.GetDgnModelP ()))
            {
            ElementPropertiesSetter::ApplyTemplate (newEeh, eh);

            dropGeom.Insert (newEeh);
            }
        }

    if (fabs (r1) > 1.0e-12 && SUCCESS == ArcHandler::CreateArcElement (newEeh, NULL, center1, r1, r1, rMatrix, 0.0, msGeomConst_2pi, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (newEeh, eh);

        dropGeom.Insert (newEeh);
        }

    if (fabs (r2) > 1.0e-12 && SUCCESS == ArcHandler::CreateArcElement (newEeh, NULL, center2, r2, r2, rMatrix, 0.0, msGeomConst_2pi, true, *eh.GetDgnModelP ()))
        {
        ElementPropertiesSetter::ApplyTemplate (newEeh, eh);

        dropGeom.Insert (newEeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConeHandler::CreateConeElement (EditElementHandleR eeh, ElementHandleCP templateEh, double topRadius, double bottomRadius, 
                                              DPoint3dCR topCenter, DPoint3dCR bottomCenter, RotMatrixCR rMatrix, bool isCapped, DgnModelR modelRef)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, CONE_ELM, LevelId(in->GetLevel()), false, ElementUtil::ELEMDIM_3d);

        if (CONE_ELM != in->GetLegacyType())
            memset (out.ToSurfaceR().reserved, 0, sizeof (out.ToSurface().reserved));
        }
    else
        {
        memset (&out, 0, sizeof (Cone_3d));
        ElementUtil::SetRequiredFields (out, CONE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, ElementUtil::ELEMDIM_3d);
        }

    Cone_3d*    cone = (Cone_3d *) &out;

    cone->radius_1 = fabs (topRadius);
    cone->radius_2 = fabs (bottomRadius);
    cone->center_1 = topCenter;
    cone->center_2 = bottomCenter;
    cone->b.surf   = !isCapped;

    rMatrix.GetQuaternion(cone->quat, true);

    // determine type of cone - find bvector from center_1 to center_2
    DVec3d      zVector, vec12;

    vec12.NormalizedDifference (bottomCenter, topCenter);
    rMatrix.GetColumn(zVector,  2);

    // if dot product is 1, it is perpendicular
    int         right = ((fabs (bsiDVec3d_dotProduct (&vec12, &zVector) - 1.0)) < mgds_fc_epsilon);

    // is it a cylinder ?
    if (fabs (bottomRadius - topRadius) < mgds_fc_epsilon)
        cone->b.type = 2 - right;
    else if (bottomRadius < mgds_fc_epsilon)
        cone->b.type = 4 - right;
    else
        cone->b.type = 6 - right;

    int         elmSize = sizeof (Cone_3d);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }
