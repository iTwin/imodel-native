/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/RelativeOffsetAssociation.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatformInternal/DgnHandlers/RelativeOffsetAssociation.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_LinkageIter, typename T_ElementHandleType>
T_LinkageIter   T_GetRelativeAssociationVectorLinkage (T_ElementHandleType noteCell)
    {
    T_LinkageIter findIt = noteCell.BeginElementLinkages();
    for (; findIt != noteCell.EndElementLinkages(); ++findIt)
        {
        UInt16          linkageKey;
        UInt32          numEntries;
        double const*   doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (findIt, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_OriginRelativeOffset != linkageKey)
            continue;
        
        break;
        }

    return findIt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConstElementLinkageIterator  RelativeOffsetAssociation::GetConstRelativeAssociationVectorLinkage(ElementHandleCR element)
    {
    return T_GetRelativeAssociationVectorLinkage<ConstElementLinkageIterator, ElementHandleCR> (element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ElementLinkageIterator      RelativeOffsetAssociation::GetRelativeAssociationVectorLinkage(EditElementHandleR element)
    {
    return T_GetRelativeAssociationVectorLinkage<ElementLinkageIterator, EditElementHandleR> (element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RelativeOffsetAssociation::IsOfRelativeOffsetAssociationType(Handler& handler)
    {
    IAssocPointRootsChangedExtension* extension = IAssocPointRootsChangedExtension::Cast (handler);
    if (NULL == extension)
        return false;

    return NULL != dynamic_cast<RelativeAssocPointRootsChangedExtension*> (extension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RelativeOffsetAssociation::RemoveOffsetAssociation (EditElementHandleR element)
    {
    if (!IsOfRelativeOffsetAssociationType(element.GetHandler()))
        return ERROR;

    if (SUCCESS != AssociativePoint::RemoveAllAssociations(element))
        return ERROR;
        
    ElementLinkageIterator iter = GetRelativeAssociationVectorLinkage(element);
    if (iter != element.EndElementLinkages() && SUCCESS != element.RemoveElementLinkage(iter))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RelativeOffsetAssociation::AddOffsetAssociation (EditElementHandleR element, ElementHandleCR targetElement, AssocPoint const& assoc, DPoint3dCR origin)
    {
    if (!IsOfRelativeOffsetAssociationType(element.GetHandler()))
        return ERROR;

    ConstElementLinkageIterator iter = GetRelativeAssociationVectorLinkage(element);
    if (iter != element.EndElementLinkages() && SUCCESS != RemoveOffsetAssociation(element))
        return ERROR;

    DPoint3d targetPoint;
    if (SUCCESS != AssociativePoint::GetPoint (&targetPoint, assoc, targetElement.GetDgnModelP()))
        return ERROR;
    
    //Store targetPoint - cellorigin
    DVec3d offset = DVec3d::FromStartEnd(origin, targetPoint);
    double offVals [3] = {offset.x, offset.y, offset.z};
    if (SUCCESS != ElementLinkageUtil::AppendDoubleArrayData(element, DOUBLEARRAY_LINKAGE_KEY_OriginRelativeOffset, 3, offVals))
        return ERROR;
    
    StatusInt status = SUCCESS;
    if (SUCCESS != (status = AssociativePoint::InsertPoint (element, assoc, 0, 1)))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RelativeOffsetAssociation::GetOffsetValue(ElementHandleCR element, DVec3dR offset)
    {
    if (!IsOfRelativeOffsetAssociationType(element.GetHandler()))
        return ERROR;

    ConstElementLinkageIterator iter = GetConstRelativeAssociationVectorLinkage(element);
    if (iter == element.EndElementLinkages())
        return ERROR;

    UInt16 linkageKey;
    UInt32 numEntries;
    double const* offsetData = ElementLinkageUtil::GetDoubleArrayDataCP(iter, linkageKey, numEntries);
    if (3 != numEntries)
        return ERROR;

    offset = DVec3d::FromArray(offsetData);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RelativeAssocPointRootsChangedExtension::_GetPoint (ElementHandleCR element, DPoint3dR point, int index)
    {
    if (0 != index)
        return ERROR;
        
    DVec3d offset;
    if (SUCCESS != RelativeOffsetAssociation::GetOffsetValue(element, offset))
        return ERROR;

    if (SUCCESS != _GetOffsetReferenceLocation(point, element))
        return ERROR;

    point.Add(offset);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RelativeAssocPointRootsChangedExtension::_SetPoint (EditElementHandleR eeh, DPoint3dCR point, int index)
    {
    DPoint3d    origin;

    if (SUCCESS != _GetPoint (eeh, origin, index))
        return ERROR;

    DVec3d      offset;
    offset.DifferenceOf (point, origin);
        
    Transform   transform;
    transform.InitFrom (offset);

    TransformInfo tInfo (transform);

    return eeh.GetHandler().ApplyTransform (eeh, tInfo);
    }

StatusInt IAssocPointRootsChangedExtension::_GetRootComparePoint (DPoint3dR, RootsChangedState&, int) {return ERROR;}                          // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnRootChanged (EditElementHandleR, DPoint3dCR, RootsChangedState&, int) {return ERROR;}           // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnDependentChanged (EditElementHandleR, DPoint3dCR, RootsChangedState&, int) {return ERROR;}      // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnProcessRoot (EditElementHandleR, RootsChangedState&, int) {return ERROR;}                       // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnUpdateElementRootsFinish (EditElementHandleR, RootsChangedState&) {return ERROR;}               // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnUpdateElementRoots (EditElementHandleR, RootsChangedState&) {return ERROR;}                     // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
StatusInt IAssocPointRootsChangedExtension::_OnProcessRootsChanged (ElementHandleCR, DependencyLinkageCR, UInt8*, UInt8, bool) {return ERROR;} // stubbed out in graphite (are implemented in depcallback.cpp in Vancouver)
