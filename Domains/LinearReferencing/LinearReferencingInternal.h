/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __LINEARREFERENCINGINTERNAL_H__
#define __LINEARREFERENCINGINTERNAL_H__

#include <LinearReferencing/LinearReferencing.h>


USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING


//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_LINEARREFERENCING    "LinearReferencing"
#if defined (ANDROID)
    #include <android/log.h>
    #define LINEARREFERENCING_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_LINEARREFERENCING, __VA_ARGS__);
    #define LINEARREFERENCING_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_LINEARREFERENCING, __VA_ARGS__);
    #define LINEARREFERENCING_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_LINEARREFERENCING, __VA_ARGS__);
#else
    #include <Logging/BentleyLogging.h>
    #define LINEARREFERENCING_LOG         (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_LINEARREFERENCING))
    #define LINEARREFERENCING_LOGD(...)   LINEARREFERENCING_LOG.debugv (__VA_ARGS__);
    #define LINEARREFERENCING_LOGI(...)   LINEARREFERENCING_LOG.infov (__VA_ARGS__);
    #define LINEARREFERENCING_LOGW(...)   LINEARREFERENCING_LOG.warningv (__VA_ARGS__);
    #define LINEARREFERENCING_LOGE(...)   LINEARREFERENCING_LOG.errorv (__VA_ARGS__);
#endif

#endif
