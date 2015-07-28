/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/WebServicesCache.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/WebServices.h>

#define LOGGER_NAMESPACE_WSCACHE "WSCache"

#ifdef __WSCACHE_DLL_BUILD__
    #define WSCACHE_EXPORT EXPORT_ATTRIBUTE
#else
    #define WSCACHE_EXPORT IMPORT_ATTRIBUTE
#endif
