/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/Render.h>
#include <folly/BeFolly.h>
#include <ThreeMx/ThreeMxApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_THREEMX

#define RENDER_LOGGING 1
#if defined (RENDER_LOGGING)
#   define THREADLOG (*NativeLogging::LoggingManager::GetLogger(DgnDb::GetThreadIdName()))
#   define DEBUG_PRINTF THREADLOG.debugv
#   define WARN_PRINTF THREADLOG.debugv
#   define ERROR_PRINTF THREADLOG.errorv
#else
#   define DEBUG_PRINTF(...)
#   define WARN_PRINTF(...)
#   define ERROR_PRINTF(...)
#endif
