/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/ConversionTools.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <RealityPlatform/WebResourceData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct ConversionTools
    {
public:
    //Fills a bvector with WebResourceData objects, created with data extracted from JSON
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToWebResourceData(Utf8CP data, bvector<WebResourceDataPtr>* outData);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE