/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/Logging.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/Logging.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

NativeLogging::ILogger* DgnDbServerLogHelper::logger = BentleyApi::NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_DGNDBSERVER);

void DgnDbServerLogHelper::LogInternal(SEVERITY severity, const Utf8String methodName, float timeSpent, Utf8CP messageFormat, va_list args)
    {
    if (!logger->isSeverityEnabled(severity))
        return;
    const int DATETIME_COLUMN_WIDTH = 25;
    const int METHOD_NAME_COLUMN_WIDTH = 55;
    const int TIME_COLUMN_WIDTH = 12;

    Utf8String timeFormatted;
    Utf8String logItem;
    Utf8String message;

    message.VSprintf(messageFormat, args);

    if (timeSpent < 0)
        timeFormatted = "";
    else
        timeFormatted.Sprintf("%.2f ms.", timeSpent);

    logItem.Sprintf("%-*s %-*s %-*s %s", DATETIME_COLUMN_WIDTH, DateTime::GetCurrentTimeUtc().ToUtf8String().c_str(), METHOD_NAME_COLUMN_WIDTH, methodName.c_str(), TIME_COLUMN_WIDTH, timeFormatted.c_str(), message.c_str());

    switch (severity)
        {
            case SEVERITY::LOG_DEBUG:
                DgnDbServerLogHelper::logger->debug(logItem.c_str());
                break;
            case SEVERITY::LOG_ERROR:
                DgnDbServerLogHelper::logger->error(logItem.c_str());
                break;
            case SEVERITY::LOG_FATAL:
                DgnDbServerLogHelper::logger->fatal(logItem.c_str());
                break;
            case SEVERITY::LOG_INFO:
                DgnDbServerLogHelper::logger->info(logItem.c_str());
                break;
            case SEVERITY::LOG_TRACE:
                DgnDbServerLogHelper::logger->trace(logItem.c_str());
                break;
            case SEVERITY::LOG_WARNING:
                DgnDbServerLogHelper::logger->warning(logItem.c_str());
        }
    }

void DgnDbServerLogHelper::Log(SEVERITY severity, const Utf8String methodName, float timeSpent, Utf8CP messageFormat, ...)
    {
    va_list args;
    va_start(args, messageFormat);
    DgnDbServerLogHelper::LogInternal(severity, methodName, timeSpent, messageFormat, args);
    va_end(args);
    }

void DgnDbServerLogHelper::Log(SEVERITY severity, const Utf8String methodName, Utf8CP message, ...)
    {
    va_list args;
    va_start(args, message);
    DgnDbServerLogHelper::LogInternal(severity, methodName, -1, message, args);
    va_end(args);
    }
END_BENTLEY_DGNDBSERVER_NAMESPACE