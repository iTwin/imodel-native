/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ChainHeaderHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
ElementHandlerId CivilComplexStringHandler::GetElemHandlerId()
    {
    static const UInt16 CIVIL_COMPLEXSTRING = 7;
    return ElementHandlerId (XATTRIBUTEID_CivilPlatform, CIVIL_COMPLEXSTRING);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/13
//---------------------------------------------------------------------------------------
ElementHandlerId CivilComplexShapeHandler::GetElemHandlerId()
    {
    static const UInt16 CIVIL_COMPLEXSHAPE = 8;
    return ElementHandlerId (XATTRIBUTEID_CivilPlatform, CIVIL_COMPLEXSHAPE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void addGapSegments (CurveVectorR pathCurve)
    {
    // NOTE: Complex shape closure gap won't display or locate...expects real segment...
    if (pathCurve.size () < 2)
        return;

    double      tolerance = pathCurve.ResolveTolerance (0.0);
    DPoint3d    lastEndPoint;

    for (size_t iPrimitive = 0; iPrimitive < pathCurve.size (); iPrimitive++)
        {
        DPoint3d    endPoints[2];

        if (!pathCurve.at (iPrimitive)->GetStartEnd (endPoints[0], endPoints[1]))
            return;

        if (0 != iPrimitive && !lastEndPoint.IsEqual (endPoints[0], tolerance))
            {
            ICurvePrimitivePtr  gapSegment = ICurvePrimitive::CreateLine (DSegment3d::From (lastEndPoint, endPoints[0]));

            gapSegment->SetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
            pathCurve.insert (pathCurve.begin () + iPrimitive, gapSegment);
            lastEndPoint = endPoints[0];
            }
        else
            {
            lastEndPoint = endPoints[1];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void detectGapAndSetBoundaryTypeNone (CurveVectorR pathCurve)
    {
    if (pathCurve.size () < 2)
        return;

    bool        haveLast = false;
    double      tolerance = pathCurve.ResolveTolerance (0.0);
    DPoint3d    lastEndPoint;

    for (ICurvePrimitivePtr& pathMember: pathCurve)
        {
        DPoint3d    endPoints[2];

        if (!pathMember->GetStartEnd (endPoints[0], endPoints[1]))
            return;

        if (haveLast && !lastEndPoint.IsEqual (endPoints[0], tolerance))
            {
            pathCurve.SetBoundaryType (CurveVector::BOUNDARY_TYPE_None);
            return;
            }

        lastEndPoint = endPoints[1];
        haveLast = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isCivilElement (ElementHandleCR eh)
    {
    return (XATTRIBUTEID_CivilPlatform == eh.GetHandler().GetHandlerId().GetMajorId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ChainHeaderHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    CurveVectorPtr pathCurve = CurveVector::Create (CMPLX_SHAPE_ELM == eh.GetLegacyType() ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);

    // step through all of the children...
    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        CurveVectorPtr childCurveVector = ICurvePathQuery::ElementToCurveVector (childEh);

        if (childCurveVector.IsNull ())
            continue;

        // children should really all be a single path member...
        for (ICurvePrimitivePtr pathMember : *childCurveVector)
            {
            if (pathMember.IsNull ())
                continue;

            pathCurve->push_back (pathMember);
            }
        }

    // Previously, Civil registered its own ChainHeaderHandler solely to achieve this gap behavior. Vancouver
    // DgnPlatform incorporated this logic into the base handler for cases where the Civil handler isn't
    // available.
    if (isCivilElement (eh))
        detectGapAndSetBoundaryTypeNone (*pathCurve); // Consumers should treat as a collection of primitives...
    else
        addGapSegments (*pathCurve); // Consumers should treat gaps as real segments...

    if (pathCurve->size () > 0)
        curves = pathCurve;

    return (curves.IsValid () ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ChainHeaderHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (!path.IsOpenPath () && !path.IsClosedPath ())
        return ERROR;

    ChildElemIter       firstChildEh (eeh, ExposeChildrenReason::Count); // Save first child for default template...
    bool                is3d = eeh.GetElementCP ()->Is3d();
    EditElementHandle   newEeh;

    ChainHeaderHandler::CreateChainHeaderElement (newEeh, &eeh, path.IsClosedPath (), is3d, *eeh.GetDgnModelP ());

    ChildElemIter       childEh (eeh, ExposeChildrenReason::Count);

    for (ICurvePrimitivePtr pathMember : path)
        {
        if (pathMember.IsNull ())
            continue;

        double  length = 0.0;

        // Ignore gaps in the middle of chain...gaps and end could have come from partial copy...
        if (pathMember->GetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve) && (!(pathMember == path.front () || pathMember == path.back ()) || !pathMember->FastLength (length) || 0.0 == length))
            continue;

        CurveVectorPtr    childCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

        childCurveVector->push_back (pathMember);

        EditElementHandle  tmpEeh;

        if (childEh.IsValid ())
            {
            ICurvePathEdit* childPathEdit;

            if (NULL != (childPathEdit = dynamic_cast <ICurvePathEdit*> (&childEh.GetHandler ())))
                {
                tmpEeh.Duplicate (childEh);

                if (SUCCESS != childPathEdit->SetCurveVector (tmpEeh, *childCurveVector))
                    tmpEeh.Invalidate ();
                }
            }

        if (!tmpEeh.IsValid ())
            {
            // Try to preserve component symbology when count is the same...
            ElementHandleCP templateEh = (childEh.IsValid () ? &childEh : (firstChildEh.IsValid () ? &firstChildEh : NULL));

            if (SUCCESS != DraftingElementSchema::ToElement (tmpEeh, *childCurveVector, templateEh ? templateEh : &eeh, is3d, *eeh.GetDgnModelP ()))
                return ERROR;
            }

        if (SUCCESS != ChainHeaderHandler::AddComponentElement (newEeh, tmpEeh))
            return ERROR;

        if (childEh.IsValid ())
            childEh = childEh.ToNext ();
        }

    // NOTE: Don't call ChainHeaderHandler::AddComponentComplete to preserve multi-symbology components...
    if (!newEeh.IsValid ())
        return ERROR;

    // Update component count and call ValidateElementRange...
    newEeh.GetElementDescrP ()->Validate ();

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ChainHeaderHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // Need to draw to get correct range for extrude thickness linkage...
    if (mdlElement_attributePresent (elHandle.GetElementCP (), LINKAGEID_Thickness, NULL))
        return DisplayHandler::_ValidateElementRange (elHandle);

    return T_Super::_ValidateElementRange (elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ChainHeaderHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ChainHeaderHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DisplayHandler::EditThicknessProperty (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      ChainHeaderHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    SnapPathP snap = context->GetSnapPath ();
    ElementRefP cmpnElm = snap->GetTailElem ();

    // Always check snap status using component, headers are typically not snappable...
    if (CMPLX_SHAPE_ELM == cmpnElm->GetLegacyType() || CMPLX_STRING_ELM == cmpnElm->GetLegacyType())
        {
        SubElementRefVecP  subElements = snap->GetHeadElem ()->GetSubElements ();

        if (NULL != subElements)
            cmpnElm = subElements->front ();
        }

    if (!cmpnElm || !cmpnElm->GetUnstableMSElementCP ()->IsGraphic() || cmpnElm->GetUnstableMSElementCP ()->IsSnappable())
        return SnapStatus::NotSnappable;

    SnapMode    snapMode = context->GetSnapMode ();

    switch (snapMode)
        {
        case SnapMode::Center:
            {
            GeomDetailCR  detail = snap->GetGeomDetail ();

            // NOTE: if we're doing "center" snapping, and component is arc...use it's center not chains!
            if (HitGeomType::Arc == detail.GetGeomType ())
                break;

            // For center snap entire chain treated as single path...
            CurveVectorPtr  curve;

            if (SUCCESS != _GetCurveVector (ElementHandle (snap->GetPathElem (snapPathIndex)), curve))
                break;

            DPoint3d    centroid;
            Transform   localToWorld, worldToLocal;
            DRange3d    localRange;

            if (curve->IsAnyRegionType () && curve->IsPlanar (localToWorld, worldToLocal, localRange))
                {
                DVec3d  normal;
                double  area;

                if (curve->CentroidNormalArea (centroid, normal, area))
                    {
                    context->ElmLocalToWorld (centroid);
                    context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), centroid, true /* force hot */, false);

                    return SnapStatus::Success;
                    }
                }

            double  length;

            if (!curve->WireCentroid (length, centroid))
                break;

            context->ElmLocalToWorld (centroid);
            context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), centroid, true /* force hot */, false);

            return SnapStatus::Success;
            }

        case SnapMode::Origin:
            {
            // For these snap modes entire chain treated as single path...
            CurveVectorPtr  curve;

            if (SUCCESS != _GetCurveVector (ElementHandle (snap->GetPathElem (snapPathIndex)), curve))
                break;

            DPoint3d    hitPoint;

            if (!curve->GetStartPoint (hitPoint))
                break;

            context->ElmLocalToWorld (hitPoint);
            context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Bisector:
            {
            // For these snap modes entire chain treated as single path...
            CurveVectorPtr  curve;

            if (SUCCESS != _GetCurveVector (ElementHandle (snap->GetPathElem (snapPathIndex)), curve))
                break;

            bvector <double>              distances;
            bvector<CurveLocationDetail>  locations;

            distances.push_back (curve->Length () * 0.5);

            if (!curve->AddSpacedPoints (distances, locations) || locations.size () < 1)
                break;

            DPoint3d    hitPoint = locations.at (0).point;

            context->ElmLocalToWorld (hitPoint);
            context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }
        }

    if (snap->GetCount () > snapPathIndex+1)
        return context->DoSnapUsingNextInPath (snapPathIndex);

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus ChainHeaderHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionH, bool inChain)
    {
    ReprojectStatus     status = REPROJECT_Success;

    for (ChildEditElemIter childIter (source, ExposeChildrenReason::Count); childIter.IsValid();)
        {
        ChildEditElemIter   nextChild = childIter.ToNext();
        DisplayHandlerP     childHandler;

        if (NULL != (childHandler = childIter.GetDisplayHandler()))
            {
            ReprojectStatus childStatus;

            if (REPROJECT_Success != (childStatus = childHandler->GeoCoordinateReprojection (childIter, reprojectionH, true)))
                status = childStatus;

            childHandler->ValidateElementRange (childIter);
            }

        childIter = nextChild;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexStringHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_CMPLX_STRING_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ComplexStringHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
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
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ChainHeaderHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ChainHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()))
        return ERROR;

    return ComplexHeaderDisplayHandler::DropComplex (eh, dropGeom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexShapeHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_CMPLX_SHAPE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ComplexShapeHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
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
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexShapeHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexStringHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  ChainHeaderHandler::_DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP context)
    {
    // Need to test class/level of components to support uSmart elements...
    if (ScanTestResult::Pass == T_Super::_DoScannerTests (eh, levelsOn, classMask, context))
        return ScanTestResult::Pass;

    for (ChildElemIter childIter (eh); childIter.IsValid(); childIter = childIter.ToNext())
        {
        if (ScanTestResult::Pass == childIter.GetHandler().DoScannerTests (childIter, levelsOn, classMask, context))
            return ScanTestResult::Pass;
        }

    return ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ChainHeaderHandler::IsValidChainComponentType (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    // Complex shapes/chains only support specific open element types...
    switch (eh.GetLegacyType())
        {
        case LINE_ELM:
        case LINE_STRING_ELM:
        case CURVE_ELM:
        case ARC_ELM:
            return true;

        case BSPLINE_CURVE_ELM:
            return !eh.GetElementCP()->ToBspline_curve().flags.closed;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ChainHeaderHandler::AddComponentElement (EditElementHandleR eeh, EditElementHandleR componentEeh)
    {
    if (!IsValidChainComponentType (componentEeh))
        return ERROR;

    if (!componentEeh.GetElementDescrP ())
        return ERROR;

    eeh.GetElementDescrP()->AddComponent(*componentEeh.ExtractElementDescr().get());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ChainHeaderHandler::AddComponentComplete (EditElementHandleR eeh)
    {
    if (!eeh.IsValid ())
        return ERROR;

    // Enforce closure of complex shapes by adding segment when there is a gap...
    if (CMPLX_SHAPE_ELM == eeh.GetLegacyType())
        {
        CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (eeh);
        DPoint3d        endPoints[2];

        if (!pathCurve.IsValid () || !pathCurve->GetStartEnd (endPoints[0], endPoints[1]))
            return ERROR;

        if (!endPoints[1].IsEqual (endPoints[0], pathCurve->ResolveTolerance (0.0)))
            {
            DSegment3d          segment;
            EditElementHandle   gapEeh;

            segment.Init (endPoints[1], endPoints[0]);

            if (SUCCESS != LineHandler::CreateLineElement (gapEeh, NULL, segment, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
                return ERROR;

            eeh.GetElementDescrP()->AddComponent(*gapEeh.ExtractElementDescr().get());
            }
        }

    // Ensure that component symb matches header...
    ElementPropertiesSetter::ApplyTemplate (eeh, eeh);

    // Update component count and call ValidateElementRange...
    eeh.GetElementDescrP()->Validate ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void ChainHeaderHandler::CreateChainHeaderElement (EditElementHandleR eeh, ElementHandleCP templateEh, bool isClosed, bool is3d, DgnModelR modelRef)
    {
    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, isClosed ? CMPLX_SHAPE_ELM : CMPLX_STRING_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);

        if (CMPLX_SHAPE_ELM != in->GetLegacyType() && CMPLX_STRING_ELM != in->GetLegacyType())
            out.ToComplex_stringR().reserved = 0;
        }
    else
        {
        memset (&out, 0, sizeof (Complex_string));
        ElementUtil::SetRequiredFields (out, isClosed ? CMPLX_SHAPE_ELM : CMPLX_STRING_ELM, LevelId(LEVEL_DEFAULT_LEVEL_ID), false, (ElementUtil::ElemDim) is3d);
        }

    out.SetSnappable(true); // shape/chain header always non snappable...

    int elmSize = sizeof (Complex_string);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    ElementUtil::InitScanRangeForUnion (out.GetRangeR(), is3d);

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);
    }
