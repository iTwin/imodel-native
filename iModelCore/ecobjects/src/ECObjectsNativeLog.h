/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsNativeLog.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"ECObjectsNative"))
