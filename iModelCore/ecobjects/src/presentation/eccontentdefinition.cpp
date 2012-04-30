/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/eccontentdefinition.cpp $
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
IECViewContentDefinitionPtr     IAUIContentServiceProvider::GetContent (IECViewDefinitionCR viewDef) const
    {
    return _GetContent(viewDef);
    }