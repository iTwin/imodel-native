/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Handler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace std;

enum
    {
    HIGHEST_ELEM_TYPE                  = 128,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    ElementHandleCP    saveEh = context.GetCurrentElemHandleP ();

    context.SetCurrentElemHandleP (&eh);
    _QueryProperties (eh, context);
    context.SetCurrentElemHandleP (saveEh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    ElementHandleCP    saveEh = context.GetCurrentElemHandleP ();

    context.SetCurrentElemHandleP (&eeh);
    _EditProperties (eeh, context);
    XGraphicsContainer::EditProperties (eeh, context);
    context.SetCurrentElemHandleP (saveEh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::ConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    if (eeh.GetElementCP ()->Is3d())
        return;

    _OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void Handler::ConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    if (!eeh.GetElementCP ()->Is3d())
        return;

    _OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Handler::ApplyTransform (EditElementHandleR element, TransformInfoCR transform)
    {
    return _ApplyTransform (element, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void EditElementHandle::ChangeElementHandler(HandlerR handler)
    {
    GetElementDescrP()->SetElementHandler(&handler);
    }
