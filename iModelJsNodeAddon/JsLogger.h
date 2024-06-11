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

/** a log message that is deferred because it was logged from a worker thread */
struct LoggedMessage {
  Utf8String m_message;
  Utf8String m_category;
  NativeLogging::SEVERITY m_severity;
  LoggedMessage(Utf8CP category, SEVERITY severity, Utf8CP message) : m_message(message), m_category(category), m_severity(severity) {}
};

/**
 * The JavaScript logging interface. All log messages are sent here from native code and the severity
 * settings are cached so we don't have to use JavaScript to get them.
 */
struct JsLogger : NativeLogging::Logger {
private:
  /** to synchronize access severities and JS objects */
  BeMutex m_mutex;

  /** the default severity for categories not specified in `m_categoryFilter` */
  SEVERITY m_defaultSeverity = LOG_ERROR;
  /** cached set of severity levels. Can be cleared from JS by `NativeLibrary.clearLogLevelCache` */
  std::map<Utf8String, NativeLogging::SEVERITY> m_categoryFilter;

  /** the logger object from JS */
  Napi::ObjectReference m_loggerObj;
  /** a function used to send logs from threads other that the main thread */
  Napi::ThreadSafeFunction m_processLogsOnMainThread;

  bool canUseJavaScript();
  bool isEnabled(Utf8CP category, NativeLogging::SEVERITY sev);
  void logToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg);

public:
  Napi::Value GetJsLogger();
  void SetJsLogger(Napi::CallbackInfo const& info);

  void SyncLogLevels();
  void Cleanup();

  /** override of virtual method from NativeLogging::Logger */
  void LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) override;
  /** override of virtual method from NativeLogging::Logger */
  bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) override;

  static SEVERITY JsLevelToSeverity(Napi::Number jsLevel);
};

}; // namespace IModelJsNative
