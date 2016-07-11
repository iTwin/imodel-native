/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MockHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "MockHttpHandler.h"
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

#define EXPECTED_COUNT_ANY -1

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::MockHttpHandler()
    {
    m_perfomedRequests = 0;
    m_expectedRequests = EXPECTED_COUNT_ANY;
    m_onAnyRequestCallback = [&] (RequestCR request)
        {
        Utf8PrintfString message
            (
            "\n"
            "Uninteresting HttpRequest was performed: %s \n"
            "Got %u requests.",
            request.GetUrl().c_str(),
            m_perfomedRequests
            );
        BeDebugLog(message.c_str());

        return Response(HttpResponseContent::Create(HttpStringBody::Create()), "", ConnectionStatus::CouldNotConnect, HttpStatus::None);
        };
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::~MockHttpHandler()
    {
    if (m_expectedRequests != EXPECTED_COUNT_ANY)
        {
        EXPECT_EQ (m_expectedRequests, m_perfomedRequests);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint32_t MockHttpHandler::GetRequestsPerformed() const
    {
    return m_perfomedRequests;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Http::Response> MockHttpHandler::_PerformRequest(RequestCR request)
    {
    auto task = std::make_shared<PackagedAsyncTask<Http::Response>> ([&] ()
        {
        EXPECT_LT (m_perfomedRequests, std::numeric_limits<uint32_t>::max());
        m_perfomedRequests ++;

        if (m_expectedRequests != EXPECTED_COUNT_ANY && m_expectedRequests < m_perfomedRequests)
            {
            Utf8PrintfString message
                (
                "\n"
                "Unexpected HttpRequest: %s \n"
                "Expected %lld requests. Got %u requests.",
                request.GetUrl().c_str(),
                m_expectedRequests,
                m_perfomedRequests
                );
            BeDebugLog(message.c_str());
            }

        if (m_onSpecificRequestMap.find(m_perfomedRequests) != m_onSpecificRequestMap.end())
            {
            return m_onSpecificRequestMap[m_perfomedRequests] (request);
            }

        return m_onAnyRequestCallback(request);
        });
    task->Execute();
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectRequests(uint32_t count)
    {
    m_expectedRequests = count;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectOneRequest()
    {
    m_expectedRequests = 1;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, OnResponseCallback callback)
    {
    EXPECT_NE (0, requestNumber);
    m_onSpecificRequestMap[requestNumber] = callback;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, ResponseCR response)
    {
    return ForRequest(requestNumber, [=] (RequestCR)
        {
        return response;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForFirstRequest(OnResponseCallback callback)
    {
    return ForRequest(1, callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForFirstRequest(ResponseCR response)
    {
    return ForRequest(1, response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(OnResponseCallback callback)
    {
    EXPECT_TRUE (nullptr != callback);
    m_onAnyRequestCallback = callback;
    return *this;
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(ResponseCR response)
    {
    return ForAnyRequest([=] (RequestCR)
        {
        return response;
        });
    }
