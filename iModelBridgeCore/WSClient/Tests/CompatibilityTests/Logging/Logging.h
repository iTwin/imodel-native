/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Logging/bentleylogging.h>
#include <WebServices/WebServices.h>

#define LOGGER_NAMESPACE_WSCCTESTS "WSCCTests"

#define LOG (*NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_WSCCTESTS))
