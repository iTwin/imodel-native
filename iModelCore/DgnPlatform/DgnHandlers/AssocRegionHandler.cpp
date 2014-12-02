/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AssocRegionHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define     REGION_QVELEM_LOOP_INDEX    100 // NOTE: Can't conflict with indices used by gh, cmplx shape, etc...

enum RegionUpdateAction
    {
    ASSOCREGION_NO_ACTION        = 0,
    ASSOCREGION_UPDATE_REGION    = 1,
    ASSOCREGION_RESET_TMATRIX    = 2,
    ASSOCREGION_SINGLE_BOUNDARY  = 3,
    ASSOCREGION_RESOLVED_ROOTS   = 4,
    ASSOCREGION_UNRESOLVED_ROOTS = 5,
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void regionParams_setLegacyParams (RegionParams& out, AssocRegionParams const& params)
    {
    out.SetType ((RegionType) params.flags.type);
    out.SetFloodParams (params.flags.interiorShapes ? (params.flags.regionParity ? RegionLoops::Alternating : RegionLoops::Outer) : RegionLoops::Ignore, params.tolerance);
    out.SetInteriorText (params.flags.interiorText, 0.0);
    out.SetAssociative (params.flags.associationOn);
    out.SetInvisibleBoundary (params.flags.invisibleBoundary);
    out.SetFlattenBoundary (params.flags.forcePlanar, NULL);
    out.SetDirty (params.flags.dirty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void regionParams_getLegacyParams (AssocRegionParams& params, RegionParams const& in)
    {
    memset (&params, 0, sizeof (params));

    RegionLoops regionLoops         = in.GetFloodParams (&params.tolerance);

    params.flags.type               = static_cast<UInt32>(in.GetType ());
    params.flags.regionParity       = RegionLoops::Alternating == regionLoops;
    params.flags.interiorShapes     = RegionLoops::Alternating == regionLoops || RegionLoops::Outer == regionLoops;
    params.flags.interiorText       = in.GetInteriorText (NULL);
    params.flags.associationOn      = in.GetAssociative ();
    params.flags.invisibleBoundary  = in.GetInvisibleBoundary ();
    params.flags.forcePlanar        = in.GetFlattenBoundary (NULL);
    params.flags.dirty              = in.GetDirty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    // Cell origin is only meaningful for user-defined cells...use centroid for orphan cells (el->IsHole())
    return GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // Persistent elements have valid ranges...
    if (NULL == elHandle.PeekElementDescrCP ())
        return SUCCESS;

    ElemRangeCalc   newRange;

    for (ChildElemIter childIter (elHandle, ExposeChildrenReason::Count); childIter.IsValid (); childIter = childIter.ToNext ())
        {
        if (!childIter.GetElementCP()->IsGraphic()) // Must include invisible boundary!
            continue;

        newRange.Union (&childIter.GetElementCP()->GetRange(), NULL);
        }

    if (!newRange.IsValid ())
        return ERROR;

    DgnElementP  elP = elHandle.GetElementP ();

    newRange.ToScanRange (elP->GetRangeR(), Is3dElem (elP));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_AssocRegion));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    ElementHandle  templateElHandle;

    if (GetComponentForDisplayParams (templateElHandle, thisElm))
        {
        DisplayHandlerP dHandler = templateElHandle.GetDisplayHandler();

        // Avoid infinite loop, GetComponentForDisplayParams may return thisElm!
        if (dHandler && dHandler != this)
            {
            dHandler->GetElemDisplayParams (templateElHandle, params, wantMaterials);
            params.SetIsRenderable (false);

            return;
            }
        }

    T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context)
    {
    ElementHandle  templateElHandle;

    if (GetComponentForDisplayParams (templateElHandle, eh))
        context.SetCurrentLevelID (templateElHandle.GetElementCP()->GetLevel()); // Level for pattern/by-level pattern symbology...

    T_Super::_QueryHeaderProperties (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    ElementHandle  templateElHandle;

    if (GetComponentForDisplayParams (templateElHandle, eeh))
        context.SetCurrentLevelID (templateElHandle.GetElementCP()->GetLevel()); // Level for pattern/by-level pattern symbology...

    T_Super::_EditHeaderProperties (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentely   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus AssocRegionCellHeaderHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionH, bool inChain)
    {
    // For AssocRegion we have to apply transform to children, and also transform the seed points.
    ReprojectStatus status = REPROJECT_Success;

    for (ChildEditElemIter childIter (source, ExposeChildrenReason::Count); childIter.IsValid (); )
        {
        ChildEditElemIter nextChild = childIter.ToNext();

        DisplayHandlerP dispHandler;
        if (NULL != (dispHandler = childIter.GetDisplayHandler ()))
            {
            ReprojectStatus childStatus;
            if (REPROJECT_Success != (childStatus = dispHandler->GeoCoordinateReprojection (childIter, reprojectionH, inChain)))
                status = childStatus;
            dispHandler->ValidateElementRange (childIter);
            }
        childIter = nextChild;
        }

    bvector<DPoint3d> points;

    if (SUCCESS == AssocRegionCellHeaderHandler::GetFloodSeedPoints (source, &points))
        {
        if (REPROJECT_Success == reprojectionH.ReprojectPoints (&points[0], NULL, NULL, &points[0], (int) points.size ()))
            AssocRegionCellHeaderHandler::SetFloodSeedPoints (source, &points[0], points.size ());
        }

    // Note: don't bother to transform the cell origin and rotation, they get set back by the dependency handler.

    // we need to scale the associative pattern linkage.
    double      scale = reprojectionH.GetUnitRatio();
    Transform   transform;
    transform.scaleMatrixColumns (NULL, scale, scale, scale);
    PatternLinkageUtil::OnElementTransform (source, transform, false);

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setBoundaryVisible (MSElementDescrP edP, bool visible)
    {
    if (edP->Element().IsGraphic())
        edP->ElementR().SetInvisible(!visible);

    for (auto& child : edP->Components())
        setBoundaryVisible (child.get(), visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssocRegionCellHeaderHandler::_FenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    // Don't want optimized clip handled by base class...
    return _OnFenceClip (inside, outside, eh, fp, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssocRegionCellHeaderHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    /* NOTE: mdlElmdscr_fromReference relies on getting some clip result to know to drop associative patterns.
             This is rather bogus, since now it has to cull invisible clip results (which it wasn't doing).
             The pattern drop should really move into the handler, then the Associative Region could just clip
             the pattern when its boundary is not displayed! */
    StatusInt   status = SUCCESS;

    for (ChildElemIter childEh (elemHandle, ExposeChildrenReason::Count); childEh.IsValid () && SUCCESS == status; childEh = childEh.ToNext ())
        {
        EditElementHandle  tmpChildEeh;

        // NOTE: If we don't make boundary visible AcceptElement always fails...
        tmpChildEeh.Duplicate (childEh);
        setBoundaryVisible (tmpChildEeh.GetElementDescrP (), true);

        ElementAgenda   tmpInside, tmpOutside;

        if (SUCCESS == (status = tmpChildEeh.GetHandler().FenceClip (inside ? &tmpInside : NULL, outside ? &tmpOutside : NULL, tmpChildEeh, fp, options)))
            {
            if (inside)
                {
                MSElementDescrPtr regionEdP;

                if (FenceClipFlags::None != (options & FenceClipFlags::Optimized))
                    {
                    regionEdP = MSElementDescr::Allocate (*elemHandle.GetElementCP (), *elemHandle.GetDgnModelP ());
                    DependencyManagerLinkage::DeleteLinkageFromMSElement (&regionEdP->ElementR(), DEPENDENCYAPPID_AssocRegion, 0);
                    }

                EditElementHandleP curr = tmpInside.GetFirstP ();
                EditElementHandleP end  = curr + tmpInside.GetCount ();

                for (; curr < end ; curr++)
                    {
                    MSElementDescrPtr currEdP = curr->ExtractElementDescr();

                    if (!currEdP.IsValid())
                        continue;

                    if (regionEdP.IsValid())
                        {
                        setBoundaryVisible (currEdP.get(), !childEh.GetElementDescrCP()->Element().IsInvisible());
                        regionEdP->AddComponent(*currEdP.get());

                        continue;
                        }

                    inside->InsertElemDescr (currEdP.get());
                    }

                if (regionEdP.IsValid())
                    inside->InsertElemDescr (regionEdP.get());
                }

            if (outside)
                {
                MSElementDescrPtr regionEdP;

                if (FenceClipFlags::None != (options & FenceClipFlags::Optimized))
                    {
                    regionEdP = MSElementDescr::Allocate (*elemHandle.GetElementCP (), *elemHandle.GetDgnModelP ());
                    DependencyManagerLinkage::DeleteLinkageFromMSElement (&regionEdP->ElementR(), DEPENDENCYAPPID_AssocRegion, 0);
                    }

                EditElementHandleP curr = tmpOutside.GetFirstP ();
                EditElementHandleP end  = curr + tmpOutside.GetCount ();

                for (; curr < end ; curr++)
                    {
                    MSElementDescrPtr currEdP = curr->ExtractElementDescr();

                    if (!currEdP.IsValid())
                        continue;

                    if (regionEdP.IsValid())
                        {
                        setBoundaryVisible (currEdP.get(), !childEh.GetElementDescrCP()->Element().IsInvisible());
                        regionEdP->AddComponent(*currEdP.get());

                        continue;
                        }

                    outside->InsertElemDescr(currEdP.get());
                    }

                if (regionEdP.IsValid())
                    outside->InsertElemDescr(regionEdP.get());
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssocRegionCellHeaderHandler::_OnFenceStretch
(
EditElementHandleR  eeh,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count);

    if (!childElm.IsValid ())
        return ERROR;

    // NOTE: If we don't make boundary visible AcceptElement always fails...
    bool        invisibleBoundary = childElm.GetElementCP ()->IsInvisible();

    if (invisibleBoundary)
        setBoundaryVisible (childElm.GetElementDescrP (), true);

    StatusInt   status = T_Super::_OnFenceStretch (eeh, transform, fp, options);

    if (invisibleBoundary)
        {
        ChildEditElemIter childElm2 (eeh, ExposeChildrenReason::Count);

        if (childElm2.IsValid ())
            setBoundaryVisible (childElm2.GetElementDescrP (), false);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssocRegionCellHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()))
        return ERROR;

    CurveVectorPtr  curves;

    if (SUCCESS != _GetCurveVector (eh, curves))
        return ERROR;

    ElementAgenda   regionGeom;

    if (SUCCESS != GetBoundaryFromCurveVector (regionGeom, eh, *curves))
        return ERROR;

    EditElementHandleP curr = regionGeom.GetFirstP ();
    EditElementHandleP end  = curr + regionGeom.GetCount ();

    for (; curr < end ; curr++)
        {
        ElementPropertiesSetter::ApplyTemplate (*curr, eh);
        dropGeom.Insert (*curr);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssocRegionCellHeaderHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));

    if (DISPLAY_INFO_None == info)
        return;

    // Output boundary for measure even if it's not visible...
    if (DrawPurpose::Measure != context.GetDrawPurpose ())
        {
        ElementHandle  templateEh;

        GetComponentForDisplayParams (templateEh, thisElm); // Use any non-cell component for boundary template...

        bool  invisibleBoundary = (templateEh.IsValid () ? templateEh.GetElementCP ()->IsInvisible() : false);

        if (invisibleBoundary)
            info &= ~(DISPLAY_INFO_Edge | DISPLAY_INFO_Surface);
        }

    context.DrawCurveVector (thisElm, *this, (GeomRepresentations) info, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AssocRegionCellHeaderHandler::_OnTransformFinish
(
EditElementHandleR elemHandle,
TransformInfoCR trans
)
    {
    bvector<DPoint3d> points;

    if (SUCCESS == AssocRegionCellHeaderHandler::GetFloodSeedPoints (elemHandle, &points))
        {
        ( trans.GetTransform())->Multiply (&points[0],  (int) points.size ());
        AssocRegionCellHeaderHandler::SetFloodSeedPoints (elemHandle, &points[0], points.size ());
        }

    return T_Super::_OnTransformFinish (elemHandle, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssocRegionCellHeaderHandler::_ClaimElement(ElementHandleCR thisElm)
    {
    return mdlElement_attributePresent (thisElm.GetElementCP(), LINKAGEID_AssocRegion, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr fixupHoleLoopsFromDWG (ElementHandleCR thisElm, bool isCreate)
    {
    // NOTE: DWG always validated loops for filled regions. This was not originally required for DWG Hatch pattern display but it is now...
    if (!isCreate)
        {
        PatternParamsP                  params;
        ViewContext::PatternParamSource source;

        if (NULL == (params = source.GetParams (thisElm, NULL, NULL, NULL, NULL)))
            return NULL;

        // NOTE: Detect situation where we need to construct valid hole loops by subtracting holes from solids...
        if (! (PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::DwgHatchDef) && PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::HoleStyle) && PatternParamsHoleStyleType::Parity == params->GetHoleStyle()))
            return NULL;

        if (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::RawDwgLoops))
            return NULL; // UpdateAssocRegionBoundary clear this, so if it's not set it should mean the dependency callback as already fixed it...
        }

    ElementAgenda   solidAgenda, holeAgenda;

    for (ChildElemIter childIter (thisElm, ExposeChildrenReason::Count); childIter.IsValid (); childIter = childIter.ToNext ())
        {
        DgnElementCP childElmCP = childIter.GetElementCP ();

        if (!childElmCP->IsGraphic())
            continue;

        EditElementHandle childEeh (childIter, true); // Need to handle non-persistent element from DWG cache load...

        if (childElmCP->IsHole() && CELL_HEADER_ELM != childElmCP->GetLegacyType())
            holeAgenda.Insert (childEeh);
        else
            solidAgenda.Insert (childEeh);
        }

    if (solidAgenda.GetCount () < 2 && 0 == holeAgenda.GetCount ())
        return NULL;

    RegionGraphicsContext  regionContext;

    // TR#111079 - Duplicate loops within hatch entities are ignored by AutoCAD. Confirmed by Neil from ODA that this "logic" was in the AD27 display code.
    if (isCreate)
        regionContext.SetCullRedundantLoops ();

    regionContext.SetAssociativeRegionUpdate (); // Allow junk/open profile geometry as input and do the best we can...

    if (SUCCESS != regionContext.BooleanWithHoles (*thisElm.GetDgnModelP (), solidAgenda, holeAgenda, NULL, NULL, RegionType::ExclusiveOr)) // Is XOR always right?!?
        return NULL;

    CurveVectorPtr  region;

    return (SUCCESS == regionContext.GetRegion (region) ? region : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    // Single "solid" region...can just return child curve bvector...
    if (!childEh.ToNext ().IsValid ())
        {
        curves = ICurvePathQuery::ElementToCurveVector (childEh);

        return (curves.IsValid () ? SUCCESS : ERROR);
        }

    curves = fixupHoleLoopsFromDWG (eh, false);

    if (curves.IsValid ())
        return SUCCESS;

    CurveVectorPtr regionLoops = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);

    // step through all of the child loops...
    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        CurveVectorPtr childCurveVector = ICurvePathQuery::ElementToCurveVector (childEh);

        if (childCurveVector.IsNull ())
            continue;

        regionLoops->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childCurveVector.get ()));
        }

    if (regionLoops->size () > 1)
        curves = regionLoops;
    
    return (curves.IsValid () ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::GetBoundaryFromCurveVector (ElementAgendaR boundary, ElementHandleCR eh, CurveVectorCR curves)
    {
    if (curves.IsUnionRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            EditElementHandle   tmpEeh;

            if (SUCCESS != DraftingElementSchema::ToElement (tmpEeh, *curve->GetChildCurveVectorCP (), NULL, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
                return ERROR;

            boundary.Insert (tmpEeh);
            }
        }
    else
        {
        EditElementHandle   tmpEeh;

        if (SUCCESS != DraftingElementSchema::ToElement (tmpEeh, curves, NULL, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
            return ERROR;

        boundary.Insert (tmpEeh);
        }

    return (0 != boundary.GetCount () ? SUCCESS : ERROR);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR curves)
    {
    if (!curves.IsAnyRegionType ())
        return ERROR;

    if (SUCCESS == DependencyManagerLinkage::GetLinkage (NULL, eeh, DEPENDENCYAPPID_AssocRegion, false))
        return ERROR; // Only supported for un-associated regions...

    ElementAgenda   boundary;

    if (SUCCESS != GetBoundaryFromCurveVector (boundary, eeh, curves))
        return ERROR;

    return UpdateAssocRegionBoundary (eeh, boundary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AssocRegionCellHeaderHandler::_GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const
    {
    ChildElemIter   firstChild (eh, ExposeChildrenReason::Count);

    IAreaFillPropertiesQuery* childAreaQueryObj = dynamic_cast <IAreaFillPropertiesQuery*> (&firstChild.GetHandler());

    if (!childAreaQueryObj)
        return false;

    // NOTE: Assoc region can have multiple "regions"...area fill should be uniform...so just return first!
    return childAreaQueryObj->GetSolidFill (firstChild, fillColorP, alwaysFilledP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssocRegionCellHeaderHandler::_GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const
    {
    ChildElemIter   firstChild (eh, ExposeChildrenReason::Count);

    IAreaFillPropertiesQuery* childAreaQueryObj = dynamic_cast <IAreaFillPropertiesQuery*> (&firstChild.GetHandler());

    if (!childAreaQueryObj)
        return false;

    // NOTE: Assoc region can have multiple "regions"...area fill should be uniform...so just return first!
    return childAreaQueryObj->GetGradientFill (firstChild, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AssocRegionCellHeaderHandler::_RemoveAreaFill (EditElementHandleR eeh)
    {
    bool        changed = false;

    // NOTE: Fill added to all top-level children...
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        {
        IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&childElm.GetHandler());

        if (!childAreaEditObj)
            continue;

        if (childAreaEditObj->RemoveAreaFill (childElm))
            changed = true;
        }

    // NOTE: Visible boundary required for fill...can be invisible if no fill and has pattern...
    if (changed)
        setBoundaryVisible (eeh.GetElementDescrP()->Components().begin()->get(), !mdlElement_attributePresent (eeh.GetElementCP (), PATTERN_ID, NULL));

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AssocRegionCellHeaderHandler::_AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP)
    {
    bool        changed = false;

    // NOTE: Fill added to all top-level children...
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        {
        IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&childElm.GetHandler());

        if (!childAreaEditObj)
            continue;

        if (childAreaEditObj->AddSolidFill (childElm, fillColorP, alwaysFilledP))
            changed = true;
        }

    // NOTE: Visible boundary required for fill...
    if (changed)
        setBoundaryVisible (eeh.GetElementDescrP()->Components().begin()->get(), true);

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssocRegionCellHeaderHandler::_AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb)
    {
    bool        changed = false;

    // NOTE: Fill added to all top-level children...
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        {
        IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&childElm.GetHandler());

        if (!childAreaEditObj)
            continue;

        if (childAreaEditObj->AddGradientFill (childElm, symb))
            changed = true;
        }

    // NOTE: Visible boundary required for fill...
    if (changed)
        setBoundaryVisible(eeh.GetElementDescrP()->Components().begin()->get(), true);

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_GetParams (ElementHandleCR eh, RegionParams& params) const
    {
    return AssocRegionCellHeaderHandler::GetRegionParams (eh, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_GetSeedPoints (ElementHandleCR eh, bvector<DPoint3d>* points) const
    {
    return AssocRegionCellHeaderHandler::GetFloodSeedPoints (eh, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_GetRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots) const
    {
    return AssocRegionCellHeaderHandler::GetRegionRoots (eh, boundaryRoots);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::_GetLoopData (ElementHandleCR eh, bvector<LoopData>* loopData) const
    {
    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (CELL_HEADER_ELM == childEh.GetLegacyType())
            {
            // Process loops of grouped holes individually...
            _GetLoopData (childEh, loopData);
            }
        else
            {
            LoopData    loop;

            if (SUCCESS != AssocRegionCellHeaderHandler::GetLoopOedCode (childEh, loop.m_loopCode))
                loop.m_loopCode = 0;

            AssocRegionCellHeaderHandler::GetLoopRoots (childEh, &loop.m_loopRoots);

            loopData->push_back (loop);
            }
        }

    return (loopData->size () > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::GetRegionParams (ElementHandleCR eh, RegionParams& params)
    {
    bool            isAssociative = false;
    BentleyStatus   status = ERROR;

    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        if (!li->user)
            continue;

        if (LINKAGEID_AssocRegion == li->primaryID)
            {
            DataInternalizer    reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);
            AssocRegionParams   rawParams;

            reader.get ((UInt32*) &rawParams.flags);
            reader.get (&rawParams.tolerance);

            regionParams_setLegacyParams (params, rawParams);
            status = SUCCESS;

            continue;
            }
        else if (LINKAGEID_Dependency == li->primaryID)
            {
            DependencyLinkageCP depLink = (DependencyLinkageCP) li.GetData ();

            if (DEPENDENCYAPPID_AssocRegion == depLink->appID && 0 == depLink->appValue)
                isAssociative = true;

            continue;
            }

        UInt16          linkageKey;
        UInt32          numEntries;
        double const*   doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData)
            continue;

        switch (linkageKey)
            {
            case DOUBLEARRAY_LINKAGE_KEY_FlattenTransform:
                {
                if (numEntries != (sizeof (RotMatrix) / sizeof (double)))
                    break;

                params.SetFlattenBoundary (params.GetFlattenBoundary (NULL), (RotMatrixCP) doubleData);
                break;
                }

            case DOUBLEARRAY_LINKAGE_KEY_RegionTextMarginFactor:
                {
                if (numEntries != 1)
                    break;

                params.SetInteriorText (params.GetInteriorText (NULL), *doubleData);
                break;
                }
            }
        }

    // Override saved value of association flag based on existance of dependency linkage...
    if (SUCCESS == status)
        params.SetAssociative (isAssociative);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::SetRegionParams (EditElementHandleR eeh, RegionParams const& params)
    {
    double              textMargin;
    RotMatrix           rMatrix;
    bool                addParams = true;
    bool                addTextMargin = params.GetInteriorText (&textMargin);
    bool                addFlatten = params.GetFlattenBoundary (&rMatrix);
    AssocRegionParams   rawParams;

    regionParams_getLegacyParams (rawParams, params);

    eeh.GetElementDescrP (); // Make sure we have an element to update...

    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        if (!li->user)
            continue;

        if (LINKAGEID_AssocRegion == li->primaryID)
            {
            DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

            UInt32*     flagsP = (UInt32*) reader.getPos ();
            UInt32      flags;

            *flagsP = *((UInt32*) &rawParams.flags);
            reader.get (&flags);

            double*     gapToleranceP = (double*) reader.getPos ();

            *gapToleranceP = rawParams.tolerance;
            addParams = false;

            continue;
            }

        UInt16      linkageKey;
        UInt32      numEntries;
        double*     doubleData = (double*) ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData)
            continue;

        switch (linkageKey)
            {
            case DOUBLEARRAY_LINKAGE_KEY_FlattenTransform:
                {
                if (!addFlatten || numEntries != (sizeof (rMatrix) / sizeof (double)) || rMatrix.isIdentity ())
                    {
                    eeh.RemoveElementLinkage (li);
                    break;
                    }

                memcpy (doubleData, &rMatrix.form3d[0][0], numEntries * sizeof (double));
                addFlatten = false;
                break;
                }

            case DOUBLEARRAY_LINKAGE_KEY_RegionTextMarginFactor:
                {
                if (!addTextMargin || 1 != numEntries || 0.0 == textMargin)
                    {
                    eeh.RemoveElementLinkage (li);
                    break;
                    }

                *doubleData = textMargin;
                addTextMargin = false;
                break;
                }
            }
        }

    if (addParams)
        {
        DataExternalizer   writer;

        writer.put (*((UInt32*) &rawParams.flags));
        writer.put (rawParams.tolerance);

        if (SUCCESS != ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_AssocRegion, writer))
            return ERROR;
        }

    if (addFlatten && !rMatrix.isIdentity ())
        ElementLinkageUtil::AppendDoubleArrayData (eeh, DOUBLEARRAY_LINKAGE_KEY_FlattenTransform, (sizeof (rMatrix) / sizeof (double)), &rMatrix.form3d[0][0]);

    if (addTextMargin && 0.0 != textMargin)
        ElementLinkageUtil::AppendDoubleArrayData (eeh, DOUBLEARRAY_LINKAGE_KEY_RegionTextMarginFactor, 1, &textMargin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::GetFloodSeedPoints (ElementHandleCR eh, bvector<DPoint3d>* points)
    {
    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_SeedPoints != li->primaryID)
            continue;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        UInt32      numSeed;

        reader.get (&numSeed);

        if (0 == numSeed || MAX_FloodSeedPoints < numSeed)
            return ERROR;

        for (UInt32 i=0; i < numSeed; i++)
            {
            DPoint3d    tmpPt;

            reader.get (&tmpPt.x);
            reader.get (&tmpPt.y);
            reader.get (&tmpPt.z);

            points->push_back (tmpPt);
            }

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::SetFloodSeedPoints (EditElementHandleR eeh, DPoint3dCP points, size_t numSeedIn)
    {
    UInt32      numSeed = (UInt32) numSeedIn;

    if (numSeed > MAX_FloodSeedPoints)
        return ERROR;

    eeh.GetElementDescrP (); // Make sure we have an element to update...

    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_SeedPoints != li->primaryID)
            continue;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        UInt32      numCurrentSeed;

        reader.get (&numCurrentSeed);

        if (numCurrentSeed != numSeed || !numSeed)
            {
            eeh.RemoveElementLinkage (li);

            break;
            }

        DPoint3dP   currentSeed = (DPoint3dP) reader.getPos ();

        for (UInt32 i=0; i < numSeed; i++)
            currentSeed[i] = points[i];

        return SUCCESS;
        }

    if (!numSeed)
        return SUCCESS;

    DataExternalizer   writer;

    writer.put (numSeed);

    for (UInt32 i=0; i < numSeed; i++)
        {
        writer.put (points[i].x);
        writer.put (points[i].y);
        writer.put (points[i].z);
        }

    return ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_SeedPoints, writer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::GetBoundaryRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots, bool isRegionLoop)
    {
    DependencyLinkageAccessor depP;

    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&depP, eh, DEPENDENCYAPPID_AssocRegion, isRegionLoop))
        return ERROR;

    for (UInt16 iRoot=0; iRoot < depP->nRoots; iRoot++)
        {
        DependencyRoot roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];

        if (0 == DependencyManagerLinkage::GetRoots (roots, eh.GetDgnModelP (), *depP, iRoot))
            continue;

        boundaryRoots->push_back (roots[0]);
        }

    return (boundaryRoots->size () > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::GetRegionRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots)
    {
    return AssocRegionCellHeaderHandler::GetBoundaryRoots (eh, boundaryRoots, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::GetLoopRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots)
    {
    return AssocRegionCellHeaderHandler::GetBoundaryRoots (eh, boundaryRoots, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssocRegionCellHeaderHandler::SetBoundaryRoots (EditElementHandleR eeh, DependencyRoot const* boundaryRoots, size_t numBoundaryRootsIn, bool isRegionLoop)
    {
    if (0 == numBoundaryRootsIn)
        {
        DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_AssocRegion, isRegionLoop);

        return SUCCESS;
        }

    // bool        needFarElemId = false;

    //for (size_t i=0; i < numBoundaryRootsIn; i++)          no such thing as "far" dependencies in graphite
    //    {                                                  no such thing as "far" dependencies in graphite
    //    if (0 == boundaryRoots[i].refattid)                no such thing as "far" dependencies in graphite
    //        continue;                                      no such thing as "far" dependencies in graphite
    //                                                       no such thing as "far" dependencies in graphite
    //    needFarElemId = true;                              no such thing as "far" dependencies in graphite
    //    break;                                             no such thing as "far" dependencies in graphite
    //    }                                                  no such thing as "far" dependencies in graphite

    DependencyLinkage*  depLinkageP = NULL;

    //if (needFarElemId)
    //    {
    //    if (isRegionLoop) // Roots on component loops only needed for DWG which doesn't support far refs...
    //        return ERROR;
    //
    //    if (numBoundaryRootsIn > DEPENDENCY_MAX_FARELEMIDS)
    //        return ERROR;
    //
    //    if (NULL == (depLinkageP = (DependencyLinkage *) _alloca (offsetof (DependencyLinkage, root) + (numBoundaryRootsIn * sizeof (DependencyRootFarElementID)))))
    //        return ERROR;
    //
    //    DependencyManagerLinkage::InitLinkage (*depLinkageP, DEPENDENCYAPPID_AssocRegion, DEPENDENCY_DATA_TYPE_FAR_ELEM_ID, DEPENDENCY_ON_COPY_RemapRootsWithinSelection);
    //
    //    depLinkageP->appValue = isRegionLoop;
    //    depLinkageP->nRoots   = 0;
    //
    //    for (size_t i=0; i < numBoundaryRootsIn; i++)
    //        {
    //        bool    uniqueRoot = true;
    //
    //        for (size_t j=0; j < depLinkageP->nRoots; j++)
    //            {
    //            if (depLinkageP->root.far_elemid[j].elemid   == boundaryRoots[i].elemid &&
    //                depLinkageP->root.far_elemid[j].refattid == boundaryRoots[i].refattid)
    //                {
    //                uniqueRoot = false;
    //                break;
    //                }
    //            }
    //
    //        if (!uniqueRoot)
    //            continue;
    //
    //        depLinkageP->root.far_elemid[depLinkageP->nRoots].elemid   = boundaryRoots[i].elemid;
    //        depLinkageP->root.far_elemid[depLinkageP->nRoots].refattid = boundaryRoots[i].refattid;
    //        depLinkageP->nRoots++;
    //        }
    //    }
    //else
        {
        if (numBoundaryRootsIn > DEPENDENCY_MAX_ELEMIDS)
            return ERROR;

        if (NULL == (depLinkageP = (DependencyLinkage *) _alloca (offsetof (DependencyLinkage, root) + (numBoundaryRootsIn * sizeof (ElementId)))))
            return ERROR;

        DependencyManagerLinkage::InitLinkage (*depLinkageP, DEPENDENCYAPPID_AssocRegion, DEPENDENCY_DATA_TYPE_ELEM_ID, DEPENDENCY_ON_COPY_RemapRootsWithinSelection);

        depLinkageP->appValue = isRegionLoop;
        depLinkageP->nRoots   = 0;

        for (size_t i=0; i < numBoundaryRootsIn; i++)
            {
            bool    uniqueRoot = true;

            for (size_t j=0; j < depLinkageP->nRoots; j++)
                {
                if (depLinkageP->root.elemid[j] == boundaryRoots[i].elemid)
                    {
                    uniqueRoot = false;
                    break;
                    }
                }

            if (!uniqueRoot)
                continue;

            depLinkageP->root.elemid[depLinkageP->nRoots++] = boundaryRoots[i].elemid;
            }
        }

    DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_AssocRegion, isRegionLoop);

    if (SUCCESS != DependencyManagerLinkage::AppendLinkage (eeh, *depLinkageP, 0))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::SetRegionRoots (EditElementHandleR eeh, DependencyRoot const* boundaryRoots, size_t numBoundaryRootsIn)
    {
    return AssocRegionCellHeaderHandler::SetBoundaryRoots (eeh, boundaryRoots, numBoundaryRootsIn, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::SetLoopRoots (EditElementHandleR eeh, DependencyRoot const* boundaryRoots, size_t numBoundaryRootsIn)
    {
    // Roots of loop components...not region!
    if (mdlElement_attributePresent (eeh.GetElementCP(), LINKAGEID_AssocRegion, NULL))
        return ERROR;

    return AssocRegionCellHeaderHandler::SetBoundaryRoots (eeh, boundaryRoots, numBoundaryRootsIn, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::GetLoopOedCode (ElementHandleCR eh, int& loopCode)
    {
    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_LoopOEDCode != li->primaryID)
            continue;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        reader.get (&loopCode);

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::SetLoopOedCode (EditElementHandleR eeh, int loopCode)
    {
    // Code of loop components...not region!
    if (mdlElement_attributePresent (eeh.GetElementCP(), LINKAGEID_AssocRegion, NULL))
        return ERROR;

    eeh.GetElementDescrP (); // Make sure we have an element to update...

    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_LoopOEDCode != li->primaryID)
            continue;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        int*    currLoopCode = (int*) reader.getPos ();

        *currLoopCode = loopCode;

        return SUCCESS;
        }

    DataExternalizer   writer;

    writer.put (loopCode);

    return ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_LoopOEDCode, writer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssocRegionCellHeaderHandler::IsValidRegionBoundaryType (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    // Computed boundary is limited to specific element types...actual roots may be text, mlines, shared cells, etc.
    switch (eh.GetLegacyType())
        {
        case LINE_ELM:
        case LINE_STRING_ELM:
        case SHAPE_ELM:
        case CURVE_ELM:
        case CMPLX_STRING_ELM:
        case CMPLX_SHAPE_ELM:
        case ELLIPSE_ELM:
        case ARC_ELM:
        case BSPLINE_CURVE_ELM:
            return true;

        case CELL_HEADER_ELM:
            return GroupedHoleHandler::IsGroupedHole (eh);

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::CreateAssocRegionElement (EditElementHandleR eeh, ElementAgendaR boundary, DependencyRoot const* boundaryRoots, size_t numBoundaryRoots, DPoint3dCP seedPoints, size_t numSeedPoints, RegionParams const& params, WCharCP cellName)
    {
    if (boundary.IsEmpty ())
        return ERROR;

    NormalCellHeaderHandler::CreateOrphanCellElement (eeh, cellName, boundary.GetFirstP ()->GetElementCP ()->Is3d(), *boundary.GetFirstP ()->GetDgnModelP ());

    if (SUCCESS != AssocRegionCellHeaderHandler::SetRegionParams (eeh, params))
        return ERROR;

    if (RegionType::Flood == params.GetType ())
        {
        // NOTE: Seed points are required for flood type region...
        if (!numSeedPoints || SUCCESS != AssocRegionCellHeaderHandler::SetFloodSeedPoints (eeh, seedPoints, numSeedPoints))
            return ERROR;
        }

    if (SUCCESS != SetRegionRoots (eeh, boundaryRoots, numBoundaryRoots))
        return ERROR;

    EditElementHandleP curr = boundary.GetFirstP ();
    EditElementHandleP end  = curr + boundary.GetCount ();

    for (; curr < end ; curr++)
        {
        if (!curr->IsValid () || !IsValidRegionBoundaryType (*curr))
            continue;
            
        NormalCellHeaderHandler::AddChildElement (eeh, *curr);
        }

    ChildElemIter   childEh (eeh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    // Force boundary visible when computing range/range diag as _Draw won't output anything for range context since it doesn't want patterns...
    setBoundaryVisible (eeh.GetElementDescrP()->Components().begin()->get(), true);

    if (SUCCESS != NormalCellHeaderHandler::AddChildComplete (eeh))
        return ERROR;

    // Set correct boundary visibility now that we no longer need to visit components...
    setBoundaryVisible (eeh.GetElementDescrP()->Components().begin()->get(), !params.GetInvisibleBoundary ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::UpdateAssocRegionBoundary (EditElementHandleR eeh, ElementAgendaR boundary)
    {
    if (boundary.IsEmpty ())
        return ERROR;

    RegionParams    params;

    if (SUCCESS != AssocRegionCellHeaderHandler::GetRegionParams (eeh, params))
        return ERROR;

    EditElementHandle  newEeh (*eeh.GetElementCP (), *eeh.GetDgnModelP ()); // Copy old header...
    ElementHandle      templateEh;

    GetComponentForDisplayParams (templateEh, eeh); // Use any non-cell component for boundary template...

    bool    invisibleBoundary = (templateEh.IsValid () ? templateEh.GetElementCP ()->IsInvisible() : params.GetInvisibleBoundary ());

    EditElementHandleP curr = boundary.GetFirstP ();
    EditElementHandleP end  = curr + boundary.GetCount ();

    for (; curr < end ; curr++)
        {
        if (!curr->IsValid () || !IsValidRegionBoundaryType (*curr))
            continue;
            
        if (templateEh.IsValid ())
            ElementPropertiesSetter::ApplyTemplate (*curr, templateEh);

        NormalCellHeaderHandler::AddChildElement (newEeh, *curr);
        }

    ChildElemIter   childEh (newEeh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    // Force boundary visible when computing range/range diag as _Draw won't output anything for range context since it doesn't want patterns...
    setBoundaryVisible (newEeh.GetElementDescrP()->Components().begin()->get(), true);

    if (SUCCESS != NormalCellHeaderHandler::AddChildComplete (newEeh))
        return ERROR;

    // Set correct boundary visibility now that we no longer need to visit components...
    setBoundaryVisible (newEeh.GetElementDescrP()->Components().begin()->get(), !invisibleBoundary);

    void*           linkage;
    HatchLinkage    hatchLink;

    // Once we've modified a DWG hatch the loops are no longer in "raw" (parity with holes) format.
    if (NULL != (linkage = linkage_extractLinkageByIndex (&hatchLink, newEeh.GetElementCP (), PATTERN_ID, NULL, 0)) && 0 != (hatchLink.modifiers & (UInt32)PatternParamsModifierFlags::RawDwgLoops))
        {
        hatchLink.modifiers &= (Int32) ~PatternParamsModifierFlags::RawDwgLoops;
        elemUtil_replaceLinkage (newEeh.GetElementP (), (UShort*) linkage, (UShort*) &hatchLink, false);
        }

    eeh.SetElementDescr (newEeh.ExtractElementDescr().get(), false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::ResolveAssociations (EditElementHandleR eeh)
    {
    if (0 == DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_AssocRegion, 0))
        return ERROR;

    for (ChildEditElemIter childEeh (eeh, ExposeChildrenReason::Count); childEeh.IsValid (); childEeh = childEeh.ToNext ())
        DependencyManagerLinkage::DeleteLinkage (childEeh, DEPENDENCYAPPID_AssocRegion, 1);

    return SUCCESS;
    }

///*=================================================================================**//**
//* @bsiclass                                                     BrienBastings   09/09
//+===============+===============+===============+===============+===============+======*/
//struct AssocRegionAppIdRootsCallback : DependencyManagerLinkage::IRootsChangedCallback
//{
//virtual UInt32 AddRef() const override {return 1;}
//virtual UInt32 Release() const override {return 1;}
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Brien.Bastings  09/09
//+---------------+---------------+---------------+---------------+---------------+------*/
//void DoForcedRedraw (ElementHandleCR eh, DgnDrawMode drawMode)
//    {
//    if (!eh.IsValid () || eh.GetElementCP ()->IsInvisible())
//        return;
//
//    ViewHandler::GetDefaultHandler().IncludeNonTransactionableElementInHealRegion ((ElementHandleR) eh, drawMode);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Brien.Bastings  09/09
//+---------------+---------------+---------------+---------------+---------------+------*/
//void SetFailedAssociation (bool* failedAssocStatusChanged, ElementHandleCR eh, bool failed)
//    {
//    if (!eh.IsValid ())
//        return;
//
//    ElementRefP  elmRef = eh.GetElementRef ();
//    
//    if (!elmRef || elmRef->IsFailedAssoc () == failed)
//        return;
//
//    elmRef->SetFailedAssoc (failed);
//
//    if (failedAssocStatusChanged)
//        *failedAssocStatusChanged = true;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Brien.Bastings  09/09
//+---------------+---------------+---------------+---------------+---------------+------*/
//void DisplayFailedAssociationChange (ElementHandleCR depEh, bool isFailed)
//    {
//    // Display normally handled by rewrite/undo...special case when just changing failed assoc status...
//    DoForcedRedraw (depEh, DRAW_MODE_Erase);
//    SetFailedAssociation (NULL, depEh, isFailed);
//    DoForcedRedraw (depEh, DRAW_MODE_Normal);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Brien.Bastings  09/09
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool            HasRootChanges
//(
//bool*                    onlySelf,      /* <=  dependent changed and all roots resolved or unchanged */
//DependencyLinkage const& depData,       /*  => copy of dependent element's linkage */
//UChar*                   rootStatus,    /*  => change status of each root in linkage: DEPENDENCY_STATUS_XXX */
//UChar                    selfStatus,    /*  => change status of the dimension itself: DEPENDENCY_STATUS_XXX */
//bool                     realChangeOnly /*  => don't consider resolved/unresolved as a change */
//)
//    {
//    bool        allResolvedOrUnchanged = true, hasChanges = false;
//
//    for (int iRoot=0; iRoot < depData.nRoots; iRoot++)
//        {
//        if (rootStatus[iRoot] != DEPENDENCY_STATUS_UNCHANGED)
//            {
//            if (realChangeOnly)
//                {
//                if (rootStatus[iRoot] == DEPENDENCY_STATUS_CHANGED ||
//                    rootStatus[iRoot] == DEPENDENCY_STATUS_DELETED)
//                    hasChanges = true;
//                }
//            else
//                {
//                hasChanges = true;
//                }
//
//            // If not selfStatus we don't need to do further checks
//            if (!selfStatus || !onlySelf)
//                break;
//
//            if (rootStatus[iRoot] != DEPENDENCY_STATUS_RESOLVED &&
//                rootStatus[iRoot] != DEPENDENCY_STATUS_UNRESOLVED)
//                allResolvedOrUnchanged = false;
//            }
//        }
//
//    if (onlySelf)
//        *onlySelf = (selfStatus && allResolvedOrUnchanged);
//
//    return hasChanges;
//    }
//
///*----------------------------------------------------------------------------------*//**
//* @bsimethod                                                    RalphPinheiro   10/00
//+---------------+---------------+---------------+---------------+---------------+------*/
//BentleyStatus   CheckSelfDependency
//(
//RegionUpdateAction&         updateStatus, // <= update origin and seed pts or update region
//ElementHandleCR             depEh,
//DependencyLinkage const&    depData,
//UInt8*                      rootStatus,
//UInt8                       selfStatus,
//RegionParams const&         params
//)
//    {
//    int         deleted = 0, changed = 0, unchanged = 0, resolved = 0, unresolved = 0;
//    double      rootModifiedTime = 0.0;
//
//    for (UInt16 iRoot=0; iRoot < depData.nRoots; iRoot++)
//        {
//        switch (rootStatus[iRoot])
//            {
//            case DEPENDENCY_STATUS_CHANGED:
//                {
//                DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
//                int             nRoots = DependencyManagerLinkage::GetRoots (roots, depEh.GetDgnModelP (), depData, iRoot);
//
//                // Ignore self-dependencies...sufficient to just test roots[0]
//                if (nRoots > 0 && depEh.GetElementRef () != roots[0].ref)
//                    changed++;
//                break;
//                }
//
//            case DEPENDENCY_STATUS_RESOLVED:
//                {
//                DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
//                int             nRoots = DependencyManagerLinkage::GetRoots (roots, depEh.GetDgnModelP (), depData, iRoot);
//
//                // Don't need to check roots[1]...sufficient to just test roots[0]
//                if (nRoots > 0 && roots[0].ref)
//                    {
//                    ElementHandle  rootEh (roots[0].ref);
//
//                    if (rootEh.GetElementCP ()->ehdr.lastModified > rootModifiedTime)
//                        rootModifiedTime = rootEh.GetElementCP ()->ehdr.lastModified;
//                    }
//
//                resolved++;
//                break;
//                }
//
//            case DEPENDENCY_STATUS_UNRESOLVED:
//                {
//                bool    found = false;
//
//                // HACK ALERT: hatch in nested sc w/tag as root won't be found in dictionary model.
//                if (0 == unresolved && depEh.GetDgnModelP ()->IsDictionaryModel ())
//                    {
//                    DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
//                    int             nRoots = DependencyManagerLinkage::GetRoots (roots, depEh.GetDgnModelP (), depData, iRoot);
//
//                    // See if we can find roots[0] in another cache...
//                    if (nRoots > 0 && roots[0].elemid)
//                        {
//                        ElementRefP  elemRef = NULL;
//
//                        // Don't bother with last modified time for these...
//                        if (NULL != (elemRef = depEh.GetDgnProject ()->FindByElementId (roots[0].elemid, false)))
//                            found = true;
//                        }
//                    }
//
//                if (found)
//                    resolved++;
//                else
//                    unresolved++;
//                break;
//                }
//
//            case DEPENDENCY_STATUS_DELETED:
//                deleted++;
//                break;
//
//            case DEPENDENCY_STATUS_UNCHANGED:
//                unchanged++;
//                break;
//            }
//        }
//
//    // if all roots are unresolved at load/reload time, don't try to create the regions
//    if (depData.nRoots == (unresolved + unchanged) && unresolved > 0)
//        {
//        updateStatus = ASSOCREGION_UNRESOLVED_ROOTS;
//
//        return SUCCESS;
//        }
//
//    // this is a load/reload of a cache...
//    if (depData.nRoots == (resolved + unchanged) && resolved > 0)
//        {
//        // do quick test to see if roots have not changed
//        if (rootModifiedTime <= depEh.GetElementCP ()->ehdr.lastModified)
//            {
//            updateStatus = ASSOCREGION_RESOLVED_ROOTS;
//
//            return SUCCESS;
//            }
//        }
//
//    // Special case of flood w/single root, update seed point to remain inside boundary...
//    if (1 == changed && 1 == depData.nRoots)
//        {
//        updateStatus = ASSOCREGION_SINGLE_BOUNDARY;
//
//        return SUCCESS;
//        }
//
//    // if boundary elem/s deleted OR if one or more or all boundary elems changed and region didn't change regenerate.
//    updateStatus = ASSOCREGION_UPDATE_REGION;
//
//    if (deleted > 0 || (DEPENDENCY_STATUS_UNCHANGED == selfStatus && changed > 0 && changed <= depData.nRoots))
//        return SUCCESS;
//
//    // if region changed and boundary elems changed as well, this is a block copy/move reset region origin and regenerate.
//    if (DEPENDENCY_STATUS_CHANGED == selfStatus && changed > 0)
//        {
//        updateStatus = ASSOCREGION_RESET_TMATRIX;
//
//        return SUCCESS;
//        }
//
//    // check whether region moved independently or not
//    if (DEPENDENCY_STATUS_CHANGED == selfStatus && changed == 0)
//        {
//        DPoint3d    origin;
//
//        // simple region rewrite - don't regenerate the region
//        CellUtil::ExtractOrigin (origin, depEh);
//
//        if (fabs (origin.x) <= 1.0e-6 && fabs (origin.y) <= 1.0e-6 && fabs (origin.z) <= 1.0e-6)
//            {
//            if (params.GetDirty ())
//                updateStatus = ASSOCREGION_UPDATE_REGION;
//            else
//                updateStatus = ASSOCREGION_NO_ACTION;
//
//            return SUCCESS;
//            }
//
//        return ERROR; // independent region move - disallow the move
//        }
//
//    return SUCCESS;
//    }
//
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// StatusInt       WriteChangedRegion (EditElementHandleR eeh, ElementHandleCR depEh, bool allowWrite)
//     {
//     // If we are part of a complex...need to rewrite header
//     if (depEh.GetElementCP ()->IsComplexComponent())
//         {
//         ElementRefP  myParent, elemRef = depEh.GetElementRef ();
// 
//         // Find the file position of the outermost complex header and read that
//         while (NULL != (myParent = elemRef->GetParentElementRef ()))
//             elemRef = myParent;
// 
//         EditElementHandle  hdrEeh (elemRef, depEh.GetDgnModelP ());
//         MSElementDescrP tmpEdP;
// 
//         // Find ourselves and replace with changed descriptor
//         if (NULL != (tmpEdP = DependencyManagerLinkage::FindElementIDInDescriptor (hdrEeh.GetElementDescrP (), depEh.GetElementCP ()->ehdr.uniqueId)))
//             {
//             tmpEdP = tmpEdP->ReplaceDescr (*eeh.ExtractElementDescr ());
//             eeh.SetElementDescr (hdrEeh.ExtractElementDescr (), true, false);
//             }
//         }
// 
//     DgnModelP   dgnModel = depEh.GetDgnModelP ();
//     bool        readOnlyCache = dgnModel->IsReadOnly ();
// 
//     // Need to update association in reference file cache
//     if (readOnlyCache)
//         dgnModel->SetReadOnly (false);
// 
//     StatusInt   replaceResult;
//     ElementRefP  oldElemRef = depEh.GetElementRef ();
// 
//     // Force validation logic in rewrite to make sure range is updated
//     eeh.GetElementDescrP ()->h.isValid = false;
// 
//     if (!allowWrite) // Must do non-undoable write! It is illegal to use the TxnMgr during an undo callback!
//         replaceResult = dgnModel->ReplaceElementDescr (eeh.GetElementDescrP (), dynamic_cast <PersistentElementRef*> (oldElemRef), false, false);
//     else
//         replaceResult = eeh.ReplaceInModel (oldElemRef);
// 
//     // If we are unable to do rewrite set failed symbology
//     if (SUCCESS != replaceResult)
//         SetFailedAssociation (NULL, depEh, true);
// 
//     if (readOnlyCache)
//         dgnModel->SetReadOnly (true);
// 
//     // If complex component find/return updated region not outermost parent...
//     if (SUCCESS == replaceResult && depEh.GetElementCP ()->IsComplexComponent())
//         eeh.SetElementRef (eeh.GetDgnModelP ()->FindByElementId (depEh.GetElementRef ()->GetElementId ()), eeh.GetDgnModelP ());
// 
//     return replaceResult;
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// StatusInt       WriteAsFailedRegion (ElementHandleCR depEh, bool allowWrite)
//     {
//     // Put this region in permanantly failed state
//     if (!allowWrite)
//         {
//         DisplayFailedAssociationChange (depEh, true);
// 
//         return SUCCESS;
//         }
// 
//     EditElementHandle      eeh (depEh, true);
//     DependencyLinkage   failedDepLinkage;
// 
//     DependencyManagerLinkage::InitLinkage (failedDepLinkage, DEPENDENCYAPPID_AssocRegion, DEPENDENCY_DATA_TYPE_ELEM_ID, DEPENDENCY_ON_COPY_RemapRootsWithinSelection);
// 
//     failedDepLinkage.appValue       = 0;
//     failedDepLinkage.nRoots         = 1;
//     failedDepLinkage.root.elemid[0] = INVALID_ELEMENTID;
// 
//     DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_AssocRegion, 0);
//     DependencyManagerLinkage::AppendLinkage (eeh, failedDepLinkage, 0);
// 
//     if (SUCCESS == WriteChangedRegion (eeh, depEh, allowWrite))
//         SetFailedAssociation (NULL, eeh, true);
// 
//     return SUCCESS;
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// bool            ResetCellTMatrix (EditElementHandleR eeh)
//     {
//     DPoint3d    origin;
//     RotMatrix   rMatrix;
// 
//     CellUtil::ExtractOrigin (origin, eeh);
//     CellUtil::ExtractRotation (rMatrix, eeh);
// 
//     if (0.0 == origin.magnitude () && rMatrix.isIdentity ())
//         return false;
// 
//     origin.zero ();
//     rMatrix.initIdentity ();
// 
//     // reset the region cell origin & rotation matrix
//     if (eeh.GetElementCP ()->Is3d())
//         {
//         eeh.GetElementP ()->ToCell_3d().origin = origin;
//         memcpy (eeh.GetElementP ()->ToCell_3d().transform, &rMatrix, sizeof (eeh.GetElementP ()->ToCell_3d().transform));
//         }
//     else
//         {
//         eeh.GetElementP ()->ToCell_2d().origin.init (&origin);
//         rMatrix.GetRowValuesXY(&eeh.GetElementP ()->ToCell_2d().transform[0][0]);
//         }
// 
//     NormalCellHeaderHandler::SetCellRange (eeh);
// 
//     return true;
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// static bool     CheckRegionReallyChanged (ElementHandleCR newEh, ElementHandleCR oldEh)
//     {
//     GPArraySmartP   oldGPA, newGPA;
// 
//     oldGPA->Add (oldEh);
//     newGPA->Add (newEh);
// 
//     return !oldGPA->IsSameGeometryPointByPoint (*newGPA, 0.0, 0.0);
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// static void     CollectRegionRoots
// (
// EditElementHandleR      eeh,
// ElementHandleCR         depEh,
// DependencyLinkage&      depData,
// UInt8*                  rootStatus,
// bool&                   bDepFailed,
// bool&                   bDepChanged,
// RegionParams const&     params,
// ElementAgendaR          rootAgenda,
// bvector<Transform>&     rootTransforms
// )
//     {
//     bool        replaceLinkage = false;
// 
//     // get only the changed/unchanged boundary roots - NOT deleted roots.
//     for (UInt16 iRoot=0; iRoot < depData.nRoots; iRoot++)
//         {
//         switch (rootStatus[iRoot])
//             {
//             case DEPENDENCY_STATUS_CHANGED:
//             case DEPENDENCY_STATUS_RESOLVED:
//             case DEPENDENCY_STATUS_UNCHANGED:
//                 {
//                 DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
// 
//                 if (0 < DependencyManagerLinkage::GetRoots (roots, depEh.GetDgnModelP (), depData, iRoot) && NULL != roots[0].ref)
//                     {
//                     rootAgenda.Insert (roots[0].ref, roots[0].ref->GetDgnModelP ());
// 
//                     if (!roots[0].refattid)
//                         roots[0].refTransform.initIdentity ();
// 
//                     rootTransforms.push_back (roots[0].refTransform);
//                     }
//                 else
//                     {
//                     // NOTE: Couldn't find root for DWG loop fixup at create time. This should be the only time we can't get an ElementRef
//                     //       for a unchanged root as the Dependency Manager would have set a different root status for missing roots...
//                     bDepFailed = true;
//                     }
// 
//                 break;
//                 }
// 
//             case DEPENDENCY_STATUS_DELETED:
//             case DEPENDENCY_STATUS_UNRESOLVED:
//                 {
//                 if (iRoot == 0 && RegionType::Difference == params.GetType ())
//                     {
//                     // Leave root unresolved so association continues to fail
//                     bDepFailed = true;
//                     }
//                 else if (DEPENDENCY_STATUS_DELETED == rootStatus[iRoot])
//                     {
//                     // Clear this root so we don't keep seeing it as unresolved
//                     if (RegionType::Flood == params.GetType ())
//                         {
//                         DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
// 
//                         memset (&roots, 0, sizeof (roots));
//                         DependencyManagerLinkage::SetRoots (depData, NULL, roots, iRoot);
// 
//                         replaceLinkage = true;
//                         }
//                     }
//                 
//                 break;
//                 }
//             }
//         }
// 
//     // Have all boundary elements failed
//     if (rootAgenda.IsEmpty ())
//         bDepFailed = true;
// 
//     if (!replaceLinkage)
//         return;
// 
//     DependencyManagerLinkage::UpdateLinkage (eeh, depData, 0);
//     bDepChanged = true;
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// static bool     GetFlattenTransform
// (
// TransformR          flattenTrans,
// ElementHandleCR     depEh,
// RegionParams const& params
// )
//     {
//     if (!depEh.GetElementCP ()->Is3d())
//         return false;
// 
//     RotMatrix   flattenMatrix;
// 
//     if (!params.GetFlattenBoundary (&flattenMatrix))
//         return false;
// 
//     GPArraySmartP   gpa;
//     DPlane3d        plane;
// 
//     gpa->Add (depEh);
//     gpa->GetPlane (plane);
// 
//     if (flattenMatrix.isIdentity ())
//         flattenTrans.InitFromProjectionToPlane (*( &plane.origin),*( &plane.normal));
//     else
//         flattenTrans.initFromMatrixAndFixedPoint (&flattenMatrix, &plane.origin);
// 
//     return true;
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// static void     UpdateRegion
// (
// EditElementHandleR          eeh,
// ElementHandleCR             depEh,
// DependencyLinkage&          depData,
// UInt8*                      rootStatus,
// RegionUpdateAction          updateStatus,
// bool&                       bDepFailed,
// bool&                       bDepChanged,
// RegionParams const&         params
// )
//     {
//     ElementAgenda       rootAgenda;
//     bvector<Transform>  rootTransforms;
// 
//     CollectRegionRoots (eeh, depEh, depData, rootStatus, bDepFailed, bDepChanged, params, rootAgenda, rootTransforms);
// 
//     if (bDepFailed)
//         return;
// 
//     double      gapTolerance, textMarginFactor;
//     bool        interiorText = params.GetInteriorText (&textMarginFactor);
//     RegionLoops regionLoops = params.GetFloodParams (&gapTolerance);
// 
//     // NOTE: In case single root is grouped hole, need interior shapes...shouldn't matter for other stuff...
//     if (ASSOCREGION_SINGLE_BOUNDARY == updateStatus && RegionLoops::Ignore == regionLoops)
//         regionLoops = RegionLoops::Outer;
// 
//     RegionGraphicsContext   regionContext;
// 
//     regionContext.SetAssociativeRegionUpdate (); // Allow open curves and text for boolean, i.e. DWG goop...
//     regionContext.SetFloodParams (regionLoops, gapTolerance, true); // fail if seed within hole instead of flooding hole!
//     regionContext.SetInteriorText (interiorText, textMarginFactor);
// 
//     Transform   flattenTrans;
// 
//     if (GetFlattenTransform (flattenTrans, depEh, params))
//         regionContext.SetFlattenBoundary (flattenTrans);
// 
//     if (RegionType::Flood == params.GetType ())
//         {
//         bvector<DPoint3d> seedPoints;
// 
//         if (SUCCESS != AssocRegionCellHeaderHandler::GetFloodSeedPoints (depEh, &seedPoints))
//             {
//             bDepFailed = true;
// 
//             return;
//             }
// 
//         if (SUCCESS != regionContext.Flood (*depEh.GetDgnModelP (), rootAgenda, &rootTransforms[0], &seedPoints[0], seedPoints.size ()))
//             {
//             // NOTE: For single boundary flood try to compute a "good" seed point location so it doesn't fail...
//             if (ASSOCREGION_SINGLE_BOUNDARY != updateStatus || 1 != seedPoints.size ())
//                 {
//                 bDepFailed = true;
// 
//                 return;
//                 }
// 
//             GPArraySmartP   gpa;
//             double          pathLength;
// 
//             gpa->Add (*rootAgenda.GetFirstP ());
// 
//             if (!gpa->GetPathCentroid (seedPoints[0], pathLength) ||
//                 SUCCESS != regionContext.Flood (*depEh.GetDgnModelP (), rootAgenda, &rootTransforms[0], &seedPoints[0], seedPoints.size ()))
//                 {
//                 bDepFailed = true;
// 
//                 return;
//                 }
//             }
//         }
//     else
//         {
//         if (SUCCESS != regionContext.Boolean (*depEh.GetDgnModelP (), rootAgenda, &rootTransforms[0], params.GetType ()))
//             {
//             bDepFailed = true;
// 
//             return;
//             }
//         }
// 
//     if (SUCCESS != regionContext.UpdateAssociativeRegion (eeh))
//         {
//         bDepFailed = true;
// 
//         return;
//         }
// 
//     if (bDepChanged)
//         return;
// 
//     bDepChanged = CheckRegionReallyChanged (eeh, depEh);
//     }
//     
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  09/09
// +---------------+---------------+---------------+---------------+---------------+------*/
// StatusInt       OnRootsChangedInternal (ElementHandleCR depEh, DependencyLinkage const& depData, UInt8* rootStatus, UInt8 selfStatus, bool allowWrite)
//     {
//     if (0 != depData.appValue)
//         return SUCCESS; // return if not root boundary dependency callback...
// 
//     if (DEPENDENCY_STATUS_DELETED == selfStatus)
//         return SUCCESS; // do nothing if we have been deleted
// 
//     if (depEh.GetElementCP ()->IsComplexComponent())
//         return SUCCESS; // Vancouver change: Treat as frozen if part of a cell, VERY inefficient if multiple dependents all rewrite entire cell...
// 
//     RegionParams    params;
// 
//     if (SUCCESS != AssocRegionCellHeaderHandler::GetRegionParams (depEh, params))
//         return SUCCESS;
// 
//     RegionUpdateAction  updateStatus;
// 
//     if (SUCCESS != CheckSelfDependency (updateStatus, depEh, depData, rootStatus, selfStatus, params))
//         {
//         WriteAsFailedRegion (depEh, allowWrite);
// 
//         return SUCCESS;
//         }
// 
//     switch (updateStatus)
//         {
//         case ASSOCREGION_UNRESOLVED_ROOTS:
//             {
//             // Display region as failed...if it's not already
//             if (!depEh.GetElementRef ()->IsFailedAssoc ())
//                 DisplayFailedAssociationChange (depEh, true);
// 
//             return SUCCESS;
//             }
// 
//         case ASSOCREGION_RESOLVED_ROOTS:
//             {
//             // Clear display of region as failed...if it's not already
//             if (depEh.GetElementRef ()->IsFailedAssoc ())
//                 DisplayFailedAssociationChange (depEh, false);
// 
//             return SUCCESS;
//             }
// 
//         case ASSOCREGION_NO_ACTION:
//             {
//             // settle down...
//             return SUCCESS;
//             }
//         }
// 
//     bool               bDepFailed = false, bDepChanged = false;
//     EditElementHandle  eeh (depEh, true);
// 
//     // NOTE: Always reset cell transform...not considered a change by itself unless single boundary or reset status...
//     if (ResetCellTMatrix (eeh) && (ASSOCREGION_SINGLE_BOUNDARY == updateStatus || ASSOCREGION_RESET_TMATRIX == updateStatus))
//         bDepChanged = true;
// 
//     size_t              depDataBytes = DependencyManagerLinkage::GetSizeofLinkage (depData, 0);
//     DependencyLinkage*  localDepData = (DependencyLinkage*) alloca (depDataBytes);
// 
//     memcpy (localDepData, &depData, depDataBytes); // local copy so we can update roots...
// 
//     UpdateRegion (eeh, depEh, *localDepData, rootStatus, updateStatus, bDepFailed, bDepChanged, params);
// 
//     if (!allowWrite || !bDepChanged)
//         {
//         DisplayFailedAssociationChange (depEh, bDepFailed);
// 
//         return SUCCESS;
//         }
// 
//     if (params.GetDirty ()) // Clear the region dirty flag
//         {
//         params.SetDirty (false);
//         AssocRegionCellHeaderHandler::SetRegionParams (eeh, params);
//         }
// 
//     if (SUCCESS == WriteChangedRegion (eeh, depEh, allowWrite))
//         SetFailedAssociation (NULL, eeh, bDepFailed);
//     else
//         DisplayFailedAssociationChange (depEh, true); // Unable to change...force redraw to show failed assoc...
// 
//     return SUCCESS;
//     }
// 
// }; // AssocRegionAppIdRootsCallback
// 
// /*=================================================================================**//**
// * @bsiclass                                                     BrienBastings   09/09
// +===============+===============+===============+===============+===============+======*/
// struct AssocRegionAppIdRootsChanged : AssocRegionAppIdRootsCallback
// {
//     DEFINE_T_SUPER(AssocRegionAppIdRootsCallback)
// virtual WString GetDescription () const override {return L"AssocRegionAppIdRootsChanged";}
// 
// virtual StatusInt   OnRootsChanged (ElementHandleCR depEh, DependencyLinkage const& depData, UInt8* pRootStatus, UInt8 selfStatus) override
//     {
//     return T_Super::OnRootsChangedInternal (depEh, depData, pRootStatus, selfStatus, true);
//     }
// 
// }; // AssocRegionAppIdRootsChanged
// 
// /*=================================================================================**//**
// * @bsiclass                                                     BrienBastings   09/09
// +===============+===============+===============+===============+===============+======*/
// struct AssocRegionAppIdRootsChangedForUndoRedo : AssocRegionAppIdRootsCallback
// {
//     DEFINE_T_SUPER(AssocRegionAppIdRootsCallback)
// virtual WString GetDescription () const override {return L"AssocRegionAppIdRootsChangedForUndoRedo";}
// 
// virtual StatusInt   OnRootsChanged (ElementHandleCR depEh, DependencyLinkage const& depData, UInt8* pRootStatus, UInt8 selfStatus) override
//     {
//     return T_Super::OnRootsChangedInternal (depEh, depData, pRootStatus, selfStatus, false);
//     }
// 
// }; // AssocRegionAppIdRootsChangedForUndoRedo
// 
// static AssocRegionAppIdRootsChanged                 s_static_assocRegionAppIdRootsChanged;
// static AssocRegionAppIdRootsChangedForUndoRedo      s_static_assocRegionAppIdRootsChangedForUndoRedo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            assocRegion_staticInitialize ()
    {
//    DependencyManagerLinkage::RegisterRootsChangedCallback (DEPENDENCYAPPID_AssocRegion, &s_static_assocRegionAppIdRootsChanged);                         dependency mgr not in graphite
//    DependencyManagerLinkage::RegisterRootsChangedForUndoRedoCallback (DEPENDENCYAPPID_AssocRegion, &s_static_assocRegionAppIdRootsChangedForUndoRedo);   dependency mgr not in graphite
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssocRegionCellHeaderHandler::ValidateLoops (EditElementHandleR eeh, bool checkRoots)
    {
#ifdef WIP_ASSOC_REGION
    ChildElemIter childEh (eeh, ExposeChildrenReason::Count);

    if (!childEh.IsValid () || !childEh.ToNext ().IsValid ())
        return ERROR; // Invalid or region with a single loop, nothing to fix...

    DependencyLinkageAccessor depP;

    if (checkRoots && SUCCESS == DependencyManagerLinkage::GetLinkage (&depP, eeh, DEPENDENCYAPPID_AssocRegion, false))
        {
        RegionParams  params;

        if (SUCCESS != AssocRegionCellHeaderHandler::GetRegionParams (eeh, params))
            return ERROR;

        size_t  rootStatusBytes = (depP->nRoots * sizeof (UInt8));
        UInt8*  rootStatus = (UInt8*) alloca (rootStatusBytes);

        memset (rootStatus, DEPENDENCY_STATUS_UNCHANGED, rootStatusBytes);

        size_t              depDataBytes = DependencyManagerLinkage::GetSizeofLinkage (*depP, 0);
        DependencyLinkage*  localDepData = (DependencyLinkage*) alloca (depDataBytes);

        memcpy (localDepData, depP, depDataBytes); // local copy so we can update roots...

        bool                bDepFailed = false, bDepChanged = false;
        EditElementHandle   newEeh (eeh, true);

        AssocRegionAppIdRootsCallback::UpdateRegion (newEeh, eeh, *localDepData, rootStatus, ASSOCREGION_UPDATE_REGION, bDepFailed, bDepChanged, params);

        if (bDepFailed)
            return ValidateLoops (eeh, false);

        if (!bDepChanged)
            return ERROR;

        eeh.ReplaceElementDescr (newEeh.ExtractElementDescr ());

        return SUCCESS;
        }

    CurveVectorPtr  curves = fixupHoleLoopsFromDWG (eeh, true);

    if (!curves.IsValid ())
        return ERROR;

    ElementAgenda   boundary;

    if (SUCCESS != GetBoundaryFromCurveVector (boundary, eeh, *curves))
        return ERROR;

    return UpdateAssocRegionBoundary (eeh, boundary);
#endif
return SUCCESS;
    }

