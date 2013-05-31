/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsNativeLog.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Logging/bentleylogging.h>

#define LOG (*Bentley::NativeLogging::LoggingManager::GetLogger (L"ECObjectsNative"))


