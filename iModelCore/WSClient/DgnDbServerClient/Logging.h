/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/Logging.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <DgnDbServer/DgnDbServerCommon.h>
#include <Logging/bentleylogging.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_DGNDBSERVER))