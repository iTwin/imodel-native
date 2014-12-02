/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlersLog.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnHandlers that wish to log with DgnHandlers namespace

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnHandlers"))
