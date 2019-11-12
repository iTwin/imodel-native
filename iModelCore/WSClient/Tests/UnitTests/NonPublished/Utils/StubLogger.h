/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLogger : NativeLogging::ILogger
    {
    private:
        template<typename StringType>
        struct StubLog
            {
            private:
                StringType m_nameSpace;
                NativeLogging::SEVERITY m_severity;
                StringType m_message;

            public:
                StubLog(const StringType& nameSpace, NativeLogging::SEVERITY severity, const StringType& message) :
                    m_nameSpace(nameSpace),
                    m_severity(severity),
                    m_message(message)
                    {}

                const StringType& GetNameSpace() const { return m_nameSpace; }
                const NativeLogging::SEVERITY GetSeverity() const { return m_severity; }
                const StringType& GetMessage() const { return m_message; }
            };

    public:
        struct StubUtf8CPLog : StubLog<Utf8String>
            {
            StubUtf8CPLog(Utf8StringCR nameSpace, NativeLogging::SEVERITY severity, Utf8StringCR message) :
                StubLog(nameSpace, severity, message) {}
            };
        struct StubWCharCPLog : StubLog<WString>
            {
            StubWCharCPLog(WStringCR nameSpace, NativeLogging::SEVERITY severity, WStringCR message) :
                StubLog(nameSpace, severity, message) {}
            };

        typedef const StubUtf8CPLog& StubUtf8CPLogCR;
        typedef const StubWCharCPLog& StubWCharCPLogCR;

    private:
        uint64_t m_logsCount;
        std::shared_ptr<StubUtf8CPLog> m_lastUtf8CPLog;
        std::shared_ptr<StubWCharCPLog> m_lastWCharCPLog;

        std::function<bool(NativeLogging::SEVERITY)> m_onIsSeverityEnabledCallback;

    public:
        StubLogger() :
            m_logsCount(0),
            m_lastUtf8CPLog(nullptr),
            m_lastWCharCPLog(nullptr),
            m_onIsSeverityEnabledCallback(nullptr)
            {}

        bool HasUtf8CPLogs() const;
        bool HasWCharCPLogs() const;
        const uint64_t GetLogsCount() const;
        StubUtf8CPLogCR GetLastUtf8CPLog() const;
        StubWCharCPLogCR GetLastWCharCPLog() const;

        void OnIsSeverityEnabled(std::function<bool(NativeLogging::SEVERITY)> onIsSeverityEnabledCallback);

        bool isSeverityEnabled(NativeLogging::SEVERITY sev) const override;
        void message(NativeLogging::SEVERITY sev, WCharCP msg) override;
        void message(NativeLogging::SEVERITY sev, Utf8CP msg) override;
        void messagev(NativeLogging::SEVERITY sev, WCharCP msg, ...) override;
        void messagev(NativeLogging::SEVERITY sev, Utf8CP msg, ...) override;
        void messageva(NativeLogging::SEVERITY sev, WCharCP msg, va_list args) override;
        void messageva(NativeLogging::SEVERITY sev, Utf8CP msg, va_list args) override;
        void message(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg) override;
        void message(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg) override;
        void messagev(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, ...) override;
        void messagev(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, ...) override;
        void messageva(WCharCP nameSpace, NativeLogging::SEVERITY sev, WCharCP msg, va_list args) override;
        void messageva(Utf8CP nameSpace, NativeLogging::SEVERITY sev, Utf8CP msg, va_list args) override;
        void fatal(WCharCP msg) override;
        void fatal(Utf8CP msg) override;
        void fatalv(WCharCP msg, ...) override;
        void fatalv(Utf8CP msg, ...) override;
        void error(WCharCP msg) override;
        void error(Utf8CP msg) override;
        void errorv(WCharCP msg, ...) override;
        void errorv(Utf8CP msg, ...) override;
        void warning(WCharCP msg) override;
        void warning(Utf8CP msg) override;
        void warningv(WCharCP msg, ...) override;
        void warningv(Utf8CP msg, ...) override;
        void info(WCharCP msg) override;
        void info(Utf8CP msg) override;
        void infov(WCharCP msg, ...) override;
        void infov(Utf8CP msg, ...) override;
        void debug(WCharCP msg) override;
        void debug(Utf8CP msg) override;
        void debugv(WCharCP msg, ...) override;
        void debugv(Utf8CP msg, ...) override;
        void trace(WCharCP msg) override;
        void trace(Utf8CP msg) override;
        void tracev(WCharCP msg, ...) override;
        void tracev(Utf8CP msg, ...) override;
    };

END_WSCLIENT_UNITTESTS_NAMESPACE