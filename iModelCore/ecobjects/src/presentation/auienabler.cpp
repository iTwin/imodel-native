/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auienabler.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/uienabler.h>
#include <EcPresentation/UIPresentationMgr.h>
USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       UIECEnabler::GetUIClass (WCharCP className)
    {
    ECSchemaCR displaySchema = UIPresentationManager::GetManager().GetAUISchema();
    ECClassP classInstance   =  displaySchema.GetClassP(className);
    assert (NULL != classInstance);

    return *classInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemPtr     UIECEnabler::GetUIItem (IECInstanceP instanceData)
    {
    return _GetUIItem(instanceData);
    }