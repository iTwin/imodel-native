/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/AuthHandlerProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "IAuthHandlerProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct AuthHandlerProvider : IAuthHandlerProvider
    {
    protected:
        std::shared_ptr<WebServices::IConnectAuthenticationProvider> m_authProvider;
        Http::IHttpHandlerPtr m_httpHandler;
    public:
        LICENSING_EXPORT AuthHandlerProvider
            (
            std::shared_ptr<WebServices::IConnectAuthenticationProvider> authProvider,
            Http::IHttpHandlerPtr httpHandler
            );
        LICENSING_EXPORT Http::IHttpHandlerPtr GetAuthHandler(Utf8StringCR serverUrl, WebServices::IConnectAuthenticationProvider::HeaderPrefix prefix);
    };

END_BENTLEY_LICENSING_NAMESPACE
