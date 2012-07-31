/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ECLocalizationProvider.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationLocalizationProvider::GetString (WCharCP rscFileName, UInt tableId, UInt rscId)
    {
    return _GetString (rscFileName, tableId, rscId);
    }
