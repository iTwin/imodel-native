/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <BeHttp/IHttpHandler.h>
#include <BeHttp/DefaultHttpHandler.h>
#include "../../../iModelHubClient/Logging.h"
#include <Bentley/BeTest.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP

//=======================================================================================
//@bsistruct                                Algirdas.Mikoliunas                 12/2018
//=======================================================================================
typedef std::shared_ptr<struct RequestVerifyHttpHandler> RequestVerifyHttpHandlerPtr;
struct RequestVerifyHttpHandler : IHttpHandler
{
private:
    IHttpHandlerPtr m_httpHandler;
    Utf8CP m_headerXCorrelationId = "X-Correlation-Id";

public:
    RequestVerifyHttpHandler(IHttpHandlerPtr handler) : m_httpHandler(handler ? handler : DefaultHttpHandler::GetInstance()) {}
    virtual ~RequestVerifyHttpHandler() {};

    virtual Tasks::AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override
        {
        if (request.GetUrl().Contains("imodelhubapi") && request.GetUrl().Contains(".bentley.com") && !request.GetUrl().EndsWithI("/sv1.1/Plugins"))
            {
            auto correlationHeaderValue = request.GetHeaders().GetValue(m_headerXCorrelationId);
            EXPECT_FALSE(Utf8String::IsNullOrEmpty(correlationHeaderValue)) << request.GetUrl().c_str();
            }

        return m_httpHandler->_PerformRequest(request);
        }
};
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
