/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ECResourceProvider.cpp $
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
IECNativeImagePtr ECPresentationResourceProvider::GetImage (ECImageKeyCR imageKey, DPoint2dCR size)
    {
    return _GetImage(imageKey, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationResourceProvider::GetString (WCharCP rscFileName, UInt tableId, UInt rscId)
    {
    return _GetString (rscFileName, tableId, rscId);
    }
