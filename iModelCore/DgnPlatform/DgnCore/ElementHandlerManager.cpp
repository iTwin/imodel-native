/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementHandlerManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandlerManager::OnHostInitialize ()
    {
    }

StatusInt ElementHandlerLoader::_LoadElementHandler (ElementHandlerId const& hid) {return ERROR;}
StatusInt ElementHandlerLoader::_LoadXAttributeHandler (XAttributeHandlerId const& hid) {return ERROR;}
