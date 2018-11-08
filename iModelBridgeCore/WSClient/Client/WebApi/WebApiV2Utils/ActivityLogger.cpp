/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2Utils/ActivityLogger.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ActivityLogger.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WString ActivityLogger::CreateLogMessage(WCharCP messageFormat, va_list messageArguments) const
    {
    WString message;
    message.VSprintf(messageFormat, messageArguments);
    return CreateLogMessage(message);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ActivityLogger::CreateLogMessage(Utf8CP messageFormat, va_list messageArguments) const
    {
    Utf8String message;
    message.VSprintf(messageFormat, messageArguments);
    return CreateLogMessage(message);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WString ActivityLogger::CreateLogMessage(WString message) const
    {
    WString activityName;
    BeStringUtilities::Utf8ToWChar(activityName, m_activityName.c_str());

    WString logMessage;
    logMessage.Sprintf(L"%s: %s", activityName.c_str(), message.c_str());

    if (!m_activityId.empty())
        {
        WString activityId;
        BeStringUtilities::Utf8ToWChar(activityId, m_activityId.c_str());
        logMessage += WPrintfString(L" (ActivityId: %s)", activityId.c_str());
        }

    return logMessage;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ActivityLogger::CreateLogMessage(Utf8String message) const
    {
    Utf8String logMessage;
    logMessage.Sprintf("%s: %s", m_activityName.c_str(), message.c_str());

    if (!m_activityId.empty())
        logMessage += Utf8PrintfString(" (ActivityId: %s)", m_activityId.c_str());

    return logMessage;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ActivityLogger::ActivityLogger(NativeLogging::ILogger& logger, Utf8StringCR activityName, Utf8StringCR headerName, Utf8StringCR activityId) :
    m_logger(logger),
    m_activityId(activityId),
    m_activityName(activityName),
    m_headerName(headerName)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ActivityLogger::HasValidActivityInfo() const
    {
    return !m_activityId.empty() && !m_headerName.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ActivityLogger::GetActivityId() const
    {
    return m_activityId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ActivityLogger::GetHeaderName() const
    {
    return m_headerName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ActivityLogger::isSeverityEnabled(NativeLogging::SEVERITY sev) const
    {
    return m_logger.isSeverityEnabled(sev);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(NativeLogging::SEVERITY sev, WCharCP msg)
    {
    messageva(sev, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    messageva(sev, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messagev(NativeLogging::SEVERITY sev, WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messagev(NativeLogging::SEVERITY sev, Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messageva(NativeLogging::SEVERITY sev, WCharCP msg, va_list args)
    {
    if (!isSeverityEnabled(sev))
        return;

    WString logMessage = CreateLogMessage(msg, args);
    m_logger.message(sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messageva(NativeLogging::SEVERITY sev, Utf8CP msg, va_list args)
    {
    if (!isSeverityEnabled(sev))
        return;

    Utf8String logMessage = CreateLogMessage(msg, args);
    m_logger.message(sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg)
    {
    messageva(nameSpace, sev, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    messageva(nameSpace, sev, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messagev(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(nameSpace, sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messagev(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(nameSpace, sev, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messageva(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, va_list args)
    {
    if (!isSeverityEnabled(sev))
        return;

    WString logMessage = CreateLogMessage(msg, args);
    m_logger.message(nameSpace, sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::messageva(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, va_list args)
    {
    if (!isSeverityEnabled(sev))
        return;

    Utf8String logMessage = CreateLogMessage(msg, args);
    m_logger.message(nameSpace, sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::fatal(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::fatal(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::fatalv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::fatalv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_FATAL, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::error(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::error(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::errorv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::errorv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_ERROR, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::warning(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::warning(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::warningv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::warningv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_WARNING, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::info(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::info(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::infov(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::infov(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_INFO, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::debug(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::debug(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::debugv(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::debugv(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_DEBUG, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::trace(WCharCP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::trace(Utf8CP msg)
    {
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, va_list());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::tracev(WCharCP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::tracev(Utf8CP msg, ...)
    {
    va_list args;
    va_start(args, msg);
    messageva(NativeLogging::SEVERITY::LOG_TRACE, msg, args);
    va_end(args);
    }