/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#if defined (V10_WIP_ELEMENTHANDLER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool doSegmentFlash (DisplayPathCR path)
    {
    if (path.GetPathType () < DisplayPathType::Snap)
        return true;

    switch (static_cast <SnapPathCR> (path).GetSnapMode ())
        {
        case SnapMode::Center:
        case SnapMode::Origin:
        case SnapMode::Bisector:
            return false; // Snap point for these is computed using entire linestring, not just the hit segment...

        default:
            return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DisplayHandler::_DrawPath (DisplayPathCR path, ViewContextR context)
    {
    if (DrawPurpose::Flash != context.GetDrawPurpose () || path.GetPathType () < DisplayPathType::Hit)
        return ERROR;

    HitPathCR hitPath = static_cast <HitPathCR> (path);

    if (!hitPath.GetComponentMode ())
        return ERROR;

    ICurvePrimitiveCP primitive = hitPath.GetGeomDetail ().GetCurvePrimitive ();

    if (NULL == primitive)
        return ERROR;

    DgnElementP   element = hitPath.GetCursorElem ();
    ElementHandle eh (element);

    context.SetCurrentElement (element);

    bool        pushedtrans = false;
    Transform   hitLocalToContextLocal;

    // NOTE: GeomDetail::LocalToWorld includes pushed transforms...
    if (SUCCESS == hitPath.GetHitLocalToContextLocal (hitLocalToContextLocal, context) && !hitLocalToContextLocal.IsIdentity ())
        {
        context.PushTransform (hitLocalToContextLocal);
        pushedtrans = true;
        }

    context.CookElemDisplayParams (eh);
    context.ActivateOverrideMatSymb ();

    DSegment3d      segment;
    CurveVectorPtr  curve;

    // Flash only the selected segment of linestrings/shapes based on snap mode...
    if (doSegmentFlash (hitPath) && hitPath.GetGeomDetail ().GetSegment (segment))
        curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine (segment));
    else
        curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, primitive->Clone ());

    if (DisplayHandler::Is3dElem (eh.GetGraphicsCP ()))
        context.GetIDrawGeom ().DrawCurveVector (*curve, false);
    else
        context.GetIDrawGeom ().DrawCurveVector2d (*curve, false, context.GetDisplayPriority ());

    if (pushedtrans)
        context.PopTransformClip ();

    context.SetCurrentElement (NULL);

    return SUCCESS;
    }
#endif
