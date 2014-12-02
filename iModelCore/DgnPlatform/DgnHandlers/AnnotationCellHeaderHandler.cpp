/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AnnotationCellHeaderHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationCellHeaderHandler::_GetAnnotationScale (double* annotationScale, ElementHandleCR eh) const
    {
    if (NULL != annotationScale)
        *annotationScale = 1.0;

    Cell_2d const* cell = (Cell_2d const*) eh.GetElementCP();
    if (!cell->flags.isAnnotation)
        return false;

    return (SUCCESS == AnnotationScale::GetFromXAttribute (annotationScale, eh));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationCellHeaderHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    ViewContext::ContextMark mark (&context);

    AnnotationDisplayParameters parms;
    if (ComputeAnnotationDisplayParameters (parms, eh, context))
        {
        // Just scale everything up geometrically. We do this because:
        // - It makes it easy to understand what the concept of annotation size means when applied to a cell as a whole: the cell is scaled up.
        // - It currently generates the same results as transforming the elements individually(!)
        // - Makes AnnotationCellHeaderHandler consistent with ShareCellHandler for annotation cells.
        //   NB: If you decide to scale components individually here, then you must change sharcell.c to do the same.
        DPoint3d    origin;
        _GetTransformOrigin (eh, origin);

        IAnnotationStrokeForCache::PushTransformToRescale (context, origin, parms, true); // This will rescale the display of entire cell (uniformly)
        }

    // Always turn off text node number for annotation cells
    ViewFlags viewFlags = *context.GetViewFlags();
    bool drawTextNodeNumber = viewFlags.text_nodes;
    viewFlags.text_nodes = false;
    context.SetViewFlags (&viewFlags);

    VisitChildren (eh, context);

    // Restore text node number flag
    viewFlags.text_nodes = drawTextNodeNumber;
    context.SetViewFlags (&viewFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationCellHeaderHandler::_IsTransformGraphics (ElementHandleCR eh, TransformInfoCR tInfo)
    {
    if (tInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale || tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource)
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AnnotationCellHeaderHandler::ApplyAnnotationScaleTransform (EditElementHandleR eh, TransformInfoCR trans, bool& newScaleFlag, double& newScaleValue)
    {
    double  geometryScale = 1.0;
    StatusInt   status;
    if (SUCCESS != (status = GetAnnotationScaleChange (newScaleFlag, newScaleValue, geometryScale, eh, trans)))
        {
        /*------------------------------------------------------------------
            Special case for removing AnnotationScale from a cell that does
            not have AnnotationScale.  This case comes up during cell
            creation.  In this case, we want to propagate the request to
            our child elements.
        ------------------------------------------------------------------*/
        if (AnnotationScaleAction::Remove == trans.GetAnnotationScaleAction() && !GetAnnotationScale (NULL, eh))
            {
            newScaleFlag  = false;
            geometryScale = 1.0;
            status = SUCCESS;
            }
        }

    if (SUCCESS != status)
        return status;
        
    /*------------------------------------------------------------------
        Determine the transform that we need for this cell (annotation scale
        is applied as a regular scale for all child elements)
    ------------------------------------------------------------------*/
    DPoint3d    origin;

    _GetTransformOrigin (eh, origin);

    Transform           transform;
    transform.InitIdentity ();
    transform.ScaleMatrixColumns (transform,  geometryScale,  geometryScale,  geometryScale);
    transform.SetFixedPoint (origin);

    TransformInfo tInfo (transform);
    if (AnnotationScaleAction::Update != trans.GetAnnotationScaleAction ())
        {
        /*------------------------------------------------------------------
            Remove the annotation scale from any child elements that
            have it.  This way, all the children can be treated the same.
        ------------------------------------------------------------------*/

        tInfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale | TRANSFORM_OPTIONS_AnnotationSizeMatchSource | TRANSFORM_OPTIONS_DimValueMatchSource);
        tInfo.SetAnnotationScaleAction (AnnotationScaleAction::Remove);
        }
    else
        {
        /*-------------------------------------------------------------------
            Generally, the children of annotation calls will not be annotations
            since we remove the annotation scale (see above).  However,
            pre-8.11.5 we used to create annotation cells with annotation
            children, so we have to update their scale here.
        -------------------------------------------------------------------*/
        tInfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale | TRANSFORM_OPTIONS_AnnotationSizeMatchSource | TRANSFORM_OPTIONS_DimValueMatchSource);
        tInfo.SetAnnotationScaleAction (AnnotationScaleAction::Update);
        tInfo.SetAnnotationScale (newScaleValue);
        }
    T_Super::_OnTransform (eh, tInfo);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AnnotationCellHeaderHandler::_OnTransform (EditElementHandleR eh, TransformInfoCR trans)
    {
    /*-------------------------------------------------------------------
        Step 1 : Propagate annotation scale
    -------------------------------------------------------------------*/
    if (trans.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale)
        {
        double newScaleValue = 1.0;
        bool newScaleFlag    = false;
        ApplyAnnotationScaleTransform (eh, trans, newScaleFlag, newScaleValue);
        /*------------------------------------------------------------------
              Mark / Unmark the cell as an annotation
            ------------------------------------------------------------------*/
        AnnotationScale::SetAsXAttribute (eh, newScaleFlag ? &newScaleValue : NULL);
        }

    /*-------------------------------------------------------------------
        Step 2 : Apply regular transform (without annotation scale effects)
    -------------------------------------------------------------------*/
    TransformInfo modifiedTransInfo = trans;
    modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_ApplyAnnotationScale);

    if (modifiedTransInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource)
        {
        DPoint3d    origin;

        _GetTransformOrigin (eh, origin);

        Transform   unscaledTransform;
        LegacyMath::TMatrix::Unscale (&unscaledTransform, modifiedTransInfo.GetTransform (), &origin);

        modifiedTransInfo.GetTransformR() = unscaledTransform;
        modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_AnnotationSizeMatchSource);
        }

    return T_Super::_OnTransform (eh, modifiedTransInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AnnotationCellHeaderHandler::_ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR drange, double rescale)
    {
    if (!GetAnnotationScale (NULL, eh))
        return ERROR;
    DPoint3d origin;
    _GetTransformOrigin (eh, origin);

    return ComputeAnnotationScaledRange (eh, drange, rescale, &origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       AnnotationCellHeaderHandler::_OnPreprocessCopy (EditElementHandleR eeh, ElementCopyContextP ccP)
//    {
//    StatusInt status = T_Super::_OnPreprocessCopy (eeh, ccP);
//    if (SUCCESS != status)
//        return status;
//    return _OnPreprocessCopyAnnotationScale (eeh, ccP);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool   AnnotationCellHeaderHandler::_ClaimElement (ElementHandleCR eh)
    {
    Cell_2d const* cell = (Cell_2d const*) eh.GetElementCP();
    return cell->flags.isAnnotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationCellHeaderHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_AnnotationCell));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationCellHeaderHandler::_GetDescription (ElementHandleCR eh, WStringR descr, UInt32 desiredLength)
    {
    _GetTypeName (descr, desiredLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                03/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AnnotationCellHeaderHandler::CreateFromNormalCell (EditElementHandleR cellEEH)
    {
#ifdef WIP_VANCOUVER_MERGE // annotation
    if (ElementHandlerManager::QueryElementHasElementHandler (cellEEH))
        return ERROR;
 
    DgnElementP el = cellEEH.GetElementP ();
    if (NULL == el || CELL_HEADER_ELM != el->GetLegacyType())
        return ERROR;

    // Mark in element data 
    if (el->Is3d())
        {
        Cell_3d* cell = (Cell_3d*) el;
        cell->flags.isAnnotation = true;
        }
    else
        {
        Cell_2d* cell = (Cell_2d*) el;
        cell->flags.isAnnotation = true;
        }

    // Remove old element handler
    ElementRefP elRef = cellEEH.GetElementRef ();
    if (elRef)
        elRef->SetHandler (NULL);

    // Propagate model's annotation scale (if model is provided)
    DgnModelP modelRef = cellEEH.GetDgnModel ();
    if (NULL == modelRef)
        return SUCCESS;

    DgnModelP    model    = modelRef->GetDgnModelP ();
    if (NULL == model)
        return SUCCESS;

    if (!dgnModel_getModelFlag (model, MODELFLAG_USE_ANNOTATION_SCALE))
        return SUCCESS;

    double annotationScale = dgnModel_getAnnotationScale (model);

    TransformInfo  tinfo;
    tinfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale);
    tinfo.SetAnnotationScaleAction (AnnotationScaleAction::Add);
    tinfo.SetAnnotationScale (annotationScale);
    cellEEH.GetHandler().ApplyTransform (cellEEH, tinfo);

    return AnnotationScale::SetAsXAttribute (cellEEH, &annotationScale);
#endif
BeAssert(false);
return ERROR;
    }


