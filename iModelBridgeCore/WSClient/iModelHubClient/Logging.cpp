/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

NativeLogging::ILogger* LogHelper::logger = BentleyApi::NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_IMODELHUB);

void LogHelper::LogInternal(SEVERITY severity, Utf8String const methodName, float timeSpent, IWSRepositoryClient::RequestOptionsPtr requestOptions, Utf8CP messageFormat, va_list args)
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

    if (nullptr != requestOptions)
        {
        BeAssert(requestOptions->GetActivityOptions()->HasActivityId() && "ActivityId not set.");
        
        if (!Utf8String::IsNullOrEmpty(message.c_str()))
            message.append(" ");

        Utf8String activityIdFormatted;
        activityIdFormatted.Sprintf("ActivityId: %s.", requestOptions->GetActivityOptions()->GetActivityId().c_str());
        message.append(activityIdFormatted);
        }

    logItem.Sprintf("%-*s %-*s %-*s %s", DATETIME_COLUMN_WIDTH, DateTime::GetCurrentTimeUtc().ToString().c_str(), METHOD_NAME_COLUMN_WIDTH, 
                    methodName.c_str(), TIME_COLUMN_WIDTH, timeFormatted.c_str(), message.c_str());

    switch (severity)
        {
        case SEVERITY::LOG_DEBUG:
            LogHelper::logger->debug(logItem.c_str());
            break;
        case SEVERITY::LOG_ERROR:
            LogHelper::logger->error(logItem.c_str());
            break;
        case SEVERITY::LOG_FATAL:
            LogHelper::logger->fatal(logItem.c_str());
            break;
        case SEVERITY::LOG_INFO:
            LogHelper::logger->info(logItem.c_str());
            break;
        case SEVERITY::LOG_TRACE:
            LogHelper::logger->trace(logItem.c_str());
            break;
        case SEVERITY::LOG_WARNING:
            LogHelper::logger->warning(logItem.c_str());
        }
    }

void LogHelper::Log(SEVERITY severity, const Utf8String methodName, float timeSpent, Utf8CP messageFormat, ...)
    {
    va_list args;
    va_start(args, messageFormat);
    LogHelper::LogInternal(severity, methodName, timeSpent, nullptr, messageFormat, args);
    va_end(args);
    }

void LogHelper::Log(SEVERITY severity, const Utf8String methodName, Utf8CP message, ...)
    {
    va_list args;
    va_start(args, message);
    LogHelper::LogInternal(severity, methodName, -1, nullptr, message, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LogHelper::Log(SEVERITY severity, const Utf8String methodName, float timeSpent, IWSRepositoryClient::RequestOptionsPtr requestOptions, Utf8CP messageFormat, ...)
    {
    va_list args;
    va_start(args, messageFormat);
    LogHelper::LogInternal(severity, methodName, timeSpent, requestOptions, messageFormat, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LogHelper::Log(SEVERITY severity, const Utf8String methodName, IWSRepositoryClient::RequestOptionsPtr requestOptions, Utf8CP message, ...)
    {
    va_list args;
    va_start(args, message);
    LogHelper::LogInternal(severity, methodName, -1, requestOptions, message, args);
    va_end(args);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IWSRepositoryClient::RequestOptionsPtr LogHelper::CreateiModelHubRequestOptions()
    {
    auto requestOptions = std::make_shared<IWSRepositoryClient::RequestOptions>();
    FilliModelHubRequestOptions(requestOptions);
    return requestOptions;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LogHelper::FilliModelHubRequestOptions(IWSRepositoryClient::RequestOptionsPtr requestOptions)
    {
    requestOptions->GetActivityOptions()->SetHeaderName(IWSRepositoryClient::ActivityOptions::HeaderName::XCorrelationId);

    if (!requestOptions->GetActivityOptions()->HasActivityId())
        {
        BeSQLite::BeGuid id = BeSQLite::BeGuid(true);
        requestOptions->GetActivityOptions()->SetActivityId(id.ToString());
        }
    }

END_BENTLEY_IMODELHUB_NAMESPACE
