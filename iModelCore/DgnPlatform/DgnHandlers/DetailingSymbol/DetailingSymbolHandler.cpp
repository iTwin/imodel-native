/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DetailingSymbol/DetailingSymbolHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include <DgnPlatform/DgnHandlers/DgnLinkTable.h>

ELEMENTHANDLER_DEFINE_MEMBERS(SectionCalloutHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(PlanCalloutHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(DetailCalloutHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(ElevationCalloutHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(TitleTextHandler)
ELEMENTHANDLER_DEFINE_MEMBERS(DrawingTitleHandler)

#undef  fc_epsilon
#define fc_epsilon                              0.00001

#define MAX_INFO_STRING_LEN                     4096
#define LEVEL_EQUAL_String                      680

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailingSymbolBaseHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_DetailingSymbol));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::GetLevel
(
LevelId *       level,
ElementHandleCR     symbol
)
    {
    for (ChildElemIter child (symbol, ExposeChildrenReason::Query); child.IsValid (); child = child.ToNext ())
        {
        if (CELL_HEADER_ELM == child.GetLegacyType())
            {
            if (SUCCESS == GetLevel (level, child))
                return SUCCESS;

            continue;
            }

        *level = LevelId(child.GetElementCP ()->GetLevel());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailingSymbolBaseHandler::_GetPathDescription
(
ElementHandleCR    symbol,
WStringR        descr,
DisplayPathCP    path,
WCharCP       levelStr,
WCharCP       modelStr,
WCharCP       groupStr,
WCharCP       delimiter
)
    {
    _GetDescription (symbol, descr, 100); // start with element's description

    if (NULL != levelStr && '\0' != levelStr[0])
        {
        descr.append(delimiter);
        descr.append(levelStr);
        }

    if (NULL != modelStr && '\0' != modelStr[0])
        {
        descr.append(delimiter);
        descr.append(modelStr);
        }

    if (NULL != groupStr && '\0' != groupStr[0])
        {
        descr.append(L"\n");
        descr.append(groupStr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    getAnnotationScale (double& scale, ElementHandleCR thisElm)
    {
#ifdef WIP_DETAILINGSYMBOLS
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (thisElm);
    if (!def.get ())
        return false;

    scale = def->GetAnnotationScale ();

    return def->GetUseAnnotationScale () ? true : false;
#else
    return 1.0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetailingSymbolBaseHandler::_GetAnnotationScale (double* annotationScale, ElementHandleCR element) const
    {
    double dummy;
    if (NULL == annotationScale)
        annotationScale = &dummy;
    return getAnnotationScale (*annotationScale, element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    DetailingSymbolBaseHandler::DrawWithAnnotationScale (ElementHandleCR thisElm, ViewContextP context, AnnotationDisplayParameters const& parms)
    {
#ifdef WIP_DETAILINGSYMBOLS
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (thisElm);
    if (def.get ())
        {
        def->SetAnnotationScale (parms.GetDesiredScale());

        EditElementHandle      workElm;
        if (SUCCESS == def->RegenerateSymbol (workElm, &thisElm, thisElm.GetDgnModel ()))
            {
            DPoint3d origin;
            def->GetPoints (&origin, 0, 1);
            context->ReplaceAspectRatioSkewWithTranslationEffect (origin, parms.GetAspectRatioSkew());

            context->SetIgnoreRefAnnotationScale (true); // Do not allow components to behave as annotations independently.

            return T_Super::_Draw (workElm, context);
            }
        }

    return T_Super::_Draw (thisElm, context);
#endif
BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawWithThickness (ElementHandleCR el, ViewContextP context, DVec3dCR thickness)
    {
    ChildElemIter childIter (el, ExposeChildrenReason::Query);

    if (childIter.IsValid())
        {
        for (; childIter.IsValid(); childIter = childIter.ToNext())
            drawWithThickness (childIter, context, thickness);
        }
    else
        {
#ifdef WIP_DETAILINGSYMBOLS
        ElemDisplayParamsP      displayParams = context->GetCurrentDisplayParams();

        displayParams->m_hasThickness = true;
        displayParams->m_thicknessVector = thickness;
#endif
         /* NEEDSWORK_PORTING: STROKER
        if (TEXT_ELM != el.GetElementCP()->GetLegacyType())
            context->DrawWithThickness (el, 0, 0);
        */
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2009
*   Draw non-test extruded and slightly transparent in a WIP attempt to make them
*   stand out in 3D models.   This is ugly as the thickness stuff is a mess, but good
*   enough to try it out.
+---------------+---------------+---------------+---------------+---------------+------*/
static void    drawWithThicknessIn3DModels (ElementHandleCR el, ViewContextP context)
    {
    DPoint3d            range;
    RotMatrix           rMatrix;
    ViewportP           viewport  = context->GetViewport();

    if ((NULL == viewport || viewport->Is3dView()) /* NEEDSWORK &&
        SUCCESS == mdlCell_directExtractHeader (NULL, &range, &rMatrix, NULL, NULL, el.GetElementCP()) */)
        {
        ViewFlags           viewFlags = *context->GetViewFlags();

        DVec3d      thickness, backup;
        Transform   transform;
        double      extrudeDistance = bsiDPoint3d_magnitude (&range) / 2.0;

        bsiRotMatrix_getColumn (&rMatrix, &thickness, 2);
        bsiDVec3d_scale (&thickness, &thickness, extrudeDistance);

        bsiDVec3d_scale (&backup, &thickness, -.5);
        bsiTransform_initFromTranslation (&transform, &backup);

        viewFlags.renderMode = (UInt32) MSRenderMode::SmoothShade;
        viewFlags.renderDisplayEdges = false;
        OvrMatSymbP     overrideMatSymb = context->GetOverrideMatSymb();

        overrideMatSymb->SetTransparentFillColor (127);
        context->ActivateOverrideMatSymb ();

        context->GetIViewDraw().PushRenderOverrides (viewFlags);
        context->PushTransform (transform);
        for (ChildElemIter childIter (el, ExposeChildrenReason::Query); childIter.IsValid(); childIter = childIter.ToNext())
            drawWithThickness (childIter, context, thickness);

        context->PopTransformClip();

        context->GetIViewDraw().PopRenderOverrides ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailingSymbolBaseHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    ViewContext::ContextMark mark (context, thisElm);              // Is this necessary????

    ViewFlags           viewFlags = *context.GetViewFlags(), saveViewFlags = viewFlags;

    if (MSRenderMode::Wireframe != saveViewFlags.renderMode)
        {
        viewFlags.renderMode = (UInt32) MSRenderMode::Wireframe;

        context.SetViewFlags (&viewFlags);
        context.GetIViewDraw().PushRenderOverrides (viewFlags);
        }

    AnnotationDisplayParameters parms;
    if (ComputeAnnotationDisplayParameters (parms, thisElm, context))
        DrawWithAnnotationScale (thisElm, &context, parms);
    else
        T_Super::_Draw (thisElm, context);

    if (MSRenderMode::Wireframe != saveViewFlags.renderMode)
        context.GetIViewDraw().PopRenderOverrides ();

    context.SetViewFlags (&saveViewFlags);

    drawWithThicknessIn3DModels (thisElm, &context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DETAILINGSYMBOLS
bool            DetailingSymbolBaseHandler::_IsTransformGraphics
(
ElementHandleCR                elemHandle,
TransformInfoCR        tInfo
)   const
    {
    double dummy;
    if (tInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale ||
        (tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource) && getAnnotationScale (dummy, elemHandle))
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double getScaleFromTransform
(
Transform const *   transform,
bool                is3d
)
    {
    DPoint3d    scaleVector;
    LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix (NULL, &scaleVector, transform);

    if (is3d)
        return (scaleVector.x + scaleVector.y + scaleVector.z) / 3.0;
    else
        return (scaleVector.x + scaleVector.y) / 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::_OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo)
    {
    bool canReplaceElement = (0 == (tInfo.GetOptions () & TRANSFORM_OPTIONS_DisallowSizeChange));

    /*-------------------------------------------------------------------
        Step 1 : Propagate annotation scale
    -------------------------------------------------------------------*/
    TransformInfo   modifiedTransInfo = tInfo;
    if ((modifiedTransInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale) && canReplaceElement)
        {
        if (!canReplaceElement)
            {
            // BeAssert (("Unable to replace symbol element", 0));
            return SUCCESS;
            }
        else
            {
            ChangeAnnotationScale *  changeContext = mdlChangeAnnotationScale_new (elHandle.GetDgnModel ());
            mdlChangeAnnotationScale_setAction (changeContext, (CASAction) modifiedTransInfo.GetAnnotationScaleAction (), modifiedTransInfo.GetAnnotationScale ());

            IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (elHandle);
            if (def.get ())
                {
                EditElementHandle newSymbolEEH;
                def->UpdateAnnotationScale (changeContext);
                if (SUCCESS == def->RegenerateSymbol (newSymbolEEH, &elHandle, elHandle.GetDgnModel ()))
                    elHandle.ReplaceElementDescr (newSymbolEEH.ExtractElementDescr ());
                }

            mdlChangeAnnotationScale_free (&changeContext);
            }

        modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_ApplyAnnotationScale);
        }

    /*-------------------------------------------------------------------
        Step 2 : Propagate transform
    -------------------------------------------------------------------*/
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (elHandle);
    if (!def.get ())
        return ERROR;

    bool isAnnotation = def->GetUseAnnotationScale ();

    if ((tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource) && isAnnotation)
        {
        // If annotation detailingsymbols should not be scaled, unplug the scale component
        DPoint3d    origin;
        def->GetPoints (&origin, 0, 1);

        Transform   unscaledTransform;
        LegacyMath::TMatrix::Unscale (&unscaledTransform, tInfo.GetTransform (), &origin);
        modifiedTransInfo.GetTransformR () = unscaledTransform;
        modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_AnnotationSizeMatchSource);
        }

    double scale = getScaleFromTransform (modifiedTransInfo.GetTransform (), elHandle.GetElementCP ()->Is3d());
    if (fabs (scale - 1.0) > fc_epsilon)
        {
        // Apply the transform to the children
        T_Super::_OnTransform (elHandle, modifiedTransInfo);

        // Reget the def since the datapoints may have changed
        def = DetailingSymbolManager::CreateDetailingSymbol (elHandle);

        //  Scale the distances stored in the def
        def->ApplyScale (scale);

        // Transform RotMatrix
        RotMatrix rMatrix;
        def->GetRotation (rMatrix);
        rMatrix.InitProduct(*(modifiedTransInfo.GetTransform ()), rMatrix);
        {
        DVec3d scaleVector;
        rMatrix.NormalizeColumnsOf (rMatrix, scaleVector);
        }
        def->SetRotation (rMatrix);

        // Transform the symbol size and rebuild symbol
        if (!canReplaceElement)
            {
            // BeAssert (("Unable to replace symbol element", 0));
            return SUCCESS;
            }
        else
            {
            if (SUCCESS != def->RegenerateSymbol (elHandle, &elHandle, elHandle.GetDgnModel()))
                return ERROR;
            }

        return SUCCESS;
        }

    return T_Super::_OnTransform (elHandle, modifiedTransInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DetailingSymbolBaseHandler::_ApplyAnnotationScaleDifferential
(
EditElementHandleR thisElm,
double          scale
)
    {
    // ***TBD: scale text, don't change distances
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::_OnPreprocessCopy (EditElementHandleR thisElm, CopyContextP ccP)
    {
    StatusInt status = T_Super::_OnPreprocessCopy (thisElm, ccP);
    if (SUCCESS != status)
        return status;
    return _OnPreprocessCopyAnnotationScale (thisElm, ccP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/08
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DetailingSymbolBaseHandler::UpdateFields (EditElementHandleR eeh)
    {
    size_t             numUpdated = 0;
    EditElementHandle  newChildEEH;

    if (TEXT_ELM == eeh.GetLegacyType() || TEXT_NODE_ELM == eeh.GetLegacyType())
        {
        TextBlockPtr textBlock = TextBlock::Create (eeh);

        if (textBlock.get () && (numUpdated = textBlock->ReevaluateFields (EvaluationReason::Unconditional)))
            {
            if (SUCCESS == textBlock->ToElement (newChildEEH, eeh.GetDgnModel (), &eeh))
                {
                newChildEEH.GetElementDescrP ()->GetElementRef() = NULL;
                newChildEEH.GetHandler().OnChildrenModified (newChildEEH, ExposeChildrenReason::Edit);
                eeh.ReplaceElementDescr (newChildEEH.ExtractElementDescr ());
                return numUpdated;
                }
            }

        return 0;
        }

    size_t totalUpdated = 0;
    for (ChildEditElemIter childEEH (eeh, ExposeChildrenReason::Edit); childEEH.IsValid(); childEEH = childEEH.ToNext())
        {
        totalUpdated += UpdateFields (childEEH);
        }

    return totalUpdated;
    }

#ifdef UNUSED_FUNCTION
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 09/05
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    findTextElmDescr
(
MSElementDescrCP *  ppTextEd,
MSElementDescrCP    pCellEd
)
    {
    MSElementDescr* pLocal = NULL;

    if (!pCellEd)
        return ERROR;

    pLocal = pCellEd->h.firstElem;
    while   (pLocal)
        {
        if (TEXT_NODE_ELM == pLocal->el.GetLegacyType() || TEXT_ELM == pLocal->el.GetLegacyType())
            {
            if (ppTextEd)
                *ppTextEd = pLocal;
            return SUCCESS;
            }

        pLocal = pLocal->h.next;
        }

    return  ERROR;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::CopyGeneratedView
(
EditElementHandleR symbolEEH
)
    {
    EditElementHandle      viewEEH;
    bool                controlExtents;
    if (SUCCESS != FindGeneratedView (&viewEEH, &controlExtents, symbolEEH, false))
        return SUCCESS;

    NamedViewPtr viewPtr = NamedView::Create (viewEEH);
    if (viewPtr.IsNull ())
        return ERROR;

#ifdef DGNV10FORMAT_CHANGES_WIP
    NamedViewCollectionR nvCollection = viewPtr->GetDgnProject ()->GetNamedViewsR ();
    WString             newName;
    nvCollection.GetUniqueNameFromBase (newName, viewPtr->GetName ().c_str ());

    viewPtr->SetName (newName.c_str ());
    ViewControllerR vip = viewPtr->GetViewControllerR ();
    vip.GetDynamicViewSettingsR().SetClipBoundElementID (0);
    viewPtr->SetViewController (vip);

    EditElementHandle clipEEH;
    viewPtr->GetClipElement (clipEEH);
    mdlElmdscr_zeroElementIds (clipEEH.GetElementDescrP ());
    viewPtr->SetClipElement (ElementHandle ());
    viewPtr->SetClipElement (clipEEH);

    if (SUCCESS != viewPtr->WriteToFile ())
        return ERROR;

    DgnProjectP                dgnfile = mdlDgnModel_getDgnFile (viewEEH.GetDgnModel ());
    NamedViewCollectionCR   nvc = dgnfile->GetNamedViews();
    NamedViewPtr            newView  = nvc.FindByName (newName.c_str (), false);

    PersistentElementPath           pep (viewEEH.GetDgnModel (), newView->GetElementRef()); // newViewEH.GetDgnModel will return NULL. We know we want the new view to point to the same root model as the old view.
    UInt16                          minorID = controlExtents? SYMBOLSETTINGS_MINORID_VIEWPEPID: SYMBOLSETTINGS_MINORID_ASSOCIATEDVIEWPEPID;
    return PersistentElementPathXAttributeHandler::ScheduleWritePersistentElementPath (symbolEEH, minorID, pep, PersistentElementPath::COPYOPTION_PreserveReferences);
#endif
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::FindGeneratedViewName
(
WStringR        viewName,
ElementHandleCR    symbolEH
)
    {
    EditElementHandle viewEEH;

    if (SUCCESS != FindGeneratedView (&viewEEH, NULL, symbolEH, false) ||
        SUCCESS != FindViewName (viewName, viewEEH))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::FindViewName
(
WStringR            viewName,
ElementHandleCR     viewEH
)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    ElementRefP           elementRef  = viewEH.GetElementRef();
    DgnProjectP              dgnFile     = viewEH.GetDgnModel()->GetDgnProject();
    NamedViewCollectionCR   nvc         = dgnFile->GetNamedViews();
    FOR_EACH (NamedViewPtr namedView , nvc)
        {
        if (namedView->GetElementRef() == elementRef)
            { viewName = namedView->GetName (); return SUCCESS; }
        }
#endif

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool isNamedView (ElementRefP elementRef, DgnModelP modelRef)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    DgnProjectP              dgnFile   = modelRef->GetDgnProject();
    NamedViewCollectionCR   nvc     = dgnFile->GetNamedViews();
    FOR_EACH (NamedViewPtr namedView , nvc)
        {
        if (namedView->GetElementRef() == elementRef)
            return true;
        }
#endif

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::FindGeneratedView
(
EditElementHandleP     viewOutEEH,
bool*               controlExtentsOut,
ElementHandleCR        markerEH,
bool                makeSavedViewModelReadWrite
)
    {
    bool controlExtents = false;

    PersistentElementPath       pep;
    if (SUCCESS == PersistentElementPathXAttributeHandler::GetPersistentElementPath (pep, markerEH, SYMBOLSETTINGS_MINORID_VIEWPEPID))
        controlExtents = true;
    else if (SUCCESS == PersistentElementPathXAttributeHandler::GetPersistentElementPath (pep, markerEH, SYMBOLSETTINGS_MINORID_ASSOCIATEDVIEWPEPID))
        controlExtents = false;

    ElementHandle v1 = pep.EvaluateElement (markerEH.GetDgnModel ());
    if (!v1.IsValid ())
        return ERROR;

    EditElementHandle viewEEH;
    viewEEH.SetElementRef (v1.GetElementRef(), v1.GetDgnModel());

    if (makeSavedViewModelReadWrite /* NEEDSWORK && viewEEH.GetDgnModel()->IsReadonlyOrLocked () */)
        {
        StatusInt status = DetailingSymbolFileAccess::GetReadWriteFileByDgnModel (viewEEH.GetDgnModel(), true);
        if (SUCCESS != status)
            return status;

        //  That just invalidated the elementref stored in viewEEH. Look it up again.
        //  (markerEH's modelref was fixed up, so it should still be good.)
        ElementHandle v2 = pep.EvaluateElement (markerEH.GetDgnModel ());
        if (!v2.IsValid ())
            {
            BeAssert (false);
            return ERROR;
            }

        viewEEH.SetElementRef (v2.GetElementRef(), v2.GetDgnModel());
        }

    if (elementRef_isDeleted (viewEEH.GetElementRef()))
        {
        BeAssert (false && "detailing symbol points to deleted view");
        return ERROR;
        }

    if (viewOutEEH)
        viewOutEEH->SetElementRef (viewEEH.GetElementRef (), viewEEH.GetDgnModel ());

    if (controlExtentsOut)
        *controlExtentsOut = controlExtents;

    return SUCCESS;
    }

#ifdef BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
/*=================================================================================**//**
* @bsiclass                                                     Jeff.Marker     08/2009
+===============+===============+===============+===============+===============+======*/
struct DetSymbPartId : public ITextPartId
    {
    private: int m_sequenceIndex;

#ifdef BEIJING_DGNPLATFORM_WIP
    // You probably want the find text methods here and in DetailingSymbolManager to search for ITextQuery instead of TEXT_ELM and TEXT_NODE_ELM.
    // See DigitalSignatureCellHeaderHandler and DigSigCellTextPartId for examples.
#endif // BEIJING_DGNPLATFORM_WIP

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     08/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static ITextPartIdPtr Create (ElementHandleCR detSymbEh, HitPathCR hitPath)
        {
        ElementRefP effectiveTextElRef = hitPath.GetHitElem ();
        if (TEXT_ELM != effectiveTextElRef->GetLegacyType() && TEXT_NODE_ELM != effectiveTextElRef->GetLegacyType())
            return NULL;
        
        if (TEXT_ELM == desiredTextElRef->GetLegacyType())
            {
            ElementRefP parentElRef = desiredTextElRef->GetParentElementRef ();
            if (TEXT_NODE_ELM == parentElRef->GetLegacyType())
                desiredTextElRef = parentElRef;
            }
        
        int numPreceedingText = 0;
        return Create (detSymbEh, effectiveTextElRef, numPreceedingText);
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     08/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    private: static ITextPartIdPtr Create (ElementHandleCR eh, ElementRefP effectiveTextElRef, int& numPreceedingText)
        {
        for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
            {
            if (eh.GetElementRef () == effectiveTextElRef)
                return new DetSymbPartId (numPreceedingText);
            
            if (TEXT_NODE_ELM == childEh.GetLegacyType() || TEXT_ELM == childEh.GetLegacyType())
                {
                ++numPreceedingText;
                continue;
                }
            
            ITextPartIdPtr retVal = Create (childEh, effectiveTextElRef, numPreceedingText);
            if (retVal.IsValid ())
                return retVal;
            }
        
        return NULL;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     08/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: DetSymbPartId (int sequenceIndex) :
        m_sequenceIndex (sequenceIndex)
        {
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     08/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: int GetSequenceIndex () const { return m_sequenceIndex; }
    
    }; // DetSymbPartId
#endif // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ITextPartIdPtr DetailingSymbolBaseHandler::_GetTextPartId (ElementHandleCR detSymbEh, HitPathCR hitPath) const
    {
#ifdef BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    return DetSymbPartId::Create (detSymbEh, hitPath);
#else // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    return NULL;
#endif // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DetailingSymbolBaseHandler::_GetTextPartIds (ElementHandleCR detSymbEh, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR textPartIds) const
    {
#ifdef BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    int numTextParts = 0;
    if (SUCCESS != DetailingSymbolManager::GetTextCount (&numTextParts, detSymbEh))
        { BeAssert (false); return; }
    
    for (int i = 0; i < numTextParts; ++i)
        textPartIds.push_back (new DetSymbPartId (i));
#endif // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextBlockPtr DetailingSymbolBaseHandler::_GetTextPart (ElementHandleCR detSymbEh, ITextPartIdCR textPartId) const
    {
#ifdef BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    DetSymbPartId const * detSymbPartId = dynamic_cast<DetSymbPartId const *>(&textPartId);
    if (NULL == detSymbPartId)
        { BeAssert (false); return NULL; }
    
    TextBlockP textBlock;
    if (SUCCESS != DetailingSymbolManager::FindTextBySequenceIndex (NULL, &textBlock, detSymbPartId->GetSequenceIndex (), detSymbEh))
        { BeAssert (false); return NULL; }
    
    return textBlock;
#else // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    return NULL;
#endif // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ITextEdit::ReplaceStatus DetailingSymbolBaseHandler::_ReplaceTextPart (EditElementHandleR detSymbEeh, ITextPartIdCR textPartId, TextBlockCR textBlock)
    {
#ifdef BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    DetSymbPartId const * detSymbPartId = dynamic_cast<DetSymbPartId const *>(&textPartId);
    if (NULL == detSymbPartId)
        { BeAssert (false); return NULL; }
    
    DetailingTextIdentifier textIdentifier;
    if (SUCCESS != DetailingSymbolManager::FindTextBySequenceIndex (&textIdentifier, NULL, detSymbPartId->GetSequenceIndex (), detSymbEeh))
        return ReplaceStatus_Error;

    TextBlockP tb = textBlock.IsEmpty () ? NULL : &textBlock;

    if (SUCCESS != DetailingSymbolManager::ReplaceTextByTextID (detSymbEeh, tb, &textIdentifier))
        return ReplaceStatus_Error;

    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (detSymbEeh);
    if (!def.IsValid () || SUCCESS != def->RegenerateSymbol (detSymbEeh, &detSymbEeh, detSymbEeh.GetDgnModel ()))
        return ReplaceStatus_Error;

    return ReplaceStatus_Success;
#else // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    return ReplaceStatus_Error;
#endif // BEIJING_DGNPLATFORM_WIP_FinishWhenThisBuildsAgain
    }

#ifdef MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER

// NEEDSWORK: MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/07
+---------------+---------------+---------------+---------------+---------------+------*/
WString DetailingSymbolBaseHandler::GetPropName (int propId)
    {
    return g_dgnHandlersResources->GetString (propId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DetailingSymbolBaseHandler::AddProperties
(
IEcPropertyHandler::T_EcCategories& cats,
ElementHandleCR                      eh
)
    {
    IEcPropertyHandler::EcPropertyCategory general (IEcPropertyHandler::EcPropertyCategory::General);

    IEcPropertyHandler::EcValueAccessor boolValue  (GetMyBooleanDelegate, SetMyBooleanDelegate);
    IEcPropertyHandler::EcValueAccessor scaleValue (GetMyScaleDelegate,   SetMyScaleDelegate);

    general.push_back (EcPropertyDescriptor (boolValue,  PROPID_IsAnnotation,    L"IsAnnotation",    GetPropName(PROPID_IsAnnotation),    SORTPRIORITY_IsAnnotation));
    general.push_back (EcPropertyDescriptor (scaleValue, PROPID_AnnotationScale, L"AnnotationScale", GetPropName(PROPID_AnnotationScale), SORTPRIORITY_AnnotationScale));

    cats.push_back (general);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DetailingSymbolBaseHandler::_GetEcProperties
(
IEcPropertyHandler::T_EcCategories& cats,
ElementHandleCR                      eh
)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool   DetailingSymbolBaseHandler::_IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName)
    {
    if (wcscmp (enabler, L"MstnProp"))
        return false;

    if (!wcscmp (className, L"MstnCellProperties"))
        return !wcscmp (propName, L"Origin") || !wcscmp (propName, L"CellName") || !wcscmp (propName, L"CellType") || !wcscmp (propName, L"AnnotationPurpose");

    if (!wcscmp (className, L"MstnComplex"))
        return !wcscmp (propName, L"NumElems");

    if (!wcscmp (className, L"MstnRotation"))
        return !wcscmp (propName, L"RotationAngle");

    if (!wcscmp (className, L"MstnScale"))
        return !wcscmp (propName, L"ScaleX") || !wcscmp (propName, L"ScaleY") || !wcscmp (propName, L"ScaleZ");

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
IsNullReturnType   DetailingSymbolBaseHandler::_IsNullProperty (ElementHandleCR eh, UInt32 propId, size_t arrayIndex)
    {
    switch (propId)
        {
        case PROPID_AnnotationScale:
            {
            if ( ! IsAnnotation (eh))
                return ISNULLRETURN_IsNullCantSet;

            if (0 == dgnModel_getModelFlag (eh.GetDgnModel(), MODELFLAG_NO_PROPAGATE_ANNSCALE))
                return ISNULLRETURN_IsNullCantSet;

            break;
            }
        }
        
    return ISNULLRETURN_NotNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
/*--------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::GetMyBooleanDelegate
(
bool&           propVal,
ElementHandleCR    eh,
UInt32          propId,
size_t          arrayIndex
)
    {
    switch (propId)
        {
        case PROPID_IsAnnotation:
            propVal = IsAnnotation (eh);
            break;
        default:
            BeAssert (false);
            return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::SetMyBooleanDelegate
(
EditElementHandleR eh,
UInt32          propId,
size_t          arrayIndex,
bool          valueIn
)
    {
    switch (propId)
        {
        case PROPID_IsAnnotation:
            SetIsAnnotation (eh, valueIn);
            break;
        default:
            BeAssert (false);
            return ERROR;
        }

    return eh.ReplaceInModel (eh.GetElementRef ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::GetMyScaleDelegate
(
ScaleInfo&      propVal,
ElementHandleCR eh,
UInt32          propId,
size_t          arrayIndex
)
    {
    switch (propId)
        {
        case PROPID_AnnotationScale:
            {
            double scaleVal;

            DisplayHandler* displayHandler = eh.GetDisplayHandler ();

            if (NULL == displayHandler)
                { BeAssert(0); return ERROR; }

            IAnnotationHandlerP annHandler = displayHandler->GetIAnnotationHandler(eh);

            if (NULL == annHandler)
                { BeAssert(0); return ERROR; }

            annHandler->GetAnnotationScale (&scaleVal, eh);
            mdlScales_findByFactor (&propVal, scaleVal);
            break;
            }

        default:
            BeAssert (false);
            return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DetailingSymbolBaseHandler::SetMyScaleDelegate
(
EditElementHandleR eh,
UInt32          propId,
size_t          arrayIndex,
ScaleInfo       valueIn
)
    {
    switch (propId)
        {
        case PROPID_AnnotationScale:
            {
            DisplayHandler* displayHandler = eh.GetDisplayHandler ();

            if (NULL == displayHandler)
                { BeAssert(0); return ERROR; }

            IAnnotationHandlerP annHandler = displayHandler->GetIAnnotationHandler(eh);

            if (NULL == annHandler)
                { BeAssert(0); return ERROR; }

            annHandler->UpdateSpecifiedAnnotationScale (eh, valueIn.GetScaleFactor());

            break;
            }

        default:
            BeAssert (false);
            return ERROR;
        }

    return eh.ReplaceInModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DetailingSymbolBaseHandler::IsAnnotation (ElementHandleCR elm)
    {
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (elm);
    if (!def.get ())
        return false;

    return def->GetUseAnnotationScale ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sridhar.Margam                  03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void          DetailingSymbolBaseHandler::SetIsAnnotation
(
EditElementHandleR eh,
bool            valueIn
)
    {
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (eh);
    if (!def.get ())
        return;

    if (valueIn == def->GetUseAnnotationScale ())
        return;

    if (!valueIn)
        def->SetUseAnnotationScale (false);
    else
        {
        def->SetUseAnnotationScale (true);
        def->SetAnnotationScale (mdlDgnModel_getEffectiveAnnotationScale (eh.GetDgnModel()));
        }

    def->RegenerateSymbol (eh, &eh, eh.GetDgnModel ());
    }

#endif // MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void purgeNoOpXAttributeChanges (EditElementHandleR eeh)
    {
    while (1)
        {
        bool cancelled = false;

        // Loop through xattribute changeset
        for (XAttributeChangeIter changeIter (eeh); changeIter.IsValid (); changeIter.ToNext ())
            {
            if (XAttributeChange::CHANGETYPE_Delete != changeIter->GetChangeType ())
                continue;

            // For every Delete, look for a persistent xattribute. If there isn't one, then there is nothing to delete.
             ElementHandle::XAttributeIter  persistentIter (eeh, changeIter->GetHandlerId (), changeIter->GetId ());
            if (!persistentIter.IsValid ())
                {
                eeh.CancelDeleteXAttribute (changeIter->GetHandlerId (), changeIter->GetId ());
                cancelled = true;

                // changeIter is no longer valid. Restart it.
                break;
                }
            }

        // If no changes were cancelled, then end while
        if (!cancelled)
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void dumpSavedViews (MSElementDescrCP newEdP, MSElementDescrP oldEdP)
    {
    NamedViewPtr newView = NamedView::Create (ElementHandle (newEdP, false, false));
    NamedViewPtr oldView = NamedView::Create (ElementHandle (oldEdP, false, false));

    NamedViewPropMaskPtr diffMask = newView->Compare (*oldView);

    printf ("Named View %S, Element ID %d\n", newView->GetName ().c_str (), newEdP->el.ehdr.uniqueId);
    diffMask->Dump ();
    printf ("**********************************************************\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void dumpDetailingSymbols (MSElementDescrCP newEdP, MSElementDescrCP oldEdP)
    {
    IDetailingSymbolPtr newDef = DetailingSymbolManager::CreateDetailingSymbol (ElementHandle (newEdP, false, false));
    IDetailingSymbolPtr oldDef = DetailingSymbolManager::CreateDetailingSymbol (ElementHandle (oldEdP, false, false));

    // We don't have a compare function. Just dump the details of both symbols.
    printf ("Detailing Symbol Element ID %d\n", newEdP->el.ehdr.uniqueId);
    newDef->Dump (WString (L"New Symbol"));
    oldDef->Dump (WString (L"Old Symbol"));
    printf ("**********************************************************\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetailingSymbolBaseHandler::AreIdentical (EditElementHandleR eh, ElementHandleCR    oldEh)
    {
    // oldElem's EdP doesn't have xattrs loaded. Call duplicate and pass true for loadPersistentXasAsChanges
    MSElementDescrP oldEdP = NULL;
    oldEh.GetElementDescrCP()->Duplicate (&oldEdP, true, true);
    EditElementHandle __freeOldDescr (oldEdP, true, true);

    // oldEdP may be part of a complex header. Since the new one is not written yet, its complex bit will be off.
    // Match it for oldEdP.
    oldEdP->el.IsComplexComponent()= 0;

    // Remove no-op xattribute changes in new element
    purgeNoOpXAttributeChanges (eh);

    eh.SetDgnModel (oldEh.GetDgnModel ());
    eh.GetElementDescrP ()->GetElementRef() = oldEh.GetElementRef ();

    // Validate the new element (to set IsComplexComponent()flag on child elems like clip)
    eh.GetElementDescrP()->Validate (eh.GetDgnModel ());

    bool identical = false; /* NEEDSWORK TO_BOOL( mdlElmdscr_areIdentical (eh.GetElementDescrP (), oldEdP, COMPAREOPT_IGNORE_MODIFIED | COMPAREOPT_IGNORE_IDS) ); */

#ifdef DEBUG_DETAILINGSYMBOLHANDLER
    if (!identical)
        {
        Handler& handler = eh.GetHandler ();

        if (dynamic_cast <ViewElementHandler*> (&handler))
            dumpSavedViews (eh.GetElementDescrCP (), oldEdP);
        else if (dynamic_cast <CalloutBaseHandler*> (&handler))
            dumpDetailingSymbols (eh.GetElementDescrCP (), oldEdP);
        }
#endif

    return identical;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
* Returns true if the PEP was found
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetailingSymbolBaseHandler::IsRootChanged
(
IDependencyHandler::ChangeStatus&                   changeStatus,
PersistentElementPath*                              rootPepOut,
ElementHandleR                                         dependent,
bvector<IDependencyHandler::RootChange> const&  rootsChanged,
bvector<XAttributeHandle> const&                xAttrsAffected,
UInt32                                              pepIDToCheck
)
    {
    changeStatus = (IDependencyHandler::ChangeStatus)0; // in case the search fails

    if (elementRef_isDeleted (dependent.GetElementRef()))
        {
        changeStatus = IDependencyHandler::CHANGESTATUS_Deleted;
        return true;
        }

    for (bvector<XAttributeHandle>::const_iterator iXAttr = xAttrsAffected.begin(); iXAttr != xAttrsAffected.end(); ++iXAttr)
        {
        if (pepIDToCheck == iXAttr->GetId())
            {
            PersistentElementPath   rootPep;
            PersistentElementPathXAttributeHandler::GetPersistentElementPath (rootPep, *iXAttr);
            for (bvector<IDependencyHandler::RootChange>::const_iterator iRoot = rootsChanged.begin(); iRoot != rootsChanged.end(); ++iRoot)
                {
                if (rootPep.EqualElementRef (iRoot->root, mdlDgnModel_getDgnModel(dependent.GetDgnModel())))
                    {
                    changeStatus = iRoot->changeStatus;
                    if (rootPepOut)
                        *rootPepOut = rootPep;
                    return true;
                    }
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 03/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool DetailingSymbolBaseHandler::IsSelfChanged
(
IDependencyHandler::ChangeStatus&                   changeStatus,
ElementHandleR                                         dependent,
bvector<IDependencyHandler::RootChange> const&  rootsChanged
)
    {
    changeStatus = (IDependencyHandler::ChangeStatus)0; // in case the search fails

    if (elementRef_isDeleted (dependent.GetElementRef()))
        {
        changeStatus = IDependencyHandler::CHANGESTATUS_Deleted;
        return true;
        }

    for (bvector<IDependencyHandler::RootChange>::const_iterator iRoot = rootsChanged.begin(); iRoot != rootsChanged.end(); ++iRoot)
        {
        if (dependent.GetElementRef () == iRoot->root)
            {
            changeStatus = iRoot->changeStatus;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   01/2008
* return true if a new linktree was scheduled for addition
+---------------+---------------+---------------+---------------+---------------+------*/
bool detailingSymbol_addProxyLinkTree (EditElementHandleR destEEH)
    {
    DgnLinkTreePtr    tree            = NULL; /* NEEDSWORK manager.ReadLinkTree (destEEH.GetElementRef (), false); */
    if (!tree.IsNull ())
        return false;

    tree = NULL; /* NEEDSWORK  manager.ReadLinkTree (destEEH.GetElementRef (), true); */
    if (tree.IsNull ())
        {
        BeAssert (false);
        return false;
        }

    return /* NEEDSWORK SUCCESS == manager.ScheduleLinkTreeDirect (tree, destEEH) ? true : */ false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Yogesh.Sajanikar                07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    detailingSymbol_isSyncRequired
(
DgnLinkTreeBranchCR source,
DgnLinkTreeBranchCR destination
)
    {
    UInt32 appId = XATTRIBUTEID_SymbolSettings;
    UInt32 subId = SYMBOLSETTINGS_MINORID_VIEWPEPID;

    size_t nSourceCount = source.GetChildCount ();
    size_t nDestCount   = destination.GetChildCount ();
    size_t nTaggedCount = 0;
    size_t nTaggedSourceCount = 0;

    bvector <DgnLinkCP> sourceLinkList;
    bvector <DgnLinkCP> linkList;

    for (size_t isrc = 0; isrc < nSourceCount; ++isrc)
        {
        DgnLinkTreeNodeCP node = source.GetChildCP (isrc);
        DgnLinkTreeLeafCP leaf = dynamic_cast <DgnLinkTreeLeafCP> (node);
        if (NULL == leaf)
            continue;

        DgnLinkCP               link        = leaf->GetLinkCP ();
        DgnRegionLinkCP         regionLink  = dynamic_cast <DgnRegionLinkCP> (link); // NEEDSWORK: Check for XMLTAG_RegionTypeDrawing
        if (NULL == regionLink)
            continue;

        nTaggedSourceCount++;
        sourceLinkList.push_back (link);
        }

    // Count the children with user data.
    for (size_t idst = 0; idst < nDestCount; ++idst)
        {
        DgnLinkTreeNodeCP node = destination.GetChildCP (idst);
        DgnLinkTreeLeafCP leaf = dynamic_cast <DgnLinkTreeLeafCP> (node);
        if (NULL == leaf)
            continue;

        DgnLinkUserDataCP data = node->GetUserData (&appId, &subId, 0);
        if (NULL != dynamic_cast <const ViewProxyData*> (data))
            {
            nTaggedCount++;
            linkList.push_back (leaf->GetLinkCP ());
            }
        }

    // If the tagged count and source count are different, we have to sync.
    if (nTaggedCount != nTaggedSourceCount)
        return true;

    // Now compare the ancestry for these links. If there is any difference, we will
    // require a sync.
    for (size_t itagged = 0; itagged < nTaggedCount; ++itagged)
        {
        DgnLinkCP        sourceLink = sourceLinkList.at (itagged);
        DgnLinkCP        destLink   = linkList.at (itagged);

        // Speed up the comparison. Ideally GetTargetAncestry is well suited to this purpose.
        // But since ModelLink::GetTargetAncestry uses expensive file opening to get the model
        // type, we use only necessary information to compare the targets.
        DgnRegionLinkCP  sourceRegion = dynamic_cast <DgnRegionLinkCP> (sourceLink);
        DgnRegionLinkCP  destRegion  = dynamic_cast <DgnRegionLinkCP> (destLink);

        /* NEEDSWORK: Make sure it is a drawing title link  if (0 == sourceRegion->GetTargetType ().compare (XMLTAG_RegionTypeDrawing) && 0 == destRegion->GetTargetType ().compare (XMLTAG_RegionTypeDrawing)) */
            {
            if (sourceRegion->HasSameTarget (*destRegion))
                return true;
            else
                continue;
            }
        /* NEEDSWORK: Uncomment after above issue is resolved
        else if (NULL != sourceRegion || NULL != destRegion)
            {
            BeAssert (true);   // Not expected to happen.
            return true;         // Types are different.
            }

        DgnLinkTargetSpecList sourceAncestry;
        DgnLinkTargetSpecList destAncestry;

        sourceLink->GetTargetAncestry (sourceAncestry);
        destLink->GetTargetAncestry (destAncestry);

        if (sourceAncestry.size () != destAncestry.size ())
            return true;

        for (size_t j = 0; j < sourceAncestry.size (); ++j)
            {
            DgnLinkTargetSpec* sourceSpec = dynamic_cast <DgnLinkTargetSpec*> (sourceAncestry [j].get ());
            DgnLinkTargetSpec* destSpec   = dynamic_cast <DgnLinkTargetSpec*> (destAncestry [j].get ());

            if (0 != sourceSpec->Compare (*destSpec))
                return true;
            } */
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
// Returns false if synchronization is not done, true otherwise.
static bool detailingSymbol_SyncLinkTrees (EditElementHandleR destEEH, ElementRefP source)
    {
//    DgnLinkManagerR manager             = DgnLinkManager::GetManager ();

    // Get the annotation tree for the source element. Early out if not found.
    DgnLinkTreePtr sourceTree = NULL; /* NEEDSWORK manager.ReadLinkTree (source.GetElementRef (), false); */
    if (sourceTree.IsNull ())
        return false;

    DgnLinkTreePtr    destinationLinkTree = NULL;  /* NEEDSWORK manager.ReadLinkTree (destEEH.GetElementRef (), false); */

    if (destinationLinkTree.IsNull ())
        {
        if (!detailingSymbol_addProxyLinkTree (destEEH) ||
            /* NEEDSWORK  SUCCESS != manager.ReadLinkTree (destinationLinkTree, destEEH, false) || */
            destinationLinkTree.IsNull ())
            {
            BeAssert (false);
            return false;
            }
        }

    DgnLinkTreeBranchR      root        = destinationLinkTree->GetRootR ();
    DgnLinkTreeBranchCR     sourceRoot  = sourceTree->GetRoot ();
    UInt32                  appId   = XATTRIBUTEID_SymbolSettings;
    UInt32                  subId   = SYMBOLSETTINGS_MINORID_VIEWPEPID;

    bool synchronizationRequired    = detailingSymbol_isSyncRequired (sourceRoot, root);
    if (!synchronizationRequired)
        return false;

    // Remove all tagged links from the destination.
    size_t i = 0;
    for (; i < root.GetChildCount (); ++i)
        {
        DgnLinkTreeNodeCP node = root.GetChildCP (i);
        DgnLinkUserDataCP data = node->GetUserData (&appId, &subId, 0);
        if (NULL != dynamic_cast <const ViewProxyData*> (data))
            {
            if (SUCCESS == root.DropChild (i))
                i--;
            }
        }

    // Get all the links from the source and add them to the destination.
    DgnLinkTreeSpecCR destTreeSpec = destinationLinkTree->GetTreeSpec ();
    int destinationIndex = 0;

    for (i = 0; i < sourceRoot.GetChildCount (); ++i)
        {
        DgnLinkTreeNodeCP sourceNode = sourceRoot.GetChildCP (i);
        DgnLinkTreeLeafCP sourceLeaf = dynamic_cast <DgnLinkTreeLeafCP> (sourceNode);
        if (NULL == sourceLeaf)
            continue;

        DgnLinkCP       sourceLink = sourceLeaf->GetLinkCP ();
        DgnRegionLinkCP sourceRegionLink = dynamic_cast <DgnRegionLinkCP> (sourceLink);
        if (NULL == sourceRegionLink /* NEEDSWORK: Make sure it is a drawing title link || 0 != sourceRegionLink->GetTargetType ().compare (XMLTAG_RegionTypeDrawing) */)
            continue;

        DgnLinkTreeNodePtr treeNode   = sourceLeaf->Copy (destTreeSpec);
        treeNode->SetName (sourceNode->GetName ());

        if (SUCCESS == root.AddChild (*treeNode, destinationIndex))
            {
            // NEEDSWORK treeNode->AddUserData (new ViewProxyData ());
            destinationIndex++;
            }
        }

    /* NEEDSWORK manager.ScheduleLinkTreeDirect (destinationLinkTree, destEEH); */
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 03/08
+---------------+---------------+---------------+---------------+---------------+------*/
double CalloutBaseHandler::GetZOffsetForClip (ElementHandleCR clipEH, DgnModelP viewDgnModel, bool forUpdatingView)
    {
    double          zOffsetForClip = 0;
    DisplayHandlerP displayHandler = clipEH.GetDisplayHandler ();

    if (displayHandler)
        {
        // Find zOffsetForClip to "lift" the clip volume after ApplyToView flattens it to the ground.
        // Note: The lift direction is not always global Z. It is the Z of the clip element's rotMatrix.

        // Step 1: Find the clip element's rot matrix
        RotMatrix rMatrix;
        GetClipRotMatrix (rMatrix, clipEH);

        Transform trans;
        bsiTransform_initIdentity (&trans);
        bsiTransform_setMatrix (&trans, &rMatrix);
        TransformInfo transInfo (trans);

        //Step 2: Rotate clip element to local
        EditElementHandle localClipEEH (clipEH, true);
        localClipEEH.GetHandler().ApplyTransform (localClipEEH, transInfo);

        // Step 3: Find translation component of reference transform
        Transform refTrans;
        refTrans.initIdentity ();
        
        DgnAttachment* attachment = dynamic_cast <DgnAttachment*> (viewDgnModel);
        if (NULL != attachment)
            attachment->GetTransformFromParent (refTrans, true); // NEEDSWORK: Get transform from MASTER. Old call was mdlRefFile_getTransformFromMaster (&refTrans, mdlRefFile_getInfo (viewDgnModel));

        DPoint3d refTransVec;
        bsiTransform_getTranslation (&refTrans, &refTransVec);
        trans.Multiply (&refTransVec, 1);

        //Step 4: Find the net offset of the clip element perpendicular to sheet
        DPoint3d        clipOrigin;
        if (SUCCESS == GetClipOrigin (clipOrigin, localClipEEH))
            {
            if (forUpdatingView)
                zOffsetForClip = clipOrigin.z - refTransVec.z;
            else
                zOffsetForClip = refTransVec.z - clipOrigin.z;
            }
        }

    return zOffsetForClip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::UpdateMarkerFromView (ElementHandleCR markerEH)
    {
    bool            controlExtents = false;
    EditElementHandle  viewEH;

    if (SUCCESS != FindGeneratedView (&viewEH, &controlExtents, markerEH, false))
        return SUCCESS;

    NamedViewPtr viewPtr = NamedView::Create (viewEH);
    if (!viewPtr.get ())
        return ERROR;

    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (markerEH);
    if (!def.get ())
        return ERROR;

    EditElementHandle newMarkerEEH (markerEH, true);
    if (controlExtents)
        {
        EditElementHandle clipEEH;
        if (SUCCESS != viewPtr->GetClipElement (clipEEH) || !clipEEH.IsValid ())
            return ERROR;

        DgnModelP    viewDgnModel =  GetViewRootDgnModel (viewEH);
        if (NULL == viewDgnModel)
            return ERROR;

        double zOffsetForClip = GetZOffsetForClip (clipEEH, viewDgnModel, false);

        IHasClipElementProvider* hasClipProvider = dynamic_cast <IHasClipElementProvider*> (def.get ());
        if (!hasClipProvider)
            return ERROR;

        IClipElementProviderPtr clipProvider = hasClipProvider->GetClipElementProvider ();
        if (!clipProvider.get () || SUCCESS != clipProvider->ApplyClipElement (clipEEH, viewDgnModel, markerEH.GetDgnModel (), zOffsetForClip, false))
            return ERROR;

        if (SUCCESS != def->RegenerateSymbol (newMarkerEEH, &markerEH, markerEH.GetDgnModel ()))
            return ERROR;
        }

    bool linksUpdated = detailingSymbol_SyncLinkTrees (newMarkerEEH, viewEH.GetElementRef ());

    if (AreIdentical (newMarkerEEH, markerEH))
        return SUCCESS;

    if (SUCCESS != newMarkerEEH.ReplaceInModel (markerEH.GetElementRef ()))
        return ERROR;

    // Update fields after rewriting the linktree changes because fields are evaluated by reading linktrees from persistent elements.
    if (linksUpdated)
        {
        EditElementHandle updatedMarkerEEH (newMarkerEEH, true);
        if (UpdateFields (updatedMarkerEEH))
            return updatedMarkerEEH.ReplaceInModel (markerEH.GetElementRef ());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void CalloutBaseHandler::GetClipRotMatrix (RotMatrix& rMatrix, ElementHandleCR clipEH)
    {
    IHasViewClipObject* hasViewClipObject = dynamic_cast <IHasViewClipObject*> (&clipEH.GetHandler ());
    if (hasViewClipObject)
        {
        IViewClipObjectPtr viewClipObject = hasViewClipObject->GetClipObject (clipEH);
        if (viewClipObject.get ())
            {
            rMatrix = viewClipObject->GetRotationMatrix ();
            bsiRotMatrix_transpose (&rMatrix, &rMatrix);
            return;
            }
        }

    // REMOVE Transform trans;
    // REMOVE mdlElmdscr_directOrientationExt (&trans, clipEH.GetElementDescrCP (), clipEH.GetDgnModel ());
    // mdlElmdscr_directOrientationExt (&trans, clipEH.GetElementDescrCP (), clipEH.GetDgnModel ());
    // bsiTransform_getMatrix (&trans, &rMatrix);

    clipEH.GetDisplayHandler ()->GetOrientation (clipEH, rMatrix);
    bsiRotMatrix_transpose (&rMatrix, &rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CalloutBaseHandler::GetClipOrigin (DPoint3dR origin, ElementHandleCR clipEH)
    {
    IHasViewClipObject* hasViewClipObject = dynamic_cast <IHasViewClipObject*> (&clipEH.GetHandler ());

    if (hasViewClipObject)
        {
        IViewClipObjectPtr viewClipObject = hasViewClipObject->GetClipObject (clipEH);

        if (viewClipObject.get ())
            return viewClipObject->GetPoints (&origin, 0, 1);
        }

    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (clipEH);
    DPoint3d        endPt;

    if (pathCurve.IsValid () && pathCurve->GetStartEnd (origin, endPt))
        return SUCCESS;

    DisplayHandlerP displayHandler = clipEH.GetDisplayHandler ();

    if (!displayHandler)
        return ERROR;

    displayHandler->GetTransformOrigin (clipEH, origin);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       generateClipElement (EditElementHandleR clipEEH, ElementHandleCP templateEH, ElementHandleCR calloutEH, DgnModelP sheetDgnModel, DgnModelP designDgnModel, double zOffsetForClipInDesignCoord)
    {
    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (calloutEH);
    if (!def.get ())
        return ERROR;

    IHasClipElementProvider* viewClipMarker = dynamic_cast <IHasClipElementProvider*> (def.get ());
    if (!viewClipMarker)
        {
        BeAssert (("Callout cannot provide view data", 0));
        return ERROR;
        }

    IClipElementProviderPtr clipProvider = viewClipMarker->GetClipElementProvider ();
    if (!clipProvider.get ())
        return ERROR;

    return clipProvider->GenerateClipElement (clipEEH, templateEH, 0, sheetDgnModel, designDgnModel, zOffsetForClipInDesignCoord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::UpdateViewFromMarker (ElementHandleCR markerEH)
    {
    /*-------------------------------------------------------------------
        Step 1: Find the Saved View
    -------------------------------------------------------------------*/
    bool            controlExtents = false;
    EditElementHandle  viewEH;

    StatusInt status = FindGeneratedView (&viewEH, &controlExtents, markerEH, false);
    if (SUCCESS != status)
        return status;

    // If we are not going to control the view, then there is nothing to do
    if (!controlExtents)
        return SUCCESS;

    NamedViewPtr viewPtr = NamedView::Create (viewEH);
    if (!viewPtr.get ())
        {
        BeAssert (0);
        return ERROR;
        }

    /*-------------------------------------------------------------------
        Step 3: Find the zoffset of the clip element
    -------------------------------------------------------------------*/
    DgnModelP    viewDgnModel = GetViewRootDgnModel (viewEH);
    if (NULL == viewDgnModel)
        return ERROR;

    EditElementHandle  clipEEH;
    if (SUCCESS != viewPtr->GetClipElement (clipEEH))
        {
        BeAssert (0);
        return ERROR;
        }

    double zOffsetForClip = GetZOffsetForClip (clipEEH, viewDgnModel, true);

    /*-------------------------------------------------------------------
        Step 4: Create a new clip element
    -------------------------------------------------------------------*/
    EditElementHandle  newClipEEH;
    if (SUCCESS != generateClipElement (newClipEEH, &clipEEH, markerEH, markerEH.GetDgnModel (), viewDgnModel, zOffsetForClip))
        {
        BeAssert (0);
        return ERROR;
        }

    /*-------------------------------------------------------------------
        Step 5: If the clip element didn't change, there is nothing to do
    -------------------------------------------------------------------*/
    if (AreIdentical (newClipEEH, clipEEH))
        return SUCCESS;
#ifdef DGNV10FORMAT_CHANGES_WIP

    bool openForWrite = false;
    if (viewPtr->GetViewController ().GetGeomInfo ().m_viewFlags.associativeClip)
        {
        // Associative clip element should be written separate from the
        // view element.
        viewEH.Invalidate ();
        clipEEH.Invalidate ();

        if (SUCCESS != FindGeneratedView (&viewEH, NULL, markerEH, true))
            return ERROR;

        openForWrite = true;

        viewPtr = NamedView::Create (viewEH);
        if (SUCCESS != viewPtr->GetClipElement (clipEEH))
            return ERROR;

        newClipEEH.GetElementDescrP ()->GetElementRef() = (NULL != clipEEH.GetElementRef () ? clipEEH.GetElementRef () : clipEEH.GetElementDescrP ()->GetElementRef());
        newClipEEH.ReplaceInModel (newClipEEH.GetElementDescrP ()->GetElementRef());
        viewPtr->SetClipElement (ElementHandle ());
        viewPtr->GetClipElement (newClipEEH);
        }
    else
        {
        viewPtr->SetClipElement (newClipEEH);
        }
#endif

    /*-------------------------------------------------------------------
        Step 5: Synchronize the view boundary with the clip element
    -------------------------------------------------------------------*/
    viewPtr->SynchViewBoundaryWithClipVolume ();

    EditElementHandle newViewEEH;
    if (SUCCESS != viewPtr->ToElement (newViewEEH) || !newViewEEH.IsValid ())
        return ERROR;

    /*-------------------------------------------------------------------
        Step 6: If the view has changed, open the file for write-access
        and replace the view.
    -------------------------------------------------------------------*/
    if (!AreIdentical (newViewEEH, viewEH))
        {
#ifdef DGNV10FORMAT_CHANGES_WIP
        viewEH.Invalidate ();

        if (!openForWrite && SUCCESS != FindGeneratedView (&viewEH, NULL, markerEH, true))
            return ERROR;

        ViewControllerR     vip = viewPtr->GetViewControllerR ();
        vip.SetRootModel ((viewEH.GetDgnModel()->AsDgnModelP ()));
        viewPtr->SetViewController (vip);
        return viewPtr->WriteToFile ();
#endif
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
* For detailed notes on these PEPs, refer the end of symboldef.h
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    CalloutBaseHandler::GetRefDgnModel (ElementHandleCR markerEH)
    {
    UInt32                      pepID = SYMBOLSETTINGS_MINORID_REFERENCEPEPID;
    PersistentElementPath       pep;
    if (SUCCESS != PersistentElementPathXAttributeHandler::GetPersistentElementPath (pep, markerEH, pepID))
        return NULL;

    ElementHandle refEEH = pep.EvaluateElement (markerEH.GetDgnModel ());
    if (!refEEH.IsValid ())
        return NULL;

    DgnAttachmentP refDgnModel   = DgnAttachment::FindByElementId (refEEH.GetDgnModel (), refEEH.GetElementRef ()->GetElementId());
    if (NULL == refDgnModel)
        return NULL;

    return refDgnModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    CalloutBaseHandler::GetViewRootDgnModel (ElementHandleCR viewEH)
    {
    // This will return a refDgnModel to the view's root model. This works because of the way
    // we create the Ref PEP by using the view root model and view elementref. When evaluating,
    // we start with the callout's home model and traverse up to the view's root model. We set
    // the view root model's RefDgnModel in ViewEEH.
    return viewEH.GetDgnModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar   01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void CalloutBaseHandler::_OnAdded (ElementHandleP element)
    {
    T_Super::_OnAdded (element);

    // Create a proxy link tree.
    if (!element)
        return;

    EditElementHandle newEEH (*element, true);
    if (detailingSymbol_addProxyLinkTree (newEEH))
        newEEH.ReplaceInModel (element->GetElementRef ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 05/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CalloutBaseHandler::FlattenMarker (EditElementHandleR symbolEH)
    {
    XAttributeHandlerId     pepHandlerId = PersistentElementPathXAttributeHandler::GetHandlerId ();
    if (SUCCESS != symbolEH.ScheduleDeleteXAttribute (pepHandlerId, SYMBOLSETTINGS_MINORID_VIEWPEPID) ||
        SUCCESS != symbolEH.ScheduleDeleteXAttribute (pepHandlerId, SYMBOLSETTINGS_MINORID_ASSOCIATEDVIEWPEPID) ||
        SUCCESS != symbolEH.ScheduleDeleteXAttribute (pepHandlerId, SYMBOLSETTINGS_MINORID_REFERENCEPEPID))
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 03/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CalloutBaseHandler::IsCopyInSameModel (ElementHandleCR symbolEH)
    {
    // Check if the marker's root view has another dependent marker in the same model
    EditElementHandle viewEEH;
    if (SUCCESS != FindGeneratedView (&viewEEH, NULL, symbolEH, false))
        return false;

    T_StdElementRefVector viewDepRefs;
    elementRef_getDependentsVector (&viewDepRefs, viewEEH.GetElementRef ());

    ElementRefP foundElemRef = NULL;
    for (T_StdElementRefVector::iterator viewDepIter = viewDepRefs.begin (); viewDepIter != viewDepRefs.end (); viewDepIter++)
        {
        ElementHandle viewDepEl (*viewDepIter, NULL);

        if (symbolEH.GetElementRef () != viewDepEl.GetElementRef () && NULL != dynamic_cast <CalloutBaseHandler*> (&viewDepEl.GetHandler ()))
            {
            foundElemRef = viewDepEl.GetElementRef ();
            break;
            }
        }

    if (!foundElemRef || foundElemRef->GetDgnModelP() != mdlDgnModel_getDgnModel (symbolEH.GetDgnModel ()))
        return false;

    // We have established that they are both in the same model. Now check if they are both pointing to the same attachment.
    EditElementHandle      cloneSymbolEEH (foundElemRef, symbolEH.GetDgnModel ());
    return GetRefDgnModel (symbolEH) == GetRefDgnModel (cloneSymbolEEH);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 02/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CalloutBaseHandler::_OnPreprocessCopy (EditElementHandleR symbolEH, CopyContextP ccP)
    {
    StatusInt   status;

    if (SUCCESS != (status = T_Super::_OnPreprocessCopy (symbolEH, ccP)))
        return status;

    DgnModelP    srcDgnModel = NULL, destDgnModel = NULL;
    ccP->GetModels (&srcDgnModel, &destDgnModel, NULL, NULL, NULL);

    // If it is a copy within the same model, then make a copy of the associated view and patch up view PEP.
    if (srcDgnModel == destDgnModel)
        return PRE_ACTION_Ok;

    return (SUCCESS == FlattenMarker (symbolEH)) ? PRE_ACTION_Ok : PRE_ACTION_Block;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 05/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalloutBaseHandler::IsViewDeleted (PersistentElementPath& viewPep, IDependencyHandler::ChangeStatus viewChangeStatus, DgnModelP homeCache)
    {
    if (CHANGESTATUS_Deleted == viewChangeStatus)
        return true;

    // If view is unresolved, user may have temporarily renamed the reference file. Need to check if the view's associated model is found.
    if (CHANGESTATUS_Unresolved != viewChangeStatus)
        return false;

    // If view's associated model is found but the view was unresolved, then we know that the view was deleted.
    DgnProjectP    dgnFile = NULL;
    DgnModelId modelID;
    return SUCCESS == viewPep.EvaluateRootModel (dgnFile, modelID, homeCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void    CalloutBaseHandler::_OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected)
    {
    IDependencyHandler::ChangeStatus    selfChangeStatus;

    // If the new marker is a clone of an existing marker, make a copy of the root view and point the new marker's viewPEP to the new view
    if (DependencyManager::WasAdded (dependent.GetElementRef ()) && IsCopyInSameModel (dependent))
        {
        EditElementHandle newSymbolEEH (dependent, true);

        if (SUCCESS != CopyGeneratedView (newSymbolEEH))
            FlattenMarker (newSymbolEEH);

        newSymbolEEH.ReplaceInModel (dependent.GetElementRef ());
        UpdateViewFromMarker (newSymbolEEH);
        return;
        }

    /*bool selfFound =*/ IsSelfChanged (selfChangeStatus, dependent, rootsChanged);

    if (CHANGESTATUS_Changed == selfChangeStatus)
        {
        // If a new callout was generated from a saved view, copy the saved view's links onto the callout. Else, update the view from the callout.
        if (DependencyManager::WasAdded (dependent.GetElementRef ()))
            UpdateMarkerFromView (dependent);
        else
            UpdateViewFromMarker (dependent);
        return;
        }

    IDependencyHandler::ChangeStatus    viewChangeStatus;
    IDependencyHandler::ChangeStatus    refChangeStatus;
    PersistentElementPath               viewPep;

    bool viewPepFound   = IsRootChanged (viewChangeStatus, &viewPep, dependent, rootsChanged, xAttrsAffected, SYMBOLSETTINGS_MINORID_VIEWPEPID);
    if (!viewPepFound)
        viewPepFound = IsRootChanged (viewChangeStatus, &viewPep, dependent, rootsChanged, xAttrsAffected, SYMBOLSETTINGS_MINORID_ASSOCIATEDVIEWPEPID);

    /*bool refPepFound    =*/ IsRootChanged (refChangeStatus, NULL, dependent, rootsChanged, xAttrsAffected, SYMBOLSETTINGS_MINORID_REFERENCEPEPID);

    if (IsViewDeleted (viewPep, viewChangeStatus, dependent.GetDgnModelP ()))
        {
        EditElementHandle symbolEEH (dependent, true);
        symbolEEH.DeleteFromModel ();
        return;
        }

    if (CHANGESTATUS_Changed == refChangeStatus)
        {
        // When we do save-as of a sheet by retaining its references and converting the corresponding reference files,
        // we get called in the middle of the merging process. At that time, the dependencies are not fully converted yet and it is wrong to
        // update the callout's saved view in that half-cooked state.
        if (!DependencyManager::IsRefMergeInProgress ())
            UpdateViewFromMarker (dependent);
        return;
        }

    if (CHANGESTATUS_Deleted == refChangeStatus)
        {
        if (DependencyManager::IsRefMergeInProgress ())
            {
            EditElementHandle newSymbolEEH (dependent, true);
            FlattenMarker (newSymbolEEH);
            newSymbolEEH.ReplaceInModel (dependent.GetElementRef ());
            return;
            }

        EditElementHandle symbolEEH (dependent, true);
        symbolEEH.DeleteFromModel ();
        return;
        }

    if (CHANGESTATUS_Changed == viewChangeStatus || CHANGESTATUS_Resolved == viewChangeStatus)
        {
        UpdateMarkerFromView (dependent);
        }
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Anil.Kumar                      04/2009
* NEEDSWORK: This is a copy of findDrawingLinkOnSavedView () in detailingsymbol.cpp
* Both copies should be consolidated into a static method in DetailingSymbolEnabler class.
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnRegionLinkCP findLinkOnSavedView (EditElementHandleR viewEH, bool findDrawingLink)
    {
    //DgnLinkManagerR    dlManager = DgnLinkManager::GetManager ();
    DgnLinkTreePtr     tree  = NULL; /* NEEDSWORK dlManager.ReadLinkTree (viewEH.GetElementRef (), false); */
    if (tree.IsNull ())
        return NULL;

    DgnLinkTreeBranchCR root  = tree->GetRoot ();
    size_t count = root.GetChildCount ();
    if (0 == count)
        return NULL;

    for (size_t ilink = 0; ilink < count; ilink++)
        {
        DgnLinkTreeLeafCP   leafRef = dynamic_cast <DgnLinkTreeLeafCP> (root.GetChildCP (ilink));
        DgnRegionLinkCP     regionLink = dynamic_cast <DgnRegionLinkCP> (leafRef->GetLinkCP ());

        DgnLinkValidationCenter logger (true, true, true);
//        if (NULL == regionLink || !dlManager.ValidateLinkTarget (regionLink, logger))
        if (NULL == regionLink || !DgnLinkManager::GetManager ().ValidateLinkTarget (*regionLink, logger))
            continue;

        if (findDrawingLink)
            {
            /* NEEDSWORK: Make sure it is a reference link  if (0 == regionLink->GetTargetType ().compare (XMLTAG_RegionTypeReference)) */
                return regionLink;
            }
        else
            {
            /* NEEDSWORK: Make sure it is a drawing title link  if (0 == regionLink->GetTargetType ().compare (XMLTAG_RegionTypeDrawing)) */
                return regionLink;
            }
        }

    return NULL;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void    DetailingSymbolBaseXAttributeHandler::_DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xaHandle, DgnModelP modelRef)
    {
    refs->insert (xaHandle.GetElementRef ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WString       INamedViewCreateHelper::GetViewNamePrefix ()
    {
    return g_dgnHandlersResources->GetString (GetViewNamePrefixID ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WString       INamedViewCreateHelper::GetViewType ()
    {
    return g_dgnHandlersResources->GetString (GetViewTypeID ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailingSymbolBaseHandler::EditDetailingSymbolChildProperties (EditElementHandleR eeh,
                            PropertyContextR context, EditElementHandleR detElemHdl) const
    {
    //Edit all my public children 
    for (ChildEditElemIter childEEH (eeh, ExposeChildrenReason::Edit); childEEH.IsValid(); childEEH = childEEH.ToNext())
        {
        childEEH.GetHandler().EditProperties (childEEH, context);
        EditDetailingSymbolChildProperties (childEEH, context, detElemHdl);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailingSymbolBaseHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    //This will handle only the parent cell because the detailing symbol do not expose the children
    T_Super::_EditProperties (eeh, context);

    //We need the elemedescr for modifying the properties as a bunch
    eeh.GetElementDescrP();

    //Edit all my private children 
    for (ChildEditElemIter childEEH (eeh, ExposeChildrenReason::Count); childEEH.IsValid(); childEEH = childEEH.ToNext())
        EditDetailingSymbolChildProperties (childEEH, context, eeh);

     // If purpose is just a simple id remap we don't need to regenerate detailing symbol
    if (EditPropertyPurpose::Change != context.GetIEditPropertiesP ()->GetEditPropertiesPurpose () || !context.GetElementChanged ())
        return;

    // Simple properties do not require the detailing symbol to be regenerated
    ElementProperties simpleProps  = (ElementProperties) (ELEMENT_PROPERTY_Level| ELEMENT_PROPERTY_Color| ELEMENT_PROPERTY_Linestyle| ELEMENT_PROPERTY_Weight| ELEMENT_PROPERTY_Transparency| ELEMENT_PROPERTY_ElementClass| ELEMENT_PROPERTY_DisplayPriority);
    ElementProperties complexProps = (ElementProperties) (ELEMENT_PROPERTY_All & ~simpleProps);
    if (0 == (complexProps & context.GetElementPropertiesMask ()))
        return;

    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (eeh);
    if (def.IsValid())
        def->RegenerateSymbol (eeh, &eeh, eeh.GetDgnModel ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    VolumeDefXAttributeHandler::_AreEqual (void const* data1, int size1, void const* data2, int size2, UInt32, double distTol, double dirTol)
    {
    VolumeDef const* v1 = (VolumeDef const*) data1;
    VolumeDef const* v2 = (VolumeDef const*) data2;

    UInt32*  params1 = (UInt32*) &v1->m_data.params;
    UInt32*  params2 = (UInt32*) &v2->m_data.params;
    if (*params1 != *params2)
        return false;

    if (fabs (v1->m_data.frontDepth - v2->m_data.frontDepth) > distTol ||
        fabs (v1->m_data.backDepth - v2->m_data.backDepth) > distTol ||
        fabs (v1->m_data.minWidth - v2->m_data.minWidth) > distTol ||
        fabs (v1->m_data.maxWidth - v2->m_data.maxWidth) > distTol ||
        fabs (v1->m_data.topHeight - v2->m_data.topHeight) > distTol ||
        fabs (v1->m_data.bottomHeight - v2->m_data.bottomHeight) > distTol)
        return false;

    return true;
    }

#endif

//          V8                                                              DgnDB
//  ---------------------------         ----------------------------------------------------------------------------------
/*
attachments               PEPs          Models                 Views                    Other Tables
                                        -------------          ---------------
            Design Model                Physical Model      <- Physical View            
  ,------>  Section View <-.                                <- Sectioning View          
  |   ,->   Plan View      |                                                                                
  |   |                    |            "Plan"                                                                    
  |   | "Plan" Drawing     |            Drawing Model       <- Drawing View             DgnLinkTable                    
  |   `--DgnAttachment     |                                                            -------------                    
  |      Section Callout---'             Section Callout                                DgnViewLink(SectionCallout,SectioningView)
  |
  |                                     "Section"                                                
  | "Section" Drawing                   Drawing Model       <- Drawing View             ViewGeneratedDrawing table
  `-DgnAttachment  <-------.                                                            --------------------
    Section Drawing Title -'                                                            Sectioning View, Section Drawing Model
                                                  
*/

// *** NEEDS WORK -- Extracted from SectionCalloutHandler.cpp
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SectionCalloutHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_SectionCallout));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId CalloutBaseHandler::FindGeneratedView (ElementHandleCR rootEH)
    {
    if (rootEH.GetDgnProject() == NULL)
        return DgnViewId();
    for (auto const& linkFactory : DgnLinkTable::OnElementIterator (*rootEH.GetDgnProject(), rootEH.GetElementId()))
        {
        if (linkFactory.GetLinkType() == DgnLinkType::View)
            {
            auto link = linkFactory.CreateEntry();
            return dynamic_cast<DgnLinkTable::DgnViewLinkEntry*>(link.get())->GetViewId();
            }
        }
    return DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CalloutBaseHandler::FindGeneratedViewName (WStringR viewName, ElementHandleCR symbolEH)
    {
    auto viewId = FindGeneratedView (symbolEH);
    if (!viewId.IsValid())
        return ERROR;
    auto view = symbolEH.GetDgnProject()->Views().QueryViewById (viewId);
    if (!view.IsValid())
        return ERROR;
    viewName = WString(view.GetName(), BentleyCharEncoding::Utf8);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SectionCalloutHandler::_GetDescription
(
ElementHandleCR    el,
WString &       descr,
UInt32          desiredLength
)
    {
    T_Super::_GetDescription (el, descr, desiredLength);

    WString     viewName;
    if (SUCCESS == FindGeneratedViewName (viewName, el))
        {
        descr.append (L", ");
        descr.append (viewName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlanCalloutHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_PlanCallout));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlanCalloutHandler::_GetDescription
(
ElementHandleCR    el,
WString &       descr,
UInt32          desiredLength
)
    {
    T_Super::_GetDescription (el, descr, desiredLength);

    WString     viewName;
    if (SUCCESS == FindGeneratedViewName (viewName, el))
        {
        descr.append (L", ");
        descr.append (viewName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailCalloutHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_DetailCallout));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DetailCalloutHandler::_GetDescription
(
ElementHandleCR    el,
WString &       descr,
UInt32          desiredLength
)
    {
    T_Super::_GetDescription (el, descr, desiredLength);

    WString     viewName;
    if (SUCCESS == FindGeneratedViewName (viewName, el))
        {
        descr.append (L", ");
        descr.append (viewName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElevationCalloutHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_ElevationCallout));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElevationCalloutHandler::_GetDescription
(
ElementHandleCR    el,
WString &       descr,
UInt32          desiredLength
)
    {
    T_Super::_GetDescription (el, descr, desiredLength);

    WString     viewName;
    if (SUCCESS == FindGeneratedViewName (viewName, el))
        {
        descr.append (L", ");
        descr.append (viewName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DrawingTitleHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_DrawingTitle));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DrawingTitleHandler::_GetDescription
(
ElementHandleCR    el,
WStringR        descr,
UInt32          desiredLength
)
    {
    T_Super::_GetDescription (el, descr, desiredLength);

#ifdef WIP_DETAILINGSYMBOLS

    IDetailingSymbolPtr def = DetailingSymbolManager::CreateDetailingSymbol (el);

    DrawingTitleDef* drawingDef = dynamic_cast <DrawingTitleDef*> (def.get ());
    if (!drawingDef)
        {
//        BeAssert (0);
        return;
        }

    WString str;

    drawingDef->GetDrawingName (str);
    if (str.length ())
        {
        descr.append (L", ");
        descr.append (str);
        }

    drawingDef->GetDrawingIdentifier (str);
    if (str.length ())
        {
        descr.append (L", ");
        descr.append (str);
        }
#endif
    }

// Same values as used in Vancouver
#define SYMBOLSETTINGS_MINORID_SectionCallout_IDENTIFIER    6
#define SYMBOLSETTINGS_MINORID_DetailCallout_IDENTIFIER     7
#define SYMBOLSETTINGS_MINORID_ElevationCallout_IDENTIFIER  8
#define SYMBOLSETTINGS_MINORID_TitleText_IDENTIFIER         9
#define SYMBOLSETTINGS_MINORID_DrawingTitle_IDENTIFIER      10
#define SYMBOLSETTINGS_MINORID_PlanCallout_IDENTIFIER       18

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DetailingSymbolBaseHandler::RegisterHandlers()
    {
    DgnDraftingDomain& draftingDomain = DgnDraftingDomain::GetInstance(); 
    
    // *** NEEDS WORK -- See DetailingSymbolManager::RegisterHandlers () for all of the many handlers that are registered in Vancouver
    ElementHandlerId  sectionCalloutHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_SectionCallout_IDENTIFIER);
    draftingDomain.RegisterHandler (sectionCalloutHID, ELEMENTHANDLER_INSTANCE(SectionCalloutHandler));

    ElementHandlerId  PlanCalloutHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_PlanCallout_IDENTIFIER);
    draftingDomain.RegisterHandler (PlanCalloutHID, ELEMENTHANDLER_INSTANCE(PlanCalloutHandler));

    ElementHandlerId  ElevationCalloutHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_ElevationCallout_IDENTIFIER);
    draftingDomain.RegisterHandler (ElevationCalloutHID, ELEMENTHANDLER_INSTANCE(ElevationCalloutHandler));

    ElementHandlerId  DetailCalloutHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_DetailCallout_IDENTIFIER);
    draftingDomain.RegisterHandler (DetailCalloutHID, ELEMENTHANDLER_INSTANCE(DetailCalloutHandler));

    ElementHandlerId  drawingTitleHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_DrawingTitle_IDENTIFIER);
    draftingDomain.RegisterHandler (drawingTitleHID, ELEMENTHANDLER_INSTANCE(DrawingTitleHandler));

    ElementHandlerId  TitleTextHID (XATTRIBUTEID_SymbolSettings, SYMBOLSETTINGS_MINORID_TitleText_IDENTIFIER);
    draftingDomain.RegisterHandler (TitleTextHID, ELEMENTHANDLER_INSTANCE(TitleTextHandler));
    }