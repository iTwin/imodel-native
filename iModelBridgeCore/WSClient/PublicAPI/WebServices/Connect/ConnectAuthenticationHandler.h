/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectAuthenticationHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectTokenProvider.h>
#include <DgnClientFx/Utils/Http/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectAuthenticationHandler : public AuthenticationHandler
    {
    private:
        Utf8String m_urlBaseToAuth;
        std::shared_ptr<IConnectTokenProvider> m_tokenProvider;
        std::shared_ptr<WorkerThread> m_thread;

    private:
        bool ShouldStopSendingToken(AttemptCR previousAttempt) const;
        bool IsTokenAuthorization(Utf8StringCR auth) const;

    public:
        WSCLIENT_EXPORT ConnectAuthenticationHandler
            (
            Utf8String urlBaseToAuth,
            std::shared_ptr<IConnectTokenProvider> customTokenProvider = nullptr,
            IHttpHandlerPtr customHttpHandler = nullptr
            );

        WSCLIENT_EXPORT virtual ~ConnectAuthenticationHandler();

        WSCLIENT_EXPORT virtual bool _ShouldRetryAuthentication(HttpResponseCR response) override;
        WSCLIENT_EXPORT virtual AsyncTaskPtr<AuthorizationResult> _RetrieveAuthorization(AttemptCR previousAttempt) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
