/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <BeHttp/IHttpHandler.h>
#include <map>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP
struct MockHttpHandler : IHttpHandler
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
        virtual Tasks::AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override;

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
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
