/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/ProxyHttpHandler.h>

#include <BeHttp/HttpConfigurationHandler.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> ProxyHttpHandler::_PerformRequest (RequestCR request)
    {
    if (m_proxyUrl.empty ())
        return m_handler->_PerformRequest (request);

    Request proxyRequest = request;
    proxyRequest.SetProxy (m_proxyUrl);

    if (m_proxyCredentials.IsValid())
        proxyRequest.SetProxyCredentials (m_proxyCredentials);

    return m_handler->_PerformRequest (proxyRequest);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr ProxyHttpHandler::GetFiddlerProxyIfReachable(IHttpHandlerPtr customHandler)
    {
    // Default fiddler proxy
    auto proxy = GetProxyIfReachable ("http://127.0.0.1:8888", customHandler);
    
    // Check if not reachable
    if (proxy == customHandler)
        return customHandler;

    // Remove certificate validation as Fiddler uses self-signed certificate
    proxy = std::make_shared<HttpConfigurationHandler>([=] (Request& request)
        {
        request.SetValidateCertificate(false);
        }, proxy);

    return proxy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr ProxyHttpHandler::GetProxyIfReachable(Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler)
    {
    return GetProxyIfReachable(proxyUrl, Credentials(), customHandler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr ProxyHttpHandler::GetProxyIfReachable(Utf8StringCR proxyUrl, CredentialsCR proxyCredentials, IHttpHandlerPtr customHandler)
    {
    Request request(proxyUrl, "GET", customHandler);
    if (proxyCredentials.IsValid())
        {
        request.SetProxy(proxyUrl);
        request.SetProxyCredentials(proxyCredentials);
        }
    
    Response response = request.PerformAsync ()->GetResult ();

    if (HttpStatus::OK != response.GetHttpStatus())
        return customHandler;

    auto proxyHttpHandler = std::make_shared<ProxyHttpHandler> (proxyUrl, customHandler);
    proxyHttpHandler->SetProxyCredentials(proxyCredentials);
    return proxyHttpHandler;
    }
