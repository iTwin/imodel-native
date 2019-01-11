/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/HttpConfigurationHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/DefaultHttpHandler.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpConfigurationHandler : IHttpHandler
{
private:
    IHttpHandlerPtr m_handler;
    std::function<void(RequestR request)> m_configuration;

public:
    //! @param configuration - callback that will be called for request configuration
    //! @param customHandler - handler to call after configuration.
    HttpConfigurationHandler(
        std::function<void(Request& request)> configuration,
        IHttpHandlerPtr customHandler = nullptr
        ) : m_handler(customHandler ? customHandler : DefaultHttpHandler::GetInstance()), m_configuration(configuration) {}
    virtual ~HttpConfigurationHandler() {}

    //! Configure and Perform HttpRequest
    virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override
        {
        if (!m_configuration)
            return m_handler->_PerformRequest(request);

        Request configuredRequest = request;
        m_configuration(configuredRequest);
        return m_handler->_PerformRequest(configuredRequest);
        }
};

END_BENTLEY_HTTP_NAMESPACE
