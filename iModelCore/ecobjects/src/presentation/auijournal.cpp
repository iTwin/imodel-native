/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auijournal.cpp $
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
void            IJournalProvider::JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData)
    {
    return _JournalCmd (cmd, instanceData);
    }