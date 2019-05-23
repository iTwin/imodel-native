/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "StubLogger.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_WSCLIENT_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool StubLogger::HasUtf8CPLogs() const
    {
    return nullptr != m_lastUtf8CPLog;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool StubLogger::HasWCharCPLogs() const
    {
    return nullptr != m_lastWCharCPLog;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
const uint64_t StubLogger::GetLogsCount() const
    {
    return m_logsCount;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StubLogger::StubUtf8CPLogCR StubLogger::GetLastUtf8CPLog() const
    {
    return *m_lastUtf8CPLog;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StubLogger::StubWCharCPLogCR StubLogger::GetLastWCharCPLog() const
    {
    return *m_lastWCharCPLog;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::OnIsSeverityEnabled(std::function<bool(NativeLogging::SEVERITY)> onIsSeverityEnabledCallback)
    {
    m_onIsSeverityEnabledCallback = onIsSeverityEnabledCallback;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool StubLogger::isSeverityEnabled(NativeLogging::SEVERITY sev) const
    {
    if (nullptr == m_onIsSeverityEnabledCallback)
        return true;
    return m_onIsSeverityEnabledCallback(sev);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::message(NativeLogging::SEVERITY sev, WCharCP msg)
    {
    message(nullptr, sev, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::message(NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    message(nullptr, sev, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messagev(NativeLogging::SEVERITY sev, WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messagev(NativeLogging::SEVERITY sev, Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messageva(NativeLogging::SEVERITY sev, WCharCP msg, va_list args)
    {
    messageva(nullptr, sev, msg, args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messageva(NativeLogging::SEVERITY sev, Utf8CP msg, va_list args)
    {
    messageva(nullptr, sev, msg, args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::message(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg)
    {
    m_lastWCharCPLog = std::make_shared<StubWCharCPLog>(nameSpace, sev, msg);
    m_logsCount++;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::message(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    m_lastUtf8CPLog = std::make_shared<StubUtf8CPLog>(nameSpace, sev, msg);
    m_logsCount++;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messagev(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(nameSpace, sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messagev(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(nameSpace, sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messageva(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, va_list args)
    {
    WString msgStr;
    msgStr.VSprintf(msg, args);
    message(nameSpace, sev, msgStr.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::messageva(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, va_list args)
    {
    Utf8String msgStr;
    msgStr.VSprintf(msg, args);
    message(nameSpace, sev, msgStr.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::fatal(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_FATAL, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::fatal(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_FATAL, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::fatalv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::fatalv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::error(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_ERROR, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::error(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_ERROR, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::errorv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::errorv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::warning(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_WARNING, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::warning(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_WARNING, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::warningv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::warningv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::info(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_INFO, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::info(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_INFO, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::infov(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::infov(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::debug(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_DEBUG, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::debug(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_DEBUG, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::debugv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::debugv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::trace(WCharCP msg)
    {
    message(NativeLogging::SEVERITY::LOG_TRACE, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::trace(Utf8CP msg)
    {
    message(NativeLogging::SEVERITY::LOG_TRACE, msg);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::tracev(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StubLogger::tracev(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, args);
    va_end(args);
    }