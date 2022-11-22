/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <Bentley/BeThread.h>

#define USING_NAMESPACE_BENTLEY_LOGGING using namespace BentleyApi::NativeLogging;

BEGIN_BENTLEY_NAMESPACE

namespace NativeLogging {

/** Logger message severity levels.*/
typedef enum {
    LOG_NEVER = 1,  // cannot be enabled.
    LOG_FATAL = 0,  // errors that will terminate the application
    LOG_ERROR = -1,
    LOG_WARNING = -2,
    LOG_INFO = -3,
    LOG_DEBUG = -4, // for debugging
    LOG_TRACE = -4    //alias for debug
} SEVERITY;

/** Base class for Logger implementation. Default implementation does nothing. */
struct Logger {
    virtual void LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) {}
    virtual bool IsSeverityEnabled(Utf8CP category, SEVERITY sev) { return false; }
};

/** logs to the console. */
struct ConsoleLogger : Logger {
protected:
    bmap<Utf8String, SEVERITY> m_severity;
    BeMutex m_lock;
    ConsoleLogger() {}
    virtual void LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) override;
    virtual bool IsSeverityEnabled(Utf8CP category, SEVERITY sev) override;

public:
    SEVERITY m_defaultSeverity = LOG_ERROR;
    BENTLEYDLL_EXPORT void SetSeverity(CharCP category, SEVERITY severity);
    BENTLEYDLL_EXPORT static ConsoleLogger& GetLogger();
};

/**
 *
 */
struct Logging {
    BENTLEYDLL_EXPORT static Logger& GetLogger();
    BENTLEYDLL_EXPORT static void SetLogger(Logger*);

    // Determine if logging level is enabled.
    static bool isSeverityEnabled(Utf8CP category, SEVERITY sev) {
        return GetLogger().IsSeverityEnabled(category, sev);
    }
    static void LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) {
        if (GetLogger().IsSeverityEnabled(category, sev))
            GetLogger().LogMessage(category, sev, msg);
    }
    static void LogMessageW(Utf8CP category, SEVERITY sev, WCharCP msg) {
        if (GetLogger().IsSeverityEnabled(category, sev))
            GetLogger().LogMessage(category, sev, Utf8String(msg).c_str());
    }
    static void LogMessageV(Utf8CP category, SEVERITY sev, Utf8CP fmt, ...) {
        va_list args;
        va_start(args, fmt);
        LogMessageVa(category, sev, fmt, args);
        va_end(args);
    }
    static void LogMessageVa(Utf8CP category, SEVERITY sev, Utf8CP fmt, va_list args) {
        if (GetLogger().IsSeverityEnabled(category, sev))
            GetLogger().LogMessage(category, sev, Utf8PrintfString::CreateFromVaList(fmt, args).c_str());
    }
    static void LogMessageVW(Utf8CP category, SEVERITY sev, WCharCP fmt, ...) {
        va_list args;
        va_start(args, fmt);
        LogMessageVaW(category, sev, fmt, args);
        va_end(args);
    }
    static void LogMessageVaW(Utf8CP category, SEVERITY sev, WCharCP fmt, va_list args) {
        if (GetLogger().IsSeverityEnabled(category, sev))
            GetLogger().LogMessage(category, sev, Utf8String(WPrintfString(fmt, args)).c_str());
    }
};

struct CategoryLogger {
    Utf8CP m_category;
    CategoryLogger(Utf8CP category) : m_category(category) {}

    void LogMessage(SEVERITY sev, Utf8CP msg) const  {
        Logging::LogMessage(m_category, sev, msg);
    }
     void LogMessageW(SEVERITY sev, WCharCP msg) const  {
        Logging::LogMessageW(m_category, sev, msg);
    }
    void LogMessageV(SEVERITY sev, Utf8CP fmt, ...) const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, sev, fmt, args);
        va_end(args);
    }
    void LogMessageVW(SEVERITY sev, WCharCP fmt, ...) const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, sev, fmt, args);
        va_end(args);
    }

    bool isSeverityEnabled(SEVERITY sev) const {
        return Logging::isSeverityEnabled(m_category, sev);
    }
    void message(SEVERITY sev, WCharCP msg) const  {
        LogMessageW(sev, msg);
    }
    void message(SEVERITY sev, Utf8CP msg) const  {
        LogMessage(sev, msg);
    }
    void messagev(SEVERITY sev, WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, sev, fmt, args);
        va_end(args);
    }
    void messagev(SEVERITY sev, Utf8CP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, sev, fmt, args);
        va_end(args);
    }

    void fatal(WCharCP msg)  const  {
        LogMessageW(LOG_FATAL, msg);
    }
    void fatal(Utf8CP msg) const  {
        LogMessage(LOG_FATAL, msg);
    }
    void fatalv(WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_FATAL, fmt, args);
        va_end(args);
    }
    void fatalv(Utf8CP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_FATAL, fmt, args);
        va_end(args);
    }

    void error(WCharCP msg) const {
        LogMessageW(LOG_ERROR, msg);
    }
    void error(Utf8CP msg) const {
        LogMessage(LOG_ERROR, msg);
    }
    void errorv(WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_ERROR, fmt, args);
        va_end(args);
    }
    void errorv(Utf8CP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_ERROR, fmt, args);
        va_end(args);
    }

    void warning(WCharCP msg) const {
        LogMessageW(LOG_WARNING, msg);
    }
    void warning(Utf8CP msg) const {
        LogMessage(LOG_WARNING, msg);
    }
    void warningv(WCharCP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_WARNING, fmt, args);
        va_end(args);
    }
    void warningv(Utf8CP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_WARNING, fmt, args);
        va_end(args);
    }
    void info(WCharCP msg) const {
        LogMessageW(LOG_INFO, msg);
    }
    void info(Utf8CP msg) const {
        LogMessage(LOG_INFO, msg);
    }
    void infov(WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_INFO, fmt, args);
        va_end(args);
    }
    void infov(Utf8CP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_INFO, fmt, args);
        va_end(args);
    }
    void debug(WCharCP msg) const {
        LogMessageW(LOG_DEBUG, msg);
    }
    void debug(Utf8CP msg) const {
        LogMessage(LOG_DEBUG, msg);
    }
    void debugv(WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_DEBUG, fmt, args);
        va_end(args);
    }
    void debugv(Utf8CP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_DEBUG, fmt, args);
        va_end(args);
    }
    void trace(WCharCP msg) const {
        LogMessageW(LOG_TRACE, msg);
    }
    void trace(Utf8CP msg) const {
        LogMessage(LOG_TRACE, msg);
    }
    void tracev(WCharCP fmt, ...) const  {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVaW(m_category, LOG_TRACE, fmt, args);
        va_end(args);
    }
    void tracev(Utf8CP fmt, ...)  const {
        va_list args;
        va_start(args, fmt);
        Logging::LogMessageVa(m_category, LOG_TRACE, fmt, args);
        va_end(args);
    }
};

} // END_BENTLEY_LOGGING_NAMESPACE
END_BENTLEY_NAMESPACE
