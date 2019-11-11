/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#ifndef __ROADRAILPHYSICALINTERNAL_H__
#define __ROADRAILPHYSICALINTERNAL_H__

#include <RoadRailPhysical/RoadRailPhysical.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL
USING_NAMESPACE_BENTLEY_ROADPHYSICAL
USING_NAMESPACE_BENTLEY_RAILPHYSICAL

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ROADRAILPHYSICAL    "RoadRailPhysical"
#if defined (ANDROID)
    #include <android/log.h>
    #define ROADRAILPHYSICAL_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_ROADRAILPHYSICAL, __VA_ARGS__);
    #define ROADRAILPHYSICAL_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_ROADRAILPHYSICAL, __VA_ARGS__);
    #define ROADRAILPHYSICAL_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_ROADRAILPHYSICAL, __VA_ARGS__);
#else
    #include <Logging/BentleyLogging.h>
    #define ROADRAILPHYSICAL_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ROADRAILPHYSICAL))
    #define ROADRAILPHYSICAL_LOGD(...)           ROADRAILPHYSICAL_LOG.debugv (__VA_ARGS__);
    #define ROADRAILPHYSICAL_LOGI(...)           ROADRAILPHYSICAL_LOG.infov (__VA_ARGS__);
    #define ROADRAILPHYSICAL_LOGW(...)           ROADRAILPHYSICAL_LOG.warningv (__VA_ARGS__);
    #define ROADRAILPHYSICAL_LOGE(...)           ROADRAILPHYSICAL_LOG.errorv (__VA_ARGS__);
#endif

#endif
