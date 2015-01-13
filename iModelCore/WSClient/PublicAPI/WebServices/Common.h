/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Common.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <BentleyApi/BentleyApi.h>

#ifdef __WEBSERVICES_DLL_BUILD__
    #define WS_EXPORT EXPORT_ATTRIBUTE
#else
    #define WS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_WEBSERVICES_NAMESPACE    BEGIN_BENTLEY_API_NAMESPACE namespace WebServices {
#define END_BENTLEY_WEBSERVICES_NAMESPACE      } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_WEBSERVICES    using namespace BentleyApi::WebServices;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
END_BENTLEY_WEBSERVICES_NAMESPACE