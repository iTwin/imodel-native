/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpConfigurationHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpConfigurationHandler.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpConfigurationHandler::HttpConfigurationHandler(std::function<void(Request& request)> configuration, IHttpHandlerPtr customHandler) :
m_handler(customHandler ? customHandler : DefaultHttpHandler::GetInstance()),
m_configuration(configuration)
    {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> HttpConfigurationHandler::_PerformRequest(RequestCR request)
    {
    if (!m_configuration)
        return m_handler->_PerformRequest(request);

    Request configuredRequest = request;
    m_configuration(configuredRequest);
    return m_handler->_PerformRequest(configuredRequest);
    }
