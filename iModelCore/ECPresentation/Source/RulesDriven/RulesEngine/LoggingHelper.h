/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct IndentedLogger : RefCountedBase
{
private:
    NativeLogging::ILogger& m_logger;
    unsigned m_indent;
protected:
    IndentedLogger(NativeLogging::ILogger& logger, unsigned indent) : m_logger(logger), m_indent(indent) {}
    virtual ~IndentedLogger();
    NativeLogging::ILogger& GetLogger() const {return m_logger;}
    Utf8String GetIndent() const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct PerformanceLogger : IndentedLogger
{
friend struct LoggingHelper;
private:
    Utf8String m_message;
    NativeLogging::SEVERITY m_severity;
    uint64_t m_startTime;
public:
    PerformanceLogger(NativeLogging::ILogger& logger, unsigned indent, Utf8CP message, NativeLogging::SEVERITY);
    ~PerformanceLogger();
    uint64_t GetElapsedTime() const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct MessageLogger : IndentedLogger
{
friend struct LoggingHelper;
public:
    MessageLogger(NativeLogging::ILogger& logger, unsigned indent) : IndentedLogger(logger, indent) {}
    void LogMessage(Utf8CP message, NativeLogging::SEVERITY);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
enum class Log
    {
    Default,
    Navigation,
    NavigationCache,
    Content,
    UserSettings,
    Localization,
    Update,
    Threads,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct LoggingHelper
{
friend struct IndentedLogger;
private:
    static BeAtomic<unsigned> s_indent;

private:
    LoggingHelper() {}
    static NativeLogging::ILogger& GetLogger(Log);

public:
    static RefCountedPtr<PerformanceLogger> CreatePerformanceLogger(NativeLogging::ILogger&, Utf8CP message, NativeLogging::SEVERITY = NativeLogging::LOG_DEBUG);
    static RefCountedPtr<PerformanceLogger> CreatePerformanceLogger(Log, Utf8CP message, NativeLogging::SEVERITY = NativeLogging::LOG_DEBUG);
    static void LogMessage(NativeLogging::ILogger&, Utf8CP message, NativeLogging::SEVERITY = NativeLogging::LOG_DEBUG, bool once = false);
    static void LogMessage(Log, Utf8CP message, NativeLogging::SEVERITY = NativeLogging::LOG_DEBUG, bool once = false);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
