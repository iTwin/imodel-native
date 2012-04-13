/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiprovider.cpp $
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
IAUIItemPtr     IAUIProvider::GetUIItem (IAUIItemInfoCR itemInfo, IAUIDataContextCP instanceData)
    {
    return _GetUIItem(itemInfo, instanceData);
    }