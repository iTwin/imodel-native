/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "MockHttpHandler.h"
#include <WebServices/WebServices.h>
#include <WebServices/iModelHub/Common.h>
#include <BeHttp/HttpRequest.h>
#include <Bentley/BeTest.h>

#define EXPECTED_COUNT_ANY -1

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::MockHttpHandler()
    {
    m_perfomedRequests = 0;
    m_expectedRequests = EXPECTED_COUNT_ANY;
    m_onAnyRequestCallback = [&] (Http::Request request)
        {
        return Http::Response(HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, HttpStatus::None);
        };
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::~MockHttpHandler()
    {
    if (m_expectedRequests == EXPECTED_COUNT_ANY)
        return;

    if (m_file.empty())
        {
        EXPECT_EQ(m_expectedRequests, m_perfomedRequests);
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
AsyncTaskPtr<Http::Response> MockHttpHandler::_PerformRequest(Http::RequestCR request)
    {
    auto task = std::make_shared<PackagedAsyncTask<Http::Response>>([&] ()
        {
        EXPECT_LT(m_perfomedRequests, std::numeric_limits<uint32_t>::max());
        m_perfomedRequests++;

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
