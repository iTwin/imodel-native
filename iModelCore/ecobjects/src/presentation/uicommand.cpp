/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/uicommand.cpp $
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
BentleyStatus   UICommand::Execute (IECInstanceCP instance) const
    {
    UIPresentationManager::GetManager().JournalCmd (*this, instance);
    return _ExecuteCmd(instance);
    }