/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    logMessage.Sprintf(L"%ls: %ls", activityName.c_str(), message.c_str());

    if (!m_activityId.empty())
        {
        WString activityId;
        BeStringUtilities::Utf8ToWChar(activityId, m_activityId.c_str());
        logMessage += WPrintfString(L" (ActivityId: %ls)", activityId.c_str());
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
void ActivityLogger::message(NativeLogging::SEVERITY sev, WCharCP msg)
    {
    if (!isSeverityEnabled(sev))
        return;

    WString logMessage = CreateLogMessage(msg);
    m_logger.message(sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (!isSeverityEnabled(sev))
        return;

    Utf8String logMessage = CreateLogMessage(msg);
    m_logger.message(sev, logMessage.c_str());
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
    if (!isSeverityEnabled(sev))
        return;

    WString logMessage = CreateLogMessage(msg);
    m_logger.message(nameSpace, sev, logMessage.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ActivityLogger::message(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (!isSeverityEnabled(sev))
        return;

    Utf8String logMessage = CreateLogMessage(msg);
    m_logger.message(nameSpace, sev, logMessage.c_str());
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