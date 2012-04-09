/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/uiitem.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/uipresentationmgr.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UICommandCR     IUICommandItem::GetCommand() const
    {
    return _GetCommand();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommandItemCP    IUIItem::_GetAsCommandItem () const
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommandItemCP    IUICommandItem::_GetAsCommandItem() const
    {
    return this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommandItemCP    IUIItem::GetAsCommandItem () const
    {
    return _GetAsCommandItem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   IUICommandItem::ExecuteAction () const
    {
    IECInstancePtr instance = GetDataInstance();
    return GetCommand().Execute (instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr    IUIItem::GetDataInstance() const
    {
    ECValue val;
    if (ECOBJECTS_STATUS_Success != GetValue (val, L"Data"))
        return NULL;

    return val.GetStruct();
    }