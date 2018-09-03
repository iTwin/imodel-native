/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/OIDC/OidcSignInManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcSignInManager.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<OidcSignInResult> OidcSignInManager::SignInWithCredentials(CredentialsCR credentials)
    {
    m_tokenProvider = std::make_shared<OidcTokenProvider>(credentials);
    return CreateCompletedAsyncTask(OidcSignInResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OidcSignInManager::_CheckAndUpdateToken()
    {
    m_tokenProvider->GetToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OidcSignInManager::_SignOut()
    {
    m_tokenProvider = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool OidcSignInManager::_IsSignedIn() const
    {
    return nullptr != m_tokenProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectSignInManager::UserInfo OidcSignInManager::_GetUserInfo() const
    {
    return IConnectSignInManager::UserInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String OidcSignInManager::_GetLastUsername() const
    {
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr OidcSignInManager::_GetTokenProvider(Utf8StringCR rpUri) const
    {
    return m_tokenProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OidcSignInManager::_StoreSignedInUser()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr OidcSignInManager::_GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler, HeaderPrefix prefix) const
    {
    auto configurationHandler = UrlProvider::GetSecurityConfigurator(httpHandler);

    auto connectHandler = ConnectAuthenticationHandler::Create
        (
        serverUrl,
        GetTokenProvider(""),
        configurationHandler,
        TOKENPREFIX_Bearer
        );
    connectHandler->EnableExpiredTokenRefresh();
    return connectHandler;
    }
