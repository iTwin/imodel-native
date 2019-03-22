/*--------------------------------------------------------------------------------------+
|
|     $Source: Tasks/ThreadingLogging.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Logging/bentleylogging.h>
#include <Bentley/Tasks/Tasks.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_BENTLEY_TASKS))
