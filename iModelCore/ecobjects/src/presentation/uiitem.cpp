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
UICommandCR     IAUICommandItem::GetCommand() const
    {
    return _GetCommand();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUICommandItemCP    IAUIItem::_GetAsCommandItem () const
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUICommandItemCP    IAUICommandItem::_GetAsCommandItem() const
    {
    return this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUICommandItemCP    IAUIItem::GetAsCommandItem () const
    {
    return _GetAsCommandItem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   IAUICommandItem::ExecuteAction () const
    {
    IECInstancePtr instance = GetDataInstance();
    return GetCommand().Execute (instance.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr    IAUIItem::GetDataInstance() const
    {
    ECValue val;
    if (ECOBJECTS_STATUS_Success != GetValue (val, L"Data"))
        return NULL;

    return val.GetStruct();
    }