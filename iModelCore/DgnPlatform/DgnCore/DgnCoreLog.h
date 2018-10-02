/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreLog.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// Should only be included from .cpp files in DgnCore that wish to log with DgnCore namespace

#include <Logging/bentleylogging.h>
//#define LOG (*NativeLogging::LoggingManager::GetLogger(L"DgnCore"))
#define THREADLOG (*NativeLogging::LoggingManager::GetLogger(DgnDb::GetThreadIdName()))

#if defined (ANDROID)
#include <android/log.h>
//TODO isSeverityEnabled on Android
#define LOG_IsSeverityEnabled(...) false
#define LOGT(...) __android_log_print(ANDROID_LOG_VERBOSE, "DgnCore", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "DgnCore", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "DgnCore", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  "DgnCore", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "DgnCore", __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, "DgnCore", __VA_ARGS__)
#else
#define LOG_IsSeverityEnabled(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").isSeverityEnabled (__VA_ARGS__)
#define LOGT(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").tracev (__VA_ARGS__)
#define LOGD(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").debugv (__VA_ARGS__)
#define LOGI(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").infov (__VA_ARGS__)
#define LOGW(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").warningv (__VA_ARGS__)
#define LOGE(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").errorv (__VA_ARGS__)
#define LOGF(...) (*NativeLogging::LoggingManager::GetLogger (L"DgnCore").fatalv (__VA_ARGS__)
#endif