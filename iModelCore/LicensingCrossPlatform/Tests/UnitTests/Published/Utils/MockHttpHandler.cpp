/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/MockHttpHandler.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "MockHttpHandler.h"
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

#define EXPECTED_COUNT_ANY -1

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::MockHttpHandler()
    {
    m_perfomedRequests = 0;
    m_expectedRequests = EXPECTED_COUNT_ANY;
    m_onAnyRequestCallback = [&] (Http::Request request)
        {
        if (m_expectedRequests == EXPECTED_COUNT_ANY)
            {
            BeDebugLog(Utf8PrintfString(
                "\n%s(%u): \n"
                "Uninteresting Http::Request was performed: %s\n"
                "Got %u requests.",
                m_file.c_str(), m_line,
                request.GetUrl().c_str(),
                m_perfomedRequests
                ).c_str());
            }

        return Http::Response(HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, HttpStatus::None);
        };
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::~MockHttpHandler()
    {
    ValidateAndClearExpectations();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void MockHttpHandler::ValidateAndClearExpectations()
    {
    if (m_expectedRequests == EXPECTED_COUNT_ANY)
        return;

    if (m_file.empty())
        {
        EXPECT_EQ(m_expectedRequests, m_perfomedRequests);
        }
    else if (m_expectedRequests != m_perfomedRequests)
        {
        BeDebugLog(Utf8PrintfString(
            "\n%s(%u): error: Request count does not match expectation\n"
            "  Actual: %d\n"
            "Expected: %d",
            m_file.c_str(), m_line,
            m_perfomedRequests,
            m_expectedRequests
        ).c_str());
        }

    *this = MockHttpHandler();
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
AsyncTaskPtr<Http::Response> MockHttpHandler::_PerformRequest(Http::RequestCR request)
    {
    auto task = std::make_shared<PackagedAsyncTask<Http::Response>>([&] ()
        {
        EXPECT_LT(m_perfomedRequests, std::numeric_limits<uint32_t>::max());
        m_perfomedRequests++;

        if (m_expectedRequests != EXPECTED_COUNT_ANY && m_expectedRequests < m_perfomedRequests)
            {
            BeDebugLog(Utf8PrintfString(
                "\n%s(%u): error: Unexpected Http::Request\n"
                "Received: %u requests. Current: %s\n"
                "Expected: %lld requests",
                m_file.c_str(), m_line,
                m_perfomedRequests,
                request.GetUrl().c_str(),
                m_expectedRequests
                ).c_str());
            }

        if (m_onSpecificRequestMap.find(m_perfomedRequests) != m_onSpecificRequestMap.end())
            {
            return m_onSpecificRequestMap[m_perfomedRequests](request);
            }

        return m_onAnyRequestCallback(request);
        });
    task->Execute();
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectRequests(uint32_t count, Utf8CP file, unsigned line)
    {
    m_expectedRequests = count;
    m_file = file;
    m_line = line;
    m_onSpecificRequestMap.clear();
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectOneRequest()
    {
    return ExpectRequests(1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectRequest(OnResponseCallback callback)
    {
    auto requestNumber = 0;

    auto it = m_onSpecificRequestMap.rbegin();
    if (it != m_onSpecificRequestMap.rend())
        requestNumber = it->first;

    requestNumber++;
    return ForRequest(requestNumber, callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectRequest(Http::ResponseCR response)
    {
    return ExpectRequest([=] (Http::Request) { return response; });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, OnResponseCallback callback)
    {
    EXPECT_NE(0, requestNumber);
    m_onSpecificRequestMap[requestNumber] = callback;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, Http::ResponseCR response)
    {
    return ForRequest(requestNumber, [=] (Http::Request) { return response; });
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
MockHttpHandler&  MockHttpHandler::ForFirstRequest(Http::ResponseCR response)
    {
    return ForRequest(1, response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(OnResponseCallback callback)
    {
    EXPECT_TRUE(nullptr != callback);
    m_onAnyRequestCallback = callback;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(Http::ResponseCR response)
    {
    return ForAnyRequest([=] (Http::Request)
        {
        return response;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Http::Response MockHttpHandler::StubHttpResponse()
    {
    HttpStatus httpStatus = HttpStatus::OK;
    ConnectionStatus status = ConnectionStatus::OK;

    return Response(HttpResponseContent::Create(HttpStringBody::Create()), "", status, httpStatus);
    }
