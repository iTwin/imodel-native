/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreLog.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnCore that wish to log with DgnCore namespace

#include <Logging/bentleylogging.h>
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnCore"))
