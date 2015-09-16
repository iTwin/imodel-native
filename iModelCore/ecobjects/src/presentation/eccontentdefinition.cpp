/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/eccontentdefinition.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECContentDefinitionPtr     IAUIContentServiceProvider::GetContent (IECPresentationViewDefinitionCR viewDef) const
    {
    return _GetContent(viewDef);
    }