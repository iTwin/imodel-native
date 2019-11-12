/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __CIVILBASEGEOMETRYINTERNAL_H__
#define __CIVILBASEGEOMETRYINTERNAL_H__

#include <CivilBaseGeometry/CivilBaseGeometry.h>

USING_NAMESPACE_BENTLEY_CIVILGEOMETRY

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_CIVILBASEGEOMETRY    "CivilBaseGeometry"
#if defined (ANDROID)
    #include <android/log.h>
    #define CIVILBASEGEOMETRY_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOGGER_NAMESPACE_CIVILBASEGEOMETRY, __VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_CIVILBASEGEOMETRY, __VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_CIVILBASEGEOMETRY, __VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_CIVILBASEGEOMETRY, __VA_ARGS__);
#else
    #include <Logging/bentleylogging.h>
    #define CIVILBASEGEOMETRY_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_CIVILBASEGEOMETRY))
    #define CIVILBASEGEOMETRY_LOGD(...)           CIVILBASEGEOMETRY_LOG.debugv (__VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGI(...)           CIVILBASEGEOMETRY_LOG.infov (__VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGW(...)           CIVILBASEGEOMETRY_LOG.warningv (__VA_ARGS__);
    #define CIVILBASEGEOMETRY_LOGE(...)           CIVILBASEGEOMETRY_LOG.errorv (__VA_ARGS__);
#endif


#endif // __CIVILBASEGEOMETRYINTERNAL_H__

