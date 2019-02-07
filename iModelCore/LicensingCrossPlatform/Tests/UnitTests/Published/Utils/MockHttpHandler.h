/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/MockHttpHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../TestsHelper.h"
#include <BeHttp/HttpClient.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

#define EXPECT_REQUEST_COUNT(handler, count) (handler).ExpectRequests(count, __FILE__, __LINE__)

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockHttpHandler : public IHttpHandler
    {
    public:
        typedef std::function<Http::Response(Http::RequestCR)> OnResponseCallback;

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
        virtual AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override;

        void ValidateAndClearExpectations();

        uint32_t GetRequestsPerformed() const;

        //! DEPRECATED: Use EXPECT_REQUEST_COUNT macro
        MockHttpHandler& ExpectOneRequest();
        //! DEPRECATED: Use EXPECT_REQUEST_COUNT macro
        MockHttpHandler& ExpectRequests(uint32_t count, Utf8CP file = nullptr, unsigned line = 0);

        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForRequest(uint32_t requestNumber, OnResponseCallback callback);
        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForRequest(uint32_t requestNumber, Http::ResponseCR response);

        MockHttpHandler& ExpectRequest(OnResponseCallback callback);
        MockHttpHandler& ExpectRequest(Http::ResponseCR response);

        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForFirstRequest(OnResponseCallback callback);
        //! DEPRECATED: Use ExpectRequest()
        MockHttpHandler& ForFirstRequest(Http::ResponseCR response);

        MockHttpHandler& ForAnyRequest(OnResponseCallback callback);
        MockHttpHandler& ForAnyRequest(Http::ResponseCR response);
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE