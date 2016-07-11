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
struct ProxyHttpHandler : IHttpHandler
{
    private:
        IHttpHandlerPtr m_handler;
        Utf8String m_proxyUrl;
        Credentials m_proxyCredentials;

    public:
        BEHTTP_EXPORT ProxyHttpHandler(Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler = nullptr);
        BEHTTP_EXPORT virtual ~ProxyHttpHandler();

        //! Set credentials for an authenticating proxy
        BEHTTP_EXPORT void SetProxyCredentials(CredentialsCR credentials);

        //! Returns Proxy URL
        BEHTTP_EXPORT Utf8StringCR GetProxyUrl();

        //! Returns proxy credentials
        BEHTTP_EXPORT CredentialsCR GetProxyCredentials() const;

        //! Perform HttpRequest with proxy URL
        BEHTTP_EXPORT virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;

        /*!
         * GetProxyIfReachable is mainly used for debugging. Consider using the constructor for production code.<br>
         * Returns ProxyHttpHandler for specified proxy if reachable.
         */
        BEHTTP_EXPORT static std::shared_ptr<ProxyHttpHandler> GetProxyIfReachable(Utf8StringCR proxyUrl, IHttpHandlerPtr customHandler = nullptr);

        /*!
         * GetProxyIfReachable is mainly used for debugging. Consider using the constructor for production code.<br>
         * Returns ProxyHttpHandler for specified proxy if reachable and credentials are valid.
         */
        BEHTTP_EXPORT static std::shared_ptr<ProxyHttpHandler> GetProxyIfReachable(Utf8StringCR proxyUrl, CredentialsCR proxyCredentials, IHttpHandlerPtr customHandler = nullptr);

        /*!
         * GetFiddlerProxyIfReachable is mainly used for debugging. Consider using the constructor for production code.<br>
         * Returns ProxyHttpHandler for default Fiddler proxy if reachable.
         */
        BEHTTP_EXPORT static std::shared_ptr<ProxyHttpHandler> GetFiddlerProxyIfReachable(IHttpHandlerPtr customHandler = nullptr);
};

END_BENTLEY_HTTP_NAMESPACE
