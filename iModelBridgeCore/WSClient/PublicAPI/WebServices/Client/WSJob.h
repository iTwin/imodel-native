/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSJob.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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
        WSCLIENT_EXPORT WSJob(HttpResponse response);
        WSCLIENT_EXPORT WSJob(HttpResponse response, Utf8StringCR url);

        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT bool HasResponseContent() const;
        WSCLIENT_EXPORT WSJob::Status GetStatus() const;
        WSCLIENT_EXPORT DateTime GetScheduleTime() const;
        WSCLIENT_EXPORT HttpResponse GetResponse() const;


    };

END_BENTLEY_WEBSERVICES_NAMESPACE
