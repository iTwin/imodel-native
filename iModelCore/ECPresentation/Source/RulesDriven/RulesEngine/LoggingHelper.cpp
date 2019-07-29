/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "LoggingHelper.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_LOGGING

BeAtomic<unsigned> LoggingHelper::s_indent(0);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ILogger& LoggingHelper::GetLogger(Log ns)
    {    
    ILogger* logger = nullptr;
    switch (ns)
        {
        case Log::Default: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE); break;
        case Log::Navigation: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION); break;
        case Log::NavigationCache: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION_CACHE); break;
        case Log::Update: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_UPDATE); break;
        case Log::Content: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_CONTENT); break;
        case Log::UserSettings: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_USERSETTINGS); break;
        case Log::Localization: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_LOCALIZATION); break;
        case Log::Threads: logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_THREADS); break;
        default: BeAssert(false); logger = LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE);
        }
    return *logger;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PerformanceLogger> LoggingHelper::CreatePerformanceLogger(ILogger& logger, Utf8CP message, SEVERITY severity)
    {
    if (!logger.isSeverityEnabled(severity))
        return nullptr;
    return new PerformanceLogger(logger, s_indent++, message, severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PerformanceLogger> LoggingHelper::CreatePerformanceLogger(Log ns, Utf8CP message, SEVERITY severity)
    {
    return CreatePerformanceLogger(GetLogger(ns), message, severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void LoggingHelper::LogMessage(ILogger& logger, Utf8CP message, SEVERITY severity, bool once)
    {
    if (!logger.isSeverityEnabled(severity))
        return;

    if (once)
        {
        static bset<Utf8CP> s_loggedMessages;
        if (s_loggedMessages.end() != s_loggedMessages.find(message))
            return;
        s_loggedMessages.insert(message);
        }

    RefCountedPtr<MessageLogger> messageLogger = new MessageLogger(logger, s_indent++);
    messageLogger->LogMessage(message, severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void LoggingHelper::LogMessage(Log ns, Utf8CP message, SEVERITY severity, bool once)
    {
    LogMessage(GetLogger(ns), message, severity, once);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IndentedLogger::GetIndent() const
    {
    static unsigned SPACES_PER_INDENT = 3;
    return Utf8String(SPACES_PER_INDENT * m_indent, ' ');
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IndentedLogger::~IndentedLogger() {LoggingHelper::s_indent--;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MessageLogger::LogMessage(Utf8CP message, SEVERITY severity) {GetLogger().messagev(severity, "%s%s", GetIndent().c_str(), message);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PerformanceLogger::PerformanceLogger(ILogger& logger, unsigned indent, Utf8CP message, SEVERITY severity)
    : IndentedLogger(logger, indent), m_message(message), m_startTime(BeTimeUtilities::GetCurrentTimeAsUnixMillis()), m_severity(severity)
    {
    GetLogger().messagev(severity, "%sStarted: %s", GetIndent().c_str(), message);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PerformanceLogger::~PerformanceLogger()
    {
    GetLogger().messagev(m_severity, "%sEnded: %s. Took %" PRIu64 " ms", GetIndent().c_str(), m_message.c_str(), GetElapsedTime());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t PerformanceLogger::GetElapsedTime() const {return BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_startTime;}
