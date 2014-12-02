/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/CellHeaderHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ComplexHeaderDisplayHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    StatusInt status = SUCCESS;

    MSElementDescrP header = elemHandle.GetElementDescrP ();
    BeAssert (header);// && (header->h.isHeader || !header->h.firstElem));
    header->Invalidate();

    for (ChildEditElemIter childElm (elemHandle, ExposeChildrenReason::Count); childElm.IsValid() && SUCCESS == status; childElm = childElm.ToNext())
        status = childElm.GetHandler().FenceStretch (childElm, transform, fp, options);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ComplexHeaderDisplayHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    ClipVectorPtr    clipP;
    
    if (! (clipP = fp->GetClipVector()).IsValid())
        return ERROR;

    ClipVectorPtr   linkageClip;
    size_t          originalClipSize = clipP->size();

    if (SUCCESS == CellUtil::ClipFromLinkage (linkageClip, elemHandle, fp->GetTransform ()) && linkageClip.IsValid())
        clipP->Append (*linkageClip);
    
    StatusInt   status = SUCCESS;
    
    for (ChildElemIter childEh (elemHandle, ExposeChildrenReason::Count); childEh.IsValid () && SUCCESS == status; childEh = childEh.ToNext ())
        status = childEh.GetHandler ().FenceClip (inside, outside, childEh, fp, options);

    clipP->resize (originalClipSize);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Type2Handler::_OnFenceStretchFinish (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    StatusInt   status;

    if (SUCCESS != (status = T_Super::_OnFenceStretchFinish (elemHandle, transform, fp, options)))
        return status;

    ValidateRangeDiagonal (elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Type2Handler::_GetBasisTransform (ElementHandleCR eh, TransformR transform) 
    {
    DPoint3d        origin;
    RotMatrix       rMatrix;

    if (SUCCESS == CellUtil::ExtractOrigin (origin, eh) &&
        SUCCESS == CellUtil::ExtractRotation (rMatrix, eh))
        {
        transform.InitFrom (rMatrix, origin);

        return true;
        }

    return false;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Type2Handler:: _GetBasisRange (ElementHandleCR eh, DRange3dR range)
    {
    return SUCCESS == CellUtil::ExtractRangeDiagonal (range, eh);
    }

             
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt NormalCellHeaderHandler::_OnFenceStretch (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    // Part of the cell overlaps the fence...
    // ***TRICKY: Do NOT call T_Super (ComplexHeaderDisplayHandler) OnFenceStretch -- I may or may not decide to stretch children
    if (FenceStretchFlags::None == (options & FenceStretchFlags::Cells))
        return SUCCESS;        //  We are not allowed to stretch cells: do nothing

    StatusInt   status;
    DPoint3d    origin;

    CellUtil::ExtractOrigin (origin, elemHandle);

    // We are allowed to stretch cells: apply stretch to children
    if (!fp->GetViewport () || !_IsPointCell (elemHandle))
        {
        status = T_Super::_OnFenceStretch (elemHandle, transform, fp, options);
        }
    else
        {
        RotMatrix   viewRMatrix;
        Transform   viTrans;

        viewRMatrix.inverseOf (&fp->GetViewport ()->GetRotMatrix ());
        viTrans.initFromMatrixAndFixedPoint (&viewRMatrix, &origin);

        TransformInfo tInfo (viTrans);

        elemHandle.GetHandler().ApplyTransform (elemHandle, tInfo);

        status = T_Super::_OnFenceStretch (elemHandle, transform, fp, options);

        tInfo.GetTransformR().inverseOf (tInfo.GetTransform ());
        elemHandle.GetHandler().ApplyTransform (elemHandle, tInfo);
        }

    // Apply stretch transform to cell origin if it's inside the fence...
    if (SUCCESS != status || !fp->PointInside (origin))
        return status;

    DgnElementP  el = elemHandle.GetElementP ();

    transform.GetTransform ()->Multiply (origin);

    if (el->Is3d())
        el->ToCell_3dR().origin = origin;
    else
        el->ToCell_2dR().origin.Init (origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt NormalCellHeaderHandler::_OnFenceStretchFinish (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    if (FenceStretchFlags::None == (options & FenceStretchFlags::Cells))
        return SUCCESS; // We are not allowed to stretch cells: do nothing

    return T_Super::_OnFenceStretchFinish (elemHandle, transform, fp, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NormalCellHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()))
        return ERROR;

    return ComplexHeaderDisplayHandler::DropComplex (eh, dropGeom);
    }

/*---------------------------------------------------------------------------------**//**
* Default implementation of CalulateRange for ComplexHeaderDisplayHandler.
* Calculates the range by unioning all of the ranges of the children of this complex header.
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ComplexHeaderDisplayHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // Persistent elements have valid ranges...
    if (NULL == elHandle.PeekElementDescrCP())
        return SUCCESS;

    // If header is invisible, include invisible children in range (ex. complex chain in associative region boundary)
    bool        includeInvisible = elHandle.GetElementP()->IsInvisible();

    ElemRangeCalc   newRange;

    for (ChildElemIter childIter (elHandle, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        if (!childIter.GetElementCP()->IsGraphic())
            continue;

        if (childIter.GetElementCP()->IsInvisible() && !includeInvisible)
            continue;

        newRange.Union (&childIter.GetElementCP()->GetRange(), NULL);
        }

    if (!newRange.IsValid())
        return ERROR;

    DgnElementP  el = elHandle.GetElementP();

    newRange.ToScanRange (el->GetRangeR(), Is3dElem(el));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ComplexHeaderDisplayHandler::_OnTransform
(
EditElementHandleR elemHandle,
TransformInfoCR trans
)
    {
    StatusInt   status = T_Super::_OnTransform (elemHandle, trans);

    if (SUCCESS == status)
        {
        for (ChildEditElemIter childElm (elemHandle, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
            status = childElm.GetHandler().ApplyTransform (childElm, trans);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        childElm.GetHandler().ConvertTo3d (childElm, elevation);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    TransformInfo tinfo (flattenTrans);

    ElementUtil::ApplyTransformToLinkages (eeh, tinfo);

    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        childElm.GetHandler().ConvertTo2d (childElm, flattenTrans, flattenDir);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ComplexHeaderDisplayHandler::_OnChangeOfUnits (EditElementHandleR elHandle, DgnModelP source, DgnModelP dest)
    {
    StatusInt status = T_Super::_OnChangeOfUnits (elHandle, source, dest);
    if (SUCCESS != status)
        return status;

    MSElementDescrP parent = elHandle.GetElementDescrP ();
    BeAssert (parent);
    parent->Invalidate();

    for (ChildEditElemIter childElm (elHandle, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        status = childElm.GetHandler().ChangeUnits (childElm, source, dest);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ComplexHeaderDisplayHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normalP, DPoint3dP pointP, DVec3dCP inputDefaultNormalP)
    {
    if (!Is3dElem (thisElm.GetElementCP ()))
        {
        if (normalP)
            normalP->init (0.0, 0.0, 1.0);

        if (pointP)
            _GetTransformOrigin (thisElm, *pointP);

        return true;
        }

    ChildElemIter childElm (thisElm, ExposeChildrenReason::Query);

    // Not a public collection, just call super (will attempt to answer IsPlanar using path/region interfaces, ex. Complex Shape/Chain)...
    if (!childElm.IsValid ())
        return T_Super::_IsPlanar (thisElm, normalP, pointP, inputDefaultNormalP);

    bool            isValid = false;
    DVec3d          planeNormal;
    DPoint3d        planePoint;
    GPArraySmartP   lineGpa;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        {
        CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (childElm);

        if (pathCurve.IsValid ())
            {
            bool    isLineSegment = false;

            switch (pathCurve->HasSingleCurvePrimitive ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    isLineSegment = true;
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    isLineSegment = (2 == pathCurve->front ()->GetLineStringCP ()->size ());
                    break;
                    }
                }

            // Can't tell anything about individual lines...so collect them up...
            if (isLineSegment)
                {
                lineGpa->AddCurves (*pathCurve);
                continue;
                }
            }

        DisplayHandlerP dHandler = childElm.GetDisplayHandler();

        if (!dHandler)
            continue;

        DVec3d      thisNormal;
        DPoint3d    thisPoint;

        if (!dHandler->IsPlanar (childElm, &thisNormal, &thisPoint, inputDefaultNormalP))
            return false;

        if (isValid)
            {
            if (!planeNormal.isParallelTo (&thisNormal))
                return false;

            DRay3d      ray;
            DPoint3d    rayPoint;

            bsiDRay3d_initFromDPoint3dTangent (&ray, &planePoint, &planeNormal);

            if (!bsiDRay3d_projectPoint (&ray, &rayPoint, NULL, &thisPoint) || !bsiDPoint3d_pointEqualTolerance (&rayPoint, &planePoint, 1.0e-8))
                return false;
            }
        else
            {
            planeNormal = thisNormal;
            planePoint  = thisPoint;

            if (normalP)
                *normalP = planeNormal;

            if (pointP)
                *pointP  = planePoint;

            isValid = true;
            }
        }

    // Check planarity after including lines...
    if (0 != lineGpa->GetCount ())
        {
        double          maxVariance;
        Transform       transform;
        DRange3d        localRange;

        if (!lineGpa->GraphicsPointArray::GetPlane (transform, &localRange, &maxVariance, inputDefaultNormalP))
            return false;

        DVec3d          thisNormal;
        DPoint3d thisPoint;
        transform.GetTranslation (thisPoint);
        transform.GetMatrixColumn (thisNormal, 2);

        if (maxVariance >= 1.0e-8 * bsiDRange3d_extentSquared (&localRange))
            return false;

        if (isValid)
            {
            if (!planeNormal.isParallelTo (&thisNormal))
                return false;

            DRay3d      ray;
            DPoint3d    rayPoint;

            bsiDRay3d_initFromDPoint3dTangent (&ray, &planePoint, &planeNormal);

            if (!bsiDRay3d_projectPoint (&ray, &rayPoint, NULL, &thisPoint) || !bsiDPoint3d_pointEqualTolerance (&rayPoint, &planePoint, 1.0e-8))
                return false;
            }
        else
            {
            // Only contains lines...
            if (normalP)
                *normalP = thisNormal;

            if (pointP)
                *pointP  = thisPoint;

            isValid = true;
            }
        }

    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    _QueryHeaderProperties (eh, context);

    // Don't iterate through public children...
    if (_ExposeChildren (eh, ExposeChildrenReason::Query))
        return;

    for (ChildElemIter childElm (eh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        childElm.GetHandler().QueryProperties (childElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComplexHeaderDisplayHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    _EditHeaderProperties (eeh, context);

    // Don't iterate through public children...
    if (_ExposeChildren (eeh, ExposeChildrenReason::Edit))
        return;

    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        childElm.GetHandler().EditProperties (childElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    templateFromTopLevelChildren
(
ElementHandleR     templateElHandle,
ElementHandleCR    elHandle
)
    {
    ChildElemIter   childIter (elHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid ())
        return false;

    do
        {
        DgnElementCP     elP = childIter.GetElementCP ();

        if (elP->IsGraphic() &&
            elP->GetLegacyType() != CELL_HEADER_ELM &&
            elP->GetLegacyType() != SHAREDCELL_DEF_ELM &&
            elP->GetLegacyType() != SHARED_CELL_ELM &&
            ComplexHeaderDisplayHandler::GetComponentForDisplayParams (templateElHandle, childIter))
            {
            return true;
            }

        childIter = childIter.ToNext ();

        } while (childIter.IsValid ());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    templateFromGrandChildren
(
ElementHandleR     templateElHandle,
ElementHandleCR    elHandle
)
    {
    ChildElemIter   childIter (elHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid ())
        return false;

    do
        {
        DgnElementCP     elP = childIter.GetElementCP ();

        if (elP->GetLegacyType() == CELL_HEADER_ELM ||
            elP->GetLegacyType() == SHAREDCELL_DEF_ELM)
            {
            ChildElemIter   grandChildIter ((ElementHandleCR) childIter, ExposeChildrenReason::Count); // Want FirstElem, not Next!

            if (grandChildIter.IsValid ())
                {
                do
                    {
                    elP = grandChildIter.GetElementCP ();

                    if (elP->IsGraphic() &&
                        ComplexHeaderDisplayHandler::GetComponentForDisplayParams (templateElHandle, grandChildIter) &&
                        templateElHandle.GetElementCP ()->GetLegacyType() != CELL_HEADER_ELM &&
                        templateElHandle.GetElementCP ()->GetLegacyType() != SHAREDCELL_DEF_ELM)
                        return true;

                    grandChildIter = grandChildIter.ToNext ();

                    } while (grandChildIter.IsValid ());
                }
            }

        childIter = childIter.ToNext ();

        } while (childIter.IsValid ());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ComplexHeaderDisplayHandler::GetComponentForDisplayParams (ElementHandleR templateElHandle, ElementHandleCR elHandle)
    {
    switch (elHandle.GetLegacyType())
        {
        case CMPLX_SHAPE_ELM:
        case CMPLX_STRING_ELM:
        case BSPLINE_CURVE_ELM:
        case BSPLINE_SURFACE_ELM:
        case MESH_HEADER_ELM:
        case MATRIX_HEADER_ELM:
            {
            templateElHandle = elHandle; // Complex headers that are valid templates (i.e. have a level)

            return true;
            }

        default:
            {
            if (elHandle.GetElementCP ()->IsComplexHeaderType())
                break;

            templateElHandle = elHandle; // Non-header element is always a valid template...

            return true;
            }
        }

    // First look for a non-cell on this level...
    if (templateFromTopLevelChildren (templateElHandle, elHandle))
        return true;

    // Check branches...
    if (templateFromGrandChildren (templateElHandle, elHandle))
        return true;

    return false; // No template found...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ComplexHeaderDisplayHandler::DropComplex (ElementHandleCR eh, ElementAgendaR dropGeom)
    {
    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (!childEh.GetElementCP ()->IsGraphic() || childEh.GetElementCP ()->IsInvisible())
            continue;

        EditElementHandle childEeh;

        childEeh.Duplicate (childEh);

        if (!childEeh.IsValid ())
            continue;

        childEeh.GetElementP()->SetComplexComponent(false); // Clear complex component bit...
        dropGeom.Insert (childEeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UseChildren (thisElm, context, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getNormalCellLODTemplate (ElementHandleR templateEh, ElementHandleCR eh, ViewContextR context)
    {
    // NOTE: A cell containing a shared cell is NEVER excluded by level criteria so we need to figure out if the cell is actually visible...
    for (ChildElemIter childEh (eh, ExposeChildrenReason::Query); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (childEh.GetHandler ().ExposeChildren (childEh, ExposeChildrenReason::Query))
            {
            if (getNormalCellLODTemplate (templateEh, childEh, context))
                return true;

            continue;
            }

        DisplayHandlerP dHandler = childEh.GetDisplayHandler ();

        if (!dHandler || !dHandler->IsVisible (childEh, context, false, true, true))
            continue;

        // NOTE: The child could be a complex (ex. smart solid) so we still need to get a component for a good level, etc.
        return ComplexHeaderDisplayHandler::GetComponentForDisplayParams (templateEh, childEh);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_DrawFiltered (ElementHandleCR thisElm, ViewContextR context, DPoint3dCP pts, double size)
    {
    if (DrawPurpose::UpdateDynamic == context.GetDrawPurpose ())
        return;

    if (0 == thisElm.GetElementCP ()->GetComplexComponentCount ())
        return;

    // For a "public container" cell, find component with a level in case level symbology is enabled...
    if (!_ExposeChildren (thisElm, ExposeChildrenReason::Query))
        {
        /* NOTE: Sub-types that don't override _DrawFiltered should implement _GetElemDisplayParams
                 and choose a level (since the cell header's level is 0) in order to display filtered
                 using the correct color when level symbology is enabled for the view! */
        T_Super::_DrawFiltered (thisElm, context, pts, size);
        return;
        }

    if (FILTER_LOD_ShowNothing == context.GetFilterLODFlag())
        return;

    ElementHandle  templateEh;

    // NOTE: Don't call child's DrawFiltered, i.e, line in cell may not display filtered based on size and appear as a dot!
    if (getNormalCellLODTemplate (templateEh, thisElm, context))
        T_Super::_DrawFiltered (templateEh, context, pts, size);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Type2PropertiesFilter : public ProcessPropertiesFilter
{
public:

/* NOTE: I believe we Do want Material/Element Template reported normally for cell headers!
         Other stuff like Font, TextStyle, etc. shouldn't be found...may as well report it
         if it is. Primary concern is dhdr stuff and Element Selection Tool. */

Type2PropertiesFilter (IQueryProperties* queryObj) : ProcessPropertiesFilter (queryObj) {}
Type2PropertiesFilter (IEditProperties*  editObj)  : ProcessPropertiesFilter (editObj)  {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetPropertyFlags (EachPropertyBaseArg& arg, PropsCallbackFlags addFlag)
    {
    // Only need to add ignored or undisplayed to flags for base ids...
    if (0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return;

    // NOTE: Non-zero level currently set by AssocRegion so that effective BYLEVEL/CELL pattern symbology is reported correctly!
    LevelId     levelId = arg.GetPropertyContext ().GetCurrentLevelID ();

    // If level is valid, assume everything else is ok too...otherwise include "addFlags" for ignored/undisplayed...
    if (!levelId.IsValid())
        arg.SetPropertyFlags ((PropsCallbackFlags) (addFlag | arg.GetPropertyFlags ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetLevelPropertyFlags (EachPropertyBaseArg& arg, PropsCallbackFlags addFlag)
    {
    // Only need to add ignored or undisplayed to flags for base ids...
    if (0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return;

    arg.SetPropertyFlags ((PropsCallbackFlags) (addFlag | arg.GetPropertyFlags ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachLevelCallback (EachLevelArg& arg) override
    {
    // Normal case level is "ignored" and level should be zero...see GetElemHeaderOverrides
    SetLevelPropertyFlags (arg, !arg.GetStoredValue().IsValid() ? PROPSCALLBACK_FLAGS_ElementIgnoresID : PROPSCALLBACK_FLAGS_UndisplayedID);
    m_callbackObj->_EachLevelCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachElementClassCallback (EachElementClassArg& arg) override
    {
    SetPropertyFlags (arg, PROPSCALLBACK_FLAGS_UndisplayedID);
    m_callbackObj->_EachElementClassCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachColorCallback (EachColorArg& arg) override
    {
    SetPropertyFlags (arg, PROPSCALLBACK_FLAGS_UndisplayedID);
    m_callbackObj->_EachColorCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachLineStyleCallback (EachLineStyleArg& arg) override
    {
    SetPropertyFlags (arg, PROPSCALLBACK_FLAGS_UndisplayedID);
    m_callbackObj->_EachLineStyleCallback (arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachWeightCallback (EachWeightArg& arg) override
    {
    SetPropertyFlags (arg, PROPSCALLBACK_FLAGS_UndisplayedID);
    m_callbackObj->_EachWeightCallback (arg);
    }

}; // Type2PropertiesFilter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context)
    {
    // NOTE: Need to set header "ignore" flags....
    IQueryProperties*       queryObj = context.GetIQueryPropertiesP ();
    Type2PropertiesFilter   filterObj (queryObj);

    context.SetIQueryPropertiesP (&filterObj);
    T_Super::_QueryHeaderProperties (eh, context);
    context.SetIQueryPropertiesP (queryObj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    // Make sure that cell headers are always on level 0
    if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
        eeh.GetElementP ()->GetLevel() = 0;
#endif

    // NOTE: Need to set header "ignore" flags....
    IEditProperties*        editObj = context.GetIEditPropertiesP ();
    Type2PropertiesFilter   filterObj (editObj);

    context.SetIEditPropertiesP (&filterObj);
    T_Super::_EditHeaderProperties (eeh, context);
    context.SetIEditPropertiesP (editObj);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Type2Handler::_IsVisible (ElementHandleCR eh, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    ScanCriteriaCP scanCrit;
    if ((testLevel || testClass) && (NULL != (scanCrit = context.GetScanCriteria())))
        {
        UInt32* pClassMask = NULL;
        UInt32  classMask;
        if (testClass)
            {
            classMask = scanCrit->GetClassMask();
            pClassMask = &classMask;
            }

        if (ScanTestResult::Pass != _DoScannerTests (eh, testLevel ? scanCrit->GetLevelBitMask() : NULL, pClassMask, &context))
            return  false;
        }

    return testRange ? T_Super::_IsVisible (eh, context, true, false, false) : true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  Type2Handler::_DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP context)
    {
    if (classMask && (0 == (eh.GetElementCP()->ToCell_2d().classMap & *classMask)))
        return ScanTestResult::Fail;

    for (ChildElemIter childIter (eh, ExposeChildrenReason::Count); childIter.IsValid(); childIter = childIter.ToNext())
        {
        if (ScanTestResult::Pass == childIter.GetHandler().DoScannerTests (childIter, levelsOn, classMask, context))
            return  ScanTestResult::Pass;
        }

    return  ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    DgnElementCP el = elHandle.GetElementCP();

    if (el->Is3d())
        origin = el->ToCell_3d().origin;
    else
        origin.Init (el->ToCell_2d().origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR rMatrix)
    {
    DgnElementCP  el = elHandle.GetElementCP ();

    if (el->Is3d())
        memcpy (rMatrix.form3d, el->ToCell_3d().transform, 9 * sizeof (double));
    else
        rMatrix.InitFromRowValuesXY ( &el->ToCell_2d().transform[0][0]);

    rMatrix.normalizeColumnsOf (&rMatrix, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      Type2Handler::_OnSnap (SnapContextP context, int snapPathIndex)
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
void            Type2Handler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_CELL_HEADER_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    _GetTypeName (descr, desiredLength);

    WChar     cellName[MAX_CELLNAME_LENGTH];

    CellUtil::ExtractName (cellName, MAX_CELLNAME_LENGTH, el);

    if (wcslen (cellName) > 0)
        descr.append(L": ").append(cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::ValidateRangeDiagonal (EditElementHandleR elHandle)
    {
    // calculate the range of the "unrotated" cell to get the new range diagonal
    DPoint3d    origin;
    RotMatrix   rMatrix, invertedRMatrix;
    Transform   trans, rotateTrans;

    CellUtil::ExtractOrigin (origin, elHandle);
    CellUtil::ExtractRotation (rMatrix, elHandle);

    if (!bsiRotMatrix_invertRotMatrix (&invertedRMatrix, &rMatrix))
        {
        // Possibly a flatten matrix; try to augment and invert that
        bsiRotMatrix_augmentRank (&rMatrix, &rMatrix);

        if (!bsiRotMatrix_invertRotMatrix (&invertedRMatrix, &rMatrix))
            invertedRMatrix.initIdentity ();
        }

    rotateTrans.InitFrom(invertedRMatrix);
    trans.InitFrom( -origin.x,  -origin.y,  -origin.z);
    trans.InitProduct(rotateTrans,trans);

    DRange3d    range;

    CalcElementRange (elHandle, range, &trans);

    DgnElementP  el = elHandle.GetElementP ();

    if (Is3dElem (el))
        {
        el->ToCell_3dR().rnglow  = range.low;
        el->ToCell_3dR().rnghigh = range.high;
        }
    else
        {
        el->ToCell_2dR().rnglow.x  = range.low.x;
        el->ToCell_2dR().rnglow.y  = range.low.y;
        el->ToCell_2dR().rnghigh.x = range.high.x;
        el->ToCell_2dR().rnghigh.y = range.high.y;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Type2Handler::SetOriginAndRange (EditElementHandleR eeh)
    {
    // Hopefully equivalent to: mdlCell_setOrigin/mdlCell_setRange...
    if (CELL_HEADER_ELM != eeh.GetLegacyType())
        return ERROR;

    BentleyStatus   status = _ValidateElementRange (eeh);

    ValidateRangeDiagonal (eeh);
    DPoint3d origin;
    GetRangeCenter (eeh, origin);

    DgnElementP el = eeh.GetElementP ();

    if (el->Is3d())
        el->ToCell_3dR().origin = origin;
    else
        el->ToCell_2dR().origin.Init (origin);

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Type2Handler::SetRange (EditElementHandleR eeh)
    {
    // Hopefully equivalent to: mdlCell_setRange
    if (CELL_HEADER_ELM != eeh.GetLegacyType())
        return ERROR;

    BentleyStatus   status = _ValidateElementRange (eeh);

    ValidateRangeDiagonal (eeh);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Type2Handler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // Persistent elements have valid ranges...
    if (NULL == elHandle.PeekElementDescrCP ())
        return SUCCESS;

    if (SUCCESS == T_Super::_ValidateElementRange (elHandle))
        return SUCCESS;

    // NOTE: Cells w/o children...set to the default range about the origin of the cell...
    DgnElementP  el = elHandle.GetElementP ();
    DRange3d    rDiag;

    if (el->Is3d())
        {
        rDiag.low    = el->ToCell_3d().rnglow;
        rDiag.high   = el->ToCell_3d().rnghigh;
        }
    else
        {
        rDiag.low.x  = el->ToCell_2d().rnglow.x;
        rDiag.low.y  = el->ToCell_2d().rnglow.y;
        rDiag.high.x = el->ToCell_2d().rnghigh.x;
        rDiag.high.y = el->ToCell_2d().rnghigh.y;
        rDiag.low.z  = rDiag.high.z = 0.0;
        }

    DPoint3d    rOrigin;
    RotMatrix   cellt;

    CellUtil::ExtractOrigin (rOrigin, elHandle);
    CellUtil::ExtractRotation (cellt, elHandle);

    cellt.Multiply (rDiag, rDiag);
    rDiag.low.add (&rOrigin);
    rDiag.high.add (&rOrigin);

    ElemRangeCalc   newRange;
    newRange.Union (&rDiag, NULL);

    if (!newRange.IsValid())
        return ERROR;

    newRange.ToScanRange (el->GetRangeR(), Is3dElem(el));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     transformCellHeader (EditElementHandleR eeh, TransformCR trans)
    {
    DPoint3d    rOrigin;
    RotMatrix   cellt;

    CellUtil::ExtractOrigin (rOrigin, eeh);
    CellUtil::ExtractRotation (cellt, eeh);

    cellt.InitProduct(trans, cellt);
    trans.Multiply(rOrigin);

    DgnElementP  el = eeh.GetElementP ();

    if (el->Is3d())
        {
        el->ToCell_3dR().origin = rOrigin;
        memcpy (el->ToCell_3dR().transform, &cellt, sizeof(el->ToCell_3d().transform));
        }
    else
        {
        el->ToCell_2dR().origin.Init (rOrigin);
        cellt.GetRowValuesXY(&el->ToCell_2dR().transform[0][0]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::TransformCellHeader (EditElementHandleR eeh, TransformCR trans)
    {
    transformCellHeader (eeh, trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Type2Handler::_OnTransform
(
EditElementHandleR elemHandle,
TransformInfoCR trans
)
    {
    BeAssert (CELL_HEADER_ELM == elemHandle.GetLegacyType());

    TransformCellHeader (elemHandle, *trans.GetTransform());

    return T_Super::_OnTransform (elemHandle, trans);  // transforms children
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void    convertCellHeaderTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnElementCP origP = eeh.GetElementCP ();
    DgnV8ElementBlank   elm;

    origP->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Cell_3d) / 2);

    Cell_3d& cell = (Cell_3d&) elm;
    cell.componentCount   = origP->ToCell_2d().componentCount;
    cell.classMap         = origP->ToCell_2d().classMap;
    cell.rnglow.x         = origP->ToCell_2d().rnglow.x;
    cell.rnglow.y         = origP->ToCell_2d().rnglow.y;
    cell.rnglow.z         = 0.0;
    cell.rnghigh.x        = origP->ToCell_2d().rnghigh.x;
    cell.rnghigh.y        = origP->ToCell_2d().rnghigh.y;
    cell.rnghigh.z        = 0.0;
    cell.transform[0][0]  = origP->ToCell_2d().transform[0][0];
    cell.transform[0][1]  = origP->ToCell_2d().transform[0][1];
    cell.transform[0][2]  = 0.0;
    cell.transform[1][0]  = origP->ToCell_2d().transform[1][0];
    cell.transform[1][1]  = origP->ToCell_2d().transform[1][1];
    cell.transform[1][2]  = 0.0;
    cell.transform[2][0]  = 0.0;
    cell.transform[2][1]  = 0.0;
    cell.transform[2][2]  = 1.0;

    cell.flags.isAnnotation               = origP->ToCell_2d().flags.isAnnotation;
    cell.flags.matchSourceAnnotationSize  = origP->ToCell_2d().flags.matchSourceAnnotationSize;
    cell.flags.matchSourceDimValue        = origP->ToCell_2d().flags.matchSourceDimValue;
    cell.flags.matchSourceMultilineOffset = origP->ToCell_2d().flags.matchSourceMultilineOffset;
    cell.flags.reserved                   = origP->ToCell_2d().flags.reserved;

    DataConvert::Points2dTo3d (&cell.origin, &origP->ToCell_2d().origin, 1, elevation);

    ElementUtil::CopyAttributes (&elm, origP);

    eeh.ReplaceElement (&elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    convertCellHeaderTo3d (eeh, elevation);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setFlattenMatrix (EditElementHandleR eeh, RotMatrixCR rMatrix)
    {
    int         dataEntries = (sizeof (rMatrix) / sizeof (double));

    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        UInt16      linkageKey;
        UInt32      numEntries;
        double*     doubleData = (double*) ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_FlattenTransform != linkageKey)
            continue;

        if (numEntries != dataEntries) // bad linkage...delete and add a new one...
            {
            eeh.RemoveElementLinkage (li);

            break;
            }

        memcpy (doubleData, &rMatrix.form3d[0][0], dataEntries * sizeof (double));

        return;
        }

    ElementLinkageUtil::AppendDoubleArrayData (eeh, DOUBLEARRAY_LINKAGE_KEY_FlattenTransform, dataEntries, &rMatrix.form3d[0][0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void     convertCellHeaderTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    RotMatrix   flattenMatrix;

    CellUtil::ExtractRotation (flattenMatrix, eeh);
    transformCellHeader (eeh, flattenTrans);

    DgnElementCP origP = eeh.GetElementCP ();
    DgnV8ElementBlank elm;

    origP->CopyTo (elm);

    elm.SetSizeWordsNoAttributes(sizeof (Cell_2d) / 2);

    Cell_2d& cell = (Cell_2d&) elm;
    cell.componentCount   = origP->ToCell_3d().componentCount;
    cell.classMap         = origP->ToCell_3d().classMap;

    cell.flags.isAnnotation               = origP->ToCell_3d().flags.isAnnotation;
    cell.flags.matchSourceAnnotationSize  = origP->ToCell_3d().flags.matchSourceAnnotationSize;
    cell.flags.matchSourceDimValue        = origP->ToCell_3d().flags.matchSourceDimValue;
    cell.flags.matchSourceMultilineOffset = origP->ToCell_3d().flags.matchSourceMultilineOffset;
    cell.flags.reserved                   = origP->ToCell_3d().flags.reserved;

    bool        addFlattenMatrix = false;
    double      xAxisAngle, yAxisSkewAngle, xScale, yScale, zScale;
    RotMatrix   postTransformMatrix;

    CellUtil::ExtractRotation (postTransformMatrix, eeh);

    if (bsiRotMatrix_isXYRotationSkewAndScale (&postTransformMatrix, &xAxisAngle, &yAxisSkewAngle, &xScale, &yScale, &zScale))
        {
        cell.rnglow.x  = origP->ToCell_3d().rnglow.x;
        cell.rnglow.y  = origP->ToCell_3d().rnglow.y;
        cell.rnghigh.x = origP->ToCell_3d().rnghigh.x;
        cell.rnghigh.y = origP->ToCell_3d().rnghigh.y;

        postTransformMatrix.GetRowValuesXY(&cell.transform[0][0]);

        flattenMatrix.InitIdentity (); // NOTE: Needed for replace cell...
        addFlattenMatrix = true;
        }
    else
        {
        // recompute range diagonal...
        cell.rnglow.x  = 0.0;
        cell.rnglow.y  = 0.0;
        cell.rnghigh.x = 0.0;
        cell.rnghigh.y = 0.0;

        DPoint3d    scaleVec;

        LegacyMath::RMatrix::GetColumnScaleVector (&flattenMatrix, &scaleVec, &flattenMatrix);

        /* ------------------------------------------------------------------------------
           Converting the rotation matrix is a bit tricky, and we definately can't always
           convert and arbitrary 3D to 2D here.  The solution is to get the scales correct,
           on an identity matrix and then set the pre-flatten matrix as a linkage.  This will
           allow a complete reconstruction of the cell.
        ------------------------------------------------------------------------------- */
        cell.transform[0][1]  = 0.0;
        cell.transform[1][0]  = 0.0;
        cell.transform[0][0]  = scaleVec.x;
        cell.transform[1][1]  = scaleVec.y;

        if (!flattenMatrix.IsIdentity())
            addFlattenMatrix = true;
        }

    DataConvert::Points3dTo2d (&cell.origin, &origP->ToCell_3d().origin, 1);

    ElementUtil::CopyAttributes (&elm, origP);

    eeh.ReplaceElement (&elm);

    if (addFlattenMatrix)
        setFlattenMatrix (eeh, flattenMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            Type2Handler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    convertCellHeaderTo2d (eeh, flattenTrans, flattenDir);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);

    // If range diagonal couldn't be preserved we need to recompute it...
    DPoint3d    rangeLo, rangeHi;

    rangeLo.x = eeh.GetElementCP ()->ToCell_2d().rnglow.x;
    rangeLo.y = eeh.GetElementCP ()->ToCell_2d().rnglow.y;
    rangeLo.z = 0.0;

    rangeHi.x = eeh.GetElementCP ()->ToCell_2d().rnghigh.x;
    rangeHi.y = eeh.GetElementCP ()->ToCell_2d().rnghigh.y;
    rangeHi.z = 0.0;

    if (0.0 != rangeLo.distance (&rangeHi))
        return;

    ValidateElementRange (eeh);
    ValidateRangeDiagonal (eeh);

    double      scaleX = eeh.GetElementCP ()->ToCell_2d().transform[0][0];
    double      scaleY = eeh.GetElementCP ()->ToCell_2d().transform[1][1];

    if (0.0 != scaleX)
        {
        eeh.GetElementP ()->ToCell_2dR().rnglow.x  /= scaleX;
        eeh.GetElementP ()->ToCell_2dR().rnghigh.x /= scaleX;
        }

    if (0.0 != scaleY)
        {
        eeh.GetElementP ()->ToCell_2dR().rnglow.y  /= scaleY;
        eeh.GetElementP ()->ToCell_2dR().rnghigh.y /= scaleY;
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalCellHeaderHandler::PickOrigin (ElementHandleCR thisElm, ViewContextR context)
    {
    IPickGeom*  pick = context.GetIPickGeom ();

    if (NULL == pick)
        return;

    // Cell origin is only meaningful for user-defined cells...
    if (thisElm.GetElementCP ()->IsHole())
        return;

    DPoint3d    origin;

    _GetTransformOrigin (thisElm, origin);
    pick->SetHitPriorityOverride (HitPriority::CellOrigin);
    context.GetIDrawGeom().DrawPointString3d (1, &origin, NULL);
    pick->SetHitPriorityOverride (HitPriority::Highest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NormalCellHeaderHandler::_WantFastCell (DgnElementCP header)
    {
    return header->ToCell_2d().componentCount > T_HOST.GetGraphicsAdmin()._GetFastCellThreshold ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void     drawFastCell (ElementHandleCR thisElm, ViewContextR context)
    {
    DPoint3d    origin;
    RotMatrix   rMatrix;

    CellUtil::ExtractOrigin (origin, thisElm);
    CellUtil::ExtractRotation (rMatrix, thisElm);

    DPoint3d    shp[8];
    DgnElementCP elm = thisElm.GetElementCP ();

    if (elm->Is3d())
        {
        shp[0].x = shp[3].x = shp[4].x = shp[5].x = elm->ToCell_3d().rnglow.x;
        shp[1].x = shp[2].x = shp[6].x = shp[7].x = elm->ToCell_3d().rnghigh.x;

        shp[0].y = shp[1].y = shp[4].y = shp[7].y = elm->ToCell_3d().rnglow.y;
        shp[2].y = shp[3].y = shp[5].y = shp[6].y = elm->ToCell_3d().rnghigh.y;

        shp[0].z = shp[1].z = shp[2].z = shp[3].z = elm->ToCell_3d().rnglow.z;
        shp[4].z = shp[5].z = shp[6].z = shp[7].z = elm->ToCell_3d().rnghigh.z;
        }
    else
        {
        memset (shp, 0, 8 * sizeof (DPoint3d));

        shp[0].x = shp[3].x = shp[7].x = elm->ToCell_2d().rnglow.x;
        shp[1].x = shp[2].x = shp[5].x = elm->ToCell_2d().rnghigh.x;

        shp[0].y = shp[1].y = shp[4].y = elm->ToCell_2d().rnglow.y;
        shp[2].y = shp[3].y = shp[6].y = elm->ToCell_2d().rnghigh.y;

        shp[4].x = shp[6].x = ((shp[0].x + shp[1].x) / 2.0);
        shp[5].y = shp[7].y = ((shp[4].y + shp[6].y) / 2.0);
        }

    for (int i=0; i<8; i++)
        {
        rMatrix.multiply (&shp[i]);
        shp[i].sumOf (&shp[i], &origin);
        }

    context.DrawBox (shp, elm->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalCellHeaderHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    if (DrawPurpose::Pick == context.GetDrawPurpose())
        PickOrigin (thisElm, context);

    if (context.GetViewFlags() && context.GetViewFlags()->fast_cell && _WantFastCell (thisElm.GetElementCP()))
        return drawFastCell (thisElm, context);

    ElemHeaderOverrides ovr;
    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (*this);

    if (extension && extension->_GetElemHeaderOverrides (thisElm, ovr))
        {
        ViewContext::ContextMark mark (context, thisElm); // restored in destructor!

        context.PushOverrides (&ovr);
        VisitChildren (thisElm, context);

        return;
        }

    VisitChildren (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       NormalCellHeaderHandler::_DrawCut (ElementHandleCR thisElm, ICutPlaneR cutPlane, ViewContextR context)
//    {
//    VisitChildren (thisElm, context);
//
//    return SUCCESS;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalCellHeaderHandler::VisitChildren (ElementHandleCR thisElm, ViewContextR context)
    {
    // restored in destructor!
    ViewContext::ContextMark mark (context, thisElm);
    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (*this);

    if (extension)
        extension->_PushDisplayEffects (thisElm, context);

    UseChildren (thisElm, context, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::SetPointCell (EditElementHandleR eeh, bool isPointCell)
    {
    DgnElementR  el = *eeh.GetElementP ();

    if (CELL_HEADER_ELM != el.GetLegacyType())
        return ERROR;

    el.SetViewIndependent(isPointCell);
    el.SetSnappable(!isPointCell);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalCellHeaderHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    // NOTE: Point cells have uniform symbology...
    if (_IsPointCell (thisElm))
        {
        ChildElemIter   firstChild (thisElm, ExposeChildrenReason::Count);

        if (firstChild.IsValid ())
            {
            DisplayHandlerP dHandler = firstChild.GetDisplayHandler ();

            if (NULL != dHandler)
                {
                dHandler->GetElemDisplayParams (firstChild, params, wantMaterials);

                return;
                }
            }
        }

    T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    // Persistent elements have valid ranges...
    if (NULL == elHandle.PeekElementDescrCP ())
        return SUCCESS;

    if (SUCCESS != T_Super::_ValidateElementRange (elHandle))
        return ERROR;

    if (_IsPointCell (elHandle) || mdlElement_attributePresent (elHandle.GetElementP (), LINKAGEID_ClipBoundary, NULL))
        ValidateViewIndependentElementRange (elHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NormalCellHeaderHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR trans)
    {
    // translation+scaling is ok, rotation/mirror not ok...
    if (_IsPointCell (elemHandle))
        {
        if (!trans.GetTransform ()->isTranslate (NULL))
            return false; // disallow rotation, mirror, or non-uniform scale...
        }

    // Call IsTransformGraphics on children and return false status if any...
    ChildElemIter   childIter (elemHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid ())
        return true;

    for (; childIter.IsValid(); childIter = childIter.ToNext ())
        {
        if (!childIter.GetHandler().IsTransformGraphics (childIter, trans))
            return false;
        }

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NormalCellHeaderHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    if (SupportOperation::CellUnGroup == stype && eh)
        {
        WChar cellName[MAX_CELLNAME_LENGTH];

        // Only drop orphan cells without a name...
        return (eh->GetElementCP ()->IsHole() && SUCCESS != _ExtractName (cellName, MAX_CELLNAME_LENGTH, *eh));
        }

    return T_Super::_IsSupportedOperation (eh, stype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NormalCellHeaderHandler::_ExposeChildren (ElementHandleCR el, ExposeChildrenReason reason)
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
* @bsimethod                                    Keith.Bentley                   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NormalCellHeaderHandler::_OnChildrenModified (EditElementHandleR elHandle, ExposeChildrenReason reason)
    {
    ValidateRangeDiagonal (elHandle);

    return T_Super::_OnChildrenModified (elHandle, reason);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::SetName (EditElementHandleR eeh, WCharCP cellName)
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
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::SetDescription (EditElementHandleR eeh, WCharCP descr)
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
BentleyStatus   NormalCellHeaderHandler::AdjustScale (EditElementHandleR eeh, double scale)
    {
    DgnElementP  elemP = eeh.GetElementP ();
    RotMatrix   rMatrix;
    Transform   scaleTrans;

    /* Apply scale factor to cell rot/scale matrix */
    CellUtil::ExtractRotation (rMatrix, eeh);

    scaleTrans.ScaleMatrixColumns (Transform::FromIdentity (),  scale,  scale,  scale);
    rMatrix.InitProduct(scaleTrans, rMatrix);

    /* Apply 1.0/scale to range diag */
    scaleTrans.ScaleMatrixColumns (Transform::FromIdentity (),  1.0/scale,  1.0/scale,  1.0/scale);

    if (elemP->Is3d())
        {
        scaleTrans.Multiply (&elemP->ToCell_3dR().rnglow,  2);

        memcpy (elemP->ToCell_3dR().transform, &rMatrix, sizeof (elemP->ToCell_3d().transform));
        }
    else
        {
        DRange3d   rangeDiag;

        memset (&rangeDiag, 0, sizeof (rangeDiag));

        rangeDiag.low.x = elemP->ToCell_2d().rnglow.x;
        rangeDiag.low.y = elemP->ToCell_2d().rnglow.y;

        rangeDiag.high.x = elemP->ToCell_2d().rnghigh.x;
        rangeDiag.high.y = elemP->ToCell_2d().rnghigh.y;

        scaleTrans.Multiply (&rangeDiag.low,  2);

        elemP->ToCell_2dR().rnglow.x  = rangeDiag.low.x;
        elemP->ToCell_2dR().rnglow.y  = rangeDiag.low.y;

        elemP->ToCell_2dR().rnghigh.x = rangeDiag.high.x;
        elemP->ToCell_2dR().rnghigh.y = rangeDiag.high.y;

        rMatrix.GetRowValuesXY(&elemP->ToCell_2dR().transform[0][0]);
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::SetCellRange (EditElementHandleR eeh)
    {
    Type2Handler*   cellHandler = dynamic_cast <Type2Handler*> (&eeh.GetHandler ());

    if (!cellHandler)
        return ERROR;

    return cellHandler->SetRange (eeh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::SetCellOriginAndRange (EditElementHandleR eeh)
    {
    Type2Handler*   cellHandler = dynamic_cast <Type2Handler*> (&eeh.GetHandler ());

    if (!cellHandler)
        return ERROR;

    return cellHandler->SetOriginAndRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::AddChildElement (EditElementHandleR eeh, EditElementHandleR childEeh)
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
BentleyStatus   NormalCellHeaderHandler::AddChildComplete (EditElementHandleR eeh)
    {
    if (!eeh.IsValid ())
        return ERROR;

    // NOTE: Only set origin to range center if it is an orphan cell (and not an assoc region!)...
    if (CellUtil::IsAnonymous (eeh) && !mdlElement_attributePresent (eeh.GetElementCP (), LINKAGEID_AssocRegion, NULL))
        {
        if (SUCCESS != NormalCellHeaderHandler::SetCellOriginAndRange (eeh))
            return ERROR;
        }
    else
        {
        if (SUCCESS != NormalCellHeaderHandler::SetCellRange (eeh))
            return ERROR;
        }

    // Update component count...
    eeh.GetElementDescrP()->Validate ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void NormalCellHeaderHandler::CreateOrphanCellElement (EditElementHandleR eeh, WCharCP cellName, bool is3d, DgnModelR modelRef)
    {
    DgnV8ElementBlank   out;

    memset (&out, 0, sizeof (Cell_3d));

    ElementUtil::SetRequiredFields (out, CELL_HEADER_ELM, LevelId(), false, (ElementUtil::ElemDim) is3d);

    out.SetIsHole(true); // h bit is set for orphan cell...
    out.SetSnappable(true);// s bit is set for non-point cell...

    ElementUtil::InitScanRangeForUnion (out.GetRangeR(), is3d);

    UInt32      elmSize;

    if (is3d)
        {
        double  idtrans_3d[3][3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };

        memcpy (&out.ToCell_3dR().transform[0][0], &idtrans_3d[0][0], sizeof (idtrans_3d));
        elmSize = sizeof (Cell_3d);
        }
    else
        {
        double  idtrans_2d[2][2] = { {1.0, 0.0}, {0.0, 1.0} };

        memcpy (&out.ToCell_2dR().transform[0][0], &idtrans_2d[0][0], sizeof (idtrans_2d));
        elmSize = sizeof (Cell_2d);
        }

    out.SetSizeWordsNoAttributes(elmSize/2);

    MSElementDescrP dscr = new MSElementDescr(out, modelRef);
    dscr->SetElementHandler(&GetInstance());
    eeh.SetElementDescr(dscr, false);

    if (cellName)
        SetName (eeh, cellName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalCellHeaderHandler::CreateCellElement
(
EditElementHandleR  eeh,
WCharCP             cellName,
DPoint3dCR          origin,
RotMatrixCR         rMatrix,
bool                is3d,
DgnModelR        modelRef
)
    {
    if (NULL == cellName)
        { BeAssert (false); return; }

    NormalCellHeaderHandler::CreateOrphanCellElement (eeh, cellName, is3d, modelRef);

    eeh.GetElementP()->ToCell_2dR().SetIsHole(false); // h bit is NOT set for named cell...

    Transform   transform;
    transform.initFrom (&rMatrix, &origin);
    transformCellHeader (eeh, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NormalCellHeaderHandler::CreateGroupCellElement (EditElementHandleR eeh, ElementAgendaR agenda, WCharCP cellName)
    {
    if (agenda.IsEmpty ())
        return ERROR;

    EditElementHandleP curr = agenda.GetFirstP ();
    EditElementHandleP end  = curr + agenda.GetCount ();

    bool        addedChild = false;

    eeh.Invalidate ();

    for (; curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;

        if (!eeh.IsValid ())
            NormalCellHeaderHandler::CreateOrphanCellElement (eeh, cellName, DisplayHandler::Is3dElem (curr->GetElementCP ()), *curr->GetDgnModelP ());

        if (SUCCESS != NormalCellHeaderHandler::AddChildElement (eeh, *curr))
            continue;

        addedChild = true;
        }

    if (!addedChild)
        return ERROR;

    return NormalCellHeaderHandler::AddChildComplete (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus NormalCellHeaderHandler::_OnGeoCoordinateReprojection
(
EditElementHandleR                  source,
IGeoCoordinateReprojectionHelper&   reprojectionHelper,
bool                                inChain
)
    {
    IGeoCoordinateReprojectionSettingsP settings     = reprojectionHelper.GetSettings();
    bool                                doIndividual = reprojectionHelper.ShouldStroke(source, settings->DoCellElementsIndividually());

    // If the "DoCellElementsIndividually" setting indicates, we apply the geographic reprojection to each child element.
    // If it's an orphan cell (usually created from the "Group" command) we apply the geographic reprojection to each child element.
    // Otherwise, it's a normal cell, and we apply the best fit linear transform to the entire cell.
    DPoint3d    origin;

    _GetTransformOrigin (source, origin);

    if (doIndividual || CellUtil::IsAnonymous (source))
        {
        ReprojectStatus status = REPROJECT_Success;

        for (ChildEditElemIter childIter (source, ExposeChildrenReason::Count); childIter.IsValid (); )
            {
            ChildEditElemIter   nextChild = childIter.ToNext();
            DisplayHandlerP     dispHandler;

            if (NULL != (dispHandler = childIter.GetDisplayHandler ()))
                {
                ReprojectStatus childStatus;

                if (REPROJECT_Success != (childStatus = dispHandler->GeoCoordinateReprojection (childIter, reprojectionHelper, inChain)))
                    status = childStatus;

                dispHandler->ValidateElementRange (childIter);
                }

            childIter = nextChild;
            }

        // transform the origin and rotation that are stored cell header.
        TransformInfo   transform;

        reprojectionHelper.GetLocalTransform (&transform.GetTransformR(), origin, NULL, true, true);
        TransformCellHeader (source, *transform.GetTransform());

        return status;
        }

    TransformInfo   transform;

    ReprojectStatus status = reprojectionHelper.GetLocalTransform (&transform.GetTransformR(), origin, NULL, settings->RotateCells(), settings->ScaleCells());

    if ((REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status))
        return status;

    // if can't transform, don't change what we have.
    if (SUCCESS != _ApplyTransform (source, transform))
        return REPROJECT_NoChange;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NormalCellHeaderHandlerPathEntryExtension::_GetElemHeaderOverrides (ElementHandleCR thisElm, ElemHeaderOverridesR ovr)
    {
    // The following code is to handle cells that contain level and symbology that may be inherited
    // by components that use have "ByCell" attributes.  This is not the norm - but it is used by
    // DWG proxies and may be something we want to support in the future.  I don't do this for
    // for the default case (0 == level) as an optimization.  (TR# 162837, TR# 155845).
    DgnElementCP el = thisElm.GetElementCP ();

    if (0 == el->GetLevelValue())
        return false;

    LineStyleParams styleParams;

    LineStyleLinkageUtil::ExtractParams (&styleParams, el);
    ovr.Init (NULL, LevelId(el->GetLevel()), 0, el->GetDisplayPriority(), (DgnElementClass) DgnElementClass::Primary, &el->GetSymbology(), &styleParams);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void NormalCellHeaderHandlerPathEntryExtension::_PushDisplayEffects (ElementHandleCR thisElm, ViewContextR context)
    {
    ClipVectorPtr clip;

    if (SUCCESS == CellUtil::ClipFromLinkage (clip, thisElm, NULL) && clip.IsValid())
        context.PushClip (*clip);

    NormalCellHeaderHandler& instance = NormalCellHeaderHandler::GetInstance ();

    if (!instance.IsPointCell (thisElm))
        return;

    DPoint3d  origin;

    instance.GetTransformOrigin (thisElm, origin);
    context.PushViewIndependentOrigin (&origin);
    }

