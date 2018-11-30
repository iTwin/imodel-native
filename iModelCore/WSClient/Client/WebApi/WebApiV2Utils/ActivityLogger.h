/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2Utils/ActivityLogger.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WebServicesClient.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef const struct ActivityLogger& ActivityLoggerCR;
typedef struct ActivityLogger& ActivityLoggerR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
//! ActivityLogger is ILogger wrapper that encapsulates activity id and name into logs
struct ActivityLogger : public NativeLogging::ILogger
    {
    private:
        NativeLogging::ILogger& m_logger;
        Utf8String m_activityId;
        Utf8String m_activityName;
        Utf8String m_headerName;

        WString CreateLogMessage(WCharCP messageFormat, va_list messageArguments) const;
        Utf8String CreateLogMessage(Utf8CP messageFormat, va_list messageArguments) const;
        WString CreateLogMessage(WString message) const;
        Utf8String CreateLogMessage(Utf8String message) const;

    public:
        //! Creates ActivityLogger instance with specified logger, activity name and id.
        //! @param[in] logger Logger that will be used for logging
        //! @param[in] activityName Activity name that will be included in each log
        //! @param[in] headerName header name for adding activity id into Http request. Optional
        //! @param[in] activityId Activity id that will be included in each log if it is specfied. Optional
        ActivityLogger(NativeLogging::ILogger& logger, Utf8StringCR activityName, Utf8StringCR headerName = Utf8String(), Utf8StringCR activityId = Utf8String()) 
            : m_logger(logger), m_activityId(activityId), m_activityName(activityName), m_headerName(headerName) {}

        //! Check if logger has activity id and header name
        bool HasValidActivityInfo() const { return !m_activityId.empty() && !m_headerName.empty(); }

        //! Get activity id. If activity id does not exist then returns empty string
        Utf8StringCR GetActivityId() const { return m_activityId; }

        //! Get header name for adding activity id into Http request
        Utf8StringCR GetHeaderName() const { return m_headerName; }

        bool isSeverityEnabled(NativeLogging::SEVERITY sev) const override { return m_logger.isSeverityEnabled(sev); }
        WSCLIENT_EXPORT void message(NativeLogging::SEVERITY sev, WCharCP msg) override;
        WSCLIENT_EXPORT void message(NativeLogging::SEVERITY sev, Utf8CP msg) override;
        WSCLIENT_EXPORT void messagev(NativeLogging::SEVERITY sev, WCharCP msg, ...) override;
        WSCLIENT_EXPORT void messagev(NativeLogging::SEVERITY sev, Utf8CP msg, ...) override;
        WSCLIENT_EXPORT void messageva(NativeLogging::SEVERITY sev, WCharCP msg, va_list args) override;
        WSCLIENT_EXPORT void messageva(NativeLogging::SEVERITY sev, Utf8CP msg, va_list args) override;
        WSCLIENT_EXPORT void message(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg) override;
        WSCLIENT_EXPORT void message(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg) override;
        WSCLIENT_EXPORT void messagev(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, ...) override;
        WSCLIENT_EXPORT void messagev(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, ...) override;
        WSCLIENT_EXPORT void messageva(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, va_list args) override;
        WSCLIENT_EXPORT void messageva(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, va_list args) override;
        void fatal(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_FATAL, msg); }
        void fatal(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_FATAL, msg); }
        WSCLIENT_EXPORT void fatalv(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void fatalv(Utf8CP msg, ...) override;
        void error(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_ERROR, msg); }
        void error(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_ERROR, msg); }
        WSCLIENT_EXPORT void errorv(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void errorv(Utf8CP msg, ...) override;
        void warning(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_WARNING, msg); }
        void warning(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_WARNING, msg); }
        WSCLIENT_EXPORT void warningv(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void warningv(Utf8CP msg, ...) override;
        void info(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_INFO, msg); }
        void info(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_INFO, msg); }
        WSCLIENT_EXPORT void infov(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void infov(Utf8CP msg, ...) override;
        void debug(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_DEBUG, msg); }
        void debug(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_DEBUG, msg); }
        WSCLIENT_EXPORT void debugv(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void debugv(Utf8CP msg, ...) override;
        void trace(WCharCP msg) override { message(NativeLogging::SEVERITY::LOG_TRACE, msg); }
        void trace(Utf8CP msg) override { message(NativeLogging::SEVERITY::LOG_TRACE, msg); }
        WSCLIENT_EXPORT void tracev(WCharCP msg, ...) override;
        WSCLIENT_EXPORT void tracev(Utf8CP msg, ...) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE