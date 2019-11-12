/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

#define WSJobResponse_Status       "Status"
#define WSJobResponse_ScheduleTime "ScheduleTime"
#define WSJobResponse_Content      "ResponseContent"
#define WSJobResponse_StatusCode   "ResponseStatusCode"
#define WSJobResponse_Headers      "ResponseHeaders"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     julius.cepukenas  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSJob
    {
    public:
        enum class Status
            {
            Failed,
            NotStarted,
            Running,
            Succeeded
            };

    private:
        std::shared_ptr<WSObjectsReader> m_reader;
        bool m_isValid;
        bool m_hasResponseBody;
        ConnectionStatus m_status;
        Utf8String m_url;

    private:
        void Validate();
        static Status GetErrorIdFromString(Utf8StringCR errorIdString);

    public:
        WSCLIENT_EXPORT WSJob(Http::Response response);
        WSCLIENT_EXPORT WSJob(Http::Response response, Utf8StringCR url);

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT bool HasResponseContent() const;
        WSCLIENT_EXPORT WSJob::Status GetStatus() const;
        WSCLIENT_EXPORT DateTime GetScheduleTime() const;
        WSCLIENT_EXPORT Http::Response GetResponse() const;


    };

END_BENTLEY_WEBSERVICES_NAMESPACE
