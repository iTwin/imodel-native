/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __DGNV8OPENROADSDESIGNERINTERNAL_H__
#define __DGNV8OPENROADSDESIGNERINTERNAL_H__

#include <DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesigner.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNV8OPENROADSDESIGNER

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_DGNV8OPENROADSDESIGNER    "DgnV8OpenRoadsDesigner"
#if defined (ANDROID)
#include <android/log.h>
#define DGNV8OPENROADSDESIGNER_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_DGNV8OPENROADSDESIGNER, __VA_ARGS__);
#define DGNV8OPENROADSDESIGNER_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_DGNV8OPENROADSDESIGNER, __VA_ARGS__);
#define DGNV8OPENROADSDESIGNER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_DGNV8OPENROADSDESIGNER, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define DGNV8OPENROADSDESIGNER_LOG                 (*NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_DGNV8OPENROADSDESIGNER))
#define DGNV8OPENROADSDESIGNER_LOGD(...)           DGNV8OPENROADSDESIGNER_LOG.debugv (__VA_ARGS__);
#define DGNV8OPENROADSDESIGNER_LOGI(...)           DGNV8OPENROADSDESIGNER_LOG.infov (__VA_ARGS__);
#define DGNV8OPENROADSDESIGNER_LOGW(...)           DGNV8OPENROADSDESIGNER_LOG.warningv (__VA_ARGS__);
#define DGNV8OPENROADSDESIGNER_LOGE(...)           DGNV8OPENROADSDESIGNER_LOG.errorv (__VA_ARGS__);
#endif

#endif
