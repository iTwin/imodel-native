/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __ROADRAILALIGNMENTINTERNAL_H__
#define __ROADRAILALIGNMENTINTERNAL_H__

#include <RoadRailAlignment/RoadRailAlignment.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <CivilBaseGeometry/CivilBaseGeometryApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_BENTLEY_FORMATTING
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_CIVILGEOMETRY

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
