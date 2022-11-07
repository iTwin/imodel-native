/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <map>
#include <DgnPlatform/PlatformLib.h>
#include <Napi/napi.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING

namespace IModelJsNative {

/** a log message that is deferred because it was sent from another thread */
struct LoggedMessage {
  Utf8String m_message;
  Utf8String m_category;
  NativeLogging::SEVERITY m_severity;
  LoggedMessage(Utf8CP category, SEVERITY severity, Utf8CP message) : m_message(message), m_category(category), m_severity(severity) {}
};

/**
 * The JavaScript logging interface. All log messages are sent here from native code and the severity
 * settings are cached so we don't have to call the JavaScript "getLogLevel" method repeatedly.
 */
struct JsLogger : NativeLogging::Logger {
  BeMutex m_deferredLogMutex;
  /** the default severity for categories not specified in `m_categoryFilter` */
  SEVERITY m_defaultSeverity = LOG_ERROR;
  /** the logger object from JS */
  Napi::ObjectReference m_loggerObj;
  /** messages that are deferred from other threads. They are displayed on the next log message on the main thread or when "flushLog" is called */
  bvector<LoggedMessage> m_deferredLogging;
  /** cached set of severity levels. Can be cleared from JS by `NativeLibrary.clearLogLevelCache` */
  std::map<Utf8String, NativeLogging::SEVERITY> m_categoryFilter;

  Napi::Value getJsLogger() { return m_loggerObj.Value(); }
  void setJsLogger(Napi::CallbackInfo const& info) {
      m_loggerObj = Napi::ObjectReference::New(info[0].ToObject(), 1);
      m_loggerObj.SuppressDestruct();
      SyncLogLevels();
  }

  SEVERITY JsLevelToSeverity(Napi::Number jsLevel);
  void SyncLogLevels();
  bool canUseJavaScript();
  void logToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
  void deferLogging(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);
  void processDeferred();
  void OnExit();

  /** override of virtual method from NativeLogging::Logger */
  void LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) override;
  /** override of virtual method from NativeLogging::Logger */
  bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) override;
};

}; // namespace IModelJsNative
