/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/AuthHandlerProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "IAuthHandlerProvider.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct AuthHandlerProvider : IAuthHandlerProvider
    {
    protected:
        std::shared_ptr<IConnectAuthenticationProvider> m_authProvider;
        IHttpHandlerPtr m_httpHandler;
    public:
        LICENSING_EXPORT AuthHandlerProvider
            (
            std::shared_ptr<IConnectAuthenticationProvider> authProvider,
            IHttpHandlerPtr httpHandler
            );
        LICENSING_EXPORT IHttpHandlerPtr GetAuthHandler(Utf8StringCR serverUrl, IConnectAuthenticationProvider::HeaderPrefix prefix);
    };

END_BENTLEY_LICENSING_NAMESPACE
