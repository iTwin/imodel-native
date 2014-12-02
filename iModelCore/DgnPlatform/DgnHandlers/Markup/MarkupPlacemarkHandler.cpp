/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Markup/MarkupPlacemarkHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

ELEMENTHANDLER_DEFINE_MEMBERS (MarkupPlacemarkHandler);

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/11
+---------------+---------------+---------------+---------------+---------------+------*/
ISpriteP        getStaticSprite ()
    {
    static ISpriteP s_staticSprite = NULL;

    if (NULL == s_staticSprite)
        {
        NamedSprite* namedSprite = new NamedSprite (ICONID_ELEMENT_MARKUP_PLACEMARK, g_iconSource);
        Point2d size = {32,32};
        namedSprite->LoadSpriteIconForSize(size);
        s_staticSprite = (ISpriteP)namedSprite;
        }

    return s_staticSprite;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d          getZVectorToEye (DPoint3dR inPoint, ViewContextP pViewContext)
    {
    DPoint3d    npcPt[2];

    if (pViewContext->IsCameraOn())
        {
        npcPt[0] = inPoint;//Local;
        
        npcPt[1] = pViewContext->GetViewport()->GetCameraWorld()->position;//Root
        pViewContext->FrustumToLocal (&npcPt[1], &npcPt[1], 2);//Local
        }
    else
        {
        npcPt[0].Init(0.0, 0.0, 0.0);
        npcPt[1].Init(0.0, 0.0, 1.0);
        pViewContext->NpcToFrustum (npcPt, npcPt, 2);
        pViewContext->FrustumToLocal (npcPt, npcPt, 2);
        }

    return DVec3d::FromStartEndNormalize (npcPt[0], npcPt[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double          getZScale (DPoint3dR inPoint, ViewContextP pViewContext)
    {
    return pViewContext->GetPixelSizeAtPoint (&inPoint) * 400.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        projectToEye (DPoint3dR inPoint, ViewContextP pViewContext)
    {
    DPoint3d projectedPoint;
    DVec3d zVector = getZVectorToEye (inPoint, pViewContext);
    double zScale = getZScale (inPoint, pViewContext);
    bsiDPoint3d_addScaledDVec3d(&projectedPoint, &inPoint, &zVector, zScale);
    return projectedPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::GetProjectedPoint (DPoint3dR inPoint, ViewContextP pViewContext) const
    {
    DPoint3d projectedPoint = projectToEye (inPoint, pViewContext);

    if (!pViewContext->PointInsideClip (projectedPoint))
        {
        double distance = 0;
        DVec3d zVector = getZVectorToEye (inPoint, pViewContext);
        zVector.negate();
        if (!pViewContext->GetRayClipIntersection(distance, projectedPoint, zVector))
            return;

        double zScale = getZScale (inPoint, pViewContext);
        if (fabs(distance) < mgds_fc_epsilon  || distance > zScale)
            return;

        inPoint.SumOf(projectedPoint, zVector, distance);
        }
    else
        inPoint = projectedPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::DrawPickGeometry (DPoint3dCR spritePt, ViewContextP pViewContext) const
    {
    Point2d     spriteSize;
    getStaticSprite()->GetSize (&spriteSize);

    pViewContext->GetIPickGeom ()->SetHitPriorityOverride (HitPriority::Vertex);

    DVec3d      zVector = getZVectorToEye ((DPoint3dR)spritePt, pViewContext);

    DVec3d xAxis, yAxis, zAxis;
    bsiDPoint3d_getTriad((DPoint3dCP) &zVector, (DPoint3dP) &xAxis, (DPoint3dP) &yAxis, (DPoint3dP) &zAxis);

    double      pixelSize = pViewContext->GetPixelSizeAtPoint (&spritePt);
    double      radius = spriteSize.x * pixelSize / 2;
    DEllipse3d  ellipse;

    ellipse.InitFromDGNFields3d (spritePt, xAxis, yAxis, radius, radius, 0.0, msGeomConst_2pi);

    pViewContext->GetIViewDraw()->DrawArc3d (ellipse, true, true, NULL);

    pViewContext->GetIPickGeom ()->SetHitPriorityOverride (HitPriority::Highest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::DrawPlacemark
(
ElementHandleCR elemHandle,
ViewContextP    pViewContext
)
    {
    DrawPurpose         drawPurpose = pViewContext->GetDrawPurpose();

    if (DrawPurpose::VisibilityCalculation == drawPurpose)
        return;

    ViewportP   vp = pViewContext->GetViewport ();

    if (!vp)
        return; // Don’t bother with sprite if output isn’t attached to a viewport…

    DgnModelP modelRef = mdlDgnModel_getRootParent (pViewContext->GetCurrentModel ());

    // Don't display sprite in the markup models
    if (mdlDgnModel_getModelFlag (modelRef, MODELFLAG_IS_MARKUP))
        return;

    DgnElementP elm = const_cast <DgnElementP>(elemHandle.GetElementCP ());
    DPoint3d *const pDataOrigin = (DPoint3d*) & elm->buf[TYPE106_ATTROFFSET];
    DPoint3d spritePt = {pDataOrigin->x, pDataOrigin->y, pDataOrigin->z};

    bool isDynamics = -1.0 == pDataOrigin->x && -1.0 == pDataOrigin->y && -1.0 == pDataOrigin->z;
    bool isHilited = HILITED_None != elemHandle.GetElementRef()->IsHilited() || 
            drawPurpose == DrawPurpose::Hilite || drawPurpose == DrawPurpose::Flash;

    if (!isDynamics)
        {
        if (pViewContext->Is3dView ())
            {
            // Make sure the sprite point is visible
            if (!pViewContext->PointInsideClip (spritePt))
                return;

            // Try to project the sprite point towards the eye using a scale that depending on how far away the sprite is.
            // The farther the sprite is, the more it is projected towards the eye.
            // This is done to make the sprite more discoverable so that elements that would obscure the sprite when you
            // are close will no longer obscure the sprite as you zoom out.
            GetProjectedPoint (spritePt, pViewContext);
            }
        else
            {
            if (spritePt.z != 0.0)
                spritePt = projectToEye (spritePt, pViewContext);
            else
                spritePt.z = 501.0;     // force sprite to be displayed on top (in 2D, z-value is used as priority and 500 is usually max element priority)
            }
        }

    if (NULL != pViewContext->GetIPickGeom ())
        DrawPickGeometry (spritePt, pViewContext);

    bool restoreZTest = false;
    IViewOutputP output = (pViewContext->IsAttached () ? vp->GetIViewOutput() : NULL);
    if (isDynamics)
        restoreZTest = (output ? output->EnableZTesting (false) : false);

    if (!isHilited)
        {
        // Dont allow sprite display to be effected by transparency overrides.
        OvrMatSymbP overrideMatSymb = pViewContext->GetOverrideMatSymb(); 
        overrideMatSymb->SetTransparentLineColor (0); 
        overrideMatSymb->SetTransparentFillColor (0); 
        pViewContext->ActivateOverrideMatSymb (); 
        }

    // Dont allow sprite display to be effected by render overrides.
    ViewFlags viewFlags = *vp->GetViewFlags();
    viewFlags.renderMode = MSRenderMode::SmoothShade;
    viewFlags.renderDisplayEdges = true;
    viewFlags.renderDisplayHidden = false;
    viewFlags.transparency = false;
    CookedDisplayStyle cookedStyle (viewFlags, NULL);
    pViewContext->GetIViewDraw()->PushRenderOverrides (viewFlags, &cookedStyle);

    // Draw the sprite
    pViewContext->GetIViewDraw ()->DrawSprite (getStaticSprite(), &spritePt, NULL, 0);

    pViewContext->GetIViewDraw()->PopRenderOverrides ();

    if (restoreZTest)
        output->EnableZTesting (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerId MarkupPlacemarkHandler::GetHandlerId ()
    {
    return ElementHandlerId (XATTRIBUTEID_Markup, MINORID_Placemark);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::_DrawFiltered
(
ElementHandleCR elemHandle,
ViewContextP    pViewContext,
DPoint3dCP      pPoints,
double          size
)
    {
    DrawPlacemark (elemHandle, pViewContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::_Draw
(
ElementHandleCR elemHandle,
ViewContextP    pViewContext
)
    {
    DrawPlacemark (elemHandle, pViewContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::_GetTypeName
(
WStringR            string,
UInt32              desiredLength
)
    {
    g_dgnHandlersResources->GetString (string, DgnHandlersMessage::IDS_TYPENAMES_MARKUP_PLACEMARK_ELM);
    if (string.length() > desiredLength)
    string.AssignA("Markup Placemark");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MarkupPlacemarkHandler::_GetDescription
(
ElementHandleCR     el,
Bentley::WString &  descr,
UInt32              desiredLength
)
    {
    _GetTypeName (descr, desiredLength);

    WChar     name[MAX_ACS_NAME_LENGTH];

    if (SUCCESS != LinkageUtil::ExtractNamedStringLinkageByIndex (name, MAX_LINKAGE_STRING_LENGTH, el.GetElementCP (), STRING_LINKAGE_KEY_Name, 0))
        return;

    descr.append (L": ");
    descr.append (name);
    }
