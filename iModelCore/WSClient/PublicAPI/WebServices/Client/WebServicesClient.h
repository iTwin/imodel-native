/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WebServicesClient.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/WebServices.h>

#define LOGGER_NAMESPACE_WSCLIENT "WSClient"

#ifdef __WSCLIENT_DLL_BUILD__
#define WSCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define WSCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
