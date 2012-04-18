/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ecviewdefinition.cpp $
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
IAUIItemInfoCR  IECViewDefinition::GetUIInfo()
    {
    return _GetUIInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextP IECViewDefinition::GetDataContext()
    {
    return _GetDataContext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildDefinitions IECViewDefinition::GetChildDefinitions()
    {
    return _GetChildDefinitions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinitionPtr    IECViewDefinitionProvider::GetViewDefinition ()
    {
    return _GetViewDefinition();
    }