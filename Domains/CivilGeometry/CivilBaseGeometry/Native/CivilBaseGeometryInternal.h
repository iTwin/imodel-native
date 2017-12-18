/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/Native/CivilBaseGeometryInternal.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __CIVILBASEGEOMETRYINTERNAL_H__
#define __CIVILBASEGEOMETRYINTERNAL_H__

#include <CivilBaseGeometry/CivilBaseGeometry.h>
#include <Bentley/Bentley.h>
#include <Geom/GeomApi.h>

USING_NAMESPACE_BENTLEY_CIVILBASEGEOMETRY

#if 0 //&&AG not sure whether we should be logging errors or not at this level
    //-----------------------------------------------------------------------------------------
    // Logging macros
    //-----------------------------------------------------------------------------------------
    #define LOGGER_NAMESPACE_ROADRAILALIGNMENT    "RoadRailAlignment"
    #if defined (ANDROID)
        #include <android/log.h>
        #define ROADRAILALIGNMENT_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOGGER_NAMESPACE_ROADRAILALIGNMENT, __VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_ROADRAILALIGNMENT, __VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_ROADRAILALIGNMENT, __VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_ROADRAILALIGNMENT, __VA_ARGS__);
    #else
        #include <Logging/BentleyLogging.h>
        #define ROADRAILALIGNMENT_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ROADRAILALIGNMENT))
        #define ROADRAILALIGNMENT_LOGD(...)           ROADRAILALIGNMENT_LOG.debugv (__VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGI(...)           ROADRAILALIGNMENT_LOG.infov (__VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGW(...)           ROADRAILALIGNMENT_LOG.warningv (__VA_ARGS__);
        #define ROADRAILALIGNMENT_LOGE(...)           ROADRAILALIGNMENT_LOG.errorv (__VA_ARGS__);
    #endif
#endif

#endif // __CIVILBASEGEOMETRYINTERNAL_H__

