/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/ProxyHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyHttpHandler::ProxyHttpHandler (Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler) :
m_handler (customHandler ? customHandler : DefaultHttpHandler::GetInstance ()),
m_proxyUrl (proxyUrl)
    {
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyHttpHandler::~ProxyHttpHandler ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ProxyHttpHandler::SetProxyCredentials (CredentialsCR credentials)
    {
    m_proxyCredentials = credentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ProxyHttpHandler::GetProxyUrl ()
    {
    return m_proxyUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CredentialsCR ProxyHttpHandler::GetProxyCredentials() const
    {
    return m_proxyCredentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> ProxyHttpHandler::PerformRequest (HttpRequestCR request)
    {
    if (m_proxyUrl.empty ())
        {
        return m_handler->PerformRequest (request);
        }

    HttpRequest proxyRequest = request;
    proxyRequest.SetProxy (m_proxyUrl);

    if (m_proxyCredentials.IsValid())
        {
        proxyRequest.SetProxyCredentials (m_proxyCredentials);
        }

    return m_handler->PerformRequest (proxyRequest);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ProxyHttpHandler> ProxyHttpHandler::GetFiddlerProxyIfReachable (IHttpHandlerPtr customHandler)
    {
    return GetProxyIfReachable ("http://127.0.0.1:8888", customHandler); // Default fiddler proxy
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ProxyHttpHandler> ProxyHttpHandler::GetProxyIfReachable (Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler)
    {
    HttpResponse response = HttpRequest (proxyUrl, "GET", customHandler).PerformAsync ()->GetResult ();

    if (HttpStatus::OK == response.GetHttpStatus ())
        {
        return std::make_shared<ProxyHttpHandler> (proxyUrl, customHandler);
        }
    else
        {
        return std::make_shared<ProxyHttpHandler> ("", customHandler);
        }
    }
