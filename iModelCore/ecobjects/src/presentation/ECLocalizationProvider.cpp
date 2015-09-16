/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ECLocalizationProvider.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString         ECPresentationLocalizationProvider::GetString (WCharCP rscFileName, unsigned int tableId, unsigned int rscId)
    {
    return _GetString (rscFileName, tableId, rscId);
    }
