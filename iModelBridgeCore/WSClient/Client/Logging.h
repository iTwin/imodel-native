/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Logging.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Logging/bentleylogging.h>
#include <WebServices/WebServices.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_WSCLIENT))
