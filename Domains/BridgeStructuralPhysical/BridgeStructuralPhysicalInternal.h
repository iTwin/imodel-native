/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#ifndef __BRIDGESTRUCTURALPHYSICALINTERNAL_H__
#define __BRIDGESTRUCTURALPHYSICALINTERNAL_H__

#include <BridgeStructuralPhysical/BridgeStructuralPhysical.h>

//-----------------------------------------------------------------------------------------
// Common Namespaces
//-----------------------------------------------------------------------------------------
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_BRIDGESTRUCTURALPHYSICAL
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_FORMS
USING_NAMESPACE_BENTLEY_STRUCTURAL
USING_NAMESPACE_BENTLEY_CIVILGEOMETRY

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_BRIDGESTRUCTURALPHYSICAL    "BridgeStructuralPhysical"
#if defined (ANDROID)
#include <android/log.h>
#define DGNCLIENTFX_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#define DGNCLIENTFX_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNCLIENTFX_CPP, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define BRIDGESTRUCTURALPHYSICAL_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_BRIDGESTRUCTURALPHYSICAL))
#define BRIDGESTRUCTURALPHYSICAL_LOGD(...)           BRIDGESTRUCTURALPHYSICAL_LOG.debugv (__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_LOGI(...)           BRIDGESTRUCTURALPHYSICAL_LOG.infov (__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_LOGW(...)           BRIDGESTRUCTURALPHYSICAL_LOG.warningv (__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_LOGE(...)           BRIDGESTRUCTURALPHYSICAL_LOG.errorv (__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_ASSERT_LOGD(...)    BeAssert(!__VA_ARGS__); BRIDGESTRUCTURALPHYSICAL_LOGD(__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_ASSERT_LOGI(...)    BeAssert(!__VA_ARGS__); BRIDGESTRUCTURALPHYSICAL_LOGI(__VA_ARGS__);
#define BRIDGESTRUCTURALPHYSICAL_ASSERT_LOGW(...)    BeAssert(!__VA_ARGS__); BRIDGESTRUCTURALPHYSICAL_LOGW(__VA_ARGS__);   
#define BRIDGESTRUCTURALPHYSICAL_ASSERT_LOGE(...)    BeAssert(!__VA_ARGS__); BRIDGESTRUCTURALPHYSICAL_LOGE(__VA_ARGS__);   
#endif

#endif