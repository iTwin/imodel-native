/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityPlatformAPI.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#if defined (__REALITYPLATFORM_BUILD__)
    #define REALITYDATAPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
    #define REALITYDATAPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

#include "RealityDataProvider.h"



