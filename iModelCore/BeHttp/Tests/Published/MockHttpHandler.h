/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MockHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpClient.h>
#include <map>
#include "TestsHelper.h"
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockHttpHandler : public IHttpHandler
    {
public:
    typedef std::function<HttpResponse (HttpRequestCR)> OnResponseCallback;

private:
    int64_t m_expectedRequests;
    
    bmap<uint32_t, OnResponseCallback> m_onSpecificRequestMap;
    OnResponseCallback m_onAnyRequestCallback;

    uint32_t m_perfomedRequests;

public:
    MockHttpHandler ();
    virtual ~MockHttpHandler () override;
    virtual AsyncTaskPtr<HttpResponse> PerformRequest (HttpRequestCR request) override;

    uint32_t GetRequestsPerformed () const;

    MockHttpHandler& ExpectOneRequest ();
    MockHttpHandler& ExpectRequests (uint32_t count);

    MockHttpHandler& ForRequest (uint32_t requestNumber, OnResponseCallback callback);
    MockHttpHandler& ForRequest (uint32_t requestNumber, HttpResponseCR response);

    MockHttpHandler& ForFirstRequest (OnResponseCallback callback);
    MockHttpHandler& ForFirstRequest (HttpResponseCR response);

    MockHttpHandler& ForAnyRequest (OnResponseCallback callback);
    MockHttpHandler& ForAnyRequest (HttpResponseCR response);
    };

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE