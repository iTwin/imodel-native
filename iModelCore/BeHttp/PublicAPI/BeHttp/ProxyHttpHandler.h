/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/ProxyHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/DefaultHttpHandler.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>
#include <BeHttp/Credentials.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ProxyHttpHandler> ProxyHttpHandlerPtr;
struct EXPORT_VTABLE_ATTRIBUTE ProxyHttpHandler : IHttpHandler
{
private:
    IHttpHandlerPtr m_handler;
    Utf8String m_proxyUrl;
    Credentials m_proxyCredentials;

public:
    ProxyHttpHandler(Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler = nullptr) : m_handler(customHandler ? customHandler : DefaultHttpHandler::GetInstance()), m_proxyUrl(proxyUrl) {}
    virtual ~ProxyHttpHandler() {}

    //! Check if proxy is configured
    BEHTTP_EXPORT bool IsConfigured() const {return !m_proxyUrl.empty();}

    //! Set credentials for an authenticating proxy
    void SetProxyCredentials(CredentialsCR credentials) {m_proxyCredentials = credentials;}

    //! Returns proxy credentials
    CredentialsCR GetProxyCredentials() const {return m_proxyCredentials;}

    //! Returns Proxy URL
    Utf8StringCR GetProxyUrl() {return m_proxyUrl;}

    //! Perform HttpRequest with proxy URL
    BEHTTP_EXPORT virtual Tasks::AsyncTaskPtr<Response> _PerformRequest (RequestCR request) override;

    //! Checks if proxy is reachable and returns proxy handler that uses customHandler. Returns customHandler in other case.
    BEHTTP_EXPORT static IHttpHandlerPtr GetProxyIfReachable(Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler = nullptr);
    
    //! Checks if proxy with credentials is reachable and returns proxy handler that uses customHandler. Returns customHandler in other case.
    BEHTTP_EXPORT static IHttpHandlerPtr GetProxyIfReachable(Utf8StringCR proxyUrl, CredentialsCR proxyCredentials, IHttpHandlerPtr customHandler = nullptr);

    //! Use only for testing or debugging!
    //! Checks if Fiddler proxy is reachable and returns proxy handler that uses customHandler. Returns customHandler in other case.
    //! If reachable, disables certificate validation for each request so Fiddler routing would work.
    BEHTTP_EXPORT static IHttpHandlerPtr GetFiddlerProxyIfReachable(IHttpHandlerPtr customHandler = nullptr);
    };

END_BENTLEY_HTTP_NAMESPACE
