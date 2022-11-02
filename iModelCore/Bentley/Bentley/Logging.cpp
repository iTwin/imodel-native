/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Logging.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

static Logger s_noLogger;
static Logger* s_logger = &s_noLogger;
Logger& Logging::GetLogger() {
    return *s_logger;
}
void Logging::SetLogger(Logger* logger) {
    s_logger = logger ? logger : &s_noLogger;
}

static Utf8CP getSeverityText(SEVERITY sev) {
    switch (sev) {
    case LOG_FATAL:
        return "FATAL";
    case LOG_ERROR:
        return "ERROR";
    case LOG_WARNING:
        return "WARNING";
    case LOG_INFO:
        return "INFO";
    }
    return "TRACE";
}

ConsoleLogger& ConsoleLogger::GetLogger() {
    static ConsoleLogger* s_consoleLogger = nullptr;
    if (nullptr == s_consoleLogger)
        s_consoleLogger = new ConsoleLogger();
    return *s_consoleLogger;
}

void ConsoleLogger::SetSeverity(CharCP category, SEVERITY severity) {
    BeMutexHolder lock(m_lock);
    auto it = m_severity.find(category);
    if (it != m_severity.end())
        (*it).second = severity;
    else
        m_severity.Insert(category, severity);
}

bool ConsoleLogger::IsSeverityEnabled(CharCP category, SEVERITY sev) {
    BeMutexHolder lock(m_lock);
    SEVERITY severity = m_defaultSeverity;

    auto it = m_severity.find(category);
    if (it != m_severity.end())
        severity = (*it).second;

    return (sev >= severity);
}

void ConsoleLogger::LogMessage(CharCP category, SEVERITY sev, CharCP msg) {
    if (nullptr != category && nullptr != msg && IsSeverityEnabled(category, sev))
        fprintf(stdout, "%-8s %-20s %s\n", getSeverityText(sev), category, msg);
}
