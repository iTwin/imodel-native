/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/MockHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include <DgnClientFx/Utils/Http/IHttpHandler.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

#define EXPECT_REQUEST_COUNT(handler, count) (handler).ExpectRequests(count, __FILE__, __LINE__)

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

        Utf8String m_file;
        unsigned m_line;

    public:
        MockHttpHandler();
        virtual ~MockHttpHandler() override;
        virtual AsyncTaskPtr<HttpResponse> PerformRequest(HttpRequestCR request) override;

        uint32_t GetRequestsPerformed() const;

        //! DEPRECATED: Use EXPECT_REQUEST_COUNT macro
        MockHttpHandler& ExpectOneRequest();
        //! DEPRECATED: Use EXPECT_REQUEST_COUNT macro
        MockHttpHandler& ExpectRequests(uint32_t count, Utf8CP file = nullptr, unsigned line = 0);

        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForRequest(uint32_t requestNumber, OnResponseCallback callback);
        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForRequest(uint32_t requestNumber, HttpResponseCR response);

        MockHttpHandler& ExpectRequest(OnResponseCallback callback);
        MockHttpHandler& ExpectRequest(HttpResponseCR response);

        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForFirstRequest(OnResponseCallback callback);
        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForFirstRequest(HttpResponseCR response);

        MockHttpHandler& ForAnyRequest(OnResponseCallback callback);
        MockHttpHandler& ForAnyRequest(HttpResponseCR response);
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
