/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auicommand.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   UICommand::Execute (IAUIDataContextCP instance) const
    {
    UIPresentationManager::GetManager().JournalCmd (*this, instance);
    return _ExecuteCmd(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<UICommandPtr>   IUICommandProvider::GetCommand (IAUIDataContextCR instance) const
    {
    return _GetCommand(instance);
    }