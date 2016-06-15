/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/MockHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include <BeHttp/IHttpHandler.h>

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockHttpHandler : public IHttpHandler
    {
    public:
        typedef std::function<HttpResponse(HttpRequestCR)> OnResponseCallback;

    private:
        int64_t m_expectedRequests;

        std::map<uint32_t, OnResponseCallback> m_onSpecificRequestMap;
        OnResponseCallback m_onAnyRequestCallback;

        uint32_t m_perfomedRequests;

    public:
        MockHttpHandler();
        virtual ~MockHttpHandler() override;
        virtual AsyncTaskPtr<HttpResponse> PerformRequest(HttpRequestCR request) override;

        uint32_t GetRequestsPerformed() const;

        MockHttpHandler& ExpectOneRequest();
        MockHttpHandler& ExpectRequests(uint32_t count);

        MockHttpHandler& ForRequest(uint32_t requestNumber, OnResponseCallback callback);
        MockHttpHandler& ForRequest(uint32_t requestNumber, HttpResponseCR response);

        MockHttpHandler& ForFirstRequest(OnResponseCallback callback);
        MockHttpHandler& ForFirstRequest(HttpResponseCR response);

        MockHttpHandler& ForAnyRequest(OnResponseCallback callback);
        MockHttpHandler& ForAnyRequest(HttpResponseCR response);
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
