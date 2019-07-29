/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_WSCACHE))

END_BENTLEY_WEBSERVICES_NAMESPACE
