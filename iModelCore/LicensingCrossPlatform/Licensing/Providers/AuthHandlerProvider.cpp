/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "AuthHandlerProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

AuthHandlerProvider::AuthHandlerProvider
    (
    std::shared_ptr<IConnectAuthenticationProvider> authProvider,
    IHttpHandlerPtr httpHandler
    ) :
    m_authProvider(authProvider),
    m_httpHandler(httpHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod Gets the http handler from IConnectAuthenticationProvider 
* using the passed in handler
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr AuthHandlerProvider::GetAuthHandler(Utf8StringCR serverUrl, IConnectAuthenticationProvider::HeaderPrefix prefix)
    {
    return m_authProvider->GetAuthenticationHandler(serverUrl, m_httpHandler, prefix);
    }
