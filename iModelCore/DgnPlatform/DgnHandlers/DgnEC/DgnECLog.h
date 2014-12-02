/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/DgnECLog.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnEC that wish to log with DgnEC namespace

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnEC"))
