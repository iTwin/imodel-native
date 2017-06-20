/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/Common/TeleporterLogging.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

BEGIN_BIM_TELEPORTER_NAMESPACE
#define LOG             TeleporterLogging::GetLogger (TeleporterLogging::Namespace::General)

//=======================================================================================
// Efficient access to a set of pre-allocated loggers.
// @bsiclass                                                    Sam.Wilson          03/16
//=======================================================================================
struct TeleporterLogging
    {
    enum class Namespace { General, Performance, MaxLoggers };

    static NativeLogging::ILogger& GetLogger(Namespace ns)
        {
        int idx = (int) ns;
        static NativeLogging::ILogger* s_loggers[(int) Namespace::MaxLoggers];

        if (s_loggers[idx] == nullptr)
            {
            static char const* s_loggerNs[] = {"BimTeleporter", "BimTeleporter.Performance"};
            s_loggers[idx] = NativeLogging::LoggingManager::GetLogger(s_loggerNs[idx]);
            }
        return *s_loggers[idx];
        }

    static bool IsSeverityEnabled(Namespace ns, NativeLogging::SEVERITY sev)
        {
        return GetLogger(ns).isSeverityEnabled(sev);
        }

    //Logs the current elapsed time of the passed stopwatch and stops the stopwatch
    static void LogPerformance(StopWatch& stopWatch, Utf8CP description, ...)
        {
        stopWatch.Stop();
        const NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO;
        NativeLogging::ILogger& logger = GetLogger(TeleporterLogging::Namespace::Performance);
        if (logger.isSeverityEnabled(severity))
            {
            va_list args;
            va_start(args, description);
            Utf8String formattedDescription;
            formattedDescription.VSprintf(description, args);
            va_end(args);

            logger.messagev(severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
            }
        }
    };

END_BIM_TELEPORTER_NAMESPACE
