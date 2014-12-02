/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/SharedCellHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SCPropertiesFilter : public ProcessPropertiesFilter
{
protected:

SCOverride      m_override;

public:

SCPropertiesFilter (IQueryProperties* queryObj, SCOverride const* scOverride) : ProcessPropertiesFilter (queryObj) {m_override = *scOverride;}
SCPropertiesFilter (IEditProperties*  editObj,  SCOverride const* scOverride) : ProcessPropertiesFilter (editObj)  {m_override = *scOverride;}

SCOverride*     GetSCOverrides () {return &m_override;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SetPropertyFlags (EachPropertyBaseArg& arg, bool scOverride)
    {
    if (scOverride || 0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return;

    arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_UndisplayedID | arg.GetPropertyFlags ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool SetOverride (EachPropertyBaseArg& arg, bool scOverride)
    {
    if (!m_editObj || 0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return scOverride;

    if (0 != (arg.GetPropertyCallerFlags () & PROPSCALLER_FLAGS_SharedChildOvrSet))
        return true;

    if (0 != (arg.GetPropertyCallerFlags () & PROPSCALLER_FLAGS_SharedChildOvrClear))
        return false;

    return scOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void _EachLevelCallback (EachLevelArg& arg) override
    {
    SetPropertyFlags (arg, m_override.level);
    m_callbackObj->_EachLevelCallback (arg);
    m_override.level = SetOverride (arg, m_override.level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void _EachElementClassCallback (EachElementClassArg& arg) override
    {
    // NOTE: Class override has apparently never been honored...don't allow it to be set!
    m_callbackObj->_EachElementClassCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void _EachColorCallback (EachColorArg& arg) override
    {
    SetPropertyFlags (arg, m_override.color);
    m_callbackObj->_EachColorCallback (arg);
    m_override.color = SetOverride (arg, m_override.color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void _EachLineStyleCallback (EachLineStyleArg& arg) override
    {
    SetPropertyFlags (arg, m_override.style);
    m_callbackObj->_EachLineStyleCallback (arg);
    m_override.style = SetOverride (arg, m_override.style);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void _EachWeightCallback (EachWeightArg& arg) override
    {
    SetPropertyFlags (arg, m_override.weight);
    m_callbackObj->_EachWeightCallback (arg);
    m_override.weight = SetOverride (arg, m_override.weight);
    }
}; // SCPropertiesFilter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    // NOTE: Need to check overrides, use filter to modify output of super....
    IQueryProperties*   queryObj = context.GetIQueryPropertiesP ();
    SCPropertiesFilter  filterObj (queryObj, &((SharedCellCP) eh.GetElementCP ())->m_override);

    context.SetIQueryPropertiesP (&filterObj);
    T_Super::_QueryProperties (eh, context);
    context.SetIQueryPropertiesP (queryObj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    // NOTE: Need to check/set overrides, use filter to modify output of super....
    IEditProperties*    editObj = context.GetIEditPropertiesP ();
    SCPropertiesFilter  filterObj (editObj, &((SharedCellP) eeh.GetElementP ())->m_override);

    context.SetIEditPropertiesP (&filterObj);
    T_Super::_EditProperties (eeh, context);
    context.SetIEditPropertiesP (editObj);

    if (0 == memcmp (&((SharedCellP) eeh.GetElementP ())->m_override, filterObj.GetSCOverrides (), sizeof (SCOverride)))
        return;

    memcpy (&((SharedCellP) eeh.GetElementP ())->m_override, filterObj.GetSCOverrides (), sizeof (SCOverride));
    context.SetElementChanged ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::PopulateElemHeaderOverrides (ElemHeaderOverridesR ovr, SharedCellCP pSCell)
    {
    Symbology symb = pSCell->GetSymbology();

    LineStyleParams styleParams;

    LineStyleLinkageUtil::ExtractParams (&styleParams, reinterpret_cast <DgnElementCP> (pSCell));
    ovr.Init (&pSCell->m_override, LevelId(pSCell->GetLevel()), 0, pSCell->GetDisplayPriority(), (DgnElementClass) pSCell->GetElementClassValue(), &symb, &styleParams);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getCellAnnotationScale (double& scaleValue, ElementHandleCR elHandle)
    {
    scaleValue = 1.0;

    bool        scaleFlag = (SUCCESS == AnnotationScale::GetFromXAttribute (&scaleValue, elHandle));

    if (fabs (scaleValue) < mgds_fc_epsilon)
        scaleValue = 1.0;

    return scaleFlag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static inline bool hasAnnotationScale (ElementHandleCR elHandle)
    {
    double __;
    return getCellAnnotationScale (__, elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::PushSC (ElementHandleCR scell, ElementHandleCR scDef, DgnModelP modelRef, ViewContextR context, bool pushTransform)
    {
    SharedCellCP pSCell = &scell.GetElementCP()->ToSharedCell();

    ElemHeaderOverrides  ovr;
    if (PopulateElemHeaderOverrides (ovr, pSCell))
        context.PushOverrides (&ovr);

    if (!pushTransform)
        return;

    DPoint3d    origin = pSCell->origin;

    if (!pSCell->Is3d())
        origin.z = 0;

    if (pSCell->IsViewIndependent())
        context.PushViewIndependentOrigin (&origin);

    RotMatrix   rMatrix = pSCell->rotScale;

    // 2d instance and 3d def: push flatten transform or def cmpns may be clipped by display priority range...
    if (!pSCell->Is3d())
        {
        if (scDef.GetElementCP()->Is3d())
            {
            // Create a flattening matrix so the 3d SCDef is 1/2 unit (so it rounds to 0).
            DRange3dCR defRange = scDef.GetElementCP()->GetRange();
            double defZSize = (double)defRange.high.z - (double)defRange.low.z;
            if (1 < defZSize)
                {
                RotMatrix   flattenRMatrix;
                flattenRMatrix.initFromScaleFactors (1.0, 1.0, 1.0/(defZSize*2));
                rMatrix.productOf (&flattenRMatrix, &rMatrix);
                }

            origin.z = (defRange.high.z + defRange.low.z) * -0.5; // TR#345052 Move to z = 0...
            }
        else
            {
            // 2d shared cells sometimes have a z scale. That screws up display priority. Make sure we never have a z scale.
            rMatrix.form3d[2][2] = 1.0;
            }
        }

    // NB: Do this BEFORE pushing the SC transform.
    AnnotationDisplayParameters parms;

    if (ComputeAnnotationDisplayParameters (parms, scell, context))
        {
        IAnnotationStrokeForCache::PushTransformToRescale (context, origin, parms, true); // This will rescale the display of entire cell (uniformly)
        }

#if defined (NEEDS_WORK_UNITS)
    if (scell.GetElementRef() && scell.GetElementRef()->HasDerivedRange())  // If the instance has a derived range
        context.SetNoRangeTestOnComponents (true); // Then components' stored ranges are incorrect, because they do NOT have derived ranges.
#endif

    Transform       trans;
    ClipVectorPtr   clip;

    trans.initFrom (&rMatrix, &origin);

    if (SUCCESS == CellUtil::ClipFromLinkage (clip, scell, NULL) && clip.IsValid())
        context.PushClip (*clip);

    context.PushTransform (trans);

    // First check with the scDef about special transform flags
    bool        nondefaultScaleForDims, scaleMultilines, rotateDimView;

    CellUtil::GetSharedCellDefFlags (&nondefaultScaleForDims, &scaleMultilines, &rotateDimView, NULL, scDef);

    // Second check with scInst flag to override scDef...
    bool        scaleDimsWysiwyg;

    scaleDimsWysiwyg = pSCell->m_override.scaleDimsWysiwyg;

    // When scInst is scaled, check if dims should be scaled in non-default way (scale dim size and don't scale dim value)
    context.SetIgnoreScaleForDimensions (TO_BOOL (!nondefaultScaleForDims || scaleDimsWysiwyg));

    // When scInst is scaled, check if multiline offsets can be scaled
    context.SetIgnoreScaleForMultilines (TO_BOOL (!scaleMultilines));

    // When scInst is rotated, rotate dim view also (so that its text orientation remains constant)
    context.SetApplyRotationToDimView (TO_BOOL (rotateDimView));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellHandler::SetPointCell (EditElementHandleR eeh, bool isPointCell)
    {
    SharedCellR el = eeh.GetElementP()->ToSharedCellR();

    if (SHARED_CELL_ELM != el.GetLegacyType())
        return ERROR;

    el.SetViewIndependent(isPointCell);
    el.SetSnappable(!isPointCell);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellHandler::SetName (EditElementHandleR eeh, WCharCP cellName)
    {
    if (NULL == cellName)
        { BeAssert (false); return ERROR; }
    
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    if (SUCCESS != CellUtil::SetCellName (elm, cellName))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellHandler::SetDescription (EditElementHandleR eeh, WCharCP descr)
    {
    if (NULL == descr)
        { BeAssert (false); return ERROR; }
    
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    if (SUCCESS != CellUtil::SetCellDescription (elm, descr))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellHandler::SetDefinitionID (EditElementHandleR eeh, ElementId elemID)
    {
    DgnV8ElementBlank   el;

    eeh.GetElementCP()->CopyTo (el);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (&el, DEPENDENCYAPPID_SharedCellDef, 0);

    if (SUCCESS != DependencyManagerLinkage::AppendSimpleLinkageToMSElement (&el, DEPENDENCYAPPID_SharedCellDef, 0, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, elemID.GetValue()))
        return ERROR;

    eeh.ReplaceElement (&el);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isFrozen (SharedCellCR sCell)
    {
    return TO_BOOL(sCell.freezeGroup);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::IsVisibleLevels (ElementHandleCR thisElm, ViewContextR context)
    {
    SharedCellCP    sCell = &thisElm.GetElementCP()->ToSharedCell();
    DgnModelP       modelRef = thisElm.GetDgnModelP();

    if (isFrozen (*sCell))
        return false;

    // points cells need only check the instance's level
    if (_IsPointCell (thisElm))
        {
        if (T_Super::_IsVisible (thisElm, context, false, true, false))
            return true;

        return false;
        }

    ElementHandle  defnEh (_GetDefinition (thisElm, modelRef->GetDgnProject()));
    if (!defnEh.IsValid ())
        return false; // no definition, can't be visible

    // restored in destructor!
    ViewContext::ContextMark    mark (context, thisElm);

    PushSC (thisElm, defnEh, modelRef, context, false); // Needed for BYCELL...

    for (ChildElemIter childIter (defnEh, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DisplayHandlerP dHandler = childIter.GetDisplayHandler();

        if (dHandler && dHandler->IsVisible (childIter, context, false, true, false))
            {
            if (childIter.GetElementCP ()->IsInvisible())
                continue;

            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::_IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    if (testLevel && !IsVisibleLevels (elHandle, context))
        return  false;

    // never test level on a shared cell header
    return T_Super::_IsVisible (elHandle, context, testRange, false, testClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  SharedCellHandler::_DoScannerTests (ElementHandleCR eh, BitMaskCP, UInt32 const* classMask, ViewContextP)
    {
    if (NULL == classMask)
        return ScanTestResult::Pass;

    return (0 == (eh.GetElementCP()->ToSharedCell().m_class & *classMask)) ? ScanTestResult::Fail : ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::PickOrigin (ElementHandleCR thisElm, ViewContextR context)
    {
    IPickGeom*  pick = context.GetIPickGeom();

    if (NULL == pick)
        return;

    SharedCellCP sCell = &thisElm.GetElementCP()->ToSharedCell();

    // We never reject shared cell instances on level criteria. That usually doesn't matter, since we
    // then test the level of each of the children. BUT, for snapping to the origin, we need to make sure
    // the level of the instance is on.
    if (IsVisibleLevels (thisElm, context))
        {
        // test for the shared cell origin
        pick->SetHitPriorityOverride (HitPriority::CellOrigin);
        context.GetIDrawGeom().DrawPointString3d (1, &sCell->origin, NULL);
        pick->SetHitPriorityOverride (HitPriority::Highest);
        }

#if defined (NOT_NOW)
    // if this is a "point" shared cell, only allow origin as snap point
    if (_IsPointCell (thisElm))
        pick->SetPathCursorIndex();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void getRangeShape (DPoint3dP shp, DRange3dCP range, bool is3d)
    {
    if (is3d)
        {
        shp[0].x = shp[3].x = shp[4].x = shp[5].x = range->low.x;
        shp[1].x = shp[2].x = shp[6].x = shp[7].x = range->high.x;

        shp[0].y = shp[1].y = shp[4].y = shp[7].y = range->low.y;
        shp[2].y = shp[3].y = shp[5].y = shp[6].y = range->high.y;

        shp[0].z = shp[1].z = shp[2].z = shp[3].z = range->low.z;
        shp[4].z = shp[5].z = shp[6].z = shp[7].z = range->high.z;
        }
    else
        {
        memset (shp, 0, 8 * sizeof (DPoint3d));

        shp[0].x = shp[3].x = shp[7].x = range->low.x;
        shp[1].x = shp[2].x = shp[5].x = range->high.x;

        shp[0].y = shp[1].y = shp[4].y = range->low.y;
        shp[2].y = shp[3].y = shp[6].y = range->high.y;

        shp[4].x = shp[6].x = ((shp[0].x + shp[1].x)/2.0);
        shp[5].y = shp[7].y = ((shp[4].y + shp[6].y)/2.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawFastSCell (ElementHandleCR thisElm, ViewContextR context)
    {
    DgnElementCP el = thisElm.GetElementCP ();
    DPoint3d    pts[8];

    getRangeShape (pts, (DRange3dCP)&el->ToSharedCell().rangeDiag, el->Is3d());

    for (int i=0; i<8; i++)
        {
        el->ToSharedCell().rotScale.Multiply(pts[i]);
        bsiDPoint3d_addDPoint3dDPoint3d (&pts[i], &pts[i], (DVec3dCP) &el->ToSharedCell().origin);
        }

    context.DrawBox (pts, el->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     wantFastSCell (DgnElementCP header)
    {
    return header->ToCell_2d().componentCount > T_HOST.GetGraphicsAdmin()._GetFastCellThreshold ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        // We must cache cut graphics on shared cell instances as allowing the children to cache wouldn't work.
        case SupportOperation::CacheCutGraphics:
            return true;

        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    SharedCellCP    sCell = &thisElm.GetElementCP()->ToSharedCell();
    DgnModelP       modelRef = thisElm.GetDgnModelP();

    if (isFrozen (*sCell))
        return;

    ElementHandle  defnEh (_GetDefinition (thisElm, modelRef->GetDgnProject()));

    if ( ! defnEh.IsValid ())
        return;

    if (DrawPurpose::Pick == context.GetDrawPurpose ())
        PickOrigin (thisElm, context);

    if (context.GetViewFlags() && context.GetViewFlags()->fast_cell && wantFastSCell (defnEh.GetElementCP()))
        return drawFastSCell (thisElm, context);

    // restored in destructor!
    ViewContext::ContextMark    mark (context, thisElm);

    PushSC (thisElm, defnEh, modelRef, context, true);

    // Can't use cache for nested cells with BYCELL attrs...
    bool&  useCachedGraphics = context.GetUseCachedGraphics();
    AutoRestore<bool> saveUseCachedGraphics (&useCachedGraphics);

    if (useCachedGraphics && sCell->IsComplexComponent())
        {
        // NOTE: (CACHED_COLOR_INDEX) Can use cached representation if only COLOR_BYCELL...
        if (WEIGHT_BYCELL == sCell->GetSymbology().weight || STYLE_BYCELL == sCell->GetSymbology().style)
            useCachedGraphics = false;
        }

    context.VisitElemHandle (defnEh, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_DrawFiltered (ElementHandleCR thisElm, ViewContextR context, DPoint3dCP pts, double size)
    {
    if (FILTER_LOD_ShowNothing == context.GetFilterLODFlag ())
        return;

    IDrawGeomR  output = context.GetIDrawGeom ();

    // NOTE: This is basically IsVisibleLevels with DrawFiltered thrown in...
    SharedCellCP    sCell = &thisElm.GetElementCP()->ToSharedCell();
    DgnModelP       modelRef = thisElm.GetDgnModelP();

    if (isFrozen (*sCell))
        return;

    // points cells need only check the instance's level
    if (_IsPointCell (thisElm))
        {
        if (T_Super::_IsVisible (thisElm, context, false, true, false))
            T_Super::_DrawFiltered (thisElm, context, pts, size);

        return;
        }

    ElementHandle  defnEh (_GetDefinition (thisElm, modelRef->GetDgnProject()));
    if (!defnEh.IsValid ())
        return; // no definition, can't be visible

    // restored in destructor!
    ViewContext::ContextMark    mark (context, thisElm);

    PushSC (thisElm, defnEh, modelRef, context, false); // Needed for BYCELL...

    for (ChildElemIter childIter (defnEh, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        DisplayHandlerP dHandler = childIter.GetDisplayHandler();

        if (!dHandler || !dHandler->IsVisible (childIter, context, false, true, false))
            continue;

        DgnElementCP childElmCP = childIter.GetElementCP ();

        if (childElmCP->IsInvisible())
            continue;

        // NOTE: Don't call super, need to cook symbology using sc def cmpn but 2d/3d comes from sc instance...
        context.CookElemDisplayParams (childIter);

        UInt32      info = context.GetDisplayInfo (context.GetCurrentDisplayParams ()->IsRenderable ());

        if (DISPLAY_INFO_None == info)
            return;

        DgnElementCP     el = thisElm.GetElementCP ();

        if (size < LOD_DISPLAY_AS_POINT)
            {
            // Use lod size as raster width...
            if (0 == (info & DISPLAY_INFO_Edge))
                context.GetOverrideMatSymb ()->SetWidth (MAX ((UInt32) size, context.GetCurrWidth()));

            context.ActivateOverrideMatSymb ();

            // NOTE: PointString is slow in wireframe w/o aaLines enabled! Nothing may display otherwise!
            if (el->Is3d())
                output.DrawPointString3d (1, pts, NULL);
            else
                output.DrawPointString2d (1, (DPoint2dP) pts, context.GetDisplayPriority(), NULL);

            return;
            }

        // Ignore line codes and activate overrides for hilite, etc.
        context.GetOverrideMatSymb ()->SetRasterPattern (0xffffffff);
        context.ActivateOverrideMatSymb ();

        if (el->Is3d())
            {
            DPoint3d    pts3d[4];

            if (0 == (info & DISPLAY_INFO_Edge))
                {
                pts3d[0] = pts3d[3] = pts[0];

                for (int iPoly = 0; iPoly < 3; iPoly++)
                    {
                    pts3d[1] = (0 == iPoly ? pts[5] : pts[3]);
                    pts3d[2] = (2 == iPoly ? pts[5] : pts[6]);

                    output.DrawShape3d (4, pts3d, true, NULL);
                    }
                }
            else
                {
                pts3d[0] = pts[0];
                pts3d[1] = pts[7];
                pts3d[2] = pts[5];
                pts3d[3] = pts[6];

                output.DrawLineString3d (4, pts3d, NULL);
                }

            return;
            }

        DPoint2d    pts2d[4];

        DataConvert::Points3dTo2d (pts2d, pts, 3);
        pts2d[3] = pts2d[0];

        if (0 == (info & DISPLAY_INFO_Edge))
            output.DrawShape2d (4, pts2d, true, context.GetDisplayPriority(), NULL);
        else
            output.DrawLineString2d (4, pts2d, context.GetDisplayPriority(), NULL);

        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SharedCellHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    ElementHandle  defnEh (_GetDefinition (elHandle, *elHandle.GetDgnProject()));

    if (!defnEh.IsValid ())
        return ERROR;

    // NOTE: Changed for Vancouver. SS3 called CalculateDefaultRange which invoked the scDef component's _Draw methods.
    //       This was very inefficient and would also bypass the scDef component's override of _ValidateElementRange.
    //       Instance range wasn't being set correctly when scDef contains assoc regions with invisible boundaries.
    DRange3d    range = defnEh.GetElementCP()->GetRange();

    DgnElementP  elP = elHandle.GetElementP ();
    Transform   trans;

    trans.InitFrom (elP->ToSharedCell().rotScale, elP->ToSharedCell().origin);
    trans.Multiply (range, range);

    ElemRangeCalc rangeCalc; // Use ElemRangeCalc for error checking and default 2d range z limits...

    rangeCalc.SetRange (range);
    rangeCalc.ToScanRange (elP->GetRangeR(), elP->Is3d());

    // Account for point cell range and clip linkage...
    if (_IsPointCell (elHandle) || mdlElement_attributePresent (elP, LINKAGEID_ClipBoundary, NULL))
        ValidateViewIndependentElementRange (elHandle);

    // pull these fields from the definition directly into the instance.
    elP->ToSharedCellR().rangeDiag = defnEh.GetElementCP ()->ToSharedCellDef().rangeDiag;
    elP->ToSharedCellR().m_class   = defnEh.GetElementCP ()->ToSharedCellDef().m_class;

    // If def is 3D and instance is 2D, flatten the range.
    if (!elP->ToSharedCell().Is3d() && defnEh.GetElementCP ()->Is3d())
        elP->GetRangeR().low.z = elP->GetRangeR().high.z = 0.0;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR tInfo)
    {
    // translation+scaling is ok, rotation/mirror not ok...
    if (_IsPointCell (elemHandle))
        {
        if (!tInfo.GetTransform ()->isTranslate (NULL))
            return false; // disallow rotation, mirror, or non-uniform scale...
        }

    if ((tInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale) ||
        ((tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource) && hasAnnotationScale (elemHandle)))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SharedCellHandler::_OnTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    if (SUCCESS != T_Super::_OnTransform (elemHandle, trans))
        return ERROR;

    DPoint3d    rOrigin;   
    _GetTransformOrigin (elemHandle, rOrigin);

    // TransformInfo will be modified based on options
    TransformInfo modifiedTransInfo = trans;

    /*-------------------------------------------------------------------
        Step 1 : Propagate annotation scale
    -------------------------------------------------------------------*/
    if (modifiedTransInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale && _IsAnnotation (elemHandle))
        {
        bool    newScaleFlag;
        double  newScaleValue, geometryScale;

        if (SUCCESS == GetAnnotationScaleChange (newScaleFlag, newScaleValue, geometryScale, elemHandle, trans))
            {
            modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_DimSizeMatchSource);

            (modifiedTransInfo.GetTransformR()).ScaleMatrixColumns (*( &modifiedTransInfo.GetTransformR()),  geometryScale,  geometryScale,  geometryScale);
            bsiTransform_setFixedPoint (&modifiedTransInfo.GetTransformR(), &rOrigin);

            AnnotationScale::SetAsXAttribute (elemHandle, newScaleFlag ? &newScaleValue : NULL);
            elemHandle.GetElementP ()->ToSharedCellR().m_override.scaleDimsWysiwyg = true;
            }
        }

    /*-------------------------------------------------------------------
        Step 2 : Propagate transform
    -------------------------------------------------------------------*/
    DgnElementP  el = elemHandle.GetElementP();

    ( modifiedTransInfo.GetTransform())->Multiply(rOrigin);
    el->ToSharedCellR().origin = rOrigin;

    (el->ToSharedCellR().rotScale).InitProduct(*( modifiedTransInfo.GetTransform()), *( &el->ToSharedCellR().rotScale));
    bsiRotMatrix_augmentRank (&el->ToSharedCellR().rotScale, &el->ToSharedCellR().rotScale);

    // Note: do NOT transform ToSharedCell().rangeDiagonal. The purpose of this member
    //      is to record and preserve the ORIGINAL shape of the cell.

    return SUCCESS;
    }

/*=================================================================================**//**
  @bsiclass                                                     Brien.Bastings  06/10
+===============+===============+===============+===============+===============+======*/
struct SCOverrideSetter : IEditProperties
{
private:

ElemHeaderOverridesCR   m_ovr;
LevelId                 m_parentLevel;
Symbology               m_parentSymb;
DgnModelP            m_modelRef;

public:

virtual ElementProperties _GetEditPropertiesMask () override {return (ElementProperties) (ELEMENT_PROPERTY_Level | ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight);}
virtual DgnModelP      _GetDestinationDgnModel () override {return m_modelRef;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _EachColorCallback (EachColorArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_ovr.GetFlags ().color)
        {
        arg.SetStoredValue (m_ovr.GetColor ());
        arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_SharedChildOvrSet);
        }

    if (COLOR_BYCELL == arg.GetStoredValue ())
        arg.SetStoredValue (m_parentSymb.color);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _EachLineStyleCallback (EachLineStyleArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_ovr.GetFlags ().style)
        {
        arg.SetStoredValue (m_ovr.GetLineStyle ());
        arg.SetParams (m_ovr.GetLineStyleParams ());
        arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_SharedChildOvrSet);
        }

    if (STYLE_BYCELL == arg.GetStoredValue ())
        arg.SetStoredValue (m_parentSymb.style);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _EachWeightCallback (EachWeightArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_ovr.GetFlags ().weight)
        {
        arg.SetStoredValue (m_ovr.GetWeight ());
        arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_SharedChildOvrSet);
        }

    if (WEIGHT_BYCELL == arg.GetStoredValue ())
        arg.SetStoredValue (m_parentSymb.weight);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _EachLevelCallback (EachLevelArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_ovr.GetFlags ().level || m_ovr.GetFlags ().relative)
        {
        arg.SetStoredValue (m_ovr.AdjustLevel (arg.GetStoredValue ()));
        arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_SharedChildOvrSet);
        }
    
    if (LEVEL_BYCELL == arg.GetStoredValue ().GetValueUnchecked())
        arg.SetStoredValue (m_parentLevel);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SCOverrideSetter (ElemHeaderOverridesCR ovr, LevelId const& parentLevel, Symbology const& parentSymb, DgnModelP modelRef) : m_ovr (ovr)
    {
    m_parentLevel = parentLevel;
    m_parentSymb  = parentSymb;
    m_modelRef    = modelRef;
    }

}; // SCOverrideSetter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
//static void validateElementDimension (EditElementHandleR eeh, bool is3d) unused function removed in graphite
//    {
//    if (!eeh.GetElementCP ()->IsGraphic() || eeh.GetElementCP ()->Is3d() == is3d)
//        return;
//
//    if (is3d)
//        {
//        eeh.GetHandler().ConvertTo3d (eeh, 0.0);
//        }
//    else
//        {
//        DVec3d      flattenDir;
//        Transform   flattenTrans;
//
//        flattenDir.init (0.0, 0.0, 1.0);
//        flattenTrans.initIdentity ();
//        flattenTrans.form3d[2][2] = 0.0;
//
//        eeh.GetHandler().ConvertTo2d (eeh, flattenTrans, flattenDir);
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
//static void makeUniformDimension (EditElementHandleR eeh, bool is3d)          unused function removed in graphite
//    {
//    validateElementDimension (eeh, is3d);
//
//    // NOTE: Can't rely on handler to have correctly it's private children as they may not be prepared for this invalid case...
//    for (ChildEditElemIter childEeh (eeh, ExposeChildrenReason::Count); childEeh.IsValid (); childEeh = childEeh.ToNext ())
//        makeUniformDimension (childEeh, is3d);
//    }

//BentleyStatus   SharedCellHandler::DropOneLevel (ElementAgendaR agenda, ElementHandleCR eh) graphite moved this function to DgnProjectOptimize

//BentleyStatus   SharedCellHandler::DropOneLevelToNormalCell (EditElementHandleR cellEeh, ElementHandleCR eh) graphite moved this function to DgnProjectOptimize

//void            SharedCellHandler::DropNestedToNormalCell (EditElementHandleR cellEeh) graphite moved this function to DgnProjectOptimize

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SharedCellHandler::_OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options)
    {
#ifdef WIP_VANCOUVER_MERGE // shared cell - DropOneLevel is implemented in foreignformat
    ElementAgenda   agenda;

    if (SUCCESS != DropOneLevel (agenda, eh))
        return ERROR;

    ClipVectorPtr    clipP;
    
    if (! (clipP = fp->GetClipVector()).IsValid())
        return ERROR;

    ClipVectorPtr   linkageClip;
    size_t          originalClipSize = clipP->size();

    if (SUCCESS == CellUtil::ClipFromLinkage (linkageClip, eh, fp->GetTransform ()) && linkageClip.IsValid())
        clipP->Append (*linkageClip);

    for (EditElementHandleP curr = agenda.GetFirstP (), end = curr + agenda.GetCount (); curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;

        curr->GetHandler().FenceClip (inside, outside, *curr, fp, options);
        }
    clipP->resize (originalClipSize);

    return SUCCESS;
#endif
BeAssert(false);
return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SharedCellHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
#ifdef WIP_VANCOUVER_MERGE // shared cell - DropOneLevel is implemented in foreignformat
    if (0 == (DropGeometry::OPTION_SharedCells & geometry.GetOptions ()))
        return ERROR;

    if (DropGeometry::SHAREDCELL_NormalCell == geometry.GetSharedCellOptions () || DropGeometry::SHAREDCELL_NormalCellOneLevel == geometry.GetSharedCellOptions ())
        {
        EditElementHandle cellEeh;

        if (SUCCESS != DropOneLevelToNormalCell (cellEeh, eh))
            return ERROR;

        if (DropGeometry::SHAREDCELL_NormalCell == geometry.GetSharedCellOptions ())
            DropNestedToNormalCell (cellEeh);
        
        dropGeom.Insert (cellEeh);

        return SUCCESS;
        }

    ElementAgenda   agenda;

    if (SUCCESS != DropOneLevel (agenda, eh))
        return ERROR;

    for (EditElementHandleP curr = agenda.GetFirstP (), end = curr + agenda.GetCount (); curr < end ; curr++)
        {
        if (!curr->IsValid () || !curr->GetElementCP ()->IsGraphic() || curr->GetElementCP ()->IsInvisible())
            continue;

        curr->GetElementP ()->IsComplexComponent()= 0; // Clear complex component bit...

        dropGeom.Insert (*curr);
        }

#if defined (VANCOUVER_WIP) // *** Port fix to TR#314364

    static BoolInt detectUselessDependencyLinkages (LinkageHeader *pHeader, void*)
        {
        DependencyLinkage *pDep = ((DependencyLinkage *) (pHeader + 1));
        switch (pDep->appID)
            {
            case DEPENDENCYAPPID_AssocRegion:
                return TRUE;
            }

        return FALSE;
        }

    for each component in cell:
        mdlLinkage_deleteFromElement (component, LINKAGEID_Dependency, 0L, NULL, detectUselessDependencyLinkages, NULL);
    
#endif

    return SUCCESS;
#endif
BeAssert(false);
return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                     04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::_GetAnnotationScale (double* scale, ElementHandleCR el) const
    {
    double dummy;
    if (NULL == scale)
        scale = &dummy;
    return getCellAnnotationScale (*scale, el);
    }

/*---------------------------------------------------------------------------------**//**
* The shared cell itself it not an annotation. Its definition might contain annotations
* as components. Find those annotation components, compute their rescaled ranges, and
* enlarge the range of the shared cell to include them.
* @bsimethod                                    Sam.Wilson                      06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt enlargeRangeToHoldRescaledAnnotationComponents
(                               // <=  SUCCESS if any child's range was rescaled
DRange3dR       totalRange,     // <=> in: range of parent; out: enlarged to include rescaled children, if any
ElementHandleCR    parent,         //  => the complex header to explore
double          rescale,        //  => the factor by which annotation components are to be scaled up
Transform&      trans           //  => the sc instance transform
)
    {
    bool any = false;

    for (ChildElemIter child (parent, ExposeChildrenReason::Count); child.IsValid (); child = child.ToNext ())
        {
        DisplayHandlerP dhandler = child.GetDisplayHandler ();
        if (NULL == dhandler)
            continue;

        IAnnotationHandler* ah = dhandler->GetIAnnotationHandler (child);
        if (NULL != ah)
            {
            DRange3d childRange;
            if (ah->ComputeAnnotationScaledRange (child, childRange, rescale) == SUCCESS)
                {
                trans.Multiply (&childRange.low,  2);
                bsiDRange3d_extendByRange (&totalRange, &childRange);
                any = true;
                }
            }
        else
            {
            if (enlargeRangeToHoldRescaledAnnotationComponents (totalRange, child, rescale, trans) == SUCCESS)
                any = true;
            }
        }
    return any? SUCCESS: ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SharedCellHandler::_ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR drange, double rescale)
    {
    if (hasAnnotationScale (eh))
        {
        // The shared cell is an annotation. Just scale up its range.
        return ComputeAnnotationScaledRange (eh, drange, rescale, NULL);
        }

    //  The shared cell itself it not an annotation, but its definition might contain annotations.
    Transform   trans;
    DgnElementCP el = eh.GetElementCP ();

    trans.InitFrom(*( &el->ToSharedCell().rotScale), *( &el->ToSharedCell().origin));
    drange = el->GetRange();

    DgnModelP  modelRef = eh.GetDgnModelP ();
    ElementHandle  defnEh (_GetDefinition (eh, modelRef->GetDgnProject()));

    return enlargeRangeToHoldRescaledAnnotationComponents (drange, defnEh, rescale, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    origin = elHandle.GetElementCP()->ToSharedCell().origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR rMatrix)
    {
    memcpy (&rMatrix, &elHandle.GetElementCP()->ToSharedCell().rotScale, sizeof (RotMatrix));
    rMatrix.normalizeColumnsOf (&rMatrix, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normalP, DPoint3dP pointP, DVec3dCP inputDefaultNormalP)
    {
    ElementHandle  defnEh (SharedCellHandler::GetDefinition (thisElm, *thisElm.GetDgnProject()));

    if (!defnEh.IsValid ())
        return false;

    if (!defnEh.GetDisplayHandler()->IsPlanar (defnEh, normalP, pointP, inputDefaultNormalP))
        return false;

    Transform   scTrans;
    scTrans.initFromMatrixAndFixedPoint (&thisElm.GetElementCP()->ToSharedCell().rotScale, &thisElm.GetElementCP()->ToSharedCell().origin);

    if (pointP)
        scTrans.multiply (pointP);

    if (normalP)
        scTrans.multiplyMatrixOnly (normalP, normalP);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SharedCellHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    SnapPath        *snap = context->GetSnapPath ();
    SnapMode    snapMode = context->GetSnapMode ();

    if (SnapMode::Origin == snapMode)
        {
        DPoint3d        hitPoint;
        ElementHandle   elHandle (snap->GetPathElem (snapPathIndex));

        _GetSnapOrigin (elHandle, hitPoint);

        context->ElmLocalToWorld (hitPoint);
        context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), hitPoint, true, false);

        return SnapStatus::Success;
        }

    if (snap->GetCount () > snapPathIndex+1)
        return context->DoSnapUsingNextInPath (snapPathIndex);

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_SHARED_CELL_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    _GetTypeName (descr, desiredLength);

    WChar     cellName[MAX_CELLNAME_LENGTH];

    ExtractName (cellName, MAX_CELLNAME_LENGTH, el);

    if (wcslen (cellName) > 0)
        descr.append (L": ").append(cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnElementP  elm = eeh.GetElementP ();

    elm->ToSharedCellR().origin.z = elevation;
    elm->ToSharedCellR().rangeDiag.org.z = elm->ToSharedCellR().rangeDiag.end.z = 0.0;

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandler::CreateSharedCellElement (EditElementHandleR eeh, ElementHandleCP templateEh, WCharCP cellName, DPoint3dCP origin, 
                                                 RotMatrixCP rMatrix, DPoint3dCP scale, bool is3d, DgnModelR modelRef)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, SHARED_CELL_ELM, in->GetLevel(), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, sizeof (SharedCell));
        ElementUtil::SetRequiredFields (out, SHARED_CELL_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim) is3d);
        }

    ElementUtil::InitScanRangeForUnion (out.GetRangeR(), is3d);

    out.SetSnappable(false); // shared cells are always snappable

    bool        templateOfSameType = (in && in->GetLegacyType() == out.GetLegacyType());

    if (!templateOfSameType)
        {
        out.ToSharedCellR().m_class           = 0;
        out.ToSharedCellR().freezeGroup       = 0;
        out.ToSharedCellR().m_override.relative = false;

        memset (&out.ToSharedCellR().m_override,  0, sizeof (out.ToSharedCell().m_override));
        memset (&out.ToSharedCellR().rangeDiag, 0, sizeof (out.ToSharedCell().rangeDiag));
        }

    if (origin)
        out.ToSharedCellR().origin = *origin;
    else if (!templateOfSameType)
        out.ToSharedCellR().origin.zero ();

    if (rMatrix)
        out.ToSharedCellR().rotScale = *rMatrix;
    else if (!templateOfSameType)
        out.ToSharedCellR().rotScale.initIdentity ();

    if (scale)
        out.ToSharedCellR().rotScale.scaleColumns (&out.ToSharedCell().rotScale, scale->x, scale->y, scale->z);

    int         elmSize = sizeof (SharedCell);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);

    if (cellName)
        NormalCellHeaderHandler::SetName (eeh, cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SharedCellHandler::SetSharedCellOverrides (EditElementHandleR eeh, SCOverride* overrides)
    {
    DgnElementP  elm = eeh.GetElementP ();

    if (SHARED_CELL_ELM != elm->GetLegacyType())
        return ERROR;

    if (overrides)
        elm->ToSharedCellR().m_override = *overrides;
    else
        memset (&elm->ToSharedCellR().m_override, 0, sizeof (elm->ToSharedCell().m_override));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SharedCellHandler::CreateSharedCellComplete (EditElementHandleR eeh)
    {
    if (!eeh.IsValid ())
        return ERROR;

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus SharedCellHandler::_OnGeoCoordinateReprojection (EditElementHandleR eeh, IGeoCoordinateReprojectionHelper& gcrH, bool inChain)
    {
    // In some cases (for example TR#230365), there are shared cells that have origins that are far outside the actual cell graphics.
    // That works very poorly because we get the best linear transform at the origin point. So we check the origin to see if it is
    // within the range block, and if it isn't, we use the center of the range block as the transform origin.
    DPoint3d origin;
    _GetTransformOrigin (eeh, origin);

    DRange3dCR dRange = eeh.GetElementCP()->GetRange();
    if (!dRange.isContained (&origin))
        origin.interpolate (&dRange.low, 0.5, &dRange.high);

    TransformInfo   transform;
    ReprojectStatus status = gcrH.GetLocalTransform (&transform.GetTransformR(), origin, NULL, true, true);
    if ( (REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status) )
        return status;

    // if can't transform, don't change what we have.
    if (SUCCESS != _ApplyTransform (eeh, transform))
        return REPROJECT_NoChange;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellDefHandler::_ExposeChildren (ElementHandleCR el, ExposeChildrenReason reason)
    {
    switch (reason)
        {
        case ExposeChildrenReason::Count:
        case ExposeChildrenReason::Query:
        case ExposeChildrenReason::Edit:
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UseChildren (thisElm, context, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SharedCellDefHandler::_OnChangeOfUnits (EditElementHandleR eeh, DgnModelP srcDgnModel, DgnModelP dstDgnModel)
    {
    // Shared cell defs use default model for units, solids extent, etc.
    DgnProjectR srcProject = srcDgnModel->GetDgnProject();
    DgnProjectR dstProject = dstDgnModel->GetDgnProject();

    DgnModelP srcDefaultModel = srcProject.Models().GetModelById(srcProject.Models().GetFirstModelId());
    DgnModelP dstDefaultModel = dstProject.Models().GetModelById(dstProject.Models().GetFirstModelId());

    return T_Super::_OnChangeOfUnits (eeh, srcDefaultModel, dstDefaultModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void    transformSCDefHeader (EditElementHandleR elemHandle, TransformCP trans)
    {
    DPoint3d    rOrigin;

    CellUtil::ExtractOrigin (rOrigin, elemHandle);
    trans->Multiply(rOrigin);

    DgnElementP  el = elemHandle.GetElementP ();

    el->ToSharedCellDefR().origin = rOrigin;

    el->ToSharedCellDefR().rotScale.InitProduct(*trans, *(&el->ToSharedCellDef().rotScale));
    bsiRotMatrix_augmentRank (&el->ToSharedCellDefR().rotScale, &el->ToSharedCellDef().rotScale);

    // Note: do NOT transform range diagonal. The purpose of this is to record and preserve the ORIGINAL shape of the cell.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SharedCellDefHandler::_OnTransform (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    transformSCDefHeader (elemHandle, trans.GetTransform());

    return T_Super::_OnTransform (elemHandle, trans);  // transform children
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellDefHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::Selection:
        case SupportOperation::CellGroup:
            return false;
        }

    return T_Super::_IsSupportedOperation (eh, stype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SharedCellDefHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    // Shared cell def is next in path for a shared cell instance...
    if (context->GetSnapPath ()->GetCount () > snapPathIndex+1)
        return context->DoSnapUsingNextInPath (snapPathIndex);

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_SHAREDCELL_DEF_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    _GetTypeName (descr, desiredLength);

    WChar     cellName[MAX_CELLNAME_LENGTH];

    ExtractName (cellName, MAX_CELLNAME_LENGTH, el);

    if (wcslen (cellName) > 0)
        descr.append (L": ").append(cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    origin = elHandle.GetElementCP()->ToSharedCellDef().origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR rMatrix)
    {
    memcpy (&rMatrix, &elHandle.GetElementCP()->ToSharedCellDef().rotScale, sizeof (RotMatrix));
    rMatrix.normalizeColumnsOf (&rMatrix, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    // NOTE: Required to support explicit convert calls, ex. "transform to world" and drop shared cell mess...
    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void convertSCDefTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    transformSCDefHeader (eeh, &flattenTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // NOTE: Required to support explicit convert calls, ex. "transform to world" and drop shared cell mess...
    convertSCDefTo2d (eeh, flattenTrans, flattenDir);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
//StatusInt       SharedCellDefHandler::_OnPreprocessCopy (EditElementHandleR eeh, ElementCopyContextP cc) removed in graphite
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetPointCell (EditElementHandleR eeh, bool isPointCell)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    el.SetViewIndependent(isPointCell);
    el.SetSnappable(!isPointCell);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetAnnotation (EditElementHandleR eeh, bool isAnnotation)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    bool        nondefaultScaleForDims, scaleMultilines, rotateDimView;

    CellUtil::GetSharedCellDefFlags (&nondefaultScaleForDims, &scaleMultilines, &rotateDimView, NULL, eeh);

    return CellUtil::SetSharedCellDefFlags (eeh, nondefaultScaleForDims, scaleMultilines, rotateDimView, isAnnotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetAnonymous (EditElementHandleR eeh, bool isAnonymous)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    el.ToSharedCellDefR().anonymous = isAnonymous;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetMlineScaleOption (EditElementHandleR eeh, bool option)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    bool        nondefaultScaleForDims, rotateDimView, isAnnotation;

    CellUtil::GetSharedCellDefFlags (&nondefaultScaleForDims, NULL, &rotateDimView, &isAnnotation, eeh);

    return CellUtil::SetSharedCellDefFlags (eeh, nondefaultScaleForDims, option, rotateDimView, isAnnotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetDimScaleOption (EditElementHandleR eeh, bool option)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    bool        scaleMultilines, rotateDimView, isAnnotation;

    CellUtil::GetSharedCellDefFlags (NULL, &scaleMultilines, &rotateDimView, &isAnnotation, eeh);

    return CellUtil::SetSharedCellDefFlags (eeh, option, scaleMultilines, rotateDimView, isAnnotation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetDimRotationOption (EditElementHandleR eeh, bool option)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    bool        nondefaultScaleForDims, scaleMultilines, isAnnotation;

    CellUtil::GetSharedCellDefFlags (&nondefaultScaleForDims, &scaleMultilines, NULL, &isAnnotation, eeh);

    return CellUtil::SetSharedCellDefFlags (eeh, nondefaultScaleForDims, scaleMultilines, option, isAnnotation);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetName (EditElementHandleR eeh, WCharCP cellName)
    {
    if (NULL == cellName)
        { 
        BeAssert(false); 
        return ERROR; 
        }
    
    DgnV8ElementBlank   elm;
    eeh.GetElementCP()->CopyTo(elm);

    if (SUCCESS != CellUtil::SetCellName (elm, cellName))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::SetDescription (EditElementHandleR eeh, WCharCP descr)
    {
    if (NULL == descr)
        { BeAssert (false); return ERROR; }
    
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    if (SUCCESS != CellUtil::SetCellDescription (elm, descr))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings                   09/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::AdjustScale (EditElementHandleR eeh, double scale)
    {
    DgnElementP  elemP = eeh.GetElementP ();
    Transform   scaleTrans;

    /* Apply scale factor to cell rot/scale matrix */
    scaleTrans.ScaleMatrixColumns (Transform::FromIdentity (), scale, scale, scale);
    elemP->ToSharedCellDefR().rotScale.InitProduct(scaleTrans, *(&elemP->ToSharedCellDef().rotScale));

    /* Apply 1.0/scale to range diag */
    scaleTrans.ScaleMatrixColumns (Transform::FromIdentity (), 1.0/scale, 1.0/scale, 1.0/scale);
    scaleTrans.Multiply (&elemP->ToSharedCellDefR().rangeDiag.org, 2);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::AddChildElement (EditElementHandleR eeh, EditElementHandleR childEeh)
    {
    // Don't let elements that don't want to be part of cell get added...
    if (!childEeh.IsValid () || !childEeh.GetHandler ().IsSupportedOperation (&childEeh, SupportOperation::CellGroup))
        return ERROR;

    if (!childEeh.GetElementDescrP ())
        return ERROR;

    eeh.GetElementDescrP()->AddComponent(*childEeh.ExtractElementDescr().get());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SharedCellDefHandler::AddChildComplete (EditElementHandleR eeh)
    {
    if (!eeh.IsValid ())
        return ERROR;

    // Update component count and call ValidateElementRange...
    eeh.GetElementDescrP()->Validate ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::CreateSharedCellDefElement (EditElementHandleR eeh, WCharCP cellName, bool is3d, DgnModelR modelRef)
    {
    DgnV8ElementBlank   out;
    memset (&out, 0, sizeof (SharedCellDef));

    ElementUtil::SetRequiredFields (out, SHAREDCELL_DEF_ELM, LevelId(0), true, (ElementUtil::ElemDim) is3d);
    out.ToCell_2dR().SetSnappable(true); // s bit is set for non-point cell...

    ElementUtil::InitScanRangeForUnion (out.GetRangeR(), is3d);
    out.ToSharedCellDefR().rotScale.initIdentity ();

    UInt32      elmSize = sizeof (SharedCellDef);
    out.SetSizeWordsNoAttributes(elmSize/2);
    eeh.SetElementDescr (new MSElementDescr(out, modelRef), false);

    if (cellName)
        SetName (eeh, cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
Handler::PreActionStatus SharedCellDefHandler::_OnAdd (EditElementHandleR eh)
    {
    DgnProjectP project = eh.GetDgnProject();
    if (NULL == project)
        return  PRE_ACTION_Block;

    return project->Models().IsValidNewSCDef (eh.GetElementCP()) ? PRE_ACTION_Ok : PRE_ACTION_Block; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::_OnAdded (PersistentElementRefR el)  
    {
    el.GetDgnProject()->Models().AddSharedCellDef(el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellDefHandler::ClearEntryFromCache (PersistentElementRefR el)
    {
    el.GetDgnProject()->Models().RemoveCachedSharedCellDef(el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedCellHandlerPathEntryExtension::_GetElemHeaderOverrides (ElementHandleCR thisElm, ElemHeaderOverridesR ovr)
    {
    SharedCellHandler& instance = SharedCellHandler::GetInstance ();

    DgnModelP  modelRef = thisElm.GetDgnModelP ();

    if (NULL == modelRef)
        BeAssert (false);

    return instance.PopulateElemHeaderOverrides (ovr, (SharedCellP) thisElm.GetElementCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellHandlerPathEntryExtension::_PushDisplayEffects (ElementHandleCR thisElm, ViewContextR context)
    {
    SharedCellHandler& instance = SharedCellHandler::GetInstance ();

    DgnModelP       modelRef = thisElm.GetDgnModelP();
    ElementHandle   defnEh (instance.GetDefinition (thisElm, modelRef->GetDgnProject ()));

    if (!defnEh.IsValid ())
        return;

    instance.PushSC (thisElm, defnEh, modelRef, context, true);
    }
