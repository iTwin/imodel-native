/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/GroupedHoleHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupedHoleHandler::IsGroupedHole (ElementHandleCR eh)
    {
    if (!eh.IsValid () || CELL_HEADER_ELM != eh.GetLegacyType() || !eh.GetElementCP()->ToCell_2d().IsHole())
        return false;

    // NOTE: Can't use ChildElemIter since this is called by subtype query...
    bool  first = true, holeFound = false;
    EditElementHandle  childEh;

    MSElementDescrVecCP components=NULL;
    MSElementDescrVec::const_iterator componentIter(0);
    SubElementRefVecP  subElements = NULL;
    SubElementRefVec::iterator subElIter(NULL);

    if (eh.PeekElementDescrCP ())
        {
        components = &eh.PeekElementDescrCP()->Components();
        if (components->empty())
            return false;
        componentIter = components->begin();
        childEh.SetElementDescr (componentIter->get(), false);
        }
    else
        {
        ElementRefP child = NULL;
        subElements = eh.GetElementRef()->GetSubElements();
        if (NULL != subElements)
            {
            subElIter = subElements->begin();
            child = *subElIter;
            }
        childEh.SetElementRef (child);
        }

    for (; childEh.IsValid (); first = false)
        {
        if (!IsValidGroupedHoleComponentType (childEh))
            return false;

        if (first)
            {
            if (childEh.GetElementCP ()->IsHole())
                return false;
            }
        else
            {
            if (!childEh.GetElementCP ()->IsHole())
                return false;

            holeFound = true;
            }

        if (childEh.PeekElementDescrCP ())
            {
            MSElementDescrP next = NULL;
            if (components && (++componentIter != components->end()))
                next = const_cast<MSElementDescrP> (componentIter->get());

            childEh.SetElementDescr (next, false);
            }
        else
            {
            ElementRefP next = NULL;
            if (subElements && (++subElIter != subElements->end()))
                next = *subElIter;
            childEh.SetElementRef (next);
            }
        }

    return holeFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupedHoleHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_GroupedHole));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupedHoleHandler::_ClaimElement (ElementHandleCR thisElm)
    {
    return IsGroupedHole (thisElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupedHoleHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    ChildElemIter    firstChild (thisElm, ExposeChildrenReason::Count);

    if (!firstChild.IsValid())
        {
        T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);

        return;
        }

    // group holes use their first child for their display params
    firstChild.GetDisplayHandler()->GetElemDisplayParams (firstChild, params, wantMaterials);

    params.SetIsRenderable (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    if (testLevel)
        {
        ChildElemIter   firstChild (elHandle, ExposeChildrenReason::Count);

        // group holes use their first child for their display params
        if (firstChild.IsValid () && !firstChild.GetDisplayHandler ()->IsVisible (firstChild, context, false, true, false))
            return false;
        }

    return T_Super::_IsVisible (elHandle, context, testRange, false, testClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            GroupedHoleHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GroupedHoleHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    CurveVectorPtr regionLoops = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);

    // step through all of the child loops...
    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        CurveVectorPtr childCurveVector = ICurvePathQuery::ElementToCurveVector (childEh);

        if (childCurveVector.IsNull ())
            continue;

        CurveVectorPtr childLoop = CurveVector::Create (childEh.GetElementCP ()->IsHole() ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer);

        // children should form a single closed loop...
        for (ICurvePrimitivePtr pathMember: *childCurveVector)
            {
            if (pathMember.IsNull ())
                continue;

            childLoop->push_back (pathMember);
            }

        regionLoops->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childLoop.get ()));
        }

    if (regionLoops->size () > 1)
        curves = regionLoops;
    
    return (curves.IsValid () ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GroupedHoleHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (!path.IsParityRegion ())
        return ERROR;

    EditElementHandle   newEeh (*eeh.GetElementCP (), *eeh.GetDgnModelP ());
    ChildElemIter       solidEh (eeh, ExposeChildrenReason::Count);
    ChildElemIter       holeEh = solidEh.ToNext ();

    for (ICurvePrimitivePtr pathMember : path)
        {
        if (pathMember.IsNull ())
            continue;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != pathMember->GetCurvePrimitiveType ())
            return ERROR;

        CurveVectorPtr    childCurveVector = CurveVector::Create (pathMember->GetChildCurveVectorCP ()->GetBoundaryType ());

        // children should form a single closed loop...
        for (ICurvePrimitivePtr loopPathMember : *pathMember->GetChildCurveVectorCP ())
            {
            if (loopPathMember.IsNull ())
                continue;

            childCurveVector->push_back (loopPathMember);
            }

        bool                isHoleLoop = CurveVector::BOUNDARY_TYPE_Inner == childCurveVector->GetBoundaryType ();
        EditElementHandle   tmpEeh;

        if (!isHoleLoop)
            {
            ICurvePathEdit* childPathEdit;

            if (NULL != (childPathEdit = dynamic_cast <ICurvePathEdit*> (&solidEh.GetHandler ())))
                {
                tmpEeh.Duplicate (solidEh);

                if (SUCCESS != childPathEdit->SetCurveVector (tmpEeh, *childCurveVector))
                    tmpEeh.Invalidate ();
                }
            }
        else if (holeEh.IsValid ())
            {
            ICurvePathEdit* childPathEdit;

            if (NULL != (childPathEdit = dynamic_cast <ICurvePathEdit*> (&holeEh.GetHandler ())))
                {
                tmpEeh.Duplicate (holeEh);

                if (SUCCESS != childPathEdit->SetCurveVector (tmpEeh, *childCurveVector))
                    tmpEeh.Invalidate ();
                }
            }

        if (!tmpEeh.IsValid ())
            {
            // Try to preserve component symbology when count is the same...
            ElementHandle nextChildTemporary;
            ElementHandleCP templateEh;
            if (!isHoleLoop)
                templateEh = &solidEh;
            else
                {
                if (holeEh.IsValid ())
                    templateEh = &holeEh;
                else
                    {
                    nextChildTemporary = solidEh.ToNext ();
                    templateEh = &nextChildTemporary;
                    }
                }

            if (SUCCESS != DraftingElementSchema::ToElement (tmpEeh, *pathMember->GetChildCurveVectorCP (), templateEh, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
                return ERROR;

            IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&tmpEeh.GetHandler());

            if (!areaObj || !areaObj->SetAreaType (tmpEeh, isHoleLoop))
                return ERROR;
            }

        if (SUCCESS != NormalCellHeaderHandler::AddChildElement (newEeh, tmpEeh))
            return ERROR;

        if (isHoleLoop && holeEh.IsValid ())
            holeEh = holeEh.ToNext ();
        }

    if (SUCCESS != NormalCellHeaderHandler::AddChildComplete (newEeh))
        return ERROR;

    if (!IsGroupedHole (newEeh))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const
    {
    ChildElemIter   firstChild (eh, ExposeChildrenReason::Count);

    IAreaFillPropertiesQuery* childAreaQueryObj = dynamic_cast <IAreaFillPropertiesQuery*> (&firstChild.GetHandler());

    if (!childAreaQueryObj)
        return false;

    return childAreaQueryObj->GetSolidFill (firstChild, fillColorP, alwaysFilledP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const
    {
    ChildElemIter   firstChild (eh, ExposeChildrenReason::Count);

    IAreaFillPropertiesQuery* childAreaQueryObj = dynamic_cast <IAreaFillPropertiesQuery*> (&firstChild.GetHandler());

    if (!childAreaQueryObj)
        return false;

    return childAreaQueryObj->GetGradientFill (firstChild, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_RemoveAreaFill (EditElementHandleR eeh)
    {
    ChildEditElemIter   firstChildEeh (eeh, ExposeChildrenReason::Count);

    IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&firstChildEeh.GetHandler());

    if (!childAreaEditObj)
        return false;

    return childAreaEditObj->RemoveAreaFill (firstChildEeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP)
    {
    ChildEditElemIter   firstChildEeh (eeh, ExposeChildrenReason::Count);

    IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&firstChildEeh.GetHandler());

    if (!childAreaEditObj)
        return false;

    return childAreaEditObj->AddSolidFill (firstChildEeh, fillColorP, alwaysFilledP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::_AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb)
    {
    ChildEditElemIter   firstChildEeh (eeh, ExposeChildrenReason::Count);

    IAreaFillPropertiesEdit* childAreaEditObj = dynamic_cast <IAreaFillPropertiesEdit*> (&firstChildEeh.GetHandler());

    if (!childAreaEditObj)
        return false;

    return childAreaEditObj->AddGradientFill (firstChildEeh, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupedHoleHandler::IsValidGroupedHoleComponentType (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    // GroupedHoles only support specific element types...
    switch (eh.GetLegacyType())
        {
        case SHAPE_ELM:
        case CMPLX_SHAPE_ELM:
        case ELLIPSE_ELM:
            return true;

        // NOTE: Used to allow physically closed bsplines a test that is expensive, require closed bsplines!
        case BSPLINE_CURVE_ELM:
            return eh.GetElementCP()->ToBspline_curve().flags.closed;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GroupedHoleHandler::CreateGroupedHoleElement (EditElementHandleR eeh, EditElementHandleR solidEeh, ElementAgendaR holes)
    {
    if (!solidEeh.IsValid () || holes.IsEmpty ())
        return ERROR;

    if (!IsValidGroupedHoleComponentType (solidEeh))
        return ERROR;

    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&solidEeh.GetHandler());

    if (!areaObj || !areaObj->SetAreaType (solidEeh, false))
        return ERROR;

    EditElementHandleP curr = holes.GetFirstP ();
    EditElementHandleP end  = curr + holes.GetCount ();

    eeh.Invalidate ();

    bool    addedHole = false;

    for (; curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;
            
        if (!IsValidGroupedHoleComponentType (*curr))
            continue;

        areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&curr->GetHandler());

        if (!areaObj || !areaObj->SetAreaType (*curr, true))
            continue;

        if (!eeh.IsValid ())
            {
            NormalCellHeaderHandler::CreateOrphanCellElement (eeh, NULL, solidEeh.GetElementCP ()->Is3d(), *solidEeh.GetDgnModelP ());

            if (SUCCESS != NormalCellHeaderHandler::AddChildElement (eeh, solidEeh))
                return ERROR;
            }

        if (SUCCESS != NormalCellHeaderHandler::AddChildElement (eeh, *curr))
            continue;

        addedHole = true;
        }

    if (!addedHole)
        return ERROR;

    return NormalCellHeaderHandler::AddChildComplete (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus GroupedHoleHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionH, bool inChain)
    {
    // For grouped hole apply transform to children...
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

    // get the transformation at the cell origin, and apply that to the header.
    DPoint3d        origin;
    TransformInfo   transform;

    _GetTransformOrigin (source, origin);
    reprojectionH.GetLocalTransform (&transform.GetTransformR(), origin, NULL, true, true);

    TransformCellHeader (source, *transform.GetTransform ());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GroupedHoleHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()))
        return ERROR;

    return ComplexHeaderDisplayHandler::DropComplex (eh, dropGeom);
    }

#ifdef WIP_EC_EXTENSION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GroupedHoleElementECExtension::_GetECClasses (T_ECClassCPVector& classes) const
    {
    classes.push_back (LookupECClass (DGN_ELEMENT_SCHEMA, L"GroupedHoleElement"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
GroupedHoleElementECExtension* GroupedHoleElementECExtension::Create ()
    {
    return new GroupedHoleElementECExtension ();
    }

#endif
