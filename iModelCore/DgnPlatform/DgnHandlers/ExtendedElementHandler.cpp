/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ExtendedElementHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_GetTypeName (WStringR string, UInt32 desiredLength)
    {
    string = DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_EXTENDED_ELM);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2009.
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength)
    {
    ElementHandle::XAttributeIter iterator (el, XAttributeHandlerId (XATTRIBUTEID_XGraphicsName, 0), 0);

    if (!iterator.IsValid())
        return _GetTypeName (string, desiredLength);

    UInt32 length = std::min ((UInt32) (iterator.GetSize() / sizeof(Utf16Char)), desiredLength);
    Utf16CP data = (Utf16CP) iterator.PeekData();
    BeStringUtilities::Utf16ToWChar (string, data, length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    XGraphicsContainer::QueryProperties (eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    XGraphicsContainer::EditProperties (eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    XGraphicsContainer::ConvertTo3d (eeh, elevation);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    XGraphicsContainer::ConvertTo2d (eeh, flattenTrans, flattenDir);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ExtendedElementHandler::_OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo)
    {
    XGraphicsContainer      container;
    StatusInt               status;

    if (SUCCESS !=  container.ExtractFromElement (elHandle))
        return SUCCESS;

    if (SUCCESS == (status = container.OnTransform (tInfo)))
        status = container.AddToElement (elHandle);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedElementHandler::_IsRenderable (ElementHandleCR eh)
    {
    return XGraphicsContainer::IsRenderable (eh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static void copyDisplayAttribute (EditElementHandleR newElement, ElementHandleCR originalElement, int attributeId)
    {
    Display_attribute attribute;
    if (!mdlElement_displayAttributePresent (originalElement.GetElementCP(), attributeId, &attribute))
        return;

    ScopedArray <byte>  elmData (newElement.GetElementCP()->Size() + 2*sizeof (attribute));
    DgnElementP          tmpElement = (DgnElementP) elmData.GetData();

    newElement.GetElementCP()->CopyTo (*tmpElement);
    mdlElement_displayAttributeAdd (tmpElement, &attribute);
    newElement.ReplaceElement (tmpElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendedElementHandler::InitializeElement (EditElementHandleR eh, ElementHandleCP teh, DgnModelR model, bool is3d) 
    {
    DgnElementBlank<1000> elm;

    elm.SetLevel(LEVEL_DEFAULT_LEVEL_ID.GetValueUnchecked());
    elm.SetIs3d(is3d);
    elm.SetSizeWordsNoAttributes(sizeof(DgnElement)/2);

    MSElementDescrP edP = new MSElementDescr(elm, model);
    if (eh.PeekElementDescrCP ())
        eh.ReplaceElementDescr(edP);
    else
        eh.SetElementDescr(edP, false);

    edP->SetElementHandler(&GetInstance());

    if (NULL == teh || !teh->IsValid ())
        return;

    DgnElementR out = edP->ElementR();
    out.SetLevel(teh->GetElementCP()->GetLevelValue());
    out.GetSymbologyR() = teh->GetElementCP()->GetSymbology();
    out.SetElementClass(teh->GetElementCP()->GetElementClass());

    copyDisplayAttribute (eh, *teh, TRANSPARENCY_ATTRIBUTE); // added in graphite??
    copyDisplayAttribute (eh, *teh, FILL_ATTRIBUTE);        // added in graphite??
    copyDisplayAttribute (eh, *teh, GRADIENT_ATTRIBUTE);    // added in graphite??

    LineStyleParams  styleParams;
    if (SUCCESS == LineStyleLinkageUtil::GetParamsFromElement (&styleParams, teh->GetElementCP()))
        {
        size_t bufferSize = eh.GetElementP()->Size() + sizeof (StyleLink) + 8;
        DgnElementP newEle = (DgnElementP)_alloca(bufferSize);
        eh.GetElementP()->CopyTo(*newEle);
        LineStyleLinkageUtil::SetStyleParams(newEle, &styleParams);
        BeAssert(newEle->Size() < bufferSize);
        eh.ReplaceElement(newEle);
        }
    }

