/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiprovider.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/uipresentationmgr.h>
#include <EcPresentation/uiprovider.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UICommandPtr    IUICommandProvider::GetCommand (IECInstanceCR instance) const
    {
    return _GetCommand(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerPtr    IAUIProvider::_GetEnabler (ECClassCR classInstance)
    {
    return _GetUIEnabler (classInstance).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UIECEnablerPtr  IAUIProvider::GetUIEnabler (ECClassCR classInstance)
    {
    return _GetUIEnabler(classInstance);
    }