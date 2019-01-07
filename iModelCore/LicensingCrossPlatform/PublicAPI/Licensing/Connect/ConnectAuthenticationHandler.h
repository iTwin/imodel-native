/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/ConnectAuthenticationHandler.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! DEPRECATED CODE - Use ConnectSignInManager
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "IConnectTokenProvider.h"
#include <BeHttp/AuthenticationHandler.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

#define TOKENPREFIX_Token               "token"
#define TOKENPREFIX_SAML                "SAML"
#define TOKENPREFIX_BEARER              "Bearer"
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectAuthenticationHandler : public AuthenticationHandler
    {
    private:
        Utf8String m_urlBaseToAuth;
        std::shared_ptr<IConnectTokenProvider> m_tokenProvider;
        std::shared_ptr<WorkerThread> m_thread;
        Utf8String m_tokenPrefix;
        bool m_retryExpiredToken;

    private:
        ConnectAuthenticationHandler
            (
            Utf8String urlBaseToAuth,
            std::shared_ptr<IConnectTokenProvider> customTokenProvider = nullptr,
            IHttpHandlerPtr customHttpHandler = nullptr,
            Utf8String tokenPrefix = TOKENPREFIX_Token
            );

        bool ShouldStopSendingToken(AttemptCR previousAttempt) const;
        bool IsTokenAuthorization(Utf8StringCR auth) const;
        Utf8String GetTokenAuthorization(ISecurityTokenPtr token) const;

    public:
        LICENSING_EXPORT static std::shared_ptr<ConnectAuthenticationHandler> Create
            (
            Utf8String urlBaseToAuth,
            std::shared_ptr<IConnectTokenProvider> customTokenProvider = nullptr,
            IHttpHandlerPtr customHttpHandler = nullptr,
            Utf8String tokenPrefix = TOKENPREFIX_Token
            );

        LICENSING_EXPORT static std::shared_ptr<ConnectAuthenticationHandler> CreateLegacy
            (
            Utf8String urlBaseToAuth,
            std::shared_ptr<IConnectTokenProvider> customTokenProvider = nullptr,
            IHttpHandlerPtr customHttpHandler = nullptr,
            bool shouldUseSAMLAuthorization = false
            );

        LICENSING_EXPORT void EnableExpiredTokenRefresh();

        LICENSING_EXPORT virtual ~ConnectAuthenticationHandler();

        LICENSING_EXPORT virtual bool _ShouldRetryAuthentication(Http::ResponseCR response) override;
        LICENSING_EXPORT virtual AsyncTaskPtr<AuthorizationResult> _RetrieveAuthorization(AttemptCR previousAttempt) override;
    };

END_BENTLEY_LICENSING_NAMESPACE
