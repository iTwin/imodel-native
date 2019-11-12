/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/WebServices.h>

#define LOGGER_NAMESPACE_WSCLIENT "WSClient"

#ifdef __WSCLIENT_DLL_BUILD__
#define WSCLIENT_EXPORT EXPORT_ATTRIBUTE
#else
#define WSCLIENT_EXPORT IMPORT_ATTRIBUTE
#endif
