/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/Logging.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Logging/bentleylogging.h>

#define LOGGER_NAMESPACE_CWSCC "ConnectC"
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_CWSCC))
